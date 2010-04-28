/*  libkrandr.cc     - class KRandr that makes it easy to use XRandr in KDE
    This file is part of KRandr 0.9.5
    Copyright (C) 2010  Timothy Pearson
    LibKMid's homepage : http://trinity.pearsoncomputing.net

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    Send comments and bug fixes to Timothy Pearson <kb9vqf@pearsoncomputing.net>

***************************************************************************/

#include "libkrandr.h"

ScreenInfo* KRandrSimpleAPI::read_screen_info (Display *display)
{
    return read_screen_info(display);
}

int KRandrSimpleAPI::set_screen_size (ScreenInfo *screen_info)
{
    return set_screen_size(screen_info);
}

void KRandrSimpleAPI::output_auto (ScreenInfo *screen_info, OutputInfo *output_info)
{
    output_auto (screen_info, output_info);
}

void KRandrSimpleAPI::output_off(ScreenInfo *screen_info, OutputInfo *output)
{
    output_off(screen_info, output);
}

CrtcInfo* KRandrSimpleAPI::auto_find_crtc (ScreenInfo *screen_info, OutputInfo *output_info)
{
    return auto_find_crtc (screen_info, output_info);
}

XRRModeInfo *KRandrSimpleAPI::find_mode_by_xid (ScreenInfo *screen_info, RRMode mode_id)
{
    return find_mode_by_xid (screen_info, mode_id);
}

int KRandrSimpleAPI::mode_height (XRRModeInfo *mode_info, Rotation rotation)
{
    return mode_height (mode_info, rotation);
}

int KRandrSimpleAPI::mode_width (XRRModeInfo *mode_info, Rotation rotation)
{
    return mode_width (mode_info, rotation);
}

int KRandrSimpleAPI::get_width_by_output_id (ScreenInfo *screen_info, RROutput output_id)
{
    return get_width_by_output_id (screen_info, output_id);
}

int KRandrSimpleAPI::get_height_by_output_id (ScreenInfo *screen_info, RROutput output_id)
{
    return get_height_by_output_id (screen_info, output_id);
}

char *KRandrSimpleAPI::get_output_name (ScreenInfo *screen_info, RROutput id)
{
    return get_output_name (screen_info, id);
}

Status KRandrSimpleAPI::crtc_apply (CrtcInfo *crtc_info)
{
    return crtc_apply (crtc_info);
}

Status KRandrSimpleAPI::crtc_disable (CrtcInfo *crtc)
{
    return crtc_disable (crtc);
}

int KRandrSimpleAPI::main_low_apply (ScreenInfo *screen_info)
{
    return main_low_apply (screen_info);
}

bool KRandrSimpleAPI::kRandrHasRandr(void)
{
    return isValid();
}

const char *KRandrSimpleAPI::kRandrVersion(void)
{
    return "0.9.5";
}

const char *KRandrSimpleAPI::kRandrCopyright(void)
{
   return "LibKRandr 0.9.5 (C)2010 Timothy Pearson <kb9vqf@pearsoncomputing.net>. U.S.A.";
}

/* * * * * *

 Under this line (------) there's only a C wrapper for the KRandrSimpleAPI class

* * * * * */
const char *kRandrVersion(void)
{
  return KRandrSimpleAPI::kRandrVersion();
}

const char *kRandrCopyright(void)
{
  return KRandrSimpleAPI::kRandrCopyright();
}

