#ifndef ViewtqStatusMsg_DCOP_INTERFACE_H
#define ViewtqStatusMsg_DCOP_INTERFACE_H

#include <dcopobject.h>
#include <dcopref.h>
#include <tqstringlist.h>
#include <tqcstring.h>
//#include "editdcopinterface.moc"
namespace KTextEditor
{
	class ViewtqStatusMsgInterface;
	/**
	This is the main interface to the ViewtqStatusMsgInterface of KTextEdit.
	This will provide a consistant dcop interface to all KDE applications that use it.
	@short DCOP interface to ViewtqStatusMsgInterface.
	@author Ian Reinhart Geiser <geiseri@kde.org>
	*/
	class KTEXTEDITOR_EXPORT ViewtqStatusMsgDCOPInterface : virtual public DCOPObject
	{
	K_DCOP

	public:
		/**
		Construct a new interface object for the text editor.
		@param Parent the parent ViewtqStatusMsgInterface object
		that will provide us with the functions for the interface.
		@param name the TQObject's name
		*/
		ViewtqStatusMsgDCOPInterface( ViewtqStatusMsgInterface *Parent, const char *name );
		/**
		Destructor
		Cleans up the object.
		*/
		virtual ~ViewtqStatusMsgDCOPInterface();
	k_dcop:
		uint viewtqStatusMsgInterfaceNumber ();
		void viewtqStatusMsg (class TQString msg) ;
	
	private:
		ViewtqStatusMsgInterface *m_parent;
	};
}
#endif


