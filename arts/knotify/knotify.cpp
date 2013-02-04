/*
   Copyright (c) 1997 Christian Esken (esken@kde.org)
                 2000 Charles Samuels (charles@kde.org)
                 2000 Stefan Schimanski (1Stein@gmx.de)
                 2000 Matthias Ettrich (ettrich@kde.org)
                 2000 Waldo Bastian <bastian@kde.org>
                 2000-2003 Carsten Pfeiffer <pfeiffer@kde.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

// C headers
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <config.h>
#ifndef WITHOUT_ARTS
// aRts headers
#include <connect.h>
#include <dispatcher.h>
#include <flowsystem.h>
#include <qiomanager.h>
#include <soundserver.h>
#endif

// QT headers
#include <tqfile.h>
#include <tqfileinfo.h>
#include <tqstringlist.h>
#include <tqtextstream.h>

// KDE headers
#include <dcopclient.h>
#include <kaboutdata.h>
#ifndef WITHOUT_ARTS
#include <kartsdispatcher.h>
#include <kartsserver.h>
#endif
#include <kcmdlineargs.h>
#include <tdeconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpassivepopup.h>
#include <kiconloader.h>
#include <kmacroexpander.h>
#ifndef WITHOUT_ARTS
#include <kplayobjectfactory.h>
#include <kaudiomanagerplay.h>
#endif
#include <kprocess.h>
#include <kstandarddirs.h>
#include <kuniqueapplication.h>
#include <twin.h>

#include "knotify.h"
#include "knotify.moc"

class KNotifyPrivate
{
public:
    TDEConfig* globalEvents;
    TDEConfig* globalConfig;
    TQMap<TQString, TDEConfig*> events;
    TQMap<TQString, TDEConfig*> configs;
    TQString externalPlayer;
    TDEProcess *externalPlayerProc;

#ifndef WITHOUT_ARTS
    TQPtrList<KDE::PlayObject> playObjects;
    TQMap<KDE::PlayObject*,int> playObjectEventMap;
    KAudioManagerPlay *audioManager;
#endif
    int externalPlayerEventId;

    bool useExternal;
    bool useArts;
    int volume;
    TQTimer *playTimer;
    bool inStartup;
    TQString startupEvents;
};

// Yes, it's ugly to put this here, but this facilitates the cautious startup
// procedure.
#ifndef WITHOUT_ARTS
KArtsServer *soundServer = 0;
#endif

extern "C"{

KDE_EXPORT int kdemain(int argc, char **argv)
{
    TDEAboutData aboutdata("knotify", I18N_NOOP("KNotify"),
                         "3.0", I18N_NOOP("TDE Notification Server"),
                         TDEAboutData::License_GPL, "(C) 1997-2003, KDE Developers");
    aboutdata.addAuthor("Carsten Pfeiffer",I18N_NOOP("Current Maintainer"),"pfeiffer@kde.org");
    aboutdata.addAuthor("Christian Esken",0,"esken@kde.org");
    aboutdata.addAuthor("Stefan Westerfeld",I18N_NOOP("Sound support"),"stefan@space.twc.de");
    aboutdata.addAuthor("Charles Samuels",I18N_NOOP("Previous Maintainer"),"charles@kde.org");

    TDECmdLineArgs::init( argc, argv, &aboutdata );
    KUniqueApplication::addCmdLineOptions();


    // initialize application
    if ( !KUniqueApplication::start() ) {
        kdDebug() << "Running knotify found" << endl;
        return 0;
    }

    KUniqueApplication app;
    app.disableSessionManagement();

    // KNotify is started on KDE startup and on demand (using
    // KNotifClient::startDaemon()) whenever a KNotify event occurs. Especially
    // KWin may fire many events (e.g. when a window pops up). When we have
    // problems with aRts or the installation, we might get an infinite loop
    // of knotify crashing, popping up the crashhandler window and twin firing
    // another event, starting knotify again...
    // We try to prevent this by tracking our startup and offer options to
    // abort this.

#ifndef WITHOUT_ARTS
    TDEConfigGroup config( TDEGlobal::config(), "StartProgress" );
    TDEConfig artsKCMConfig( "kcmartsrc" );
    artsKCMConfig.setGroup( "Arts" );
    bool useArts = artsKCMConfig.readBoolEntry( "StartServer", true );
    if (useArts)
        useArts = config.readBoolEntry( "Use Arts", useArts );
    bool ok = config.readBoolEntry( "Arts Init", true );

    if ( useArts && !ok )
    {
        if ( KMessageBox::questionYesNo(
                 0L,
                 i18n("During the previous startup, KNotify crashed while creating "
                      "Arts::Dispatcher. Do you want to try again or disable "
                      "aRts sound output?\n\n"
                      "If you choose to disable aRts output now, you can re-enable "
                      "it later or select an alternate sound player "
                      "in the System Notifications control panel."),
                 i18n("KNotify Problem"),
                 i18n("&Try Again"),
                 i18n("D&isable aRts Output"),
                 "KNotifyStartProgress",
                 0 /* don't call KNotify :) */
                 )
             == KMessageBox::No )
        {
            useArts = false;
        }
    }

    // when ArtsDispatcher crashes, we know it the next start.
    config.writeEntry( "Arts Init", false );
    config.writeEntry( "Use Arts", useArts );
    config.sync();

    KArtsDispatcher *dispatcher = 0;
    if ( useArts )
    {
        dispatcher = new KArtsDispatcher;
        soundServer = new KArtsServer;
    }

    // ok, seemed to work.
    config.writeEntry("Arts Init", useArts );
    config.sync();

    ok = config.readBoolEntry( "KNotify Init", true );
    if ( useArts && !ok )
    {
        if ( KMessageBox::questionYesNo(
                 0L,
                 i18n("During the previous startup, KNotify crashed while instantiating "
                      "KNotify. Do you want to try again or disable "
                      "aRts sound output?\n\n"
                      "If you choose to disable aRts output now, you can re-enable "
                      "it later or select an alternate sound player "
                      "in the System Notifications control panel."),
                 i18n("KNotify Problem"),
                 i18n("&Try Again"),
                 i18n("D&isable aRts Output"),
                 "KNotifyStartProgress",
                 0 /* don't call KNotify :) */
                 )
             == KMessageBox::No )
        {
            useArts = false;
            delete soundServer;
            soundServer = 0L;
            delete dispatcher;
            dispatcher = 0L;
        }
    }

    // when KNotify instantiation crashes, we know it the next start.
    config.writeEntry( "KNotify Init", false );
    config.writeEntry( "Use Arts", useArts );
    config.sync();

    // start notify service
    KNotify *notify = new KNotify( useArts );

    config.writeEntry( "KNotify Init", true );
    config.sync();

