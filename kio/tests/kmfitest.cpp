#include <stdio.h>
#include <tqdir.h>
#include <tqfile.h>
#include <kinstance.h>
#include <kurl.h>
#include <kfilemetainfo.h>
#include <kmimetype.h>

int main (int argc, char **argv)
{
	KInstance ins("kmfitest");

	if (argc < 2) {
		printf("usage: %s <file>\n", argv[0]);
		return 1;
	}

	for (int i = 1; i < argc; i++) { 
		TQString file = TQFile::decodeName(argv[i]);
		qWarning("File: %s", file.local8Bit().data());
		KMimeType::Ptr p;
		p = KMimeType::findByPath(file);
		qWarning("Mime type (findByPath): %s", p->name().latin1());
		KFileMetaInfo meta(file, TQString::null, KFileMetaInfo::TechnicalInfo | KFileMetaInfo::ContentInfo);
	}

	return 0;
}
