/*
    Tests for TDEConfig Compiler

    Copyright (c) 2005      by Duncan Mac-Vicar       <duncan@kde.org>

    *************************************************************************
    *                                                                       *
    * This library is free software; you can redistribute it and/or         *
    * modify it under the terms of the GNU Lesser General Public            *
    * License as published by the Free Software Foundation; either          *
    * version 2 of the License, or (at your option) any later version.      *
    *                                                                       *
    *************************************************************************
*/

#include <tqfile.h>
#include <tqstring.h>
#include <kdebug.h>
#include <kunittest/module.h>
#include "kconfigcompiler_test.h"

using namespace KUnitTest;

KUNITTEST_MODULE( kunittest_kconfigcompiler_test, "TDEConfigXT")
KUNITTEST_MODULE_REGISTER_TESTER( TDEConfigCompiler_Test )

typedef const char * CompilerTestSet[];

static CompilerTestSet testCases =
{
	"test1.cpp", "test1.h",
	"test2.cpp", "test2.h",
	"test3.cpp", "test3.h",
	"test4.cpp", "test4.h",
	"test5.cpp", "test5.h",
	"test6.cpp", "test6.h",
	"test7.cpp", "test7.h",
	"test8a.cpp", "test8a.h",
	"test8b.cpp", "test8b.h",
	"test9.h", "test9.cpp", 
	"test_dpointer.cpp", "test_dpointer.h",	 
	NULL
};

static CompilerTestSet willFailCases =
{
	// where is that TQDir comming from?
	//"test9.cpp", NULL
	NULL
};


void TDEConfigCompiler_Test::allTests()
{
	testExpectedOutput();
}

void TDEConfigCompiler_Test::testExpectedOutput()
{
	uint i = 0;
	// Known to pass test cases
	while (testCases[ i ])
	{
		performCompare(TQString::fromLatin1(testCases[ i ]));
		++i;
	}

	// broken test cases
	i= 0;
	while (willFailCases[ i ])
	{
		performCompare(TQString::fromLatin1(willFailCases[ i ]), true);
		++i;
	}
}

void TDEConfigCompiler_Test::performCompare(const TQString &fileName, bool fail)
{
	TQFile file(fileName);
	TQFile fileRef(TQString::fromLatin1(SRCDIR) + TQString::fromLatin1("/") + fileName + TQString::fromLatin1(".ref"));
	
	if ( file.open(IO_ReadOnly) && fileRef.open(IO_ReadOnly) )
	{
		TQString content = file.readAll();
		TQString contentRef = fileRef.readAll();
		
		if (!fail)
			CHECK( content, contentRef);
		else
			XFAIL( content, contentRef);
	}
	else
	{
		SKIP("Can't open file for comparision");
	}
}
