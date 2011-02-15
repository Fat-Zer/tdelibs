/*  -*- C++ -*-
 *  Copyright (C) 2005 Thiago Macieira <thiago@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include <config.h>

#include "ksrvresolverworker_p.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <stdlib.h>

#include <tqapplication.h>
#include <tqevent.h>

using namespace KNetwork;
using namespace KNetwork::Internal;

namespace 
{
  struct KSrvStartEvent: public TQCustomEvent
  {
    inline KSrvStartEvent() : TQCustomEvent(TQEvent::User) { }
  };
}

static void sortPriorityClass(KSrvResolverWorker::PriorityClass&)
{
  // do nothing
}

bool KSrvResolverWorker::preprocess()
{
  // check if the resolver flags mention SRV-based lookup
  if ((flags() & (KResolver::NoSrv | KResolver::UseSrv)) != KResolver::UseSrv)
    return false;

  TQString node = nodeName();
  if (node.tqfind('%') != -1)
    node.truncate(node.tqfind('%'));

  if (node.isEmpty() || node == TQString::tqfromLatin1("*") ||
      node == TQString::tqfromLatin1("localhost"))
    return false;		// empty == localhost

  encodedName = KResolver::domainToAscii(node);
  if (encodedName.isNull())
    return false;

  // we only work with Internet-based families
  if ((familyMask() & KResolver::InternetFamily) == 0)
    return false;

  // SRV-based resolution only works if the service isn't numeric
  bool ok;
  serviceName().toUInt(&ok);
  if (ok)
    return false;		// it is numeric

  // check the protocol for something we know
  TQCString protoname;
  int sockettype = socketType();
  if (!protocolName().isEmpty())
    protoname = protocolName();
  else if (protocol() != 0)
    {
      TQStrList names = KResolver::protocolName(protocol());
      names.setAutoDelete(true);
      if (names.isEmpty())
	return false;

      protoname = "_";
      protoname += names.tqat(0);
    }
  else if (sockettype == SOCK_STREAM || sockettype == 0)
    protoname = "_tcp";
  else if (sockettype == SOCK_DGRAM)
    protoname = "_udp";
  else
    return false;		// unknown protocol and socket type

  encodedName.prepend(".");
  encodedName.prepend(protoname);
  encodedName.prepend(".");
  encodedName.prepend(serviceName().latin1());
  encodedName.prepend("_");

  // looks like something we could process
  return true;
}

bool KSrvResolverWorker::run()
{
  sem = new TQSemaphore(1);
  // zero out
  sem->tryAccess(sem->available());

  TQApplication::postEvent(this, new KSrvStartEvent);

  // block
  (*sem)++;
  delete sem;
  sem = 0L;

  if (rawResults.isEmpty())
    {
      // normal lookup
      KResolver *r = new KResolver(nodeName(), serviceName());
      r->setFlags(flags() | KResolver::NoSrv);
      r->setFamily(familyMask());
      r->setSocketType(socketType());
      r->setProtocol(protocol(), protocolName());

      enqueue(r);

      Entry e;
      PriorityClass cl;
      e.resolver = r;
      cl.entries.append(e);
      myResults[0] = cl;

      return true;
    }
  else if (rawResults.count() == 1 && rawResults.first().name == ".")
    {
      // no name
      setError(KResolver::NoName);
      finished();
      return true;
    }
  else
    {
      // now process the results
      TQValueList<TQDns::Server>::ConstIterator it = rawResults.begin();
      while (it != rawResults.end())
	{
	  const TQDns::Server& srv = *it;
	  PriorityClass& r = myResults[srv.priority];
	  r.totalWeight += srv.weight;

	  Entry e;
	  e.name = srv.name;
	  e.port = srv.port;
	  e.weight = srv.weight;
	  e.resolver = 0L;
	  r.entries.append(e);

	  ++it;
	}
      rawResults.clear();	// free memory

      Results::Iterator mapit;
      for (mapit = myResults.begin(); mapit != myResults.end(); ++mapit)
	{
	  // sort the priority
	  sortPriorityClass(*mapit);

	  TQValueList<Entry>& entries = (*mapit).entries;

	  // start the resolvers
	  for (TQValueList<Entry>::Iterator it = entries.begin();
	       it != entries.end(); ++it)
	    {
	      Entry &e = *it;

	      KResolver* r = new KResolver(e.name, TQString::number(e.port));
	      r->setFlags(flags() | KResolver::NoSrv);
	      r->setFamily(familyMask());
	      r->setSocketType(socketType());
	      r->setProtocol(protocol(), protocolName());

	      enqueue(r);
	      e.resolver = r;
	    }
	}

      return true;
    }
}

bool KSrvResolverWorker::postprocess()
{
  setError(KResolver::NoName);
  if (myResults.isEmpty())
    return false;

  Results::Iterator mapit, mapend;
  for (mapit = myResults.begin(), mapend = myResults.end();
       mapit != mapend; ++mapit)
    {
      TQValueList<Entry>::Iterator it = (*mapit).entries.begin(),
	end = (*mapit).entries.end();
      for ( ; it != end; ++it)
	{
	  Entry &e = *it;
	  KResolverResults r = e.resolver->results();
	  if (r.isEmpty() && results.isEmpty())
	    setError(r.error(), r.systemError());
	  else
	    {
	      setError(KResolver::NoError);
	      results += r;
	    }
	}
    }

  finished();
  return true;
}

void KSrvResolverWorker::customEvent(TQCustomEvent*)
{
  dns = new TQDns(TQString::tqfromLatin1(encodedName), TQDns::Srv);
  TQObject::connect(dns, TQT_SIGNAL(resultsReady()), this, TQT_SLOT(dnsResultsReady()));
}

void KSrvResolverWorker::dnsResultsReady()
{
  (*sem)--;
  rawResults = dns->servers();
  dns->deleteLater();
  dns = 0L;
}

namespace KNetwork
{
  namespace Internal
  {

    void initSrvWorker() KDE_NO_EXPORT;
    void initSrvWorker()
    {
      if (getenv("KDE_NO_SRV") != NULL)
	return;
      
      KResolverWorkerFactoryBase::registerNewWorker(new KResolverWorkerFactory<KSrvResolverWorker>);
    }
    
  }
}

#include "ksrvresolverworker_p.moc"
