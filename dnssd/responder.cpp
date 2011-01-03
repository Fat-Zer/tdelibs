/* This file is part of the KDE project
 *
 * Copyright (C) 2004 Jakub Stachowski <qbast@go2.pl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "responder.h"
#include <tqapplication.h>
#include <tqeventloop.h>
#include <kstaticdeleter.h>
#include <kidna.h>
#include <kdebug.h>
#ifdef HAVE_DNSSD
#include <avahi-qt3/qt-watch.h>
#endif

namespace DNSSD
{

static KStaticDeleter<Responder> responder_sd;
Responder* Responder::m_self = 0;

#ifdef HAVE_DNSSD
void client_callback(AvahiClient *, AvahiClientState s, void* u)
{
    Responder *r = reinterpret_cast<Responder*>(u);
    emit (r->stateChanged(s));
}
#endif


Responder::Responder()
{
    int error;
#ifdef HAVE_DNSSD
    const AvahiPoll* poll = avahi_qt_poll_get();
#ifdef AVAHI_API_0_6
    m_client = avahi_client_new(poll, AVAHI_CLIENT_IGNORE_USER_CONFIG,client_callback, this,  &error);
#else
    m_client = avahi_client_new(poll, client_callback, this,  &error);
#endif
    if (!m_client) kdWarning() << "Failed to create avahi client" << endl;
#endif
}

Responder::~Responder()
{
#ifdef HAVE_DNSSD
    if (m_client) avahi_client_free(m_client);
#endif
}

Responder& Responder::self()
{
    if (!m_self) responder_sd.setObject(m_self, new Responder);
    return *m_self;
}

void Responder::process()
{
    tqApp->eventLoop()->processEvents(TQEventLoop::ExcludeUserInput);
}

#ifdef HAVE_DNSSD
AvahiClientState Responder::state() const
{
#ifdef AVAHI_API_0_6
	return (m_client) ? (avahi_client_get_state(m_client)) : AVAHI_CLIENT_FAILURE;
#else
	return (m_client) ? (avahi_client_get_state(m_client)) : AVAHI_CLIENT_DISCONNECTED;
#endif
}
#endif

bool domainIsLocal(const TQString& domain)
{
	return domain.section('.',-1,-1).lower()=="local";
}

TQCString domainToDNS(const TQString &domain)
{
	if (domainIsLocal(domain)) return domain.utf8();
		else return KIDNA::toAsciiCString(domain);
}

TQString DNSToDomain(const char* domain)
{
	if (domainIsLocal(domain)) return TQString::fromUtf8(domain);
		else return KIDNA::toUnicode(domain);
}


}
#include "responder.moc"
