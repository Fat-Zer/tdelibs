/*  -*- C++ -*-
 *  Copyright (C) 2003 Thiago Macieira <thiago@kde.org>
 *
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included 
 *  in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef KBUFFEREDSOCKET_H
#define KBUFFEREDSOCKET_H

#include <tqobject.h>
#include <tqcstring.h>
#include <tqvaluelist.h>
#include "kstreamsocket.h"
#include <tdelibs_export.h>

class KIOBufferBase;

namespace KNetwork {

class KBufferedSocketPrivate;
/** @class KBufferedSocket kbufferedsocket.h kbufferedsocket.h
 *  @brief Buffered stream sockets.
 *
 * This class allows the user to create and operate buffered stream sockets
 * such as those used in most Internet connections. This class is
 * also the one that resembles the most to the old @ref QSocket
 * implementation.
 *
 * Objects of this type operate only in non-blocking mode. A call to
 * setBlocking(true) will result in an error.
 *
 * @note Buffered sockets only make sense if you're using them from
 *       the main (event-loop) thread. This is actually a restriction
 *       imposed by Qt's TQSocketNotifier. If you want to use a socket
 *       in an auxiliary thread, please use KStreamSocket.
 *
 * @see KNetwork::KStreamSocket, KNetwork::TDEServerSocket
 * @author Thiago Macieira <thiago@kde.org>
 */
class TDECORE_EXPORT KBufferedSocket: public KStreamSocket
{
  Q_OBJECT
  
public:
  /**
   * Default constructor.
   *
   * @param node	destination host
   * @param service	destination service to connect to
   * @param parent      the parent object for this object
   * @param name        the internal name for this object
   */
  KBufferedSocket(const TQString& node = TQString::null, const TQString& service = TQString::null,
		  TQObject* parent = 0L, const char *name = 0L);

  /**
   * Destructor.
   */
  virtual ~KBufferedSocket();

  /**
   * Be sure to catch new devices.
   */
  virtual void setSocketDevice(TDESocketDevice* device);

protected:
  /**
   * Buffered sockets can only operate in non-blocking mode.
   */
  virtual bool setSocketOptions(int opts);

public:
  /**
   * Closes the socket for new data, but allow data that had been buffered
   * for output with @ref writeBlock to be still be written.
   *
   * @sa closeNow
   */
  virtual void close();

  /**
   * Make use of the buffers.
   */
#ifdef USE_QT3
  virtual TQ_LONG bytesAvailable() const;
#endif
#ifdef USE_QT4
  virtual qint64 bytesAvailable() const;
#endif

  /**
   * Make use of buffers.
   */
  virtual TQ_LONG waitForMore(int msecs, bool *timeout = 0L);

  /**
   * Reads data from the socket. Make use of buffers.
   */
  virtual TQT_TQIO_LONG tqreadBlock(char *data, TQT_TQIO_ULONG maxlen);

  /**
   * @overload
   * Reads data from a socket.
   *
   * The @p from parameter is always set to @ref peerAddress()
   */
  virtual TQT_TQIO_LONG tqreadBlock(char *data, TQT_TQIO_ULONG maxlen, TDESocketAddress& from);

  /**
   * Peeks data from the socket.
   */
  virtual TQ_LONG peekBlock(char *data, TQ_ULONG maxlen);

  /**
   * @overload
   * Peeks data from the socket.
   *
   * The @p from parameter is always set to @ref peerAddress()
   */
  virtual TQ_LONG peekBlock(char *data, TQ_ULONG maxlen, TDESocketAddress &from);

  /**
   * Writes data to the socket.
   */
  virtual TQT_TQIO_LONG tqwriteBlock(const char *data, TQT_TQIO_ULONG len);

  /**
   * @overload
   * Writes data to the socket.
   *
   * The @p to parameter is discarded.
   */
  virtual TQT_TQIO_LONG tqwriteBlock(const char *data, TQT_TQIO_ULONG len, const TDESocketAddress& to);

  /**
   * Catch changes.
   */
  virtual void enableRead(bool enable);

  /**
   * Catch changes.
   */
  virtual void enableWrite(bool enable);

  /**
   * Sets the use of input buffering.
   */
  void setInputBuffering(bool enable);

  /**
   * Retrieves the input buffer object.
   */
  KIOBufferBase* inputBuffer();

  /**
   * Sets the use of output buffering.
   */
  void setOutputBuffering(bool enable);

  /**
   * Retrieves the output buffer object.
   */
  KIOBufferBase* outputBuffer();

  /**
   * Returns the length of the output buffer.
   */
#ifdef USE_QT3
  virtual TQ_ULONG bytesToWrite() const;
#endif
#ifdef USE_QT4
  virtual qint64 bytesToWrite() const;
#endif

  /**
   * Closes the socket and discards any output data that had been buffered
   * with @ref writeBlock but that had not yet been written.
   *
   * @sa close
   */
  virtual void closeNow();

  /**
   * Returns true if a line can be read with @ref readLine
   */
  bool canReadLine() const;

  /**
   * Reads a line of data from the socket buffers.
   */
  TQCString readLine();

  // KDE4: make virtual, add timeout to match the Qt4 signature
  //       and move to another class up the hierarchy
  /**
   * Blocks until the connection is either established, or completely
   * failed.
   */
  void waitForConnect();

protected:
  /**
   * Catch connection to clear the buffers
   */
  virtual void stateChanging(SocketState newState);

protected slots:
  /**
   * Slot called when there's read activity.
   */
  virtual void slotReadActivity();

  /**
   * Slot called when there's write activity.
   */
  virtual void slotWriteActivity();

signals:
  /**
   * This signal is emitted whenever data is written.
   */
  void bytesWritten(int bytes);

private:
  KBufferedSocket(const KBufferedSocket&);
  KBufferedSocket& operator=(const KBufferedSocket&);

  KBufferedSocketPrivate *d;

public:
  // KDE4: remove this function
  /**
   * @deprecated
   * Closes the socket.
   *
   * This function is provided to ease porting from KExtendedSocket,
   * which required a call to reset() in order to be able to connect again
   * using the same device. This is not necessary in KBufferedSocket any more.
   */
#ifdef USE_QT3
  inline void reset()
#endif
#ifdef USE_QT4
  inline bool reset()
#endif
  { closeNow(); }
};

}				// namespace KNetwork

#endif
