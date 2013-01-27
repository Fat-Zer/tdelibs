TEMPLATE	= lib

include( $(KDELIBS)/win/common.pro )
include( $(KDELIBS)/win/zlib.pro )

# needed to export library classes:
DEFINES += MAKE_KIO_LIB

LIBS += $$KDELIBDESTDIR\tdecore$$KDELIB_SUFFIX $$KDELIBDESTDIR\tdeui$$KDELIB_SUFFIX \
	$$KDELIBDESTDIR\dcop$$KDELIB_SUFFIX $$KDELIBDESTDIR\kdewin32$$KDELIB_SUFFIX

system( bash kmoc kio tdefile misc bookmarks kssl )

TARGET = kio$$KDEBUG

INCLUDEPATH += $(KDELIBS)/tdecore/network $(KDELIBS)/tdeio/tdeio $(KDELIBS)/tdeio/misc $(KDELIBS)/tdeio/bookmarks \
  $(KDELIBS)/tdeio/kssl \
  $(KDELIBS)/libltdl $(KDELIBS)/interfaces \
  $(KDELIBS)/tdeio/tdeio/moc $(KDELIBS)/tdeio/misc/moc $(KDELIBS)/tdeio/tdefile/moc \
  $(KDELIBS)/tdeio/misc/moc \
  $(KDELIBS)/tdeio/bookmarks/moc \
  $(KDELIBS)/tdeio/kssl/moc $(KDELIBS)/tdewallet/client 

system( cd kio && dcopidl kdirnotify.h > kdirnotify.kidl && dcopidl2cpp --no-stub kdirnotify.kidl )
system( cd kio && dcopidl observer.h > observer.kidl && dcopidl2cpp observer.kidl )
system( cd bookmarks && dcopidl kbookmarknotifier.h > kbookmarknotifier.kidl && dcopidl2cpp kbookmarknotifier.kidl )
system( cd bookmarks && dcopidl kbookmarkmanager.h > kbookmarkmanager.kidl && dcopidl2cpp kbookmarkmanager.kidl )
system( cd misc && dcopidl uiserver.h > uiserver.kidl && dcopidl2cpp uiserver.kidl )