#else

    // start notify service, without aRts
    KNotify *notify = new KNotify( false );

#endif

    app.dcopClient()->setDefaultObject( "Notify" );
    app.dcopClient()->setDaemonMode( true );
    // kdDebug() << "knotify starting" << endl;

    int ret = app.exec();
    delete notify;
#ifndef WITHOUT_ARTS
    delete soundServer;
    delete dispatcher;
#endif
    return ret;
}
}// end extern "C"

KNotify::KNotify( bool useArts )
    : TQObject(), DCOPObject("Notify")
{
    d = new KNotifyPrivate;
    d->globalEvents = new TDEConfig("knotify/eventsrc", true, false, "data");
    d->globalConfig = new TDEConfig("knotify.eventsrc", true, false);
    d->externalPlayerProc = 0;
    d->useArts = useArts;
    d->inStartup = true;
#ifndef WITHOUT_ARTS
    d->playObjects.setAutoDelete(true);
    d->audioManager = 0;
    if( useArts )
    {
        connect( soundServer, TQT_SIGNAL( restartedServer() ), this, TQT_SLOT( restartedArtsd() ) );
        restartedArtsd(); //started allready need to initialize d->audioManager
    }
#endif

    d->volume = 100;

    d->playTimer = 0;

    loadConfig();
}

KNotify::~KNotify()
{
    reconfigure();

#ifndef WITHOUT_ARTS
    d->playObjects.clear();

    delete d->globalEvents;
    delete d->globalConfig;
    delete d->externalPlayerProc;
    delete d->audioManager;
#endif
    delete d;
}


