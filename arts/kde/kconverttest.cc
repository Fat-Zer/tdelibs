#include <stdio.h>

#include <tqfile.h>
#include <tqobject.h>

#include <klocale.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kapplication.h>

#include <flowsystem.h>
#include <kplayobject.h>
#include <kartsdispatcher.h>
#include <kplayobjectfactory.h>
#include <kaudioconverter.h>
#include "kconverttest.moc"

using namespace std;
using namespace Arts;

static KCmdLineOptions options[] =
{
    { "+[URL]", I18N_NOOP("URL to open"), 0 },
    KCmdLineLastOption
};

KConvertTest::KConvertTest()
{
}

void KConvertTest::slotRawStreamStart()
{
//	cout << "[START]\n\n" << endl;
}

void KConvertTest::slotNewBlockSize(long blockSize)
{
	m_blockSize = blockSize;
}

void KConvertTest::slotNewBlockPointer(long blockPointer)
{
	m_blockPointer = blockPointer;
}

void KConvertTest::slotNewData()
{
	fwrite((void *) m_blockPointer, 1, m_blockSize, stdout);
}

void KConvertTest::slotRawStreamFinished()
{
//	cout << "\n\n[END]" << endl;
}

int main(int argc, char **argv)
{
	TDEAboutData aboutData("kconverttest", I18N_NOOP("KConvertTest"), I18N_NOOP("0.1"), "", TDEAboutData::License_GPL, "");
							  
	TDECmdLineArgs::init(argc, argv, &aboutData);
	TDECmdLineArgs::addCmdLineOptions(options); 	
	TDEApplication app;

	TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs();

	KURL url;
	
	if(args->count())
		url = args->arg(0);
	else
		exit(1);

	args->clear();

	KConvertTest *get = new KConvertTest();

	KArtsDispatcher dispatcher;
	KAudioConverter converter;

	// FIXME: crashes
	// converter.setup(44100);
	converter.requestPlayObject(url);

	TQObject::connect(&converter, TQT_SIGNAL(rawStreamStart()), get, TQT_SLOT(slotRawStreamStart()));

	TQObject::connect(&converter, TQT_SIGNAL(newBlockSize(long)), get, TQT_SLOT(slotNewBlockSize(long)));
	TQObject::connect(&converter, TQT_SIGNAL(newBlockPointer(long)), get, TQT_SLOT(slotNewBlockPointer(long)));
	TQObject::connect(&converter, TQT_SIGNAL(newData()), get, TQT_SLOT(slotNewData()));
	
	TQObject::connect(&converter, TQT_SIGNAL(rawStreamFinished()), get, TQT_SLOT(slotRawStreamFinished()));

	converter.start();

	app.exec();
}

