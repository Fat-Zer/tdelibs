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

TQString KRandrSimpleAPI::getIccFileName(TQString profileName, TQString screenName, TQString kde_confdir) {
	KSimpleConfig *t_config;
	KSimpleConfig *t_systemconfig;
	int t_numberOfProfiles;
	TQStringList t_cfgProfiles;
	TQString retval;

	if (profileName != NULL) {
		t_config = new KSimpleConfig( TQString::tqfromLatin1( "kiccconfigrc" ));
	}
	else {
		t_systemconfig = new KSimpleConfig( kde_confdir + TQString("/kicc/kiccconfigrc") );
	}

	if (profileName != NULL) {
		t_config->setGroup(NULL);
		if (t_config->readBoolEntry("EnableICC", false) == true) {
			t_config->setGroup(profileName);
			retval = t_config->readEntry(screenName);
		}
		else {
			retval = "";
		}
	}
	else {
		t_systemconfig->setGroup(NULL);
		if (t_systemconfig->readBoolEntry("EnableICC", false) == true) {
			retval = t_systemconfig->readEntry("ICCFile");
		}
		else {
			retval = "";
		}
	}

	if (profileName != "") {
		delete t_config;
	}
	else {
		delete t_systemconfig;
	}

	return retval;
}

TQString KRandrSimpleAPI::applyIccFile(TQString screenName, TQString fileName) {
	int i;
	int j;
	Display *randr_display;
	ScreenInfo *randr_screen_info;
	XRROutputInfo *output_info;

	int screenNumber = 0;

	if (fileName != "") {
		// FIXME
		// This should use the RRSetCrtcGamma function when available
		// That is the only way to get proper setting when two output are active at the same time
		// (otherwise in clone mode only one screen is available)

		// HACK
		// For now, simply exit with no changes if screenName is not an active output

		if (isValid() == true) {
			screenNumber = -1;
			randr_display = XOpenDisplay(NULL);
			randr_screen_info = read_screen_info(randr_display);
			if (randr_screen_info == NULL) {
				return "";
			}
			j=0;
			for (i = 0; i < randr_screen_info->n_output; i++) {
				output_info = randr_screen_info->outputs[i]->info;
				// Look for ON outputs...
				if (!randr_screen_info->outputs[i]->cur_crtc) {
					continue;
				}
				// ...that are connected
				if (RR_Disconnected == randr_screen_info->outputs[i]->info->connection) {
					continue;
				}
				if (output_info->name == screenName) {
					screenNumber = j;
				}
				j++;
			}
		}

		if (screenNumber >= 0) {
			// Apply ICC settings with XCalib
			TQString icc_command;
			FILE *pipe_xcalib;
			char xcalib_result[2048];
			int i;
			xcalib_result[0]=0;

			icc_command = TQString("xcalib %1").arg(fileName);
			if ((pipe_xcalib = popen(icc_command.ascii(), "r")) == NULL)
			{
				printf("Xcalib pipe error\n\r");
			}
			else {
				fgets(xcalib_result, 2048, pipe_xcalib);
				pclose(pipe_xcalib);
				for (i=1;i<2048;i++) {
					if (xcalib_result[i] == 0) {
						xcalib_result[i-1]=0;
						i=2048;
					}
				}
				if (strlen(xcalib_result) > 2) {
					return xcalib_result;
				}
			}
		}
	}
	else {
		// Reset ICC profile on this screen

		// FIXME
		// This should use the RRSetCrtcGamma function when available
		// That is the only way to get proper setting when two output are active at the same time
		// (otherwise in clone mode only one screen is available)

		// HACK
		// For now, simply exit with no changes if screenName is not an active output

		if (isValid() == true) {
			screenNumber = -1;
			randr_display = XOpenDisplay(NULL);
			randr_screen_info = read_screen_info(randr_display);
			if (randr_screen_info == NULL) {
				return "";
			}
			j=0;
			for (i = 0; i < randr_screen_info->n_output; i++) {
				output_info = randr_screen_info->outputs[i]->info;
				// Look for ON outputs...
				if (!randr_screen_info->outputs[i]->cur_crtc) {
					continue;
				}
				// ...that are connected
				if (RR_Disconnected == randr_screen_info->outputs[i]->info->connection) {
					continue;
				}
				if (output_info->name == screenName) {
					screenNumber = j;
				}
				j++;
			}
		}

		if (screenNumber >= 0) {
			// Apply ICC settings with XCalib
			TQString icc_command;
			FILE *pipe_xcalib;
			char xcalib_result[2048];
			int i;
			xcalib_result[0]=0;

			icc_command = TQString("xcalib -c");
			if ((pipe_xcalib = popen(icc_command.ascii(), "r")) == NULL)
			{
				printf("Xcalib pipe error\n\r");
			}
			else {
				fgets(xcalib_result, 2048, pipe_xcalib);
				pclose(pipe_xcalib);
				for (i=1;i<2048;i++) {
					if (xcalib_result[i] == 0) {
						xcalib_result[i-1]=0;
						i=2048;
					}
				}
				if (strlen(xcalib_result) > 2) {
					return xcalib_result;
				}
			}
		}
	}
	return "";
}

