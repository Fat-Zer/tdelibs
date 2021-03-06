/*
 * threadevents.h
 *
 * Copyright (C)  2004  Zack Rusin <zack@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
#ifndef TDESPELL_THREADEVENTS_H
#define TDESPELL_THREADEVENTS_H

#include <tqevent.h>
#include <tqstring.h>

namespace KSpell2
{
    enum {
        FoundMisspelling = 2003,
        FinishedChecking  = 2004
    };
    class MisspellingEvent : public TQCustomEvent
    {
    public:
        MisspellingEvent( const TQString& word,
                         int pos )
            : TQCustomEvent( FoundMisspelling ), m_word( word ),
              m_position( pos )
            {}

        TQString word() const {
            return m_word;
        }
        int     position() const {
            return m_position;
        }
    private:
        TQString m_word;
        int     m_position;
    };
    class FinishedCheckingEvent : public TQCustomEvent
    {
    public:
        FinishedCheckingEvent()
            : TQCustomEvent( FinishedChecking )
            {}
    };

}

#endif