void KNotify::loadConfig() {
    // load external player settings
    TDEConfig *kc = TDEGlobal::config();
    kc->setGroup("Misc");
    d->useExternal = kc->readBoolEntry( "Use external player", false );
    d->externalPlayer = kc->readPathEntry("External player");

    // try to locate a suitable player if none is configured
    if ( d->externalPlayer.isEmpty() ) {
        TQStringList players;
        players << "wavplay" << "aplay" << "auplay";
        TQStringList::Iterator it = players.begin();
        while ( d->externalPlayer.isEmpty() && it != players.end() ) {
            d->externalPlayer = TDEStandardDirs::findExe( *it );
            ++it;
        }
    }

    // load default volume
    d->volume = kc->readNumEntry( "Volume", 100 );
}


void KNotify::reconfigure()
{
    kapp->config()->reparseConfiguration();
    loadConfig();

    // clear loaded config files
    d->globalConfig->reparseConfiguration();
    for ( TQMapIterator<TQString,TDEConfig*> it = d->configs.begin(); it != d->configs.end(); ++it )
        delete it.data();
    d->configs.clear();
}


void KNotify::notify(const TQString &event, const TQString &fromApp,
                     const TQString &text, TQString sound, TQString file,
                     int present, int level)
{
    notify( event, fromApp, text, sound, file, present, level, 0, 1 );
}

void KNotify::notify(const TQString &event, const TQString &fromApp,
                     const TQString &text, TQString sound, TQString file,
                     int present, int level, int winId)
{
    notify( event, fromApp, text, sound, file, present, level, winId, 1 );
}

void KNotify::notify(const TQString &event, const TQString &fromApp,
                     const TQString &text, TQString sound, TQString file,
                     int present, int level, int winId, int eventId )
{
    // kdDebug() << "event=" << event << " fromApp=" << fromApp << " text=" << text << " sound=" << sound <<
    //    " file=" << file << " present=" << present << " level=" << level <<  " winId=" << winId << " eventId=" << eventId << endl;
    if( d->inStartup ) {
        d->startupEvents += "(" + event + ":" + fromApp + ")";
    }

    TQString commandline;
    TDEConfig *eventsFile = NULL;
    TDEConfig *configFile = NULL;

    // check for valid events
    if ( !event.isEmpty() ) {

        // get config file
        if ( d->events.contains( fromApp ) ) {
            eventsFile = d->events[fromApp];
        } else {
            eventsFile=new TDEConfig(locate("data", fromApp+"/eventsrc"),true,false);
            d->events.insert( fromApp, eventsFile );
        }
        if ( d->configs.contains( fromApp) ) {
            configFile = d->configs[fromApp];
        } else {
            configFile=new TDEConfig(fromApp+".eventsrc",true,false);
            d->configs.insert( fromApp, configFile );
        }

        if ( !eventsFile->hasGroup( event ) && isGlobal(event) )
        {
            eventsFile = d->globalEvents;
            configFile = d->globalConfig;
        }

        eventsFile->setGroup( event );
        configFile->setGroup( event );

        // get event presentation
        if ( present==-1 )
            present = configFile->readNumEntry( "presentation", -1 );
        if ( present==-1 )
            present = eventsFile->readNumEntry( "default_presentation", 0 );

        // get sound file name
        if( present & KNotifyClient::Sound ) {
            TQString theSound = configFile->readPathEntry( "soundfile" );
            if ( theSound.isEmpty() )
                theSound = eventsFile->readPathEntry( "default_sound" );
            if ( !theSound.isEmpty() )
                sound = theSound;
        }

        // get log file name
        if( present & KNotifyClient::Logfile ) {
            TQString theFile = configFile->readPathEntry( "logfile" );
            if ( theFile.isEmpty() )
                theFile = eventsFile->readPathEntry( "default_logfile" );
            if ( !theFile.isEmpty() )
                file = theFile;
        }

        // get default event level
        if( present & KNotifyClient::Messagebox )
            level = eventsFile->readNumEntry( "level", 0 );

        // get command line
        if (present & KNotifyClient::Execute ) {
            commandline = configFile->readPathEntry( "commandline" );
            if ( commandline.isEmpty() )
                commandline = eventsFile->readPathEntry( "default_commandline" );
        }
    }

    // emit event
    if ( present & KNotifyClient::Sound ) // && TQFile(sound).isReadable()
        notifyBySound( sound, fromApp, eventId );

    if ( present & KNotifyClient::Execute )
        notifyByExecute( commandline, event, fromApp, text, winId, eventId );

    if ( present & KNotifyClient::Logfile ) // && TQFile(file).isWritable()
        notifyByLogfile( text, file );

    if ( present & KNotifyClient::Stderr )
        notifyByStderr( text );

    if ( present & KNotifyClient::Taskbar )
        notifyByTaskbar( checkWinId( fromApp, winId ));

    if ( present & KNotifyClient::PassivePopup )
        notifyByPassivePopup( text, fromApp, eventsFile, checkWinId( fromApp, winId ));
    else if ( present & KNotifyClient::Messagebox )
        notifyByMessagebox( text, level, checkWinId( fromApp, winId ));

    TQByteArray qbd;
    TQDataStream ds(qbd, IO_WriteOnly);
    ds << event << fromApp << text << sound << file << present << level
        << winId << eventId;
    emitDCOPSignal("notifySignal(TQString,TQString,TQString,TQString,TQString,int,int,int,int)", qbd);

}


