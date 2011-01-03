/*****************************************************************
Copyright (c) 1999 Torben Weis <weis@kde.org>
Copyright (c) 2000 Matthias Ettrich <ettrich@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/
#include <tqdom.h>
#include <tqfile.h>
#include <tqtextstream.h>
#include <tqstring.h>
#include <tqstringlist.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "main.h"
#include "type.h"

static int const primes[] =
{
    2,  3,  5,  7, 11, 13, 17, 19, 23, 29,
    31, 37, 41, 43, 47, 53, 59, 61, 67, 71,
    73, 79, 83, 89, 97, 101, 103, 107, 109, 113,
    127, 131, 137, 139, 149, 151, 157, 163, 167, 173,
    179, 181, 191, 193, 197, 199, 211, 223, 227, 229,
    233, 239, 241, 251, 257, 263, 269, 271, 277, 281,
    283, 293, 307, 311, 313, 317, 331, 337, 347, 349,
    353, 359, 367, 373, 379, 383, 389, 397, 401, 409,
    419, 421, 431, 433, 439, 443, 449, 457, 461, 463,
    467, 479, 487, 491, 499, 503, 509, 521, 523, 541,
    547, 557, 563, 569, 571, 577, 587, 593, 599, 601,0
};


struct Function
{
    Function(){}
    Function( const TQString& t, const TQString& n, const TQString&fn, bool h ) 
	: type( t ), name( n ), fullName( fn ), hidden( h ) {}
    TQString type;
    TQString name;
    TQString fullName;
    bool hidden;
};


/*
 * Writes the skeleton
 */
