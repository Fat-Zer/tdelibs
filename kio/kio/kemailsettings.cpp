/*
 * Copyright (c) 2000 Alex Zepeda <zipzippy@sonic.net>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	$Id$
 */

#include "kemailsettings.h"

#include <kconfig.h>
#include <klocale.h>
#include <kdebug.h>

class KEMailSettingsPrivate {
public:
    KEMailSettingsPrivate() : m_pConfig( 0 ) {}
    ~KEMailSettingsPrivate() { delete m_pConfig; }
	TDEConfig *m_pConfig;
	TQStringList profiles;
	TQString m_sDefaultProfile, m_sCurrentProfile;
};

TQString KEMailSettings::defaultProfileName() const
{
	return p->m_sDefaultProfile;
}

TQString KEMailSettings::getSetting(KEMailSettings::Setting s)
{
	p->m_pConfig->setGroup(TQString("PROFILE_")+p->m_sCurrentProfile);
	switch (s) {
		case ClientProgram: {
			return p->m_pConfig->readEntry("EmailClient");
			break;
		}
		case ClientTerminal: {
			return ((p->m_pConfig->readBoolEntry("TerminalClient")) ? TQString("true") : TQString("false") );
			break;
		}
		case RealName: {
			return p->m_pConfig->readEntry("FullName");
			break;
		}
		case EmailAddress: {
			return p->m_pConfig->readEntry("EmailAddress");
			break;
		}
		case ReplyToAddress: {
			return p->m_pConfig->readEntry("ReplyAddr");
			break;
		}
		case Organization: {
			return p->m_pConfig->readEntry("Organization");
			break;
		}
		case OutServer: {
			return p->m_pConfig->readEntry("OutgoingServer");
			break;
		}
		case OutServerLogin: {
			return p->m_pConfig->readEntry("OutgoingUserName");
			break;
		}
		case OutServerPass: {
			return p->m_pConfig->readEntry("OutgoingPassword");
			break;
		}
		case OutServerType: {
			return p->m_pConfig->readEntry("OutgoingServerType");
			break;
		}
		case OutServerCommand: {
			return p->m_pConfig->readEntry("OutgoingCommand");
			break;
		}
		case OutServerTLS: {
			return ((p->m_pConfig->readBoolEntry("OutgoingServerTLS")) ? TQString("true") : TQString("false") );
			break;
		}
		case InServer: {
			return p->m_pConfig->readEntry("IncomingServer");
			break;
		}
		case InServerLogin: {
			return p->m_pConfig->readEntry("IncomingUserName");
			break;
		}
		case InServerPass: {
			return p->m_pConfig->readEntry("IncomingPassword");
			break;
		}
		case InServerType: {
			return p->m_pConfig->readEntry("IncomingServerType");
			break;
		}
		case InServerMBXType: {
			return p->m_pConfig->readEntry("IncomingServerMBXType");
			break;
		}
		case InServerTLS: {
			return ((p->m_pConfig->readBoolEntry("IncomingServerTLS")) ? TQString("true") : TQString("false") );
			break;
		}
	};
	return TQString::null;
}
void KEMailSettings::setSetting(KEMailSettings::Setting s, const TQString  &v)
{
	p->m_pConfig->setGroup(TQString("PROFILE_")+p->m_sCurrentProfile);
	switch (s) {
		case ClientProgram: {
			p->m_pConfig->writePathEntry("EmailClient", v);
			break;
		}
		case ClientTerminal: {
			p->m_pConfig->writeEntry("TerminalClient", (v == "true") ? true : false );
			break;
		}
		case RealName: {
			p->m_pConfig->writeEntry("FullName", v);
			break;
		}
		case EmailAddress: {
			p->m_pConfig->writeEntry("EmailAddress", v);
			break;
		}
		case ReplyToAddress: {
			p->m_pConfig->writeEntry("ReplyAddr", v);
			break;
		}
		case Organization: {
			p->m_pConfig->writeEntry("Organization", v);
			break;
		}
		case OutServer: {
			p->m_pConfig->writeEntry("OutgoingServer", v);
			break;
		}
		case OutServerLogin: {
			p->m_pConfig->writeEntry("OutgoingUserName", v);
			break;
		}
		case OutServerPass: {
			p->m_pConfig->writeEntry("OutgoingPassword", v);
			break;
		}
		case OutServerType: {
			p->m_pConfig->writeEntry("OutgoingServerType", v);
			break;
		}
		case OutServerCommand: {
			p->m_pConfig->writeEntry("OutgoingCommand", v);
			break;
		}
		case OutServerTLS: {
			p->m_pConfig->writeEntry("OutgoingServerTLS", (v == "true") ? true : false );
			break;
		}
		case InServer: {
			p->m_pConfig->writeEntry("IncomingServer", v);
			break;
		}
		case InServerLogin: {
			p->m_pConfig->writeEntry("IncomingUserName", v);
			break;
		}
		case InServerPass: {
			p->m_pConfig->writeEntry("IncomingPassword", v);
			break;
		}
		case InServerType: {
			p->m_pConfig->writeEntry("IncomingServerType", v);
			break;
		}
		case InServerMBXType: {
			p->m_pConfig->writeEntry("IncomingServerMBXType", v);
			break;
		}
		case InServerTLS: {
			p->m_pConfig->writeEntry("IncomingServerTLS", (v == "true") ? true : false );
			break;
		}
	};
	p->m_pConfig->sync();
}

void KEMailSettings::setDefault(const TQString &s)
{
	p->m_pConfig->setGroup("Defaults");
	p->m_pConfig->writeEntry("Profile", s);
	p->m_pConfig->sync();
	p->m_sDefaultProfile=s;

}

void KEMailSettings::setProfile (const TQString &s)
{
	TQString groupname="PROFILE_";
	groupname.append(s);
	p->m_sCurrentProfile=s;
	if (!p->m_pConfig->hasGroup(groupname)) { // Create a group if it doesn't exist
		p->m_pConfig->setGroup(groupname);
		p->m_pConfig->writeEntry("ServerType", TQString::null);
		p->m_pConfig->sync();
		p->profiles+=s;
	}
}

TQString KEMailSettings::currentProfileName() const
{
	return p->m_sCurrentProfile;
}

TQStringList KEMailSettings::profiles() const
{
	return p->profiles;
}

KEMailSettings::KEMailSettings()
{
	p = new KEMailSettingsPrivate();
	p->m_sCurrentProfile=TQString::null;

	p->m_pConfig = new TDEConfig("emaildefaults");

	TQStringList groups = p->m_pConfig->groupList();
	for (TQStringList::Iterator it = groups.begin(); it != groups.end(); ++it) {
		if ( (*it).left(8) == "PROFILE_" )
			p->profiles+= (*it).mid(8, (*it).length());
	}

	p->m_pConfig->setGroup("Defaults");
	p->m_sDefaultProfile=p->m_pConfig->readEntry("Profile", i18n("Default"));
	if (!p->m_sDefaultProfile.isNull()) {
		if (!p->m_pConfig->hasGroup(TQString("PROFILE_")+p->m_sDefaultProfile))
			setDefault(i18n("Default"));
		else
			setDefault(p->m_sDefaultProfile);
	} else {
			if (p->profiles.count()) {
				setDefault(p->profiles[0]);
			} else
				setDefault(i18n("Default"));
	}
	setProfile(defaultProfileName());
}

KEMailSettings::~KEMailSettings()
{
    delete p;
}
