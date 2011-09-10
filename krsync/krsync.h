/*
   Copyright (C) 2000, 2001, 2002 Dawit Alemayehu <adawit@kde.org>

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

#ifndef _LIBKRSYNC_H
#define _LIBKRSYNC_H

#include <tqmap.h>
#include <tqstringlist.h>

#include <kurl.h>
#include <kprocess.h>
#include <kfileitem.h>
#include <klibloader.h>
#include <kparts/plugin.h>
#include <kio/global.h>
#include <kio/slavebase.h>

class KActionMenu;
class KonqDirPart;
class KLineEdit;
class RsyncConfigDialog;

namespace KParts
{
  struct URLArgs;
}

class KRsync : public TQObject
{
  Q_OBJECT
  TQ_OBJECT

public:

  KRsync (TQObject* parent, const char* name);
  virtual ~KRsync();

public:
  void loadSettings();
  void saveSettings();
  TQString findLocalFolderByName(TQString folderurl);
  TQString findLoginSyncEnabledByName(TQString folderurl);
  TQString findLogoutSyncEnabledByName(TQString folderurl);
  TQString findTimedSyncEnabledByName(TQString folderurl);
  int deleteLocalFolderByName(TQString folderurl);
  int addLocalFolderByName(TQString folderurl, TQString remoteurl, TQString syncmethod, TQString excludelist, TQString sync_on_login, TQString sync_on_logout, TQString sync_timed_interval);
  TQString findSyncMethodByName(TQString folderurl);
  /** manages initial communication setup including password queries */
  int establishConnectionRsync(char *buffer, KIO::fileoffset_t len);
  /** manages initial communication setup including password queries */
  int establishConnectionUnison(char *buffer, KIO::fileoffset_t len, TQString localfolder, TQString remotepath);
  /** creates the unidirectional sync subprocess */
  bool syncUnidirectional(TQString synccommand, TQString syncflags, int parameter_order, TQString localfolder, TQString remotepath);
  /** creates the bidirectional sync subprocess */
  bool syncBidirectional(TQString synccommand, TQString syncflags, int parameter_order, TQString localfolder, TQString remotepath);
  /** writes one chunk of data to stdin of child process */
  void writeChild(const char *buf, KIO::fileoffset_t len);
  /** AuthInfo object used for logging in */
  KIO::AuthInfo connectionAuth;
  /**
   Clean up connection
  */
  void shutdownConnection(bool forced=false, bool wait=false);
  /** Forced close of the connection */
  void closeConnection();
  /** Set current local directory URL */
  void setCurrentDirectoryURL(KURL url);

public slots:
  void slotSync();
  void slotSetup();
  void slotSetupOK();
  void slotSetupCancelled();
  void slotRsyncCancelled();
  void slotUnisonCancelled();

signals:
  void setupDone();
  void transferDone();

private:
  KURL m_pURL;
  KProgressBoxDialog* m_progressDialog;
  RsyncConfigDialog* m_configDialog;

  TQStringList cfgfolderlist;
  TQStringList cfgautosync_onlogout_list;
  bool m_progressDialogExists;

  bool m_bSettingsLoaded;

  /** true if connection is logged in successfully */
  bool isLoggedIn;
  /** the rsync process used to communicate with the remote end */
  pid_t childPid;
  /** fd for reading and writing to the process */
  int childFd;
  /** buffer for data to be written */
  const char *outBuf;
  /** current write position in buffer */
  KIO::fileoffset_t outBufPos;
  /** length of buffer */
  KIO::fileoffset_t outBufLen;
  /** use su if true else use ssh */
  //bool local;
  /**  // FIXME: just a workaround for konq deficiencies */
  bool isStat;
  /**  // FIXME: just a workaround for konq deficiencies */
  TQString redirectUser, redirectPass;
  /** user name of current connection */
  TQString connectionUser;
  /** password of current connection */
  TQString connectionPassword;
  /** true if this is the first login attempt (== use cached password) */
  bool firstLogin;

  TQString thisFn;
};
#endif
