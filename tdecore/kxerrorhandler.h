/*

  Copyright (c) 2003 Lubos Lunak <l.lunak@kde.org>

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

#ifndef KXERRORHANDLER_H
#define KXERRORHANDLER_H

#include <tqvaluelist.h>
#include <tqwindowdefs.h>
#include <tdelibs_export.h>
#include <X11/Xlib.h>

/**
 * This class simplifies handling of X errors. It shouldn't be necessary to use
 * with Qt classes, as the toolkit should handle X errors itself, so this
 * class will be mainly used with direct Xlib usage, and some lowlevel classes
 * like NETWinInfo.
 *
 * The usual usage is to create a KXErrorHandler instance right before starting
 * operations that might cause X errors, and checking if there was an error
 * by calling error() after the operations are finished. The handlers
 * may be nested, but must be destroyed in reverse order they were created.
 *
 * There's no need to do X sync before creating an instance, every instance
 * will handle only errors for request issued after the instance was created.
 * Errors for older requests will be passed to previous error handler.
 * When checking for error by calling error() at the end, it is necessary
 * to sync with X, to catch all errors that were caused by requests issued
 * before the call to error(). This can be done by passing true to error()
 * to cause explicit XSync(), however, if the last X request needed a roundtrip
 * (e.g. XGetWindowAttributes(), XGetGeometry(), etc.), it is not required
 * to do an explicit sync.
 *
 * @author Lubos Lunak <l.lunak@kde.org>
 * @short Handler for X errors
 */
class TDECORE_EXPORT KXErrorHandler
    {
    public:
        /**
         * Creates error handler that will set error flag after encountering
         * any X error.
         */
        KXErrorHandler( Display* dpy = tqt_xdisplay());
        /**
         * This constructor takes pointer to a function that will get request number,
         * error code number and resource id of the failed request, as provided
         * by XErrorEvent. If the function returns true, the error flag will be set.
         */
        KXErrorHandler( bool (*handler)( int request, int error_code, unsigned long resource_id ), Display* dpy = tqt_xdisplay());
        /**
         * This constructor takes pointer to a function whose prototype matches
         * the one that's used with the XSetErrorHandler() Xlib function.
         * NOTE: For the error flag to be set, the function must return non-zero
         * value.
         */
        KXErrorHandler( int (*handler)( Display*, XErrorEvent* ), Display* dpy = tqt_xdisplay());
        /**
         * This function returns true if the error flag is set (i.e. no custom handler
         * function was used and there was any error, or the custom handler indicated
         * an error by its return value).
         *
         * @param sync if true, and explicit XSync() will be done. Not necessary
         *             when the last X request required a roundtrip.
         */
        bool error( bool sync ) const;
        ~KXErrorHandler();
    private:
        void addHandler();
        int handle( Display* dpy, XErrorEvent* e );
        bool (*user_handler1)( int request, int error_code, unsigned long resource_id );
        int (*user_handler2)( Display*, XErrorEvent* );
        int (*old_handler)( Display*, XErrorEvent* );
        unsigned long first_request;
        Display* display;
        bool was_error;
        static int handler_wrapper( Display*, XErrorEvent* );
        static KXErrorHandler** handlers;
        static int pos;
        static int size;
        class KXErrorHandlerPrivate* d;
    };

#endif