bool KNotify::notifyBySound( const TQString &sound, const TQString &appname, int eventId )
{
    if (sound.isEmpty()) {
        soundFinished( eventId, NoSoundFile );
        return false;
    }

    bool external = d->useExternal && !d->externalPlayer.isEmpty();
    // get file name
    TQString soundFile(sound);
    if ( TQFileInfo(sound).isRelative() )
    {
        TQString search = TQString("%1/sounds/%2").arg(appname).arg(sound);
        soundFile = TDEGlobal::instance()->dirs()->findResource("data", search);
        if ( soundFile.isEmpty() )
            soundFile = locate( "sound", sound );
    }
    if ( soundFile.isEmpty() || isPlaying( soundFile ) )
    {
        soundFinished( eventId, soundFile.isEmpty() ? NoSoundFile : FileAlreadyPlaying );
        return false;
    }


    // kdDebug() << "KNotify::notifyBySound - trying to play file " << soundFile << endl;

    if (!external) {
        //If we disabled using aRts, just return,
        //(If we don't, we'll blow up accessing the null soundServer)
        if (!d->useArts)
        {
            soundFinished( eventId, NoSoundSupport );
            return false;
        }

#ifndef WITHOUT_ARTS
        // play sound finally
        while( d->playObjects.count()>5 )
            abortFirstPlayObject();

        KDE::PlayObjectFactory factory(soundServer->server());
        if( d->audioManager )
            factory.setAudioManagerPlay( d->audioManager );
        KURL soundURL;
        soundURL.setPath(soundFile);
        KDE::PlayObject *playObject = factory.createPlayObject(soundURL, false);

        if (playObject->isNull())
        {
            soundFinished( eventId, NoSoundSupport );
            delete playObject;
            return false;
        }

        if ( d->volume != 100 )
        {
            // It works to access the playObject immediately because we don't allow
            // non-file URLs for sounds.
            Arts::StereoVolumeControl volumeControl = Arts::DynamicCast(soundServer->server().createObject("Arts::StereoVolumeControl"));
            Arts::PlayObject player = playObject->object();
            Arts::Synth_AMAN_PLAY ap = d->audioManager->amanPlay();
            if( ! volumeControl.isNull() && ! player.isNull() && ! ap.isNull() )
            {
                volumeControl.scaleFactor( d->volume/100.0 );

                ap.stop();
                Arts::disconnect( player, "left", ap, "left" );
                Arts::disconnect( player, "right", ap, "right" );

                ap.start();
                volumeControl.start();

                Arts::connect(player,"left",volumeControl,"inleft");
                Arts::connect(player,"right",volumeControl,"inright");

                Arts::connect(volumeControl,"outleft",ap,"left");
                Arts::connect(volumeControl,"outright",ap,"right");

                player._addChild( volumeControl, "volume" );
            }
        }

        playObject->play();
        d->playObjects.append( playObject );
        d->playObjectEventMap.insert( playObject, eventId );

        if ( !d->playTimer )
        {
            d->playTimer = new TQTimer( this );
            connect( d->playTimer, TQT_SIGNAL( timeout() ), TQT_SLOT( playTimeout() ) );
        }
        if ( !d->playTimer->isActive() )
            d->playTimer->start( 1000 );
#endif
        return true;

    } else if(!d->externalPlayer.isEmpty()) {
        // use an external player to play the sound
        TDEProcess *proc = d->externalPlayerProc;
        if (!proc)
        {
           proc = d->externalPlayerProc = new TDEProcess;
           connect( proc, TQT_SIGNAL( processExited( TDEProcess * )),
                    TQT_SLOT( slotPlayerProcessExited( TDEProcess * )));
        }
        if (proc->isRunning())
        {
           soundFinished( eventId, PlayerBusy );
           return false; // Skip
        }
        proc->clearArguments();
        (*proc) << d->externalPlayer << TQFile::encodeName( soundFile ).data();
        d->externalPlayerEventId = eventId;
        proc->start(TDEProcess::NotifyOnExit);
        return true;
    }

    soundFinished( eventId, Unknown );
    return false;
}