TQString KRandrSimpleAPI::applyIccConfiguration(TQString profileName, TQString kde_confdir) {
	int i;
	Display *randr_display;
	ScreenInfo *randr_screen_info;
	XRROutputInfo *output_info;
	KSimpleConfig *t_config;

	int screenNumber = 0;
	TQString errorstr = "";

	t_config = new KSimpleConfig( TQString::tqfromLatin1( "kiccconfigrc" ));

	// Find all screens
	if (isValid() == true) {
		randr_display = XOpenDisplay(NULL);
		randr_screen_info = read_screen_info(randr_display);
		if (randr_screen_info == NULL) {
			return "";
		}
		for (i = 0; i < randr_screen_info->n_output; i++) {
			output_info = randr_screen_info->outputs[i]->info;
			errorstr = applyIccFile(output_info->name, getIccFileName(profileName, output_info->name, kde_confdir));
			if (errorstr != "") {
				return errorstr;
			}
		}
	}
	else {
		return applyIccFile(getIccFileName(profileName, "Default", kde_confdir), "Default");
	}

	t_config->writeEntry("CurrentProfile", profileName);
	t_config->sync();
	delete t_config;

	return "";
}

TQString KRandrSimpleAPI::getCurrentProfile () {
	TQString profileName;
	KSimpleConfig *t_config;

	t_config = new KSimpleConfig( TQString::tqfromLatin1( "kiccconfigrc" ));
	profileName = t_config->readEntry("CurrentProfile");
	delete t_config;
	return profileName;
}

TQString KRandrSimpleAPI::applySystemWideIccConfiguration(TQString kde_confdir) {
	// Apply ICC settings with XCalib
	TQString icc_command;
	FILE *pipe_xcalib;
	char xcalib_result[2048];
	int i;
	xcalib_result[0]=0;

	icc_command = TQString("xcalib %1").arg(getIccFileName(NULL, "Default", kde_confdir));
	if ((pipe_xcalib = popen(icc_command.ascii(), "r")) == NULL)
	{
		printf("Xcalib pipe error\n\r");
	}
	else {
		fgets(xcalib_result, 2048, pipe_xcalib);
		pclose(pipe_xcalib);
		for (i=1;i<2048;i++) {
			if (xcalib_result[i] == 0) {
				xcalib_result[i-1]=0;
				i=2048;
			}
		}
		if (strlen(xcalib_result) > 2) {
			return xcalib_result;
		}
	}
	return "";
}

TQString KRandrSimpleAPI::clearIccConfiguration() {
	// Clear ICC settings with XCalib
	TQString icc_command;
	FILE *pipe_xcalib;
	char xcalib_result[2048];
	int i;
	xcalib_result[0]=0;

	icc_command = TQString("xcalib -c");
	if ((pipe_xcalib = popen(icc_command.ascii(), "r")) == NULL)
	{
		printf("Xcalib pipe error\n\r");
	}
	else {
		fgets(xcalib_result, 2048, pipe_xcalib);
		pclose(pipe_xcalib);
		for (i=1;i<2048;i++) {
			if (xcalib_result[i] == 0) {
				xcalib_result[i-1]=0;
				i=2048;
			}
		}
		if (strlen(xcalib_result) > 2) {
			return xcalib_result;
		}
	}
	return "";
}

ScreenInfo* KRandrSimpleAPI::read_screen_info (Display *display)
{
    return internal_read_screen_info(display);
}

int KRandrSimpleAPI::set_screen_size (ScreenInfo *screen_info)
{
    return internal_set_screen_size(screen_info);
}

void KRandrSimpleAPI::output_auto (ScreenInfo *screen_info, OutputInfo *output_info)
{
    internal_output_auto (screen_info, output_info);
}

void KRandrSimpleAPI::output_off(ScreenInfo *screen_info, OutputInfo *output)
{
    internal_output_off(screen_info, output);
}

CrtcInfo* KRandrSimpleAPI::auto_find_crtc (ScreenInfo *screen_info, OutputInfo *output_info)
{
    return internal_auto_find_crtc (screen_info, output_info);
}

XRRModeInfo *KRandrSimpleAPI::find_mode_by_xid (ScreenInfo *screen_info, RRMode mode_id)
{
    return internal_find_mode_by_xid (screen_info, mode_id);
}

int KRandrSimpleAPI::mode_height (XRRModeInfo *mode_info, Rotation rotation)
{
    return internal_mode_height (mode_info, rotation);
}

int KRandrSimpleAPI::mode_width (XRRModeInfo *mode_info, Rotation rotation)
{
    return internal_mode_width (mode_info, rotation);
}

int KRandrSimpleAPI::get_width_by_output_id (ScreenInfo *screen_info, RROutput output_id)
{
    return internal_get_width_by_output_id (screen_info, output_id);
}

int KRandrSimpleAPI::get_height_by_output_id (ScreenInfo *screen_info, RROutput output_id)
{
    return internal_get_height_by_output_id (screen_info, output_id);
}

char *KRandrSimpleAPI::get_output_name (ScreenInfo *screen_info, RROutput id)
{
    return internal_get_output_name (screen_info, id);
}

Status KRandrSimpleAPI::crtc_apply (CrtcInfo *crtc_info)
{
    return internal_crtc_apply (crtc_info);
}

Status KRandrSimpleAPI::crtc_disable (CrtcInfo *crtc)
{
    return internal_crtc_disable (crtc);
}

int KRandrSimpleAPI::main_low_apply (ScreenInfo *screen_info)
{
    return internal_main_low_apply (screen_info);
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

