#ifndef _KACTIONSHORTCUTLIST_H
#define _KACTIONSHORTCUTLIST_H

#include <kshortcutlist.h>
#include <kaction.h>

//---------------------------------------------------------------------
// class KActionShortcutList
//---------------------------------------------------------------------

class KAccelShortcutListPrivate;
class TDEUI_EXPORT KActionShortcutList : public KShortcutList
{
 public:
	KActionShortcutList( KActionCollection* );
	virtual ~KActionShortcutList();

	virtual uint count() const;
	virtual TQString name( uint index ) const;
	virtual TQString label( uint index ) const;
	virtual TQString whatsThis( uint index ) const;
	virtual const KShortcut& shortcut( uint index ) const;
	virtual const KShortcut& shortcutDefault( uint index ) const;
	virtual bool isConfigurable( uint index ) const;
	virtual bool setShortcut( uint index, const KShortcut& shortcut );

	virtual const KInstance* instance() const;

	virtual TQVariant getOther( Other, uint index ) const;
	virtual bool setOther( Other, uint index, TQVariant );

	virtual bool save() const;

	const KAction *action( uint ) const;

 protected:
	KActionCollection& m_actions;

 protected:
        virtual void virtual_hook( int id, void* data );
 private:
	KAccelShortcutListPrivate* d;
};

//---------------------------------------------------------------------
// class KActionPtrShortcutList
//---------------------------------------------------------------------

class KAccelShortcutListPrivate;
class TDEUI_EXPORT KActionPtrShortcutList : public KShortcutList
{
 public:
	KActionPtrShortcutList( KActionPtrList& );
	virtual ~KActionPtrShortcutList();

	virtual uint count() const;
	virtual TQString name( uint index ) const;
	virtual TQString label( uint index ) const;
	virtual TQString whatsThis( uint index ) const;
	virtual const KShortcut& shortcut( uint index ) const;
	virtual const KShortcut& shortcutDefault( uint index ) const;
	virtual bool isConfigurable( uint index ) const;
	virtual bool setShortcut( uint index, const KShortcut& shortcut);

	virtual TQVariant getOther( Other, uint index ) const;
	virtual bool setOther( Other, uint index, TQVariant );

	virtual bool save() const;

 protected:
	KActionPtrList& m_actions;

 protected:
       virtual void virtual_hook( int id, void* data );
 private:
	KAccelShortcutListPrivate* d;
};

#endif // !_KACTIONSHORTCUTLIST_H