bool KNotify::notifyByMessagebox(const TQString &text, int level, WId winId)
{
    // ignore empty messages
    if ( text.isEmpty() )
        return false;

    // display message box for specified event level
    switch( level ) {
    default:
    case KNotifyClient::Notification:
        KMessageBox::informationWId( winId, text, i18n("Notification"), 0, false );
        break;
    case KNotifyClient::Warning:
        KMessageBox::sorryWId( winId, text, i18n("Warning"), false );
        break;
    case KNotifyClient::Error:
        KMessageBox::errorWId( winId, text, i18n("Error"), false );
        break;
    case KNotifyClient::Catastrophe:
        KMessageBox::errorWId( winId, text, i18n("Catastrophe!"), false );
        break;
    }

    return true;
}

bool KNotify::notifyByPassivePopup( const TQString &text,
                                    const TQString &appName,
                                    TDEConfig* eventsFile,
                                    WId senderWinId )
{
    TDEIconLoader iconLoader( appName );
    if ( eventsFile != NULL ) {
        TDEConfigGroup config( eventsFile, "!Global!" );
        TQString iconName = config.readEntry( "IconName", appName );
        TQPixmap icon = iconLoader.loadIcon( iconName, TDEIcon::Small );
        TQString title = config.readEntry( "Comment", appName );
        KPassivePopup::message(title, text, icon, senderWinId);
    } else
        kdError() << "No events for app " << appName << "defined!" <<endl;

    return true;
}

bool KNotify::notifyByExecute(const TQString &command, const TQString& event,
                              const TQString& fromApp, const TQString& text,
                              int winId, int eventId) {
    if (!command.isEmpty()) {
	// kdDebug() << "executing command '" << command << "'" << endl;
        TQMap<TQChar,TQString> subst;
        subst.insert( 'e', event );
        subst.insert( 'a', fromApp );
        subst.insert( 's', text );
        subst.insert( 'w', TQString::number( winId ));
        subst.insert( 'i', TQString::number( eventId ));
        TQString execLine = KMacroExpander::expandMacrosShellQuote( command, subst );
        if ( execLine.isEmpty() )
            execLine = command; // fallback

	TDEProcess p;
	p.setUseShell(true);
	p << execLine;
	p.start(TDEProcess::DontCare);
	return true;
    }
    return false;
}


bool KNotify::notifyByLogfile(const TQString &text, const TQString &file)
{
    // ignore empty messages
    if ( text.isEmpty() )
        return true;

    // open file in append mode
    TQFile logFile(file);
    if ( !logFile.open(IO_WriteOnly | IO_Append) )
        return false;

    // append msg
    TQTextStream strm( &logFile );
    strm << "- KNotify " << TQDateTime::currentDateTime().toString() << ": ";
    strm << text << endl;

    // close file
    logFile.close();
    return true;
}

bool KNotify::notifyByStderr(const TQString &text)
{
    // ignore empty messages
    if ( text.isEmpty() )
        return true;

    // open stderr for output
    TQTextStream strm( stderr, IO_WriteOnly );

    // output msg
    strm << "KNotify " << TQDateTime::currentDateTime().toString() << ": ";
    strm << text << endl;

    return true;
}

bool KNotify::notifyByTaskbar( WId win )
{
    if( win == 0 )
        return false;
    KWin::demandAttention( win );
    return true;
}

