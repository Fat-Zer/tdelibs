/*
 * Copyright (c) 2002,2003 Hamish Rodda <rodda@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "randr.h"
#include "lowlevel_randr.h"

#include <tqtimer.h>

#include <kdebug.h>
#include <klocale.h>
#include <kglobal.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <dcopclient.h>
#include <kipc.h>
#include <kactivelabel.h>

#include "ktimerdialog.h"

#include <X11/Xlib.h>
#define INT8 _X11INT8
#define INT32 _X11INT32
#include <X11/Xproto.h>
#undef INT8
#undef INT32
#include <X11/extensions/Xrandr.h>

class RandRScreenPrivate
{
public:
	RandRScreenPrivate() : config(0L) {};
	~RandRScreenPrivate()
	{
		if (config)
			XRRFreeScreenConfigInfo(config);
	}

	XRRScreenConfiguration* config;
};

KDE_EXPORT RandRScreen::RandRScreen(int screenIndex)
	: d(new RandRScreenPrivate())
	, m_screen(screenIndex)
	, m_shownDialog(NULL)
{
	loadSettings();
	setOriginal();
}

KDE_EXPORT RandRScreen::~RandRScreen()
{
	delete d;
}

KDE_EXPORT void RandRScreen::loadSettings()
{
	if (d->config)
		XRRFreeScreenConfigInfo(d->config);

	d->config = XRRGetScreenInfo(qt_xdisplay(), RootWindow(qt_xdisplay(), m_screen));

	Rotation rotation;
	if (d->config) {
		m_currentSize = m_proposedSize = XRRConfigCurrentConfiguration(d->config, &rotation);
		m_currentRotation = m_proposedRotation = rotation;
	}
	else {
		m_currentSize = m_proposedSize = 0;
		m_currentRotation = m_proposedRotation = 0;
	}

	m_pixelSizes.clear();
	m_mmSizes.clear();

	if (d->config) {
		int numSizes;
		XRRScreenSize* sizes = XRRSizes(qt_xdisplay(), m_screen, &numSizes);
		for (int i = 0; i < numSizes; i++) {
			m_pixelSizes.append(TQSize(sizes[i].width, sizes[i].height));
			m_mmSizes.append(TQSize(sizes[i].mwidth, sizes[i].mheight));
		}

		m_rotations = XRRRotations(qt_xdisplay(), m_screen, &rotation);
	}
	else {
		// Great, now we have to go after the information manually.  Ughh.
		ScreenInfo *screeninfo = internal_read_screen_info(qt_xdisplay());
		XRROutputInfo *output_info = screeninfo->outputs[m_screen]->info;
		CrtcInfo *current_crtc = screeninfo->outputs[m_screen]->cur_crtc;
		int numSizes = screeninfo->res->nmode;
		for (int i = 0; i < numSizes; i++) {
			TQSize newSize = TQSize(screeninfo->res->modes[i].width, screeninfo->res->modes[i].height);
			if (!m_pixelSizes.contains(newSize)) {
				m_pixelSizes.append(newSize);
				m_mmSizes.append(TQSize(output_info->mm_width, output_info->mm_height));
			}
		}
		if (current_crtc) {
			m_rotations = current_crtc->rotations;
			m_currentRotation = m_proposedRotation = current_crtc->cur_rotation;
		}
	}

	if (d->config) {
		m_currentRefreshRate = m_proposedRefreshRate = refreshRateHzToIndex(m_currentSize, XRRConfigCurrentRate(d->config));
	}
	else {
		m_currentRefreshRate = m_proposedRefreshRate = 0;
	}
}

KDE_EXPORT void RandRScreen::setOriginal()
{
	m_originalSize = m_currentSize;
	m_originalRotation = m_currentRotation;
	m_originalRefreshRate = m_currentRefreshRate;
}

KDE_EXPORT bool RandRScreen::applyProposed()
{
	//kdDebug() << k_funcinfo << " size " << (SizeID)proposedSize() << ", rotation " << proposedRotation() << ", refresh " << refreshRateIndexToHz(proposedSize(), proposedRefreshRate()) << endl;

	Status status;

	if (!d->config) {
		d->config = XRRGetScreenInfo(qt_xdisplay(), RootWindow(qt_xdisplay(), m_screen));
		Q_ASSERT(d->config);
	}

	if (d->config) {
		if (proposedRefreshRate() < 0)
			status = XRRSetScreenConfig(qt_xdisplay(), d->config, DefaultRootWindow(qt_xdisplay()), (SizeID)proposedSize(), (Rotation)proposedRotation(), CurrentTime);
		else {
			if( refreshRateIndexToHz(proposedSize(), proposedRefreshRate()) <= 0 ) {
				m_proposedRefreshRate = 0;
			}
			status = XRRSetScreenConfigAndRate(qt_xdisplay(), d->config, DefaultRootWindow(qt_xdisplay()), (SizeID)proposedSize(), (Rotation)proposedRotation(), refreshRateIndexToHz(proposedSize(), proposedRefreshRate()), CurrentTime);
		}
	}
	else {
		// Great, now we have to set the information manually.  Ughh.
		// FIXME--this does not work!
		ScreenInfo *screeninfo = internal_read_screen_info(qt_xdisplay());
		screeninfo->cur_width = (*m_pixelSizes.at(proposedSize())).width();
		screeninfo->cur_height = (*m_pixelSizes.at(proposedSize())).height();
		internal_main_low_apply(screeninfo);

		status = RRSetConfigSuccess;
	}

	//kdDebug() << "New size: " << WidthOfScreen(ScreenOfDisplay(TQPaintDevice::x11AppDisplay(), screen)) << ", " << HeightOfScreen(ScreenOfDisplay(TQPaintDevice::x11AppDisplay(), screen)) << endl;

	if (status == RRSetConfigSuccess) {
		m_currentSize = m_proposedSize;
		m_currentRotation = m_proposedRotation;
		m_currentRefreshRate = m_proposedRefreshRate;
		return true;
	}

	return false;
}

KDE_EXPORT bool RandRScreen::applyProposedAndConfirm()
{
	if (proposedChanged()) {
		setOriginal();

		if (applyProposed()) {
			if (!confirm()) {
				proposeOriginal();
				applyProposed();
				return false;
			}
		} else {
			return false;
		}
	}

	return true;
}

KDE_EXPORT bool RandRScreen::confirm()
{
	// uncomment the line below and edit out the KTimerDialog stuff to get
	// a version which works on today's tdelibs (no accept dialog is presented)

	// FIXME remember to put the dialog on the right screen

	KTimerDialog acceptDialog ( 15000, KTimerDialog::CountDown,
                KApplication::kApplication()->mainWidget(),
											"mainKTimerDialog",
											true,
											i18n("Confirm Display Setting Change"),
											KTimerDialog::Ok|KTimerDialog::Cancel,
											KTimerDialog::Cancel);

	acceptDialog.setButtonOK(KGuiItem(i18n("&Accept Configuration"), "button_ok"));
	acceptDialog.setButtonCancel(KGuiItem(i18n("&Return to Previous Configuration"), "button_cancel"));

	KActiveLabel *label = new KActiveLabel(i18n("Your screen orientation, size and refresh rate "
                    "have been changed to the requested settings. Please indicate whether you wish to "
                    "keep this configuration. In 15 seconds the display will revert to your previous "
                    "settings."), &acceptDialog, "userSpecifiedLabel");

	acceptDialog.setMainWidget(label);

	KDialog::centerOnScreen(&acceptDialog, m_screen);

	m_shownDialog = &acceptDialog;
	connect( m_shownDialog, TQT_SIGNAL( destroyed()), this, TQT_SLOT( shownDialogDestroyed()));
	connect( kapp->desktop(), TQT_SIGNAL( resized(int)), this, TQT_SLOT( desktopResized()));

        return acceptDialog.exec();
}

KDE_EXPORT void RandRScreen::shownDialogDestroyed()
{
    m_shownDialog = NULL;
    disconnect( kapp->desktop(), TQT_SIGNAL( resized(int)), this, TQT_SLOT( desktopResized()));
}

KDE_EXPORT void RandRScreen::desktopResized()
{
	if( m_shownDialog != NULL )
		KDialog::centerOnScreen(m_shownDialog, m_screen);
}

KDE_EXPORT TQString RandRScreen::changedMessage() const
{
	if (currentRefreshRate() == -1)
		return i18n("New configuration:\nResolution: %1 x %2\nOrientation: %3")
			.arg(currentPixelWidth())
			.arg(currentPixelHeight())
			.arg(currentRotationDescription());
	else
		return i18n("New configuration:\nResolution: %1 x %2\nOrientation: %3\nRefresh rate: %4")
			.arg(currentPixelWidth())
			.arg(currentPixelHeight())
			.arg(currentRotationDescription())
			.arg(currentRefreshRateDescription());
}

KDE_EXPORT bool RandRScreen::changedFromOriginal() const
{
	return m_currentSize != m_originalSize || m_currentRotation != m_originalRotation || m_currentRefreshRate != m_originalRefreshRate;
}

KDE_EXPORT void RandRScreen::proposeOriginal()
{
	m_proposedSize = m_originalSize;
	m_proposedRotation = m_originalRotation;
	m_proposedRefreshRate = m_originalRefreshRate;
}

KDE_EXPORT bool RandRScreen::proposedChanged() const
{
	return m_currentSize != m_proposedSize || m_currentRotation != m_proposedRotation || m_currentRefreshRate != m_proposedRefreshRate;
}

KDE_EXPORT TQString RandRScreen::rotationName(int rotation, bool pastTense, bool capitalised)
{
	if (!pastTense)
		switch (rotation) {
			case RR_Rotate_0:
				return i18n("Normal");
			case RR_Rotate_90:
				return i18n("Left (90 degrees)");
			case RR_Rotate_180:
				return i18n("Upside-down (180 degrees)");
			case RR_Rotate_270:
				return i18n("Right (270 degrees)");
			case RR_Reflect_X:
				return i18n("Mirror horizontally");
			case RR_Reflect_Y:
				return i18n("Mirror vertically");
			default:
				return i18n("Unknown orientation");
		}

	switch (rotation) {
		case RR_Rotate_0:
			return i18n("Normal");
		case RR_Rotate_90:
			return i18n("Rotated 90 degrees counterclockwise");
		case RR_Rotate_180:
			return i18n("Rotated 180 degrees counterclockwise");
		case RR_Rotate_270:
			return i18n("Rotated 270 degrees counterclockwise");
		default:
			if (rotation & RR_Reflect_X)
				if (rotation & RR_Reflect_Y)
					if (capitalised)
						return i18n("Mirrored horizontally and vertically");
					else
						return i18n("mirrored horizontally and vertically");
				else
					if (capitalised)
						return i18n("Mirrored horizontally");
					else
						return i18n("mirrored horizontally");
			else if (rotation & RR_Reflect_Y)
				if (capitalised)
					return i18n("Mirrored vertically");
				else
					return i18n("mirrored vertically");
			else
				if (capitalised)
					return i18n("Unknown orientation");
				else
					return i18n("unknown orientation");
	}
}

KDE_EXPORT TQPixmap RandRScreen::rotationIcon(int rotation) const
{
	// Adjust icons for current screen orientation
	if (!(m_currentRotation & RR_Rotate_0) && rotation & (RR_Rotate_0 | RR_Rotate_90 | RR_Rotate_180 | RR_Rotate_270)) {
		int currentAngle = m_currentRotation & (RR_Rotate_90 | RR_Rotate_180 | RR_Rotate_270);
		switch (currentAngle) {
			case RR_Rotate_90:
				rotation <<= 3;
				break;
			case RR_Rotate_180:
				rotation <<= 2;
				break;
			case RR_Rotate_270:
				rotation <<= 1;
				break;
		}

		// Fix overflow
		if (rotation > RR_Rotate_270) {
			rotation >>= 4;
		}
	}

	switch (rotation) {
		case RR_Rotate_0:
			return SmallIcon("up");
		case RR_Rotate_90:
			return SmallIcon("back");
		case RR_Rotate_180:
			return SmallIcon("down");
		case RR_Rotate_270:
			return SmallIcon("forward");
		case RR_Reflect_X:
		case RR_Reflect_Y:
		default:
			return SmallIcon("stop");
	}
}

KDE_EXPORT TQString RandRScreen::currentRotationDescription() const
{
	TQString ret = rotationName(m_currentRotation & RotateMask);

	if (m_currentRotation != (m_currentRotation & RotateMask)) {
		if (m_currentRotation & RR_Rotate_0) {
			ret = rotationName(m_currentRotation & (RR_Reflect_X + RR_Reflect_X), true, true);
		}
		else {
			ret += ", " + rotationName(m_currentRotation & (RR_Reflect_X + RR_Reflect_X), true, false);
		}
	}

	return ret;
}

KDE_EXPORT int RandRScreen::rotationIndexToDegree(int rotation) const
{
	switch (rotation & RotateMask) {
		case RR_Rotate_90:
			return 90;

		case RR_Rotate_180:
			return 180;

		case RR_Rotate_270:
			return 270;

		default:
			return 0;
	}
}

KDE_EXPORT int RandRScreen::rotationDegreeToIndex(int degree) const
{
	switch (degree) {
		case 90:
			return RR_Rotate_90;

		case 180:
			return RR_Rotate_180;

		case 270:
			return RR_Rotate_270;

		default:
			return RR_Rotate_0;
	}
}

KDE_EXPORT int RandRScreen::currentPixelWidth() const
{
	return m_pixelSizes[m_currentSize].width();
}

KDE_EXPORT int RandRScreen::currentPixelHeight() const
{
	return m_pixelSizes[m_currentSize].height();
}

KDE_EXPORT int RandRScreen::currentMMWidth() const
{
	return m_pixelSizes[m_currentSize].width();
}

KDE_EXPORT int RandRScreen::currentMMHeight() const
{
	return m_pixelSizes[m_currentSize].height();
}

KDE_EXPORT TQStringList RandRScreen::refreshRates(int size) const
{
	int nrates;
	TQStringList ret;

	if (d->config) {
		short* rates = XRRRates(qt_xdisplay(), m_screen, (SizeID)size, &nrates);

		for (int i = 0; i < nrates; i++)
			ret << refreshRateDirectDescription(rates[i]);
	}
	else {
		// Great, now we have to go after the information manually.  Ughh.
		ScreenInfo *screeninfo = internal_read_screen_info(qt_xdisplay());
		int numSizes = screeninfo->res->nmode;
		for (int i = 0; i < numSizes; i++) {
			int refresh_rate = ((screeninfo->res->modes[i].dotClock*1.0)/((screeninfo->res->modes[i].hTotal)*(screeninfo->res->modes[i].vTotal)*1.0));
			TQString newRate = refreshRateDirectDescription(refresh_rate);
			if (!ret.contains(newRate)) {
				ret.append(newRate);
			}
		}
	}

	return ret;
}

KDE_EXPORT TQString RandRScreen::refreshRateDirectDescription(int rate) const
{
	return i18n("Refresh rate in Hertz (Hz)", "%1 Hz").arg(rate);
}

KDE_EXPORT TQString RandRScreen::refreshRateIndirectDescription(int size, int index) const
{
	return i18n("Refresh rate in Hertz (Hz)", "%1 Hz").arg(refreshRateIndexToHz(size, index));
}

KDE_EXPORT TQString RandRScreen::refreshRateDescription(int size, int index) const
{
	return refreshRates(size)[index];
}

KDE_EXPORT bool RandRScreen::proposeRefreshRate(int index)
{
	if (index >= 0 && (int)refreshRates(proposedSize()).count() > index) {
		m_proposedRefreshRate = index;
		return true;
	}

	return false;
}

KDE_EXPORT int RandRScreen::currentRefreshRate() const
{
	return m_currentRefreshRate;
}

KDE_EXPORT TQString RandRScreen::currentRefreshRateDescription() const
{
	return refreshRateIndirectDescription(m_currentSize, m_currentRefreshRate);
}

KDE_EXPORT int RandRScreen::proposedRefreshRate() const
{
	return m_proposedRefreshRate;
}

KDE_EXPORT int RandRScreen::refreshRateHzToIndex(int size, int hz) const
{
	int nrates;
	short* rates = XRRRates(qt_xdisplay(), m_screen, (SizeID)size, &nrates);

	for (int i = 0; i < nrates; i++)
		if (hz == rates[i])
			return i;

	if (nrates != 0)
		// Wrong input Hz!
		Q_ASSERT(false);

	return -1;
}

KDE_EXPORT int RandRScreen::refreshRateIndexToHz(int size, int index) const
{
	int nrates;
	short* rates = XRRRates(qt_xdisplay(), m_screen, (SizeID)size, &nrates);

	if (nrates == 0 || index < 0)
		return 0;

	// Wrong input Hz!
	if(index >= nrates)
		return 0;

	return rates[index];
}

KDE_EXPORT int RandRScreen::numSizes() const
{
	return m_pixelSizes.count();
}

KDE_EXPORT const TQSize& RandRScreen::pixelSize(int index) const
{
	return m_pixelSizes[index];
}

KDE_EXPORT const TQSize& RandRScreen::mmSize(int index) const
{
	return m_mmSizes[index];
}

KDE_EXPORT int RandRScreen::sizeIndex(TQSize pixelSize) const
{
	for (uint i = 0; i < m_pixelSizes.count(); i++)
		if (m_pixelSizes[i] == pixelSize)
			return i;

	return -1;
}

KDE_EXPORT int RandRScreen::rotations() const
{
	return m_rotations;
}

KDE_EXPORT int RandRScreen::currentRotation() const
{
	return m_currentRotation;
}

KDE_EXPORT int RandRScreen::currentSize() const
{
	return m_currentSize;
}

KDE_EXPORT int RandRScreen::proposedRotation() const
{
	return m_proposedRotation;
}

KDE_EXPORT void RandRScreen::proposeRotation(int newRotation)
{
	m_proposedRotation = newRotation & OrientationMask;
}

KDE_EXPORT int RandRScreen::proposedSize() const
{
	return m_proposedSize;
}

KDE_EXPORT bool RandRScreen::proposeSize(int newSize)
{
	if ((int)m_pixelSizes.count() > newSize) {
		m_proposedSize = newSize;
		return true;
	}

	return false;
}

KDE_EXPORT void RandRScreen::load(KConfig& config)
{
	config.setGroup(TQString("Screen%1").arg(m_screen));

	if (proposeSize(sizeIndex(TQSize(config.readNumEntry("width", currentPixelWidth()), config.readNumEntry("height", currentPixelHeight())))))
		proposeRefreshRate(refreshRateHzToIndex(proposedSize(), config.readNumEntry("refresh", currentRefreshRate())));

	proposeRotation(rotationDegreeToIndex(config.readNumEntry("rotation", 0)) + (config.readBoolEntry("reflectX") ? ReflectX : 0) + (config.readBoolEntry("reflectY") ? ReflectY : 0));
}

KDE_EXPORT void RandRScreen::save(KConfig& config) const
{
	config.setGroup(TQString("Screen%1").arg(m_screen));
	config.writeEntry("width", currentPixelWidth());
	config.writeEntry("height", currentPixelHeight());
	config.writeEntry("refresh", refreshRateIndexToHz(currentSize(), currentRefreshRate()));
	config.writeEntry("rotation", rotationIndexToDegree(currentRotation()));
	config.writeEntry("reflectX", (bool)(currentRotation() & ReflectMask) == ReflectX);
	config.writeEntry("reflectY", (bool)(currentRotation() & ReflectMask) == ReflectY);
}

KDE_EXPORT RandRDisplay::RandRDisplay()
	: m_valid(true)
{
	// Check extension
	Status s = XRRQueryExtension(qt_xdisplay(), &m_eventBase, &m_errorBase);
	if (!s) {
		m_errorCode = TQString("%1, base %1").arg(s).arg(m_errorBase);
		m_valid = false;
		return;
	}

	// Sometimes the extension is available but does not return any screens (!)
	// Check for that case
	Display *randr_display = XOpenDisplay(NULL);
	int screen_num;
	Window root_window;

	screen_num = DefaultScreen (randr_display);
	root_window = RootWindow (randr_display, screen_num);
	if (XRRGetScreenResources (randr_display, root_window) == NULL) {
		m_errorCode = i18n("No screens detected");
		m_valid = false;
		return;
	}

	int major_version, minor_version;
	XRRQueryVersion(qt_xdisplay(), &major_version, &minor_version);

	m_version = TQString("X Resize and Rotate extension version %1.%1").arg(major_version).arg(minor_version);

	m_numScreens = ScreenCount(qt_xdisplay());

	// This assumption is WRONG with Xinerama
	// Q_ASSERT(TQApplication::desktop()->numScreens() == ScreenCount(qt_xdisplay()));

	m_screens.setAutoDelete(true);
	for (int i = 0; i < m_numScreens; i++) {
		m_screens.append(new RandRScreen(i));
	}

	setCurrentScreen(TQApplication::desktop()->primaryScreen());
}

KDE_EXPORT bool RandRDisplay::isValid() const
{
	return m_valid;
}

KDE_EXPORT const TQString& RandRDisplay::errorCode() const
{
	return m_errorCode;
}

KDE_EXPORT int RandRDisplay::eventBase() const
{
	return m_eventBase;
}

KDE_EXPORT int RandRDisplay::screenChangeNotifyEvent() const
{
	return m_eventBase + RRScreenChangeNotify;
}

KDE_EXPORT int RandRDisplay::errorBase() const
{
	return m_errorBase;
}

KDE_EXPORT const TQString& RandRDisplay::version() const
{
	return m_version;
}

KDE_EXPORT void RandRDisplay::setCurrentScreen(int index)
{
	m_currentScreenIndex = index;
	m_currentScreen = m_screens.at(m_currentScreenIndex);
	Q_ASSERT(m_currentScreen);
}

KDE_EXPORT int RandRDisplay::screenIndexOfWidget(TQWidget* widget)
{
	int ret = TQApplication::desktop()->screenNumber(widget);
	return ret != -1 ? ret : TQApplication::desktop()->primaryScreen();
}

KDE_EXPORT int RandRDisplay::currentScreenIndex() const
{
	return m_currentScreenIndex;
}

KDE_EXPORT void RandRDisplay::refresh()
{
	for (RandRScreen* s = m_screens.first(); s; s = m_screens.next())
		s->loadSettings();
}

KDE_EXPORT int RandRDisplay::numScreens() const
{
	return m_numScreens;
}

KDE_EXPORT RandRScreen* RandRDisplay::screen(int index)
{
	return m_screens.at(index);
}

KDE_EXPORT RandRScreen* RandRDisplay::currentScreen()
{
	return m_currentScreen;
}

KDE_EXPORT bool RandRDisplay::loadDisplay(KConfig& config, bool loadScreens)
{
	if (loadScreens)
		for (RandRScreen* s = m_screens.first(); s; s = m_screens.next())
			s->load(config);

	return applyOnStartup(config);
}

KDE_EXPORT bool RandRDisplay::applyOnStartup(KConfig& config)
{
	config.setGroup("Display");
	return config.readBoolEntry("ApplyOnStartup", false);
}

KDE_EXPORT bool RandRDisplay::syncTrayApp(KConfig& config)
{
	config.setGroup("Display");
	return config.readBoolEntry("SyncTrayApp", false);
}

KDE_EXPORT void RandRDisplay::saveDisplay(KConfig& config, bool applyOnStartup, bool syncTrayApp)
{
	Q_ASSERT(!config.isReadOnly());

	config.setGroup("Display");
	config.writeEntry("ApplyOnStartup", applyOnStartup);
	config.writeEntry("SyncTrayApp", syncTrayApp);

	for (RandRScreen* s = m_screens.first(); s; s = m_screens.next())
		s->save(config);
}

KDE_EXPORT void RandRDisplay::applyProposed(bool confirm)
{
	for (int screenIndex = 0; screenIndex < numScreens(); screenIndex++) {
		if (screen(screenIndex)->proposedChanged()) {
			if (confirm)
					screen(screenIndex)->applyProposedAndConfirm();
			else
					screen(screenIndex)->applyProposed();
		}
	}
}

KDE_EXPORT bool RandRDisplay::showTestConfigurationDialog()
{
	return screen(0)->showTestConfigurationDialog();
}

KDE_EXPORT bool RandRScreen::showTestConfigurationDialog()
{
	// uncomment the line below and edit out the KTimerDialog stuff to get
	// a version which works on today's tdelibs (no accept dialog is presented)

	// FIXME remember to put the dialog on the right screen

	KTimerDialog acceptDialog ( 15000, KTimerDialog::CountDown,
                KApplication::kApplication()->mainWidget(),
											"mainKTimerDialog",
											true,
											i18n("Confirm Display Settings"),
											KTimerDialog::Ok|KTimerDialog::Cancel,
											KTimerDialog::Cancel);

	acceptDialog.setButtonOK(KGuiItem(i18n("&Accept Configuration"), "button_ok"));
	acceptDialog.setButtonCancel(KGuiItem(i18n("&Return to Previous Configuration"), "button_cancel"));

	KActiveLabel *label = new KActiveLabel(i18n("Your display devices has been configured "
                    "to match the settings shown above. Please indicate whether you wish to "
                    "keep this configuration. In 15 seconds the display will revert to your previous "
                    "settings."), &acceptDialog, "userSpecifiedLabel");

	acceptDialog.setMainWidget(label);

	KDialog::centerOnScreen(&acceptDialog, 0);

	m_shownDialog = &acceptDialog;
	connect( m_shownDialog, TQT_SIGNAL( destroyed()), this, TQT_SLOT( shownDialogDestroyed()));
	connect( kapp->desktop(), TQT_SIGNAL( resized(int)), this, TQT_SLOT( desktopResized()));

        return acceptDialog.exec();
}

KDE_EXPORT int RandRScreen::pixelCount( int index ) const
{
	TQSize sz = pixelSize(index);
	return sz.width() * sz.height();
}

#include "randr.moc"
