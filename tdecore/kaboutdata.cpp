/*
 * This file is part of the KDE Libraries
 * Copyright (C) 2000 Espen Sand (espen@kde.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */


#include <kaboutdata.h>
#include <kstandarddirs.h>
#include <tqfile.h>
#include <tqtextstream.h>

TQString
KAboutPerson::name() const
{
   return TQString::fromUtf8(mName);
}

TQString
KAboutPerson::task() const
{
   if (mTask && *mTask)
      return i18n(mTask);
   else
      return TQString::null;
}

TQString
KAboutPerson::emailAddress() const
{
   return TQString::fromUtf8(mEmailAddress);
}


TQString
KAboutPerson::webAddress() const
{
   return TQString::fromUtf8(mWebAddress);
}


KAboutTranslator::KAboutTranslator(const TQString & name,
                const TQString & emailAddress)
{
    mName=name;
    mEmail=emailAddress;
}

TQString KAboutTranslator::name() const
{
    return mName;
}

TQString KAboutTranslator::emailAddress() const
{
    return mEmail;
}

class KAboutDataPrivate
{
public:
    KAboutDataPrivate()
        : translatorName("_: NAME OF TRANSLATORS\nYour names")
        , translatorEmail("_: EMAIL OF TRANSLATORS\nYour emails")
        , productName(0)
        , programLogo(0)
        , customAuthorTextEnabled(false)
        , mTranslatedProgramName( 0 )
        {}
    ~KAboutDataPrivate()
        {
             delete programLogo;
             delete[] mTranslatedProgramName;
        }
    const char *translatorName;
    const char *translatorEmail;
    const char *productName;
    TQImage* programLogo;
    TQString customAuthorPlainText, customAuthorRichText;
    bool customAuthorTextEnabled;
    const char *mTranslatedProgramName;
};

static const char *KAboutData::defaultBugTracker = "http://bugs.trinitydesktop.org";

KAboutData::KAboutData( const char *appName,
                        const char *programName,
                        const char *version,
                        const char *shortDescription,
			int licenseType,
			const char *copyrightStatement,
			const char *text,
			const char *homePageAddress,
			const char *bugsEmailAddress
			) :
  mProgramName( programName ),
  mVersion( version ),
  mShortDescription( shortDescription ),
  mLicenseKey( licenseType ),
  mCopyrightStatement( copyrightStatement ),
  mOtherText( text ),
  mHomepageAddress( homePageAddress ),
  mBugEmailAddress( (bugsEmailAddress!=0)?bugsEmailAddress:&defaultBugTracker ),
  mLicenseText (0)
{
   d = new KAboutDataPrivate;

   if( appName ) {
     const char *p = strrchr(appName, '/');
     if( p )
	 mAppName = p+1;
     else
	 mAppName = appName;
   } else
     mAppName = 0;
}

KAboutData::~KAboutData()
{
    if (mLicenseKey == License_File)
        delete [] mLicenseText;
    delete d;
}

void
KAboutData::addAuthor( const char *name, const char *task,
		    const char *emailAddress, const char *webAddress )
{
  mAuthorList.append(KAboutPerson(name,task,emailAddress,webAddress));
}

void
KAboutData::addCredit( const char *name, const char *task,
		    const char *emailAddress, const char *webAddress )
{
  mCreditList.append(KAboutPerson(name,task,emailAddress,webAddress));
}

void
KAboutData::setTranslator( const char *name, const char *emailAddress)
{
  d->translatorName=name;
  d->translatorEmail=emailAddress;
}

void
KAboutData::setLicenseText( const char *licenseText )
{
  mLicenseText = licenseText;
  mLicenseKey = License_Custom;
}

void
KAboutData::setLicenseTextFile( const TQString &file )
{
  mLicenseText = qstrdup(TQFile::encodeName(file));
  mLicenseKey = License_File;
}

void
KAboutData::setAppName( const char *appName )
{
  mAppName = appName;
}

void
KAboutData::setProgramName( const char* programName )
{
  mProgramName = programName;
  translateInternalProgramName();
}

void
KAboutData::setVersion( const char* version )
{
  mVersion = version;
}

void
KAboutData::setShortDescription( const char *shortDescription )
{
  mShortDescription = shortDescription;
}

void
KAboutData::setLicense( LicenseKey licenseKey)
{
  mLicenseKey = licenseKey;
}

void
KAboutData::setCopyrightStatement( const char *copyrightStatement )
{
  mCopyrightStatement = copyrightStatement;
}

void
KAboutData::setOtherText( const char *otherText )
{
  mOtherText = otherText;
}

void
KAboutData::setHomepage( const char *homepage )
{
  mHomepageAddress = homepage;
}

void
KAboutData::setBugAddress( const char *bugAddress )
{
  mBugEmailAddress = bugAddress;
}

void
KAboutData::setProductName( const char *productName )
{
  d->productName = productName;
}

const char *
KAboutData::appName() const
{
   return mAppName;
}

const char *
KAboutData::productName() const
{
   if (d->productName)
      return d->productName;
   else
      return appName();
}

TQString
KAboutData::programName() const
{
   if (mProgramName && *mProgramName)
      return i18n(mProgramName);
   else
      return TQString::null;
}

const char*
KAboutData::internalProgramName() const
{
   if (d->mTranslatedProgramName)
      return d->mTranslatedProgramName;
   else
      return mProgramName;
}