SOURCES = \
tdeio/authinfo.cpp \
tdeio/chmodjob.cpp \
tdeio/connection.cpp \
tdeio/dataprotocol.cpp \
tdeio/dataslave.cpp \
tdeio/davjob.cpp \
tdeio/defaultprogress.cpp \
tdeio/global.cpp \
tdeio/job.cpp \
tdeio/kacl.cpp \
tdeio/kar.cpp \
tdeio/karchive.cpp \
tdeio/kdatatool.cpp \
tdeio/kdcopservicestarter.cpp \
tdeio/kdirlister.cpp \
tdeio/kdirnotify.cpp \
tdeio/kdirwatch.cpp \
tdeio/kemailsettings.cpp \
tdeio/tdefilefilter.cpp \
tdeio/tdefileitem.cpp \
tdeio/tdefilemetainfo.cpp \
tdeio/tdefileshare.cpp \
tdeio/kfilterbase.cpp \
tdeio/kfilterdev.cpp \
tdeio/kimageio.cpp \
tdeio/kmimemagic.cpp \
tdeio/kmimetype.cpp \
tdeio/kmimetypechooser.cpp \
tdeio/knfsshare.cpp \
tdeio/kprotocolinfo.cpp \
tdeio/kprotocolmanager.cpp \
tdeio/kremoteencoding.cpp \
tdeio/krun.cpp \
tdeio/ksambashare.cpp \
tdeio/kscan.cpp \
tdeio/kservice.cpp \
tdeio/kservicefactory.cpp \
tdeio/kservicegroup.cpp \
tdeio/kservicegroupfactory.cpp \
tdeio/kservicetype.cpp \
tdeio/kservicetypefactory.cpp \
tdeio/kshellcompletion.cpp \
tdeio/kshred.cpp \
tdeio/ktar.cpp \
tdeio/ktrader.cpp \
tdeio/ktraderparse.cpp \
tdeio/ktraderparsetree.cpp \
tdeio/kurifilter.cpp \
tdeio/kurlcompletion.cpp \
tdeio/kurlpixmapprovider.cpp \
tdeio/kuserprofile.cpp \
tdeio/kzip.cpp \
tdeio/lex.c \
tdeio/metainfojob.cpp \
tdeio/netaccess.cpp \
tdeio/observer.cpp \
tdeio/passdlg.cpp \
tdeio/paste.cpp \
tdeio/pastedialog.cpp \
tdeio/previewjob.cpp \
tdeio/progressbase.cpp \
tdeio/renamedlg.cpp \
tdeio/scheduler.cpp \
tdeio/sessiondata.cpp \
tdeio/skipdlg.cpp \
tdeio/slave.cpp \
tdeio/slavebase.cpp \
tdeio/slaveconfig.cpp \
tdeio/slaveinterface.cpp \
tdeio/statusbarprogress.cpp \
tdeio/tcpslavebase.cpp \
tdeio/yacc.c \
\
bookmarks/kbookmark.cc \
bookmarks/kbookmarkbar.cc \
bookmarks/kbookmarkdombuilder.cc \
bookmarks/kbookmarkdrag.cc \
bookmarks/kbookmarkexporter.cc \
bookmarks/kbookmarkimporter.cc \
bookmarks/kbookmarkimporter_crash.cc \
bookmarks/kbookmarkimporter_ie.cc \
bookmarks/kbookmarkimporter_kde1.cc \
bookmarks/kbookmarkimporter_ns.cc \
bookmarks/kbookmarkimporter_opera.cc \
bookmarks/kbookmarkmanager.cc \
bookmarks/kbookmarkmenu.cc \
\
tdefile/kcombiview.cpp \
tdefile/kcustommenueditor.cpp \
tdefile/kdiroperator.cpp \
tdefile/kdirselectdialog.cpp \
tdefile/kdirsize.cpp \
tdefile/kdiskfreesp.cpp \
tdefile/kencodingfiledialog.cpp \
tdefile/tdefilebookmarkhandler.cpp \
tdefile/tdefiledetailview.cpp \
tdefile/tdefiledialog.cpp \
tdefile/tdefilefiltercombo.cpp \
tdefile/tdefileiconview.cpp \
tdefile/tdefilemetainfowidget.cpp \
tdefile/tdefilemetapreview.cpp \
tdefile/tdefilepreview.cpp \
tdefile/tdefilesharedlg.cpp \
tdefile/tdefilespeedbar.cpp \
tdefile/tdefiletreebranch.cpp \
tdefile/tdefiletreeview.cpp \
tdefile/tdefiletreeviewitem.cpp \
tdefile/tdefileview.cpp \
tdefile/kicondialog.cpp \
tdefile/kimagefilepreview.cpp \
tdefile/kmetaprops.cpp \
tdefile/knotifydialog.cpp \
tdefile/kopenwith.cpp \
tdefile/kpreviewprops.cpp \
tdefile/kpreviewwidgetbase.cpp \
tdefile/kpropertiesdialog.cpp \
tdefile/krecentdirs.cpp \
tdefile/krecentdocument.cpp \
tdefile/kurlbar.cpp \
tdefile/kurlcombobox.cpp \
tdefile/kurlrequester.cpp \
tdefile/kurlrequesterdlg.cpp \
\
misc/uiserver.cpp \
\
kssl/ksslcertdlg.cc \
kssl/ksslinfodlg.cc \
kssl/ksslcsessioncache.cc \
kssl/ksslsession.cc \
kssl/ksslsettings.cc \
kssl/ksslcertchain.cc \
kssl/ksslcertificate.cc \
kssl/ksslcertificatecache.cc \
kssl/ksslcertificatehome.cc \
kssl/ksslcertificatefactory.cc \
kssl/kssl.cc \
kssl/ksslconnectioninfo.cc \
kssl/ksslkeygen.cc \
kssl/ksslpkcs7.cc \
kssl/ksslpkcs12.cc \
kssl/ksslx509v3.cc \
kssl/ksslx509map.cc \
kssl/ksslsigners.cc \
kssl/ksslpeerinfo.cc \
kssl/kopenssl.cc \
kssl/ksmimecrypto.cc

#removed tdeio/kautomount.cpp \

SOURCES += \
../tdecore/kprotocolinfo_tdecore.cpp

# js 2004-08-05 ^^^^^ a hack because msvc cannot split a class between two libraries!

#kprotocolinfofactory.cpp \

#slavebase.cpp \

# generated:
SOURCES += \
tdeio/kdirnotify_stub.cpp \
tdeio/kdirnotify_skel.cpp \
tdeio/observer_stub.cpp \
tdeio/observer_skel.cpp \
\
bookmarks/kbookmarknotifier_stub.cpp \
bookmarks/kbookmarknotifier_skel.cpp \
bookmarks/kbookmarkmanager_stub.cpp \
bookmarks/kbookmarkmanager_skel.cpp \
\
misc/uiserver_stub.cpp \
misc/uiserver_skel.cpp


FORMS = \
kssl/keygenwizard.ui \
kssl/keygenwizard2.ui \
tdefile/knotifywidgetbase.ui \
tdefile/kpropertiesdesktopadvbase.ui \
tdefile/kpropertiesdesktopbase.ui \
tdefile/kpropertiesmimetypebase.ui
