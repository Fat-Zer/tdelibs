/*  libkrandr.h     - class KRandr that makes it easy to use XRandr in KDE
    This file is part of KRandr 0.9.5
    Copyright (C) 2010  Timothy Pearson
    LibKRandr's homepage : http://www.trinitydesktop.org

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
#ifndef _LIBKRANDR_H
#define _LIBKRANDR_H

#include "randr.h"
#include "lowlevel_randr.h"

#ifdef __cplusplus

#include <tqfile.h>

#include <kconfig.h>
#include <ksimpleconfig.h>
#include <kdelibs_export.h>

/**
 * Simple API covering most of the uses of libkrandr.
 *
 * You can use the members of this class in pure C applications, just by using
 * the same name as the corresponding function member.
 *
 * @short A simple API around the rest of libkrandr.
 * @version 0.9.5 27/04/2010
 * @author Timothy Pearson <kb9vqf@pearsoncomputing.net>
 */
class KRANDR_EXPORT KRandrSimpleAPI : public RandRDisplay
{
  private:

  public:
    /**
     * Retrieves the specificed ICC profile filename from the configuration database
     */
    TQString getIccFileName(TQString profileName, TQString screenName, TQString kde_confdir);

    /**
     * Applies the specificed ICC profile filename to the specified RandR output
     * If RandR is not available, the specified file is applied to the current display
     */
    TQString applyIccFile(TQString screenName, TQString fileName);

    /**
     * Applies all saved ICC profile settings to all RandR outputs
     * If RandR is not available, the settings are applied to the current display
     */
    TQString applyIccConfiguration(TQString profileName, TQString kde_confdir);

    /**
     * Applies saved system wide settings to the current display
     */
    TQString applySystemWideIccConfiguration(TQString kde_confdir);

    /**
     * Resets the current display
     */
    TQString clearIccConfiguration(void);

    /**
     * Retrieves current profile name
     */
    TQString getCurrentProfile(void);

    /**
     * Reads current screen information.
     * NOTE: The caller is responsible for calling freeScreenInfoStructure() when done
     */
    ScreenInfo* read_screen_info(Display *display);

    /**
     * Frees the ScreenInfo structure
     */
    void freeScreenInfoStructure(ScreenInfo* screen_info);

    /**
     * Sets the screen size.
     */
    int set_screen_size (ScreenInfo *screen_info);

    /**
     * Automatically selects an output port.
     */
    void output_auto (ScreenInfo *screen_info, OutputInfo *output_info);

    /**
     * Turns off a specified output on a specified screen.
     */
    void output_off(ScreenInfo *screen_info, OutputInfo *output);

    /**
     * Automatically finds the CRTC structure.
     */
    CrtcInfo* auto_find_crtc (ScreenInfo *screen_info, OutputInfo *output_info);

    /**
     * Finds a mode by XID.
     */
    XRRModeInfo *find_mode_by_xid (ScreenInfo *screen_info, RRMode mode_id);

    /**
     * Returns specified mode height in pixels.
     */
    int mode_height (XRRModeInfo *mode_info, Rotation rotation);

    /**
     * Returns specified mode width in pixels.
     */
    int mode_width (XRRModeInfo *mode_info, Rotation rotation);

    /**
     * Returns specified output width in pixels.
     */
    int get_width_by_output_id (ScreenInfo *screen_info, RROutput output_id);

    /**
     * Returns specified output height in pixels.
     */
    int get_height_by_output_id (ScreenInfo *screen_info, RROutput output_id);

    /**
     * Returns output name.
     */
    char *get_output_name (ScreenInfo *screen_info, RROutput id);

    /**
     * Applies specified CRTC.
     */
    Status crtc_apply (CrtcInfo *crtc_info);

    /**
     * Disables specificed CRTC
     */
    Status crtc_disable (CrtcInfo *crtc);

    /**
     * Applies all previously configured settings to the specified screen.
     */
    int main_low_apply (ScreenInfo *screen_info);

    /**
    * Gets the binary monitor EDID for the specified card and display
    */
    TQByteArray getEDID(int card, TQString displayname);

    /**
    * Gets the monitor EDID name for the specified card and display
    */
    TQString getEDIDMonitorName(int card, TQString displayname);

    /**
    * Saves the systemwide display configuration screenInfoArray to the specified profile
    * If profilename is empty, the default profile is utilized
    * If enable is set to true, the default profile will be applied at system startup
    */
    void saveSystemwideDisplayConfiguration(bool enable, TQString profilename, TQString kde_confdir, TQPtrList<SingleScreenData> screenInfoArray);