// KCrash should call as few things as possible and should avoid e.g. malloc()
// because it may deadlock. Since i18n() needs it, when KLocale is available
// the i18n() call will be done here in advance.
void
KAboutData::translateInternalProgramName() const
{
  delete[] d->mTranslatedProgramName;
  d->mTranslatedProgramName = 0;
  if( KGlobal::locale() )
      d->mTranslatedProgramName = qstrdup( programName().utf8());
}

TQImage
KAboutData::programLogo() const
{
    return d->programLogo ? (*d->programLogo) : TQImage();
}

void
KAboutData::setProgramLogo(const TQImage& image)
{
    if (!d->programLogo)
       d->programLogo = new TQImage( image );
    else
       *d->programLogo = image;
}

TQString
KAboutData::version() const
{
   return TQString::fromLatin1(mVersion);
}

TQString
KAboutData::shortDescription() const
{
   if (mShortDescription && *mShortDescription)
      return i18n(mShortDescription);
   else
      return TQString::null;
}

TQString
KAboutData::homepage() const
{
   return TQString::fromLatin1(mHomepageAddress);
}

TQString
KAboutData::bugAddress() const
{
   return TQString::fromLatin1(mBugEmailAddress);
}

const TQValueList<KAboutPerson>
KAboutData::authors() const
{
   return mAuthorList;
}

const TQValueList<KAboutPerson>
KAboutData::credits() const
{
   return mCreditList;
}

const TQValueList<KAboutTranslator>
KAboutData::translators() const
{
    TQValueList<KAboutTranslator> personList;

    if(d->translatorName == 0)
        return personList;

    TQStringList nameList;
    TQStringList emailList;

    TQString names = i18n(d->translatorName);
    if(names != TQString::fromUtf8(d->translatorName))
    {
        nameList = TQStringList::split(',',names);
    }


    if(d->translatorEmail)
    {
        TQString emails = i18n(d->translatorEmail);

        if(emails != TQString::fromUtf8(d->translatorEmail))
        {
            emailList = TQStringList::split(',',emails,true);
        }
    }


    TQStringList::Iterator nit;
    TQStringList::Iterator eit=emailList.begin();

    for(nit = nameList.begin(); nit != nameList.end(); ++nit)
    {
        TQString email;
        if(eit != emailList.end())
        {
            email=*eit;
            ++eit;
        }

        TQString name=*nit;

        personList.append(KAboutTranslator(name.stripWhiteSpace(), email.stripWhiteSpace()));
    }

    return personList;
}

TQString
KAboutData::aboutTranslationTeam()
{
    return i18n("replace this with information about your translation team",
            "<p>KDE is translated into many languages thanks to the work "
            "of the translation teams all over the world.</p>"
            "<p>For more information on KDE internationalization "
            "visit <a href=\"http://l10n.kde.org\">http://l10n.kde.org</a></p>"
            );
}

TQString
KAboutData::otherText() const
{
   if (mOtherText && *mOtherText)
      return i18n(mOtherText);
   else
      return TQString::null;
}


TQString
KAboutData::license() const
{
  TQString result;
  if (!copyrightStatement().isEmpty())
    result = copyrightStatement() + "\n\n";

  TQString l;
  TQString f;
  switch ( mLicenseKey )
  {
    case License_File:
       f = TQFile::decodeName(mLicenseText);
       break;
    case License_GPL_V2:
       l = "GPL v2";
       f = locate("data", "LICENSES/GPL_V2");
       break;
    case License_LGPL_V2:
       l = "LGPL v2";
       f = locate("data", "LICENSES/LGPL_V2");
       break;
    case License_BSD:
       l = "BSD License";
       f = locate("data", "LICENSES/BSD");
       break;
    case License_Artistic:
       l = "Artistic License";
       f = locate("data", "LICENSES/ARTISTIC");
       break;
    case License_QPL_V1_0:
       l = "QPL v1.0";
       f = locate("data", "LICENSES/QPL_V1.0");
       break;
    case License_Custom:
       if (mLicenseText && *mLicenseText)
          return( i18n(mLicenseText) );
       // fall through
    default:
       result += i18n("No licensing terms for this program have been specified.\n"
                   "Please check the documentation or the source for any\n"
                   "licensing terms.\n");
       return result;
      }

  if (!l.isEmpty())
     result += i18n("This program is distributed under the terms of the %1.").arg( l );

  if (!f.isEmpty())
  {
     TQFile file(f);
     if (file.open(IO_ReadOnly))
     {
        result += '\n';
        result += '\n';
        TQTextStream str(&file);
        result += str.read();
     }
  }

  return result;
}

TQString
KAboutData::copyrightStatement() const
{
  if (mCopyrightStatement && *mCopyrightStatement)
     return i18n(mCopyrightStatement);
  else
     return TQString::null;
}

TQString
KAboutData::customAuthorPlainText() const
{
  return d->customAuthorPlainText;
}

TQString
KAboutData::customAuthorRichText() const
{
  return d->customAuthorRichText;
}

bool
KAboutData::customAuthorTextEnabled() const
{
  return d->customAuthorTextEnabled;
}
    
void
KAboutData::setCustomAuthorText(const TQString &plainText, const TQString &richText)
{
  d->customAuthorPlainText = plainText;
  d->customAuthorRichText = richText;

  d->customAuthorTextEnabled = true;
}
    
void
KAboutData::unsetCustomAuthorText()
{
  d->customAuthorPlainText = TQString::null;
  d->customAuthorRichText = TQString::null;

  d->customAuthorTextEnabled = false;
}

