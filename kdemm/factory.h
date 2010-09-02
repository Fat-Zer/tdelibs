/*  This file is part of the KDE project
    Copyright (C) 2004 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
    Boston, MA 02111-1307, USA.

*/

#ifndef KDEMM_FACTORY_H
#define KDEMM_FACTORY_H

#include <kurl.h>
#include <kdemm/channel.h>
#include <tqstring.h>
#include <tqstringlist.h>
#include <dcopobject.h>

namespace KDE
{
/**
 * \brief The KDE Multimedia classes
 *
 * In this Namespace you find a few classes to access Multimedia functions like
 * playing audio files and simple video playing. Those classes are not dependent
 * on any specific framework (like they were in pre KDE4 times) but rather use
 * exchangeable backends to do the "dirty" work.
 *
 * If you want to write a new backend you also find the necessary classes in
 * this namespace.
 *
 * \since 4.0
 */
namespace Multimedia
{
class Backend;
class Player;
class VideoPlayer;

/**
 * \brief Factory to access the preferred Backend.
 *
 * This class is your entry point to KDE Multimedia usage. It provides the
 * necessary objects for playing audio and video.
 *
 * For simple access to just playing an audio file see SimplePlayer.
 *
 * \remarks
 * Extensions to the existing functionality can either be added by using the
 * reserved virtual functions in Backend or by adding a new interface e.g.
 * BackendV2 and creating a BackendV2 instance when the Backend instance is
 * created.
 *
 * \author Matthias Kretz <kretz@kde.org>
 * \since 4.0
 */
class KDE_EXPORT Factory : public TQObject, public DCOPObject
{
	Q_OBJECT
	public:
		/**
		 * Returns a pointer to the factory.
		 * Use this function to get an instance of KLibLoader.
		 *
		 * @return a pointer to the loader. If no loader exists until now then
		 * one is created
		 */
		static Factory * self();

		/**
		 * Create a new Player.
		 *
		 * You need to call it like this:
		 * \code
		 * Factory::self()->createPlayer();
		 * \endcode
		 *
		 * @return a pointer to the Player the backend provides
		 */
		Player * createPlayer();

		/**
		 * Create a new VideoPlayer object. The Backend does not have to
		 * implement this functionality so you have to check that it didn't
		 * return 0.
		 *
		 * @return A new instance to a VideoPlayer from the Backend. Or 0 if
		 * the Backend does not support the VideoPlayer interface.
		 */
		VideoPlayer * createVideoPlayer();

		/**
		 * Play the specified sound-file with no further control. Returns
		 * immediatly.
		 *
		 * @return if the file was be succesfully issued, but is not a
		 * garantie for succes of playing the entire file.
		 */
		bool playSoundEvent(const KURL & url);

		/**
		 * Creates a new Channel object that you can use for a Player object to
		 * play to.
		 *
		 * \param title A user visible title for the Channel. Most of the time
		 * you will use the application's name.
		 *
		 * \param channeltype \ref availableChannels() returns a list of strings
		 * where you can choose one to be used. Most of the time you will use
		 * the "default" channel. But the Backend could also provide special
		 * purpose channels like one for notifications, one for previews and one
		 * for media playback.
		 *
		 * \param direction Whether it's a Channel::Input or Channel::Output
		 * that you're requesting.
		 *
		 * \returns Returns a Channel object or 0 if your request couldn't be
		 * fullfilled.
		 */
		Channel * createChannel( const TQString & title,
				const TQString & channeltype = TQString::fromLatin1( "default" ),
				Channel::Direction direction = Channel::Output );

		/**
		 * Returns the names of the channels that can be used to play or record,
		 * depending on the value of \p direction.
		 *
		 * \param direction If set to Channel::Output you will get the names of
		 * all output channels, if set to Channel::Input you get the same for
		 * the input channels.
		 */
		TQStringList availableChannels( Channel::Direction direction = Channel::Output ) const;

		/**
		 * Returns the mimetypes that can be played.
		 */
		TQStringList playableMimeTypes() const;

		/**
		 * Checks whether a certain mimetype is playable.
		 */
		bool isMimeTypePlayable( const TQString & mimetype ) const;

		/**
		 * Get the name of the Backend. It's the name from the .desktop file.
		 */
		TQString backendName() const;

		/**
		 * Get the comment of the Backend. It's the comment from the .desktop file.
		 */
		TQString backendComment() const;

		/**
		 * Get the version of the Backend. It's the version from the .desktop file.
		 *
		 * The version is especially interesting if there are several versions
		 * available for binary incompatible versions of the backend's media
		 * framework.
		 */
		TQString backendVersion() const;

		/**
		 * Get the icon (name) of the Backend. It's the icon from the .desktop file.
		 */
		TQString backendIcon() const;

		/**
		 * Get the website of the Backend. It's the website from the .desktop file.
		 */
		TQString backendWebsite() const;

	signals:
		/**
		 * This signal is emitted when the user changes the backend. You then
		 * have to free all your references to Player or Channel objects.
		 */
		void deleteYourObjects();

		/**
		 * After you got a deleteYourObjects() signal the backend is changed
		 * internally. Then you will receive this signal, and only then should
		 * you reconstruct all your objects again. This time they will
		 * internally use the new backend.
		 */
		void recreateObjects();

	protected:
		Factory();
		~Factory();

	private slots:
		void objectDestroyed( TQObject * );

	private:
		static Factory * m_self;
		class Private;
		Private * d;

	K_DCOP
	k_dcop:
		/**
		 * \internal
		 * This is called via DCOP when the user changes the KDEMM Backend.
		 */
		void kdemmBackendChanged();

};
}} // namespaces

#endif // BACKENDFACTORY_H
// vim: sw=4 ts=4 tw=80 noet