    /**
    * Reads the systemwide display configuration screenInfoArray from the specified profile
    * If profilename is empty, the default profile is utilized
    * WARNING: The calling application must free the returned objects when it is done using them
    */
    TQPtrList<SingleScreenData> loadSystemwideDisplayConfiguration(TQString profilename, TQString kde_confdir);

    /**
    * Applies the systemwide display configuration screenInfoArray from the specified profile
    * If profilename is empty, the default profile is utilized
    * Returns the offset of the primary screen's top left corner
    */
    TQPoint applySystemwideDisplayConfiguration(TQString profilename, TQString kde_confdir);

    /**
    * Applies the systemwide display configuration screenInfoArray to the hardware
    * If test is true, the new configuration will be loaded for a short period of time, then reverted automatically
    * Returns true if configuration was accepted; false if not
    */
    bool applySystemwideDisplayConfiguration(TQPtrList<SingleScreenData> screenInfoArray, bool test=TRUE);

    /**
    * Applies the gamma contained within the systemwide display configuration screenInfoArray to the hardware
    */
    void applySystemwideDisplayGamma(TQPtrList<SingleScreenData> screenInfoArray);

    /**
    * Destroys a screen information object
    */
    void destroyScreenInformationObject(TQPtrList<SingleScreenData> screenInfoArray);

    /**
    * Returns the offset of the primary screen's Top Left Corner
    */
    TQPoint primaryScreenOffsetFromTLC(TQPtrList<SingleScreenData> screenInfoArray);

    /**
    * Ensures that the data contained within screenInfoArray is self consistent
    */
    void ensureMonitorDataConsistency(TQPtrList<SingleScreenData> screenInfoArray);

    /**
    * Reads the current display configuration screenInfoArray from the hardware
    */
    TQPtrList<SingleScreenData> readCurrentDisplayConfiguration();

    /**
    * Returns the hardware rotation flags given a valid SingleScreenData structure
    */
    int getHardwareRotationFlags(SingleScreenData*);

    /**
     * Returns whether or not the system supports XRandR
     */
    bool kRandrHasRandr();

    /**
     * Returns the version number of libkrandr, i.e. "0.9.5" or "1.0 Beta"
     */
    static const char *kRandrVersion(void);

    /**
     * Returns the copyright notice that applications using libkrandr should print
     * to the user in an about box or somewhere visible.
     * I.e.
     *
     * "LibKRandr 0.9.5 (C) 2010 Timothy Pearson <kb9vqf@pearsoncomputing.net>. U.S.A."
     */
    static const char *kRandrCopyright(void);

};



extern "C" {

#else
#define KRANDR_EXPORT
#endif

// KRANDR_EXPORT ScreenInfo* read_screen_info(Display *);
// KRANDR_EXPORT int         set_screen_size (ScreenInfo *screen_info);
// KRANDR_EXPORT void        output_auto (ScreenInfo *screen_info, OutputInfo *output_info);
// KRANDR_EXPORT void        output_off(ScreenInfo *screen_info, OutputInfo *output);
// KRANDR_EXPORT CrtcInfo*   auto_find_crtc (ScreenInfo *screen_info, OutputInfo *output_info);
// KRANDR_EXPORT XRRModeInfo *find_mode_by_xid (ScreenInfo *screen_info, RRMode mode_id);
// KRANDR_EXPORT int         mode_height (XRRModeInfo *mode_info, Rotation rotation);
// KRANDR_EXPORT int         mode_width (XRRModeInfo *mode_info, Rotation rotation);
// KRANDR_EXPORT int         get_width_by_output_id (ScreenInfo *screen_info, RROutput output_id);
// KRANDR_EXPORT int         get_height_by_output_id (ScreenInfo *screen_info, RROutput output_id);
// KRANDR_EXPORT char        *get_output_name (ScreenInfo *screen_info, RROutput id);
// KRANDR_EXPORT Status      crtc_apply (CrtcInfo *crtc_info);
// KRANDR_EXPORT Status      crtc_disable (CrtcInfo *crtc);
// KRANDR_EXPORT int         main_low_apply (ScreenInfo *screen_info);
// KRANDR_EXPORT bool        kRandrHasRandr();

KRANDR_EXPORT const char  *kRandrVersion(void);
KRANDR_EXPORT const char  *kRandrCopyright(void);

#ifdef __cplusplus

}


#endif


#endif
