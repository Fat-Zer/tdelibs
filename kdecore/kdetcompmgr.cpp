/***************************************************************************
 *   Copyright (C) 2011 Timothy Pearson <kb9vqf@pearsoncomputing.net>      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
#include <kdebug.h>
#include <kconfig.h>

#include <pwd.h>
#include <signal.h>

static const char description[] =
    I18N_NOOP("TDE composition manager detection utility");

static const char version[] = "0.1";

static KCmdLineOptions options[] =
{
    KCmdLineLastOption
};

int main(int argc, char **argv)
{
    KAboutData about("kdetcompmgr", I18N_NOOP("kdetcompmgr"), version, description,
		     KAboutData::License_GPL, "(C) 2011 Timothy Pearson", 0, 0, "kb9vqf@pearsoncomputing.net");
    about.addAuthor( "Timothy Pearson", 0, "kb9vqf@pearsoncomputing.net" );
    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions( options );

    KApplication app;

    KConfig config("kwinrc", true);
    config.setGroup( "Notification Messages" );
    if (!config.readBoolEntry("UseTranslucency",false)) {
        // Attempt to load the kompmgr pid file
        const char *home;
        struct passwd *p;
        p = getpwuid(getuid());
        if (p)
            home = p->pw_dir;
        else
            home = getenv("HOME");
        char *filename;
        const char *configfile = "/.kompmgr.pid";
        int n = strlen(home)+strlen(configfile)+1;
        filename = (char*)malloc(n*sizeof(char));
        memset(filename,0,n);
        strcat(filename, home);
        strcat(filename, configfile);

        printf("reading '%s' as kompmgr pidfile\n\n", filename);

        // Now that we did all that by way of introduction...read the file!
        FILE *pFile;
        char buffer[255];
        pFile = fopen(filename, "r");
        int kompmgrpid = 0;
        if (pFile) {
            // obtain file size
            fseek (pFile , 0 , SEEK_END);
            unsigned long lSize = ftell (pFile);
            if (lSize > 254)
                lSize = 254;
            rewind (pFile);
            size_t result = fread (buffer, 1, lSize, pFile);
            fclose(pFile);
            kompmgrpid = atoi(buffer);
        }

        free(filename);
        filename = NULL;

        if (kompmgrpid) {
            kill(kompmgrpid, SIGTERM);
        }
    }

    app.detectCompositionManagerAvailable();

//    if (!app.isCompositionManagerAvailable()) {
	KConfig config2("kwinrc", true);
	config2.setGroup( "Notification Messages" );
	if (config2.readBoolEntry("UseTranslucency",false)) {
		app.detectCompositionManagerAvailable(true, true);
		return 2;
	}
	else {
		app.detectCompositionManagerAvailable(true, false);
		return 0;
	}
//    }
}