bool KNotify::isGlobal(const TQString &eventname)
{
    return d->globalEvents->hasGroup( eventname );
}

void KNotify::setVolume( int volume )
{
    if ( volume<0 ) volume=0;
    if ( volume>=100 ) volume=100;
    d->volume = volume;
}

void KNotify::playTimeout()
{
#ifndef WITHOUT_ARTS
    for ( TQPtrListIterator< KDE::PlayObject > it(d->playObjects); *it;)
    {
        TQPtrListIterator< KDE::PlayObject > current = it;
        ++it;
        if ( (*current)->state() != Arts::posPlaying )
        {
            TQMap<KDE::PlayObject*,int>::Iterator eit = d->playObjectEventMap.find( *current );
            if ( eit != d->playObjectEventMap.end() )
            {
                soundFinished( *eit, PlayedOK );
                d->playObjectEventMap.remove( eit );
            }
            d->playObjects.remove( current );
        }
    }
    if ( !d->playObjects.count() )
        d->playTimer->stop();
#endif
}

bool KNotify::isPlaying( const TQString& soundFile ) const
{
#ifndef WITHOUT_ARTS
    for ( TQPtrListIterator< KDE::PlayObject > it(d->playObjects); *it; ++it)
    {
        if ( (*it)->mediaName() == soundFile )
            return true;
    }
#endif
    return false;
}

void KNotify::slotPlayerProcessExited( TDEProcess *proc )
{
    soundFinished( d->externalPlayerEventId,
                   (proc->normalExit() && proc->exitStatus() == 0) ? PlayedOK : Unknown );
}

void KNotify::abortFirstPlayObject()
{
#ifndef WITHOUT_ARTS
    TQMap<KDE::PlayObject*,int>::Iterator it = d->playObjectEventMap.find( d->playObjects.getFirst() );
    if ( it != d->playObjectEventMap.end() )
    {
        soundFinished( it.data(), Aborted );
        d->playObjectEventMap.remove( it );
    }
    d->playObjects.removeFirst();
#endif
}

void KNotify::soundFinished( int eventId, PlayingFinishedStatus reason )
{
    TQByteArray data;
    TQDataStream stream( data, IO_WriteOnly );
    stream << eventId << (int) reason;

    DCOPClient::mainClient()->emitDCOPSignal( "KNotify", "playingFinished(int,int)", data );
}

WId KNotify::checkWinId( const TQString &appName, WId senderWinId )
{
    if ( senderWinId == 0 )
    {
        TQCString senderId = kapp->dcopClient()->senderId();
        TQCString compare = (appName + "-mainwindow").latin1();
        int len = compare.length();
        // kdDebug() << "notifyByPassivePopup: appName=" << appName << " sender=" << senderId << endl;

        QCStringList objs = kapp->dcopClient()->remoteObjects( senderId );
        for (QCStringList::ConstIterator it = objs.begin(); it != objs.end(); ++it ) {
            TQCString obj( *it );
            if ( obj.left(len) == compare) {
                // kdDebug( ) << "found " << obj << endl;
                TQCString replyType;
                TQByteArray data, replyData;

                if ( kapp->dcopClient()->call(senderId, obj, "getWinID()", data, replyType, replyData) ) {
                    TQDataStream answer(replyData, IO_ReadOnly);
                    if (replyType == "int") {
                        answer >> senderWinId;
                        // kdDebug() << "SUCCESS, found getWinID(): type='" << TQString(replyType)
                        //      << "' senderWinId=" << senderWinId << endl;
                    }
		}
            }
        }
    }
    return senderWinId;
}

void KNotify::restartedArtsd()
{
#ifndef WITHOUT_ARTS
    delete d->audioManager;
    d->audioManager = new KAudioManagerPlay( soundServer );
    d->audioManager->setTitle( i18n( "Trinity System Notifications" ) );
    d->audioManager->setAutoRestoreID( "KNotify Aman Play" );
#endif
}

void KNotify::sessionReady()
{
    if( d->inStartup && !d->startupEvents.isEmpty())
        kdDebug() << "There were knotify events while startup:" << d->startupEvents << endl;
    d->inStartup = false;
}

// vim: sw=4 sts=4 ts=8 et
