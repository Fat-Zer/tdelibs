/*
  Copyright (c) 2000-2001 Trolltech AS (info@trolltech.com)

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.
*/

#include <tqstyleplugin.h>
#include <tqimage.h>
#include <tqapplication.h>

#include "lightstyle-v2.h"
#include "lightstyle-v3.h"

class LightStylePlugin : public TQStylePlugin
{
public:
    LightStylePlugin();

    TQStringList keys() const;
    TQStyle *create(const TQString &);
};

LightStylePlugin::LightStylePlugin()
    : TQStylePlugin()
{
}

TQStringList LightStylePlugin::keys() const
{
    TQStringList list;
    list << "Light, 2nd revision";
    list << "Light, 3rd revision";
    return list;
}

TQStyle *LightStylePlugin::create(const TQString &s)
{
    if (s.lower() == "light, 2nd revision")
	return new LightStyleV2;
    if (s.lower() == "light, 3rd revision")
	return new LightStyleV3;
    return 0;
}

KDE_Q_EXPORT_PLUGIN( LightStylePlugin )
