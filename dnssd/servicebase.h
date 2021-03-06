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

#ifndef DNSSDSERVICEBASE_H
#define DNSSDSERVICEBASE_H

#include <tqmap.h>
#include <ksharedptr.h>

class TQString;
class TQDataStream;
namespace DNSSD
{
class ServiceBasePrivate;

/**
This class is used to carry information about service. It can be remote, local,
metaservice or domain. Metaservice has only type and domain - it means that
services of given type are present in given domain.
@short Describes any type of service.
@author Jakub Stachowski
 */
class TDEDNSSD_EXPORT ServiceBase : public TDEShared
{
public:
	typedef TDESharedPtr<ServiceBase> Ptr;

	/**
	@param name Service name - empty for metaservices
	@param type Service type - empty for domains
	@param domain Domain name
	@param host Host name
	@param port Port number
	 */
	ServiceBase(const TQString& name=TQString::null,const TQString& type=TQString::null,
		    const TQString& domain=TQString::null, const TQString& host=TQString::null,
		    unsigned short port=0);

	virtual  ~ServiceBase();

	/**
	Returns name of service. This is empty for metaservices
	 */
	const TQString& serviceName() const;

	/**
	Returns type of service. It always in format _sometype._udp or _sometype._tcp and
	it is empty for domains.
	 */
	const TQString& type() const;

	/**
	Returns domain that given service belongs to. It is "local." for link-local services.
	 */
	const TQString& domain() const;

	/**
	Returns hostname. It is only valid for local and resolved remote services.
	 */
	const TQString& hostName() const;

	/**
	Returns port number. It is only valid for local and resolved remote services.
	 */
	unsigned short port() const;

	/**
	Returns read only map of text properties.  It is only valid for local and resolved remote services.
	 */
	const TQMap<TQString,TQString>& textData() const;

protected:
	TQString m_serviceName;
	TQString m_type;
	TQString m_domain;
	TQString m_hostName;
	unsigned short m_port;

	/**
	Map of TXT properties
	 */
	TQMap<TQString,TQString> m_textData;
	/**
	Encode service name, type and domain into string that can be used as DNS-SD PTR label
	 */
	TQString encode();
	/**
	Decode PTR label returned by DNS resolver into service name, type and domain. It also
	handles special cases - metaservices and domains.
	 */
	void decode(const TQString& name);

	friend TDEDNSSD_EXPORT TQDataStream & operator<< (TQDataStream & s, const ServiceBase & a);
	friend TDEDNSSD_EXPORT TQDataStream & operator>> (TQDataStream & s, ServiceBase & a);

	virtual void virtual_hook(int, void*);
private:
	ServiceBasePrivate* d;

};

}

#endif
