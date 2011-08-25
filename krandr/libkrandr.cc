/*  libkrandr.cc     - class KRandr that makes it easy to use XRandr in KDE
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

#include <tqtimer.h>
#include <tqstringlist.h>

#include <klocale.h>
#include <kapplication.h>

#include <stdlib.h>
#include <cmath>

#include "libkrandr.h"

// FIXME
// For now, just use the standalone xrandr program to apply the display settings
#define USE_XRANDR_PROGRAM

// This routine is courtsey of an answer on "Stack Overflow"
// It takes an LSB-first int and makes it an MSB-first int (or vice versa)
unsigned int reverse_bits(register unsigned int x)
{
    x = (((x & 0xaaaaaaaa) >> 1) | ((x & 0x55555555) << 1));
    x = (((x & 0xcccccccc) >> 2) | ((x & 0x33333333) << 2));
    x = (((x & 0xf0f0f0f0) >> 4) | ((x & 0x0f0f0f0f) << 4));
    x = (((x & 0xff00ff00) >> 8) | ((x & 0x00ff00ff) << 8));
    return((x >> 16) | (x << 16));
}

TQString capitalizeString(TQString in) {
	return in.left(1).upper() + in.right(in.length()-1);
}

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
			randr_display = qt_xdisplay();
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
			freeScreenInfoStructure(randr_screen_info);
		}

		if (screenNumber >= 0) {
			// Apply ICC settings with XCalib
			TQString icc_command;
			FILE *pipe_xcalib;
			char xcalib_result[2048];
			int i;
			xcalib_result[0]=0;

			icc_command = TQString("xcalib \"%1\"").arg(fileName);
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
			randr_display = qt_xdisplay();
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
			freeScreenInfoStructure(randr_screen_info);
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
		randr_display = qt_xdisplay();
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
		freeScreenInfoStructure(randr_screen_info);
	}
	else {
		return applyIccFile(getIccFileName(profileName, "Default", kde_confdir), "Default");
	}

	t_config->writeEntry("CurrentProfile", profileName);
	t_config->sync();
	delete t_config;

	return "";
}

TQString KRandrSimpleAPI::getEDIDMonitorName(int card, TQString displayname) {
	TQString edid;
	TQByteArray binaryedid = getEDID(card, displayname);
	if (binaryedid.isNull())
		return TQString();

	// Get the manufacturer ID
	unsigned char letter_1 = ((binaryedid[8]>>2) & 0x1F) + 0x40;
	unsigned char letter_2 = (((binaryedid[8] & 0x03) << 3) | ((binaryedid[9]>>5) & 0x07)) + 0x40;
	unsigned char letter_3 = (binaryedid[9] & 0x1F) + 0x40;
	TQChar qletter_1 = TQChar(letter_1);
	TQChar qletter_2 = TQChar(letter_2);
	TQChar qletter_3 = TQChar(letter_3);
	TQString manufacturer_id = TQString("%1%2%3").arg(qletter_1).arg(qletter_2).arg(qletter_3);

	// Get the model ID
	unsigned int raw_model_id = (((binaryedid[10] << 8) | binaryedid[11]) << 16) & 0xFFFF0000;
	// Reverse the bit order
	unsigned int model_id = reverse_bits(raw_model_id);

	// Try to get the model name
	bool has_friendly_name = false;
	unsigned char descriptor_block[18];
	int i;
	for (i=72;i<90;i++) {
		descriptor_block[i-72] = binaryedid[i] & 0xFF;
	}
	if ((descriptor_block[0] != 0) || (descriptor_block[1] != 0) || (descriptor_block[3] != 0xFC)) {
		for (i=90;i<108;i++) {
			descriptor_block[i-90] = binaryedid[i] & 0xFF;
		}
		if ((descriptor_block[0] != 0) || (descriptor_block[1] != 0) || (descriptor_block[3] != 0xFC)) {
			for (i=108;i<126;i++) {
				descriptor_block[i-108] = binaryedid[i] & 0xFF;
			}
		}
	}

	TQString monitor_name;
	if ((descriptor_block[0] == 0) && (descriptor_block[1] == 0) && (descriptor_block[3] == 0xFC)) {
		char* pos = strchr((char *)(descriptor_block+5), '\n');
		if (pos) {
			*pos = 0;
			has_friendly_name = true;
			monitor_name = TQString((char *)(descriptor_block+5));
		}
		else {
			has_friendly_name = false;
		}
	}

	// [FIXME]
	// Look up manudacturer names if possible!

	if (has_friendly_name)
		edid = TQString("%1 %2").arg(manufacturer_id).arg(monitor_name);
	else
		edid = TQString("%1 0x%2").arg(manufacturer_id).arg(model_id, 0, 16);

	return edid;
}

TQByteArray KRandrSimpleAPI::getEDID(int card, TQString displayname) {
	TQFile file(TQString("/sys/class/drm/card%1-%2/edid").arg(card).arg(displayname));
	if (!file.open (IO_ReadOnly))
		return TQByteArray();
	TQByteArray binaryedid = file.readAll();
	file.close();
	return binaryedid;
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

	icc_command = TQString("xcalib \"%1\"").arg(getIccFileName(NULL, "Default", kde_confdir));
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

void KRandrSimpleAPI::saveSystemwideDisplayConfiguration(bool enable, TQString profilename, TQString kde_confdir, TQPtrList<SingleScreenData> screenInfoArray) {
	int i;

	TQString filename;

	filename = "displayglobals";
	filename.prepend(kde_confdir.append("/"));
	KSimpleConfig* display_config = new KSimpleConfig( filename );
	display_config->setGroup("General");
	display_config->writeEntry("ApplySettingsOnStart", enable);
	display_config->sync();
	delete display_config;

	filename = profilename;
	if (filename == "")
		filename = "default";
	filename.prepend(kde_confdir.append("/displayconfig/"));

	display_config = new KSimpleConfig( filename );

	i=0;
	SingleScreenData *screendata;
	for ( screendata=screenInfoArray.first(); screendata; screendata=screenInfoArray.next() ) {
		display_config->setGroup(TQString("SCREEN %1").arg(i));
		display_config->writeEntry("ScreenFriendlyName", screendata->screenFriendlyName);
		display_config->writeEntry("GenericScreenDetected", screendata->generic_screen_detected);
		display_config->writeEntry("ScreenConnected", screendata->screen_connected);
		display_config->writeEntry("Resolutions", screendata->resolutions);
		display_config->writeEntry("RefreshRates", screendata->refresh_rates);
		display_config->writeEntry("ColorDepths", screendata->color_depths);
		display_config->writeEntry("AvailableRotations", screendata->rotations);
		display_config->writeEntry("CurrentResolution", screendata->current_resolution_index);
		display_config->writeEntry("CurrentRefreshRate", screendata->current_refresh_rate_index);
		display_config->writeEntry("CurrentColorDepth", screendata->current_color_depth_index);
		display_config->writeEntry("CurrentRotation", screendata->current_rotation_index);
		display_config->writeEntry("CurrentOrientiation", screendata->current_orientation_mask);
		display_config->writeEntry("GammaRed", screendata->gamma_red);
		display_config->writeEntry("GammaGreen", screendata->gamma_green);
		display_config->writeEntry("GammaBlue", screendata->gamma_blue);
		display_config->writeEntry("CurrentXFlip", screendata->has_x_flip);
		display_config->writeEntry("CurrentYFlip", screendata->has_y_flip);
		display_config->writeEntry("SupportsTransformation", screendata->supports_transformations);
		display_config->writeEntry("IsPrimary", screendata->is_primary);
		display_config->writeEntry("IsExtended", screendata->is_extended);
		display_config->writeEntry("AbsXPos", screendata->absolute_x_position);
		display_config->writeEntry("AbsYPos", screendata->absolute_y_position);
		display_config->writeEntry("CurrentXPixelCount", screendata->current_x_pixel_count);
		display_config->writeEntry("CurrentYPixelCount", screendata->current_y_pixel_count);
		i++;
	}

	display_config->sync();
	delete display_config;
}

TQPoint KRandrSimpleAPI::applySystemwideDisplayConfiguration(TQString profilename, TQString kde_confdir) {
	TQPoint ret;

	TQString filename = "displayglobals";
	filename.prepend(kde_confdir.append("/"));
	KSimpleConfig* display_config = new KSimpleConfig( filename );
	display_config->setGroup("General");
	bool enabled = display_config->readBoolEntry("ApplySettingsOnStart", false);
	delete display_config;

	if (enabled) {
		TQPtrList<SingleScreenData> screenInfoArray;
		screenInfoArray = loadSystemwideDisplayConfiguration(profilename, kde_confdir);
		if (screenInfoArray.count() > 0) {
			applySystemwideDisplayConfiguration(screenInfoArray, FALSE);
		}
		destroyScreenInformationObject(screenInfoArray);
		screenInfoArray = readCurrentDisplayConfiguration();
		ensureMonitorDataConsistency(screenInfoArray);
		ret = primaryScreenOffsetFromTLC(screenInfoArray);
		destroyScreenInformationObject(screenInfoArray);
	}

	return ret;
}

TQPtrList<SingleScreenData> KRandrSimpleAPI::loadSystemwideDisplayConfiguration(TQString profilename, TQString kde_confdir) {
	int i;

	TQString filename;
	filename = profilename;
	if (filename == "")
		filename = "default";
	filename.prepend(kde_confdir.append("/displayconfig/"));

	KSimpleConfig* display_config = new KSimpleConfig( filename );

	TQStringList grouplist = display_config->groupList();
	SingleScreenData *screendata;
	TQPtrList<SingleScreenData> screenInfoArray;
	for ( TQStringList::Iterator it = grouplist.begin(); it != grouplist.end(); ++it ) {
		if ((*it).startsWith("SCREEN ")) {
			display_config->setGroup(*it);
			i = ((*it).remove("SCREEN ")).toInt();
			screendata = new SingleScreenData;
			screenInfoArray.append(screendata);
			screendata->screenFriendlyName = display_config->readEntry("ScreenFriendlyName");
			screendata->generic_screen_detected = display_config->readBoolEntry("GenericScreenDetected");
			screendata->screen_connected = display_config->readBoolEntry("ScreenConnected");
			screendata->resolutions = display_config->readListEntry("Resolutions");
			screendata->refresh_rates = display_config->readListEntry("RefreshRates");
			screendata->color_depths = display_config->readListEntry("ColorDepths");
			screendata->rotations = display_config->readListEntry("AvailableRotations");
			screendata->current_resolution_index = display_config->readNumEntry("CurrentResolution");
			screendata->current_refresh_rate_index = display_config->readNumEntry("CurrentRefreshRate");
			screendata->current_color_depth_index = display_config->readNumEntry("CurrentColorDepth");
			screendata->current_rotation_index = display_config->readNumEntry("CurrentRotation");
			screendata->current_orientation_mask = display_config->readNumEntry("CurrentOrientiation");
			screendata->gamma_red = display_config->readDoubleNumEntry("GammaRed");
			screendata->gamma_green = display_config->readDoubleNumEntry("GammaGreen");
			screendata->gamma_blue = display_config->readDoubleNumEntry("GammaBlue");
			screendata->has_x_flip = display_config->readBoolEntry("CurrentXFlip");
			screendata->has_y_flip = display_config->readBoolEntry("CurrentYFlip");
			screendata->supports_transformations = display_config->readBoolEntry("SupportsTransformation");
			screendata->is_primary = display_config->readBoolEntry("IsPrimary");
			screendata->is_extended = display_config->readBoolEntry("IsExtended");
			screendata->absolute_x_position = display_config->readNumEntry("AbsXPos");
			screendata->absolute_y_position = display_config->readNumEntry("AbsYPos");
			screendata->current_x_pixel_count = display_config->readNumEntry("CurrentXPixelCount");
			screendata->current_y_pixel_count = display_config->readNumEntry("CurrentYPixelCount");
		}
	}

	delete display_config;

	return screenInfoArray;
}

int KRandrSimpleAPI::getHardwareRotationFlags(SingleScreenData* screendata) {
	int rotationFlags = 0;
	TQString rotationDesired = *screendata->rotations.at(screendata->current_rotation_index);
	if (rotationDesired == "Normal") {
		rotationFlags = rotationFlags | RandRScreen::Rotate0;
	}
	else if (rotationDesired == "Rotate 90 degrees") {
		rotationFlags = rotationFlags | RandRScreen::Rotate90;
	}
	else if (rotationDesired == "Rotate 180 degrees") {
		rotationFlags = rotationFlags | RandRScreen::Rotate180;
	}
	else if (rotationDesired == "Rotate 270 degrees") {
		rotationFlags = rotationFlags | RandRScreen::Rotate270;
	}
	if (screendata->has_x_flip) {
		rotationFlags = rotationFlags | RandRScreen::ReflectX;
	}
	if (screendata->has_y_flip) {
		rotationFlags = rotationFlags | RandRScreen::ReflectY;
	}
	return rotationFlags;
}

#define USE_XRANDR_PROGRAM

bool KRandrSimpleAPI::applySystemwideDisplayConfiguration(TQPtrList<SingleScreenData> screenInfoArray, bool test) {
	int i;
	int j;
	bool accepted = true;
	Display *randr_display;
	XRROutputInfo *output_info;
	ScreenInfo *randr_screen_info;

	SingleScreenData *screendata;

	TQPtrList<SingleScreenData> oldconfig;
	if (test == TRUE) {
		oldconfig = readCurrentDisplayConfiguration();
	}

	if (isValid() == true) {
#ifdef USE_XRANDR_PROGRAM
		// Assemble the command string for xrandr
		TQString command;
		command = "xrandr";

		randr_display = qt_xdisplay();
		randr_screen_info = read_screen_info(randr_display);
		for (i = 0; i < randr_screen_info->n_output; i++) {
			screendata = screenInfoArray.at(i);
			output_info = randr_screen_info->outputs[i]->info;
			command.append(" --output ").append(output_info->name);
			if (screendata->is_primary || screendata->is_extended) {
				command.append(TQString(" --mode %1x%2").arg(screendata->current_x_pixel_count).arg(screendata->current_y_pixel_count));
				command.append(TQString(" --pos %1x%2").arg(screendata->absolute_x_position).arg(screendata->absolute_y_position));
				command.append(TQString(" --refresh %1").arg((*screendata->refresh_rates.at(screendata->current_refresh_rate_index)).replace("Hz", "")));
				command.append(TQString(" --gamma %1:%2:%3").arg(screendata->gamma_red).arg(screendata->gamma_green).arg(screendata->gamma_blue));
				if (screendata->current_rotation_index == 0) command.append(" --rotate ").append("normal");
				if (screendata->current_rotation_index == 1) command.append(" --rotate ").append("left");
				if (screendata->current_rotation_index == 2) command.append(" --rotate ").append("inverted");
				if (screendata->current_rotation_index == 3) command.append(" --rotate ").append("right");
				if ((screendata->has_x_flip == 0) && (screendata->has_y_flip == 0)) command.append(" --reflect ").append("normal");
				if ((screendata->has_x_flip == 1) && (screendata->has_y_flip == 0)) command.append(" --reflect ").append("x");
				if ((screendata->has_x_flip == 0) && (screendata->has_y_flip == 1)) command.append(" --reflect ").append("y");
				if ((screendata->has_x_flip == 1) && (screendata->has_y_flip == 1)) command.append(" --reflect ").append("xy");
				if (screendata->is_primary) {
					command.append(" --primary");
				}
			}
			else {
				command.append(" --off");
			}
		}
		freeScreenInfoStructure(randr_screen_info);

		system(command.ascii());

		// HACK
		// This is needed because Qt does not properly generate screen
		// resize events when switching screens, so KDE gets stuck in the old resolution
		// This only seems to happen with more than one screen, so check for that condition...
		// FIXME: This also only occurs when the primary display has been changed
		// FIXME: Check for that condition as well!
		if (kapp->desktop()->numScreens() > 1) {
			for (i = 0; i < randr_screen_info->n_output; i++) {
				screendata = screenInfoArray.at(i);
				if (screendata->is_primary == true) {
					kapp->desktop()->emitResizedSignal(i);
				}
			}
		}
#else
		randr_display = qt_xdisplay();
		randr_screen_info = read_screen_info(randr_display);
		// Turn off all displays
		for (i = 0; i < randr_screen_info->n_output; i++) {
			screendata = screenInfoArray.at(i);
			output_info = randr_screen_info->outputs[i]->info;

			randr_screen_info->cur_crtc = randr_screen_info->outputs[i]->cur_crtc;
			randr_screen_info->cur_output = randr_screen_info->outputs[i];
			randr_screen_info->cur_output->auto_set = 0;
			randr_screen_info->cur_output->off_set = 1;
			output_off (randr_screen_info, randr_screen_info->cur_output);
			j=main_low_apply(randr_screen_info);
		}
		freeScreenInfoStructure(randr_screen_info);
		randr_screen_info = read_screen_info(randr_display);
		// Turn on the primary display
		for (i = 0; i < randr_screen_info->n_output; i++) {
			screendata = screenInfoArray.at(i);
			output_info = randr_screen_info->outputs[i]->info;

			if (screendata->is_primary == true) {
				randr_screen_info->cur_crtc = randr_screen_info->outputs[i]->cur_crtc;
				randr_screen_info->cur_output = randr_screen_info->outputs[i];
				randr_screen_info->cur_output->auto_set = 1;
				randr_screen_info->cur_output->off_set = 0;
				output_auto (randr_screen_info, randr_screen_info->cur_output);
				j=main_low_apply(randr_screen_info);
			}
		}
		freeScreenInfoStructure(randr_screen_info);
		// Handle the remaining displays
		randr_screen_info = read_screen_info(randr_display);
		for (i = 0; i < randr_screen_info->n_output; i++) {
			screendata = screenInfoArray.at(i);
			output_info = randr_screen_info->outputs[i]->info;

			// Activate or deactivate the screens as necessary
			randr_screen_info->cur_crtc = randr_screen_info->outputs[i]->cur_crtc;
			randr_screen_info->cur_output = randr_screen_info->outputs[i];
			if (screendata->is_primary == false) {
				if (screendata->is_primary || screendata->is_extended) {
					randr_screen_info->cur_output->auto_set = 1;
					randr_screen_info->cur_output->off_set = 0;
					output_auto (randr_screen_info, randr_screen_info->cur_output);
					j=main_low_apply(randr_screen_info);
				}
				else {
					randr_screen_info->cur_output->auto_set = 0;
					randr_screen_info->cur_output->off_set = 1;
					output_off (randr_screen_info, randr_screen_info->cur_output);
					j=main_low_apply(randr_screen_info);
				}
			}
		}
		freeScreenInfoStructure(randr_screen_info);
		randr_screen_info = read_screen_info(randr_display);
		for (i = 0; i < randr_screen_info->n_output; i++) {
			screendata = screenInfoArray.at(i);
			output_info = randr_screen_info->outputs[i]->info;

			if (screendata->is_primary || screendata->is_extended) {
				// Set rotation, refresh rate, and size
				RandRScreen *cur_screen = new RandRScreen(i);
				cur_screen->proposeSize(screendata->current_resolution_index);
				cur_screen->proposeRefreshRate(screendata->current_refresh_rate_index);
				cur_screen->proposeRotation(getHardwareRotationFlags(screendata));
				cur_screen->applyProposed();
				delete cur_screen;

				// Force data reload
				randr_screen_info = read_screen_info(randr_display);
				output_info = randr_screen_info->outputs[i]->info;

				// Finally, set the screen's position
				randr_screen_info->cur_crtc = randr_screen_info->outputs[i]->cur_crtc;
				if (randr_screen_info->cur_crtc) {
					randr_screen_info->cur_crtc->cur_x = screendata->absolute_x_position;
					randr_screen_info->cur_crtc->cur_y = screendata->absolute_y_position;
					j=main_low_apply(randr_screen_info);
				}
			}
		}
		freeScreenInfoStructure(randr_screen_info);
#endif
	}

	applySystemwideDisplayGamma(screenInfoArray);

	if (test == TRUE) {
		int ret = showTestConfigurationDialog();
		if (!ret) {
			applySystemwideDisplayConfiguration(oldconfig, FALSE);
			accepted = false;
		}
		destroyScreenInformationObject(oldconfig);
	}

	return accepted;
}

void KRandrSimpleAPI::destroyScreenInformationObject(TQPtrList<SingleScreenData> screenInfoArray) {
	SingleScreenData *screendata;
	for ( screendata = screenInfoArray.first(); screendata; screendata = screenInfoArray.next() ) {
		screenInfoArray.remove(screendata);
		delete screendata;
	}
}

void KRandrSimpleAPI::ensureMonitorDataConsistency(TQPtrList<SingleScreenData> screenInfoArray) {
	int i;
	SingleScreenData *screendata;

	int numberOfScreens = screenInfoArray.count();

	for (i=0;i<numberOfScreens;i++) {
		screendata = screenInfoArray.at(i);
		if (!screendata->screen_connected) {
			screendata->is_primary = false;
			screendata->is_extended = false;
		}
	}

	bool has_primary_monitor = false;
	for (i=0;i<numberOfScreens;i++) {
		screendata = screenInfoArray.at(i);
		if (screendata->is_primary)
			has_primary_monitor = true;
	}
	if (!has_primary_monitor) {
		for (i=0;i<numberOfScreens;i++) {
			screendata = screenInfoArray.at(i);
			if (!has_primary_monitor) {
				if (screendata->screen_connected && screendata->is_extended) {
					screendata->is_primary = true;
					screendata->is_extended = true;
					has_primary_monitor = true;
				}
			}
		}
	}
	if (!has_primary_monitor) {
		for (i=0;i<numberOfScreens;i++) {
			screendata = screenInfoArray.at(i);
			if (!has_primary_monitor) {
				if (screendata->screen_connected) {
					screendata->is_primary = true;
					screendata->is_extended = true;
					has_primary_monitor = true;
				}
			}
		}
	}

	for (i=0;i<numberOfScreens;i++) {
		screendata = screenInfoArray.at(i);
		if (screendata->is_primary) {
			screendata->is_extended = true;
		}
	}

	for (i=0;i<numberOfScreens;i++) {
		screendata = screenInfoArray.at(i);
		TQString resolutionstring = screendata->resolutions[screendata->current_resolution_index];
		int separator_pos = resolutionstring.find(" x ");
		TQString x_res_string = resolutionstring.left(separator_pos);
		TQString y_res_string = resolutionstring.right(resolutionstring.length()-separator_pos-3);
		screendata->current_x_pixel_count = x_res_string.toInt();
		screendata->current_y_pixel_count = y_res_string.toInt();
		screendata->current_orientation_mask = getHardwareRotationFlags(screendata);
	}

	// Each screen's absolute position is given relative to the primary monitor
	// Fix up the absolute positions
	int primary_offset_x = 0;
	int primary_offset_y = 0;
	for (i=0;i<numberOfScreens;i++) {
		screendata = screenInfoArray.at(i);
		if (screendata->is_primary) {
			primary_offset_x = screendata->absolute_x_position;
			primary_offset_y = screendata->absolute_y_position;
			primary_offset_x = primary_offset_x * (-1);
			primary_offset_y = primary_offset_y * (-1);
		}
	}
	for (i=0;i<numberOfScreens;i++) {
		screendata = screenInfoArray.at(i);
		screendata->absolute_x_position = screendata->absolute_x_position + primary_offset_x;
		screendata->absolute_y_position = screendata->absolute_y_position + primary_offset_y;
	}
}

TQPoint KRandrSimpleAPI::primaryScreenOffsetFromTLC(TQPtrList<SingleScreenData> screenInfoArray) {
	int i;
	SingleScreenData *screendata;
	int numberOfScreens = screenInfoArray.count();

	int primary_offset_x = 0;
	int primary_offset_y = 0;
	for (i=0;i<numberOfScreens;i++) {
		screendata = screenInfoArray.at(i);
		if (screendata->absolute_x_position < primary_offset_x) {
			primary_offset_x = screendata->absolute_x_position;
		}
		if (screendata->absolute_y_position < primary_offset_y) {
			primary_offset_y = screendata->absolute_y_position;
		}
	}
	primary_offset_x = primary_offset_x * (-1);
	primary_offset_y = primary_offset_y * (-1);

	return TQPoint(primary_offset_x, primary_offset_y);
}

void KRandrSimpleAPI::applySystemwideDisplayGamma(TQPtrList<SingleScreenData> screenInfoArray) {
	int i;
	Display *randr_display;
	XRROutputInfo *output_info;
	ScreenInfo *randr_screen_info;
	XRRCrtcGamma *gamma;

	SingleScreenData *screendata;

	if (isValid() == true) {
		randr_display = qt_xdisplay();
		randr_screen_info = read_screen_info(randr_display);
		for (i = 0; i < randr_screen_info->n_output; i++) {
			screendata = screenInfoArray.at(i);
			output_info = randr_screen_info->outputs[i]->info;
			CrtcInfo *current_crtc = randr_screen_info->outputs[i]->cur_crtc;
			// vvvvvvvvv This chunk of code is borrowed from xrandr vvvvvvvvvv
			int size = XRRGetCrtcGammaSize(randr_display, current_crtc->id);
			if (!size) {
				continue;
			}
			gamma = XRRAllocGamma(size);
			if (!gamma) {
				continue;
			}
			for (i = 0; i < size; i++) {
				if (screendata->gamma_red == 1.0)
					gamma->red[i] = i << 8;
				else
					gamma->red[i] = (pow((double)i/(double)(size-1), (double)screendata->gamma_red) * (double)(size-1)*256);

				if (screendata->gamma_green == 1.0)
					gamma->green[i] = i << 8;
				else
					gamma->green[i] = (pow((double)i/(double)(size-1), (double)screendata->gamma_green) * (double)(size-1)*256);

				if (screendata->gamma_blue == 1.0)
					gamma->blue[i] = i << 8;
				else
					gamma->blue[i] = (pow((double)i/(double)(size-1), (double)screendata->gamma_blue) * (double)(size-1)*256);
			}
			XRRSetCrtcGamma(randr_display, current_crtc->id, gamma);
			free(gamma);
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
		}
		freeScreenInfoStructure(randr_screen_info);
	}
}

void KRandrSimpleAPI::freeScreenInfoStructure(ScreenInfo* screen_info) {
	int i;

	for (i=0; i<screen_info->n_crtc; i++) {
		free(screen_info->crtcs[i]);
	}
	for (i=0; i<screen_info->n_output; i++) {
		free(screen_info->outputs[i]);
	}
	free(screen_info->outputs);
	free(screen_info->crtcs);
	free(screen_info);
}

TQPtrList<SingleScreenData> KRandrSimpleAPI::readCurrentDisplayConfiguration() {
	// Discover display information
	int i;
	int j;

	XRROutputInfo *output_info;
	SingleScreenData *screendata;
	TQPtrList<SingleScreenData> screenInfoArray;

	Display *randr_display;
	ScreenInfo *randr_screen_info;

	// Clear existing info
	destroyScreenInformationObject(screenInfoArray);

	int numberOfScreens = 0;
	if (isValid() == true) {
		randr_display = qt_xdisplay();
		randr_screen_info = read_screen_info(randr_display);
		for (i = 0; i < randr_screen_info->n_output; i++) {
			output_info = randr_screen_info->outputs[i]->info;
			CrtcInfo *current_crtc = randr_screen_info->outputs[i]->cur_crtc;

			// Create new data object
			screendata = new SingleScreenData;
			screenInfoArray.append(screendata);
			screendata->screenFriendlyName = TQString(i18n("%1. %2 output on %3")).arg(i+1).arg(capitalizeString(output_info->name)).arg(":0");	// [FIXME] How can I get the name of the Xorg graphics driver currently in use?
			screendata->generic_screen_detected = false;

			// Attempt to use KMS to find screen EDID and name
			TQString edid = getEDIDMonitorName(0, output_info->name);	// [FIXME] Don't hardwire to card 0!
			if (!edid.isNull()) {
				screendata->screenFriendlyName = TQString(i18n("%1. %2 on %3 on card %4")).arg(i+1).arg(edid).arg(capitalizeString(output_info->name)).arg("0");	// [FIXME] How can I get the name of the Xorg graphics driver currently in use?
			}

			// Get resolutions
			bool screen_active;
			RandRScreen *cur_screen = 0;
			if (RR_Disconnected == randr_screen_info->outputs[i]->info->connection) {
				// Output DISCONNECTED
				screen_active = false;
			}
			else {
				if (randr_screen_info->outputs[i]->cur_crtc) {
					// Output CONNECTED and ON
					screen_active = true;
					cur_screen = new RandRScreen(i);
				}
				else {
					// Output CONNECTED and OFF
					screen_active = false;
					cur_screen = new RandRScreen(i);
				}
			}

			if (cur_screen) {
				screendata->screen_connected = true;
				for (int j = 0; j < cur_screen->numSizes(); j++) {
					screendata->resolutions.append(i18n("%1 x %2").tqarg(cur_screen->pixelSize(j).width()).tqarg(cur_screen->pixelSize(j).height()));
				}
				screendata->current_resolution_index = 0;
				if (current_crtc) {
					screendata->current_resolution_index = screendata->resolutions.findIndex(i18n("%1 x %2").tqarg(current_crtc->info->width).tqarg(current_crtc->info->height));
				}
				if (screendata->current_resolution_index < 0) {
					screendata->current_resolution_index = cur_screen->proposedSize();
				}

				// Get refresh rates
				TQStringList rr = cur_screen->refreshRates(screendata->current_resolution_index);
				for (TQStringList::Iterator it = rr.begin(); it != rr.end(); ++it) {
					screendata->refresh_rates.append(*it);
				}
				screendata->current_refresh_rate_index = cur_screen->proposedRefreshRate();

				// Get color depths
				// [FIXME]
				screendata->color_depths.append(i18n("Default"));
				screendata->current_color_depth_index = 0;

				// Get orientation flags
				// RandRScreen::Rotate0
				// RandRScreen::Rotate90
				// RandRScreen::Rotate180
				// RandRScreen::Rotate270
				// RandRScreen::ReflectX
				// RandRScreen::ReflectY

				screendata->rotations.append(i18n("Normal"));
				screendata->rotations.append(i18n("Rotate 90 degrees"));
				screendata->rotations.append(i18n("Rotate 180 degrees"));
				screendata->rotations.append(i18n("Rotate 270 degrees"));
				screendata->current_orientation_mask = cur_screen->proposedRotation();
				switch (screendata->current_orientation_mask & RandRScreen::RotateMask) {
					case RandRScreen::Rotate0:
						screendata->current_rotation_index = 0;
						break;
					case RandRScreen::Rotate90:
						screendata->current_rotation_index = 1;
						break;
					case RandRScreen::Rotate180:
						screendata->current_rotation_index = 2;
						break;
					case RandRScreen::Rotate270:
						screendata->current_rotation_index = 3;
						break;
					default:
						// Shouldn't hit this one
						Q_ASSERT(screendata->current_orientation_mask & RandRScreen::RotateMask);
						break;
				}
				screendata->has_x_flip = (screendata->current_orientation_mask & RandRScreen::ReflectX);
				screendata->has_y_flip = (screendata->current_orientation_mask & RandRScreen::ReflectY);
				screendata->supports_transformations = (cur_screen->rotations() != RandRScreen::Rotate0);

				// Determine if this display is primary and/or extended
				RROutput primaryoutput = XRRGetOutputPrimary(qt_xdisplay(), DefaultRootWindow(qt_xdisplay()));
				if (primaryoutput == randr_screen_info->outputs[i]->id)
					screendata->is_primary = false;
				else
					screendata->is_primary = true;
				screendata->is_extended = screen_active;

				// Get this screen's absolute position
				screendata->absolute_x_position = 0;
				screendata->absolute_y_position = 0;
				if (current_crtc) {
					screendata->absolute_x_position = current_crtc->info->x;
					screendata->absolute_y_position = current_crtc->info->y;
				}

				// Get this screen's current resolution
				screendata->current_x_pixel_count = cur_screen->pixelSize(screendata->current_resolution_index).width();
				screendata->current_y_pixel_count = cur_screen->pixelSize(screendata->current_resolution_index).height();

				// Get this screen's current gamma values
				// [FIXME]
				// This attempts to guess a gamma value based on the LUT settings at 50%
				// It may not always be 100% correct, or even anywhere close...
				// Essentially it "undoes" the LUT gamma calculation from xrandr
				// lut_gamma->green[i] = (pow(i/(size - 1), desired_gamma.green) * (size - 1) * 256);
				if (current_crtc) {
					//int slot = 127;
					int slot = 7;
					int size = XRRGetCrtcGammaSize(randr_display, current_crtc->id);
					XRRCrtcGamma *gammastruct = XRRGetCrtcGamma (randr_display, current_crtc->id);
					screendata->gamma_red = log(gammastruct->red[slot]/((size-1.0)*256.0))/log(slot/(size-1.0));
					screendata->gamma_green = log(gammastruct->green[slot]/((size-1.0)*256.0))/log(slot/(size-1.0));
					screendata->gamma_blue = log(gammastruct->blue[slot]/((size-1.0)*256.0))/log(slot/(size-1.0));
				}
				else {
					screendata->gamma_red = 2.2;
					screendata->gamma_green = 2.2;
					screendata->gamma_blue = 2.2;
				}
				// Round off the gamma to one decimal place
				screendata->gamma_red = floorf(screendata->gamma_red * 10 + 0.5) / 10;
				screendata->gamma_green = floorf(screendata->gamma_green * 10 + 0.5) / 10;
				screendata->gamma_blue = floorf(screendata->gamma_blue * 10 + 0.5) / 10;

				delete cur_screen;
			}
			else {
				// Fill in generic data for this disconnected output
				screendata->screenFriendlyName = screendata->screenFriendlyName + TQString(" (") + i18n("disconnected") + TQString(")");
				screendata->screen_connected = false;

				screendata->resolutions = i18n("Default");
				screendata->refresh_rates = i18n("Default");
				screendata->color_depths = i18n("Default");
				screendata->rotations = i18n("N/A");

				screendata->current_resolution_index = 0;
				screendata->current_refresh_rate_index = 0;
				screendata->current_color_depth_index = 0;

				screendata->gamma_red = 2.2;
				screendata->gamma_green = 2.2;
				screendata->gamma_blue = 2.2;

				screendata->current_rotation_index = 0;
				screendata->current_orientation_mask = 0;
				screendata->has_x_flip = false;
				screendata->has_y_flip = false;
				screendata->supports_transformations = false;

				screendata->is_primary = false;
				screendata->is_extended = false;
				screendata->absolute_x_position = 0;
				screendata->absolute_y_position = 0;
				screendata->current_x_pixel_count = 640;
				screendata->current_y_pixel_count = 480;
			}

			// Check for more screens...
			numberOfScreens++;
		}

		freeScreenInfoStructure(randr_screen_info);
	}
	else {
		screendata = new SingleScreenData;
		screenInfoArray.append(screendata);

		// Fill in a bunch of generic data
		screendata->screenFriendlyName = i18n("Default output on generic video card");
		screendata->generic_screen_detected = true;
		screendata->screen_connected = true;

		screendata->resolutions = i18n("Default");
		screendata->refresh_rates = i18n("Default");
		screendata->color_depths = i18n("Default");
		screendata->rotations = i18n("N/A");

		screendata->current_resolution_index = 0;
		screendata->current_refresh_rate_index = 0;
		screendata->current_color_depth_index = 0;

		screendata->gamma_red = 2.2;
		screendata->gamma_green = 2.2;
		screendata->gamma_blue = 2.2;

		screendata->current_rotation_index = 0;
		screendata->current_orientation_mask = 0;
		screendata->has_x_flip = false;
		screendata->has_y_flip = false;
		screendata->supports_transformations = false;

		screendata->is_primary = true;
		screendata->is_extended = true;
		screendata->absolute_x_position = 0;
		screendata->absolute_y_position = 0;
		screendata->current_x_pixel_count = 640;
		screendata->current_y_pixel_count = 480;

		numberOfScreens++;
	}

	// [FIXME]
	// Set this on the real primary monitor only!
	screendata = screenInfoArray.at(0);
	screendata->is_primary = true;

	return screenInfoArray;
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