void generateSkel( const TQString& idl, const TQString& filename, TQDomElement de )
{
    TQFile skel( filename );
    if ( !skel.open( IO_WriteOnly ) )
	qFatal("Could not write to %s", filename.local8Bit().data() );

    TQTextStream str( &skel );

    str << "/****************************************************************************" << endl;
    str << "**" << endl;
    str << "** DCOP Skeleton generated by dcopidl2cpp from " << idl << endl;
    str << "**" << endl;
    str << "** WARNING! All changes made in this file will be lost!" << endl;
    str << "**" << endl;
    str << "*****************************************************************************/" << endl;
    str << endl;

    TQDomElement e = de.firstChild().toElement();
    if ( e.tagName() == "SOURCE" ) {
	str << "#include \"" << e.firstChild().toText().data() << "\"" << endl << endl;
    }

    for( ; !e.isNull(); e = e.nextSibling().toElement() ) {
	if ( e.tagName() != "CLASS" )
	    continue;
	TQDomElement n = e.firstChild().toElement();
	Q_ASSERT( n.tagName() == "NAME" );
	TQString className = n.firstChild().toText().data();
	// tqfind dcop parent ( rightmost super class )
	TQString DCOPParent;
	TQDomElement s = n.nextSibling().toElement();
	for( ; !s.isNull(); s = s.nextSibling().toElement() ) {
	    if ( s.tagName() == "SUPER" )
		DCOPParent = s.firstChild().toText().data();
	}
    
	// get function table
	TQValueList<Function> functions;
	s = n.nextSibling().toElement();
	for( ; !s.isNull(); s = s.nextSibling().toElement() ) {
	    if ( s.tagName() != "FUNC" )
		continue;
	    TQDomElement r = s.firstChild().toElement();
	    Q_ASSERT( r.tagName() == "TYPE" );
	    TQString funcType = r.firstChild().toText().data();
	    r = r.nextSibling().toElement();
	    Q_ASSERT ( r.tagName() == "NAME" );
	    TQString funcName = r.firstChild().toText().data();
	    TQStringList argtypes;
	    TQStringList argnames;
	    r = r.nextSibling().toElement();
	    for( ; !r.isNull(); r = r.nextSibling().toElement() ) {
		Q_ASSERT( r.tagName() == "ARG" );
		TQDomElement a = r.firstChild().toElement();
		Q_ASSERT( a.tagName() == "TYPE" );
		argtypes.append( a.firstChild().toText().data() );
		a = a.nextSibling().toElement();
		if ( !a.isNull() ) {
		    Q_ASSERT( a.tagName() == "NAME" );
		    argnames.append( a.firstChild().toText().data() );
		} else {
		    argnames.append( TQString::null );
		}
	    }
	    funcName += '(';
	    TQString fullFuncName = funcName;
	    bool first = true;
	    TQStringList::Iterator ittype = argtypes.begin();
	    TQStringList::Iterator itname = argnames.begin();
	    while ( ittype != argtypes.end() && itname != argnames.end() ) {
		if ( !first ) {
		    funcName += ',';
		    fullFuncName += ',';
		}
		first = false;
		funcName += *ittype;
		fullFuncName += *ittype;
		if ( ! (*itname).isEmpty() ) {
		    fullFuncName += ' ';
		    fullFuncName += *itname;
		}
		++ittype;
		++itname;
	    }
	    funcName += ')';
	    fullFuncName += ')';
	    bool hidden = (s.attribute("hidden") == "yes");
	    functions.append( Function( funcType, funcName, fullFuncName, hidden ) );
	}

	// create static tables
    
	int fhash = functions.count() + 1;
	for ( int i = 0; primes[i]; i++ ) {
	    if ( primes[i] >  static_cast<int>(functions.count()) ) {
		fhash = primes[i];
		break;
	    }
	}
    
	str << "#include <kdatastream.h>" << endl;

	bool useHashing = functions.count() > 7;
	if ( useHashing ) {
	    str << "#include <tqasciidict.h>" << endl;
	}

	TQString classNameFull = className; // class name with possible namespaces prepended
					   // namespaces will be removed from className now
	int namespace_count = 0;
	TQString namespace_tmp = className;
	str << endl;
	for(;;) {
	    int pos = namespace_tmp.tqfind( "::" );
	    if( pos < 0 ) {
		className = namespace_tmp;
		break;
	    }
	    str << "namespace " << namespace_tmp.left( pos ) << " {" << endl;
	    ++namespace_count;
	    namespace_tmp = namespace_tmp.mid( pos + 2 );
	}

	str << endl;

	if ( useHashing ) {
	    str << "static const int " << className << "_fhash = " << fhash << ";" << endl;
	}
	str << "static const char* const " << className << "_ftable[" << functions.count() + 1 << "][3] = {" << endl;
	for( TQValueList<Function>::Iterator it = functions.begin(); it != functions.end(); ++it ){
	    str << "    { \"" << (*it).type << "\", \"" << (*it).name << "\", \"" << (*it).fullName << "\" }," << endl;
	}
	str << "    { 0, 0, 0 }" << endl;
	str << "};" << endl;

	if (functions.count() > 0) {
	    str << "static const int " << className << "_ftable_hiddens[" << functions.count() << "] = {" << endl;
	    for( TQValueList<Function>::Iterator it = functions.begin(); it != functions.end(); ++it ){
		str << "    " << !!(*it).hidden << "," << endl;
	    }
	    str << "};" << endl;
	}
    
	str << endl;
    
    
	// Write dispatcher
	str << "bool " << className;
	str << "::process(const TQCString &fun, const TQByteArray &data, TQCString& replyType, TQByteArray &replyData)" << endl;
	str << "{" << endl;
	if ( useHashing ) {
	    str << "    static TQAsciiDict<int>* fdict = 0;" << endl;
    
	    str << "    if ( !fdict ) {" << endl;
	    str << "\tfdict = new TQAsciiDict<int>( " << className << "_fhash, true, false );" << endl;
	    str << "\tfor ( int i = 0; " << className << "_ftable[i][1]; i++ )" << endl;
	    str << "\t    fdict->insert( " << className << "_ftable[i][1],  new int( i ) );" << endl;
	    str << "    }" << endl;
    
	    str << "    int* fp = fdict->tqfind( fun );" << endl;
	    str << "    switch ( fp?*fp:-1) {" << endl;
	}
	s = n.nextSibling().toElement();
	int fcount = 0; // counter of written functions
	bool firstFunc = true;
	for( ; !s.isNull(); s = s.nextSibling().toElement() ) {
	    if ( s.tagName() != "FUNC" )
		continue;
	    TQDomElement r = s.firstChild().toElement();
	    Q_ASSERT( r.tagName() == "TYPE" );
	    TQString funcType = r.firstChild().toText().data();
	    if ( funcType == "ASYNC" )
		funcType = "void";
	    r = r.nextSibling().toElement();
	    Q_ASSERT ( r.tagName() == "NAME" );
	    TQString funcName = r.firstChild().toText().data();
	    TQStringList args;
	    TQStringList argtypes;
	    r = r.nextSibling().toElement();
	    for( ; !r.isNull(); r = r.nextSibling().toElement() ) {
		Q_ASSERT( r.tagName() == "ARG" );
		TQDomElement a = r.firstChild().toElement();
		Q_ASSERT( a.tagName() == "TYPE" );
		argtypes.append( a.firstChild().toText().data() );
		args.append( TQString("arg" ) + TQString::number( args.count() ) );
	    }
	    TQString plainFuncName = funcName;
	    funcName += '(';
	    bool first = true;
	    for( TQStringList::Iterator argtypes_count = argtypes.begin(); argtypes_count != argtypes.end(); ++argtypes_count ){
		if ( !first )
		    funcName += ',';
		first = false;
		funcName += *argtypes_count;
	    }
	    funcName += ')';
		
	    if ( useHashing ) {
		str << "    case " << fcount << ": { // " << funcType << " " << funcName << endl;
	    } else {
		if ( firstFunc )
		    str << "    if ( fun == " << className << "_ftable[" << fcount << "][1] ) { // " << funcType << " " << funcName << endl;
		else
		    str << " else if ( fun == " << className << "_ftable[" << fcount << "][1] ) { // " << funcType << " " << funcName << endl;
		firstFunc = false;
	    }
	    if ( !args.isEmpty() ) {
		TQStringList::Iterator ittypes = argtypes.begin();
		TQStringList::Iterator args_count;
		for( args_count = args.begin(); args_count != args.end(); ++args_count ){
		    str << '\t'<< *ittypes << " " << *args_count << ";" <<  endl;
		    ++ittypes;
		}
		str << "\tTQDataStream arg( data, IO_ReadOnly );" << endl;
		for( args_count = args.begin(); args_count != args.end(); ++args_count ){
		    str << "\tif (arg.atEnd()) return false;" << endl; // Basic error checking
		    str << "\targ >> " << *args_count << ";" << endl;
		}
	    }

	    str << "\treplyType = " << className << "_ftable[" << fcount++ << "][0]; " << endl;
	    if ( funcType == "void" ) {
		str << '\t' << plainFuncName << '(';
	    } else {
		str << "\tTQDataStream _replyStream( replyData, IO_WriteOnly );"  << endl;
		str << "\t_replyStream << " << plainFuncName << '(';
	    }

	    first = true;
	    for ( TQStringList::Iterator args_count = args.begin(); args_count != args.end(); ++args_count ){
		if ( !first )
		    str << ", ";
		first = false;
		str << *args_count;
	    }
	    str << " );" << endl;
	    if (useHashing ) {
		str << "    } break;" << endl;
	    } else {
		str << "    }";
	    }
	}

	// only open an 'else' clause if there were one or more functions
	if ( fcount > 0 ) {
	    if ( useHashing ) {
		str << "    default: " << endl;
	    } else {
		str << " else {" << endl;
	    }
	}
	
	// if no DCOP function was called, delegate the request to the parent
	if (!DCOPParent.isEmpty()) {
	    str << "\treturn " << DCOPParent << "::process( fun, data, replyType, replyData );" << endl;
	} else {
	    str << "\treturn false;" << endl;
	}

	// only close the 'else' clause and add the default 'return true'
	// (signifying a DCOP method was found and called) if there were
	// one or more functions.
	if ( fcount > 0 ) {
	    str << "    }" << endl;
	    str << "    return true;" << endl;
	}

	// close the 'process' function
	str << "}" << endl << endl;
    
	str << "QCStringList " << className;
	str << "::interfaces()" << endl;
	str << "{" << endl;
	if (!DCOPParent.isEmpty()) {
	    str << "    QCStringList ifaces = " << DCOPParent << "::interfaces();" << endl;
	} else {
	    str << "    QCStringList ifaces;" << endl;
	}
	str << "    ifaces += \"" << classNameFull << "\";" << endl;
	str << "    return ifaces;" << endl;
	str << "}" << endl << endl;
	
	
	str << "QCStringList " << className;
	str << "::functions()" << endl;
	str << "{" << endl;
	if (!DCOPParent.isEmpty()) {
	    str << "    QCStringList funcs = " << DCOPParent << "::functions();" << endl;
	} else {
	    str << "    QCStringList funcs;" << endl;
	}
	str << "    for ( int i = 0; " << className << "_ftable[i][2]; i++ ) {" << endl;
        if (functions.count() > 0) {
	    str << "\tif (" << className << "_ftable_hiddens[i])" << endl;
	    str << "\t    continue;" << endl;
        }
	str << "\tTQCString func = " << className << "_ftable[i][0];" << endl;
	str << "\tfunc += ' ';" << endl;
	str << "\tfunc += " << className << "_ftable[i][2];" << endl;
	str << "\tfuncs << func;" << endl;
	str << "    }" << endl;
	str << "    return funcs;" << endl;
	str << "}" << endl << endl;
	
	// Add signal stubs
	for(s = e.firstChild().toElement(); !s.isNull(); s = s.nextSibling().toElement() ) {
	    if (s.tagName() != "SIGNAL")
		continue;
	    TQDomElement r = s.firstChild().toElement();
	    TQString result = writeType( str, r );

	    r = r.nextSibling().toElement();
	    Q_ASSERT ( r.tagName() == "NAME" );
	    TQString funcName = r.firstChild().toText().data();
	    str << className << "::" << funcName << "(";

	    TQStringList args;
	    TQStringList argtypes;
	    bool first = true;
	    r = r.nextSibling().toElement();
	    for( ; !r.isNull(); r = r.nextSibling().toElement() ) {
		if ( !first )
		    str << ", ";
		else
		    str << " ";
		first = false;
		Q_ASSERT( r.tagName() == "ARG" );
		TQDomElement a = r.firstChild().toElement();
		TQString type = writeType( str, a );
		argtypes.append( type );
		args.append( TQString("arg" ) + TQString::number( args.count() ) ) ;
		str << args.last();
	    }
	    if ( !first )
		str << " ";
	    str << ")";

	    if ( s.hasAttribute("qual") )
		str << " " << s.attribute("qual");
	    str << endl;
	
	    str << "{" << endl ;

	    funcName += "(";
	    first = true;
	    for( TQStringList::Iterator it = argtypes.begin(); it != argtypes.end(); ++it ){
		if ( !first )
		    funcName += ",";
		first = false;
		funcName += *it;
	    }
	    funcName += ")";
	
	    if ( result != "void" )
	       qFatal("Error in DCOP signal %s::%s: DCOP signals can not return values.", className.latin1(), funcName.latin1());
	
	    str << "    TQByteArray data;" << endl;
	    if ( !args.isEmpty() ) {
		str << "    TQDataStream arg( data, IO_WriteOnly );" << endl;
		for( TQStringList::Iterator args_count = args.begin(); args_count != args.end(); ++args_count ){
		    str << "    arg << " << *args_count << ";" << endl;
		}
	    }

	    str << "    emitDCOPSignal( \"" << funcName << "\", data );" << endl;

	    str << "}" << endl << endl;
	    
	}

	for(; namespace_count > 0; --namespace_count )
	    str << "} // namespace" << endl;
	str << endl;
    }
	
    skel.close();
}

// :set expandtab!<RETURN>:set ts=8<RETURN>:set sts=4<RETURN>:set sw=4<RETURN>
