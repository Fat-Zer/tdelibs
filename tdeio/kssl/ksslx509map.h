/* This file is part of the KDE project
 *
 * Copyright (C) 2000-2003 George Staikos <staikos@kde.org>
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
 
#ifndef _KSSLX509MAP_H
#define _KSSLX509MAP_H

#include <tqmap.h>
#include <tqstring.h>

#include <tdelibs_export.h>

/**
 * X.509 Map Parsing Class
 *
 * This class is used to parse and store a map as used in X.509 certificates.
 * It is of the form /name=value/name=value/name=value
 *
 * @author George Staikos <staikos@kde.org>
 * @see KSSL, KSSLCertificate
 * @short X.509 Map Parser
 */
class TDEIO_EXPORT KSSLX509Map {
public:
	/**
	 *  Construct an X.509 Map
	 *
	 *  @param name the map to parse
	 */
	KSSLX509Map(const TQString& name);

	/**
	 *  Destroy this map
	 */
	~KSSLX509Map();

	/**
	 *  Set a value in the map
	 *
	 *  @param key the key
	 *  @param value the value
	 */
	void setValue(const TQString& key, const TQString& value);

	/**
	 *  Get the value of an entry in the map
	 *
	 *  @param key the key
	 *
	 *  @return the value
	 */
	TQString getValue(const TQString& key) const;

	/**
	 *  Reset (clear) the internal storage.
	 *
	 *  @param name if this is not empty, it will be parsed and used as
	 *         the new map internally
	 */
	void reset(const TQString& name = "");
  
private:
	class KSSLX509MapPrivate;
	KSSLX509MapPrivate *d;
	TQMap<TQString, TQString> m_pairs;

	void parse(const TQString& name);
};

#endif
