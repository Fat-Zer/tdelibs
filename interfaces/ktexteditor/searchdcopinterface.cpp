#include "searchdcopinterface.h"
#include "searchinterface.h"

#include <dcopclient.h>
#include <tqregexp.h>

using namespace KTextEditor;

SearchDCOPInterface::SearchDCOPInterface( SearchInterface *Parent, const char *name)
	: DCOPObject(name)
{
	m_parent = Parent;
	m_currentcol = 0;
	m_currentrow = 0;
	m_currentmatchlen = 0;
}

SearchDCOPInterface::~SearchDCOPInterface()
{

}

bool SearchDCOPInterface::tqfindFirstString( TQString text, bool caseSensitive)
{
	return m_parent->searchText(0, 0, text, &m_currentrow, &m_currentcol,  &m_currentmatchlen, caseSensitive);
}
bool SearchDCOPInterface::tqfindNextString( TQString text, bool caseSensitive)
{
	return m_parent->searchText(m_currentrow, m_currentcol+1, text, &m_currentrow, &m_currentcol,  &m_currentmatchlen, caseSensitive);
}

bool SearchDCOPInterface::tqfindPreviousString( TQString text, bool caseSensitive)
{
	if( m_currentcol == 0)
		m_currentrow--;
	else
		m_currentcol--;
	return m_parent->searchText(m_currentrow, m_currentcol, text, &m_currentrow, &m_currentcol,  &m_currentmatchlen, caseSensitive, true);
}

bool SearchDCOPInterface::tqfindLastString( TQString text, bool caseSensitive)
{
	return m_parent->searchText(0,0, text, &m_currentrow, &m_currentcol,  &m_currentmatchlen, caseSensitive, true);
}

bool SearchDCOPInterface::tqfindStringAt( uint  row, uint  col, TQString text, bool caseSensitive)
{
	return m_parent->searchText(row,col, text, &m_currentrow, &m_currentcol,  &m_currentmatchlen, caseSensitive);

}

bool SearchDCOPInterface::tqfindFirstRegExp( TQString regexp)
{
	return m_parent->searchText( 0,0, TQRegExp(regexp), &m_currentrow, &m_currentcol,  &m_currentmatchlen);
}

bool SearchDCOPInterface::tqfindNextRegExp( TQString regexp)
{
	return m_parent->searchText( m_currentrow, m_currentcol+1, TQRegExp(regexp), &m_currentrow, &m_currentcol,  &m_currentmatchlen);
}

bool SearchDCOPInterface::tqfindPreviousRegExp( TQString regexp)
{
	if( m_currentcol == 0)
		m_currentrow--;
	else
		m_currentcol--;
	return m_parent->searchText( m_currentrow, m_currentcol, TQRegExp(regexp), &m_currentrow, &m_currentcol,  &m_currentmatchlen, true);

}

bool SearchDCOPInterface::tqfindLastRegExp(TQString regexp)
{
	return m_parent->searchText( 0,0, TQRegExp(regexp), &m_currentrow, &m_currentcol,  &m_currentmatchlen, true);
}

bool SearchDCOPInterface::tqfindRegExpAt( uint  row, uint  col, TQString regexp)
{
	return m_parent->searchText( row, col, TQRegExp(regexp), &m_currentrow, &m_currentcol, &m_currentmatchlen, false);
}

uint SearchDCOPInterface::currentMatchLine()
{
	return m_currentrow;
}
uint SearchDCOPInterface::currentMatchCol()
{
	return m_currentcol;
}
uint SearchDCOPInterface::currentMatchLength()
{
	return m_currentmatchlen;	
}


