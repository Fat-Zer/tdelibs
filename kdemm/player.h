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
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

*/

#ifndef PLAYOBJECT_H
#define PLAYOBJECT_H

#include <kdelibs_export.h>
#include <tqobject.h>

class KURL;

namespace KDE
{
namespace Multimedia
{
	class Channel;
	/**
	 * \short Interface for accessing media playback functions
	 *
	 * \author Matthias Kretz <kretz@kde.org>
	 * \since 4.0
	 */
	class Player : public QObject
	{
		Q_OBJECT
		public:
			/**
			 * The state the playobject is in at the moment
			 */
			enum State
			{
				/**
				 * The load method wasn't called or didn't succeed. If the
				 * object is in this state the next thing you have to do is to
				 * call load.
				 */
				NoMedia,
				/**
				 * After calling load it might take a while before the Player is
				 * ready to play(). Normally this doesn't happen for local
				 * files, but can happen for remote files where the asynchronous
				 * mimetype detection can take a while.
				 */
				Loading,
				/**
				 * The Player has a valid media file loaded and is ready for
				 * playing.
				 */
				Stopped,
				/**
				 * The Player is playing a media file.
				 */
				Playing,
				/**
				 * The Player is waiting for data to be able to continue
				 * playing.
				 */
				Buffering,
				/**
				 * The Player is paused currently.
				 */
				Paused
			};

			virtual ~Player();

			/**
			 * Set the output device the Player should use.
			 *
			 * @return Returns \p true on success. If it returns \p false the
			 * output device wasn't changed. This could happen if you pass a
			 * Channel with Channel::direction() == Channel::Input.
			 */
			virtual bool setOutputChannel( Channel * device ) = 0;

			/**
			 * Get the current state.
			 */
			virtual State state() const;

			/**
			 * Check whether the Player supports to control the volume.
			 *
			 * @returns Return \p true if the Player can change its volume.
			 */
			virtual bool hasVolumeControl() const = 0;

			/**
			 * Get the current volume.
			 *
			 * @returns the current volume
			 */
			virtual float volume() const = 0;

			/**
			 * Get the total time (in milliseconds) of the file currently being played.
			 */
			virtual long totalTime() const = 0;

			/**
			 * Get the remaining time (in milliseconds) of the file currently being played.
			 */
			virtual long remainingTime() const = 0;

			/**
			 * Get the current time (in milliseconds) of the file currently being played.
			 */
			virtual long currentTime() const = 0;

			/**
			 * If the current media may be seeked returns true.
			 *
			 * @returns whether the current media may be seeked.
			 */
			virtual bool seekable() const = 0;

			/**
			 * Return the time interval in milliseconds between two ticks.
			 */
			virtual long tickInterval() const = 0;

		public slots:
			/**
			 * Load a media file. If another media is currently loaded it is
			 * stopped and unloaded.
			 *
			 * @param url The location of the media file
			 *
			 * @returns If the call was successfull (the media file was found
			 * and can be read and decoded) returns \p true
			 */
			virtual bool load( const KURL & url ) = 0;

			/**
			 * Play the media file.
			 */
			virtual bool play() = 0;

			/**
			 * Pause a playing media. If it was paused before nothing changes.
			 */
			virtual bool pause() = 0;

			/**
			 * Stop a playback.
			 */
			virtual bool stop() = 0;

			/**
			 * Set the volume of the playback
			 *
			 * @param volume 0.0 is complete silence and 1.0 is full volume.
			 *               Higher values than 1.0 are possible but might
			 *               result in distortion of the sound.
			 *
			 * @returns returns \p true if the call was successfull, returns \p
			 * false if the volume change didn't work and therefor didn't
			 * change.
			 */
			virtual bool setVolume( float volume ) = 0;

			/**
			 * Seek to the time indicated.
			 *
			 * @param time The time in milliseconds where to continue playing.
			 *
			 * @returns whether the seek was successfull.
			 */
			virtual bool seek( long time ) = 0;

			/**
			 * Change the interval the tick signal is emitted. If you set \p ms
			 * to 0 the signal gets disabled.
			 *
			 * \param ms tick interval in milliseconds
			 *
			 * @returns Returns \p true if the tick intervall was changed.
			 */
			virtual bool setTickInterval( long ms ) = 0;

		signals:
			/**
			 * Emitted when the file has finished playing on its own.
			 * I.e. it is not emitted if you call stop(), pause() or
			 * load(), but only on end-of-file or a critical error.
			 */
			void finished();

			//XXX do we want a aboutToFinish() signal?

			/**
			 * Emitted when the state of the Player has changed.
			 * In case you're not interested in the old state you can also
			 * connect to a slot that only has one State argument.
			 *
			 * @param newstate The state the Player is in now
			 * @param oldstate The state the Player was in before
			 */
			void stateChanged( KDE::Multimedia::Player::State newstate, KDE::Multimedia::Player::State oldstate );

			/**
			 * This signal gets emitted every tickInterval milliseconds.
			 *
			 * \param time The position of the media file in milliseconds.
			 *
			 * \see setTickInterval, tickInterval
			 */
			void tick( long time );

			/**
			 * This signal is emitted as soon as the length of the media file is
			 * known or has changed.
			 *
			 * @param length The length of the media file in milliseconds.
			 */
			void length( long length );

		protected:
			/**
			 * You can not instantiate players yourself, use the Factory to
			 * create them.
			 */
			Player( TQObject * parent, const char * name );

			/**
			 * Call this in your reimplementation for state changes.
			 * It emits the stateChanged signal if the state has really changed.
			 *
			 * @param newstate The new state of the Player. If \p newstate is
			 * equal to the old state() the call will be ignored.
			 */
			void setState( State newstate );

		private:
			class Private;
			Private * d;

			State m_state;
	};
}} //namespaces

// vim: sw=4 ts=4 tw=80 noet
#endif // PLAYOBJECT_H
