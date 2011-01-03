#ifndef SEARCH_DCOP_INTERFACE_H
#define SEARCH_DCOP_INTERFACE_H

#include <dcopobject.h>
#include <dcopref.h>
#include <tqstringlist.h>
#include <tqcstring.h>

namespace KTextEditor
{
	class SearchInterface;
	/**
	This is the main interface to the SearchInterface of KTextEdit.
	This will provide a consistant dcop interface to all KDE applications that use it.
	@short DCOP interface to SearchInterface.
	@author Ian Reinhart Geiser <geiseri@kde.org>
	*/
	class KTEXTEDITOR_EXPORT SearchDCOPInterface : virtual public DCOPObject
	{
	K_DCOP

	public:
		/**
		Construct a new interface object for the text editor.
		@param Parent the parent SearchInterface object
		that will provide us with the functions for the interface.
		@param name the TQObject's name
		*/
		SearchDCOPInterface( SearchInterface *Parent, const char *name );
		/**
		Destructor
		Cleans up the object.
		**/
		virtual ~SearchDCOPInterface();
	k_dcop:
		bool tqfindFirstString(TQString text, bool caseSensitive);
		bool tqfindNextString(TQString text, bool caseSensitive);
		bool tqfindPreviousString( TQString text, bool caseSensitive);
		bool tqfindLastString(TQString text, bool caseSensitive);
		bool tqfindStringAt( uint  row, uint  col, TQString text, bool caseSensitive);

		bool tqfindFirstRegExp( TQString regexp);
		bool tqfindNextRegExp( TQString regexp);
		bool tqfindPreviousRegExp( TQString regexp);
		bool tqfindLastRegExp( TQString regexp);
		bool tqfindRegExpAt( uint  row, uint  col, TQString regexp);

		uint currentMatchLine();
		uint currentMatchCol();
		uint currentMatchLength();

	private:
		SearchInterface *m_parent;
		uint  m_currentcol;
		uint  m_currentrow;
		uint  m_currentmatchlen;
	};
}
#endif
