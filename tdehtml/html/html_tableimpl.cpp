/**
 * This file is part of the DOM implementation for KDE.
 *
 * Copyright (C) 1997 Martin Jones (mjones@kde.org)
 *           (C) 1997 Torben Weis (weis@kde.org)
 *           (C) 1998 Waldo Bastian (bastian@kde.org)
 *           (C) 1999-2003 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2003 Apple Computer, Inc.
 *           (C) 2006 Maksim Orlovich (maksim@kde.org)
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
 */

#include "html/html_documentimpl.h"
#include "html/html_tableimpl.h"

#include "dom/dom_exception.h"
#include "dom/dom_node.h"

#include "misc/htmlhashes.h"
#include "tdehtmlview.h"
#include "tdehtml_part.h"

#include "css/cssstyleselector.h"
#include "css/cssproperties.h"
#include "css/cssvalues.h"
#include "css/csshelper.h"

#include "rendering/render_table.h"

#include <kdebug.h>
#include <tdeglobal.h>

using namespace tdehtml;
using namespace DOM;

HTMLTableElementImpl::HTMLTableElementImpl(DocumentImpl *doc)
  : HTMLElementImpl(doc)
{
    rules = None;
    frame = Void;

    padding = 1;

    m_solid = false;

    // reset font color and sizes here, if we don't have strict parse mode.
    // this is 90% compatible to ie and mozilla, and the by way easiest solution...
    // only difference to 100% correct is that in strict mode <font> elements are propagated into tables.
    if ( getDocument()->parseMode() < DocumentImpl::Transitional ) {
        addCSSProperty( CSS_PROP_FONT_SIZE, CSS_VAL_MEDIUM );
        addCSSProperty( CSS_PROP_COLOR, CSS_VAL__TDEHTML_TEXT );
    }
}

HTMLTableElementImpl::~HTMLTableElementImpl()
{
}

NodeImpl::Id HTMLTableElementImpl::id() const
{
    return ID_TABLE;
}

NodeImpl* HTMLTableElementImpl::setCaption( HTMLTableCaptionElementImpl *c )
{
    int exceptioncode = 0;
    NodeImpl* r;
    if(ElementImpl* cap = caption()) {
        replaceChild ( c, cap, exceptioncode );
        r = c;
    }
    else
        r = insertBefore( c, firstChild(), exceptioncode );
    tCaption = c;
    return r;
}

NodeImpl* HTMLTableElementImpl::setTHead( HTMLTableSectionElementImpl *s )
{
    int exceptioncode = 0;
    NodeImpl* r;
    if( ElementImpl* head = tHead() ) {
        replaceChild( s, head, exceptioncode );
        r = s;
    }
    else if(ElementImpl* foot = tFoot())
        r = insertBefore( s, foot, exceptioncode );
    else if(ElementImpl* firstBody = tFirstBody())
        r = insertBefore( s, firstBody, exceptioncode );
    else
        r = appendChild( s, exceptioncode );

    head = s;
    return r;
}

NodeImpl* HTMLTableElementImpl::setTFoot( HTMLTableSectionElementImpl *s )
{
    int exceptioncode = 0;
    NodeImpl* r;
    if(ElementImpl* foot = tFoot()) {
        replaceChild ( s, foot, exceptioncode );
        r = s;
    } else if(ElementImpl* firstBody = tFirstBody())
        r = insertBefore( s, firstBody, exceptioncode );
    else
        r = appendChild( s, exceptioncode );
    foot = s;
    return r;
}

NodeImpl* HTMLTableElementImpl::setTBody( HTMLTableSectionElementImpl *s )
{
    int exceptioncode = 0;
    NodeImpl* r;

    if(ElementImpl* firstBody = tFirstBody()) {
        replaceChild ( s, firstBody, exceptioncode );
        r = s;
    } else
        r = appendChild( s, exceptioncode );
    firstBody = s;

    return r;
}

HTMLElementImpl *HTMLTableElementImpl::createTHead(  )
{
    if(!tHead())
    {
        int exceptioncode = 0;
        ElementImpl* head = new HTMLTableSectionElementImpl(docPtr(), ID_THEAD, true /* implicit */);
        if(ElementImpl* foot = tFoot())
            insertBefore( head, foot, exceptioncode );
        else if(ElementImpl* firstBody = tFirstBody())
            insertBefore( head, firstBody, exceptioncode);
        else
            appendChild(head, exceptioncode);
    }
    return tHead();
}

void HTMLTableElementImpl::deleteTHead(  )
{
    if(ElementImpl* head = tHead()) {
        int exceptioncode = 0;
        removeChild(head, exceptioncode);
    }
}

HTMLElementImpl *HTMLTableElementImpl::createTFoot(  )
{
    if(!tFoot())
    {
        int exceptioncode = 0;
        ElementImpl* foot = new HTMLTableSectionElementImpl(docPtr(), ID_TFOOT, true /*implicit */);
        if(ElementImpl* firstBody = tFirstBody())
            insertBefore( foot, firstBody, exceptioncode );
        else
            appendChild(foot, exceptioncode);
    }
    return tFoot();
}

void HTMLTableElementImpl::deleteTFoot(  )
{
    if(ElementImpl* foot = tFoot()) {
        int exceptioncode = 0;
        removeChild(foot, exceptioncode);
    }
}

HTMLElementImpl *HTMLTableElementImpl::createCaption(  )
{
    if(!caption())
    {
        int exceptioncode = 0;
        ElementImpl* tCaption = new HTMLTableCaptionElementImpl(docPtr());
        insertBefore( tCaption, firstChild(), exceptioncode );
    }
    return caption();
}

void HTMLTableElementImpl::deleteCaption(  )
{
    if(ElementImpl* tCaption = caption()) {
        int exceptioncode = 0;
        removeChild(tCaption, exceptioncode);
    }
}

/**
 Helper. This checks whether the section contains the desired index, and if so,
 returns the section. Otherwise, it adjust the index, and returns 0.
 indeces < 0 are considered to be infinite.

 lastSection is adjusted to reflect the parameter passed in.
*/
static inline HTMLTableSectionElementImpl* processSection(HTMLTableSectionElementImpl* section,
                                HTMLTableSectionElementImpl* &lastSection, long& index)
{
    lastSection = section;
    if ( index < 0 ) //append/last mode
        return 0;
    
    long rows   = section->numRows();
    if ( index >= rows ) {
        section =  0;
        index   -= rows;
    }
    return section;
}


bool HTMLTableElementImpl::findRowSection(long index,
                        HTMLTableSectionElementImpl*& outSection,
                        long&                         outIndex) const
{
    HTMLTableSectionElementImpl* foot = tFoot();
    HTMLTableSectionElementImpl* head = tHead();

    HTMLTableSectionElementImpl* section = 0L;
    HTMLTableSectionElementImpl* lastSection = 0L;

    if ( head )
        section = processSection( head, lastSection, index );

    if ( !section ) {
        for ( NodeImpl *node = firstChild(); node; node = node->nextSibling() ) {
            if ( ( node->id() == ID_THEAD || node->id() == ID_TFOOT || node->id() == ID_TBODY ) &&
                   node != foot && node != head ) {
                section = processSection( static_cast<HTMLTableSectionElementImpl *>(node),
                        lastSection, index );
                if ( section )
                    break;
            }
        }
    }

    if ( !section && foot )
        section = processSection( foot, lastSection, index );

    outIndex = index;
    if ( section ) {
        outSection = section;
        return true;
    } else {
        outSection = lastSection;
        return false;
    }
}

HTMLElementImpl *HTMLTableElementImpl::insertRow( long index, int &exceptioncode )
{
    // The DOM requires that we create a tbody if the table is empty
    // (cf DOM2TS HTMLTableElement31 test). This includes even the cases where
    // there are <tr>'s immediately under the table, as they're essentially
    // ignored by these functions.
    HTMLTableSectionElementImpl* foot = tFoot();
    HTMLTableSectionElementImpl* head = tHead();
    if(!tFirstBody() && !foot && !head)
        setTBody( new HTMLTableSectionElementImpl(docPtr(), ID_TBODY, true /* implicit */) );

    //kdDebug(6030) << k_funcinfo << index << endl;

    long sectionIndex;
    HTMLTableSectionElementImpl* section;
    if ( findRowSection( index, section, sectionIndex ) )
        return section->insertRow( sectionIndex, exceptioncode );
    else if ( index == -1 || sectionIndex == 0 )
        return section->insertRow( section->numRows(), exceptioncode );

    // The index is too big.
    exceptioncode = DOMException::INDEX_SIZE_ERR;
    return 0L;
}

void HTMLTableElementImpl::deleteRow( long index, int &exceptioncode )
{
    long sectionIndex;
    HTMLTableSectionElementImpl* section;
    if ( findRowSection( index, section, sectionIndex ) )
        section->deleteRow( sectionIndex, exceptioncode );
    else if ( section && index == -1 )
        section->deleteRow( -1, exceptioncode );
    else
        exceptioncode = DOMException::INDEX_SIZE_ERR;
}

NodeImpl *HTMLTableElementImpl::appendChild(NodeImpl *child, int &exceptioncode)
{
     NodeImpl* retval = HTMLElementImpl::appendChild( child, exceptioncode );
     if(retval)
         handleChildAppend( child );
     return retval;
}

void HTMLTableElementImpl::handleChildAdd( NodeImpl *child )
{
    if (!child) return;
    switch(child->id()) {
    case ID_CAPTION:
        tCaption.childAdded(this, child);
        break;
    case ID_THEAD:
        head.childAdded(this, child);
        break;
    case ID_TFOOT:
        foot.childAdded(this, child);
        break;
    case ID_TBODY:
        firstBody.childAdded(this, child);
        break;
    }
}

void HTMLTableElementImpl::handleChildAppend( NodeImpl *child )
{
    if (!child) return;
    switch(child->id()) {
    case ID_CAPTION:
        tCaption.childAppended(child);
        break;
    case ID_THEAD:
        head.childAppended(child);
        break;
    case ID_TFOOT:
        foot.childAppended(child);
        break;
    case ID_TBODY:
        firstBody.childAppended(child);
        break;
    }
}

void HTMLTableElementImpl::handleChildRemove( NodeImpl *child )
{
    if (!child) return;
    switch(child->id()) {
    case ID_CAPTION:
        tCaption.childRemoved(this, child);
        break;
    case ID_THEAD:
        head.childRemoved(this, child);
        break;
    case ID_TFOOT:
        foot.childRemoved(this, child);
        break;
    case ID_TBODY:
        firstBody.childRemoved(this, child);
        break;
    }
}

NodeImpl *HTMLTableElementImpl::addChild(NodeImpl *child)
{
#ifdef DEBUG_LAYOUT
    kdDebug( 6030 ) << nodeName().string() << "(Table)::addChild( " << child->nodeName().string() << " )" << endl;
#endif

    NodeImpl *retval = HTMLElementImpl::addChild( child );
    if ( retval )
        handleChildAppend( child );

    return retval;
}

NodeImpl *HTMLTableElementImpl::insertBefore ( NodeImpl *newChild, NodeImpl *refChild, int &exceptioncode )
{
    NodeImpl* retval = HTMLElementImpl::insertBefore( newChild, refChild, exceptioncode);
    if (retval)
        handleChildAdd( newChild );

    return retval;
}

void HTMLTableElementImpl::replaceChild ( NodeImpl *newChild, NodeImpl *oldChild, int &exceptioncode )
{
    handleChildRemove( oldChild ); //Always safe.
    HTMLElementImpl::replaceChild( newChild, oldChild, exceptioncode );
    if ( !exceptioncode )
        handleChildAdd( newChild );
}

void HTMLTableElementImpl::removeChild ( NodeImpl *oldChild, int &exceptioncode )
{
    handleChildRemove( oldChild );
    HTMLElementImpl::removeChild( oldChild, exceptioncode);
}

void HTMLTableElementImpl::parseAttribute(AttributeImpl *attr)
{
    // ### to CSS!!
    switch(attr->id())
    {
    case ATTR_WIDTH:
        if (!attr->value().isEmpty())
            addCSSLength( CSS_PROP_WIDTH, attr->value() );
        else
            removeCSSProperty(CSS_PROP_WIDTH);
        break;
    case ATTR_HEIGHT:
        if (!attr->value().isEmpty())
            addCSSLength(CSS_PROP_HEIGHT, attr->value());
        else
            removeCSSProperty(CSS_PROP_HEIGHT);
        break;
    case ATTR_BORDER:
    {
        int border;
        // ### this needs more work, as the border value is not only
        //     the border of the box, but also between the cells
        if(!attr->val())
            border = 0;
        else if(attr->val()->l == 0)
            border = 1;
        else
            border = attr->val()->toInt();
#ifdef DEBUG_DRAW_BORDER
        border=1;
#endif
        DOMString v = TQString::number( border );
        addCSSLength(CSS_PROP_BORDER_WIDTH, v );

        // wanted by HTML4 specs
        if(!border)
            frame = Void, rules = None;
        else
            frame = Box, rules = All;


        if (attached()) updateFrame();
        break;
    }
    case ATTR_BGCOLOR:
        if (!attr->value().isEmpty())
            addHTMLColor(CSS_PROP_BACKGROUND_COLOR, attr->value());
        else
            removeCSSProperty(CSS_PROP_BACKGROUND_COLOR);
        break;
    case ATTR_BORDERCOLOR:
        if(!attr->value().isEmpty()) {
            addHTMLColor(CSS_PROP_BORDER_COLOR, attr->value());
            m_solid = true;
        }

        if (attached()) updateFrame();
        break;
    case ATTR_BACKGROUND:
    {
        if (!attr->value().isEmpty()) {
            TQString url = tdehtml::parseURL( attr->value() ).string();
            url = getDocument()->completeURL( url );
            addCSSProperty(CSS_PROP_BACKGROUND_IMAGE, "url('"+url+"')" );
        }
        else
            removeCSSProperty(CSS_PROP_BACKGROUND_IMAGE);
        break;
    }
    case ATTR_FRAME:

        if ( strcasecmp( attr->value(), "void" ) == 0 )
            frame = Void;
        else if ( strcasecmp( attr->value(), "border" ) == 0 )
            frame = Box;
        else if ( strcasecmp( attr->value(), "box" ) == 0 )
            frame = Box;
        else if ( strcasecmp( attr->value(), "hsides" ) == 0 )
            frame = Hsides;
        else if ( strcasecmp( attr->value(), "vsides" ) == 0 )
            frame = Vsides;
        else if ( strcasecmp( attr->value(), "above" ) == 0 )
            frame = Above;
        else if ( strcasecmp( attr->value(), "below" ) == 0 )
            frame = Below;
        else if ( strcasecmp( attr->value(), "lhs" ) == 0 )
            frame = Lhs;
        else if ( strcasecmp( attr->value(), "rhs" ) == 0 )
            frame = Rhs;

        if (attached()) updateFrame();
        break;
    case ATTR_RULES:
        if ( strcasecmp( attr->value(), "none" ) == 0 )
            rules = None;
        else if ( strcasecmp( attr->value(), "groups" ) == 0 )
            rules = Groups;
        else if ( strcasecmp( attr->value(), "rows" ) == 0 )
            rules = Rows;
        else if ( strcasecmp( attr->value(), "cols" ) == 0 )
            rules = Cols;
        else if ( strcasecmp( attr->value(), "all" ) == 0 )
            rules = All;
        if (attached()) updateFrame();
        break;
   case ATTR_CELLSPACING:
        if (!attr->value().isEmpty())
            addCSSLength(CSS_PROP_BORDER_SPACING, attr->value(), true);
        else
            removeCSSProperty(CSS_PROP_BORDER_SPACING);
        break;
    case ATTR_CELLPADDING:
        if (!attr->value().isEmpty())
	    padding = kMax( 0, attr->value().toInt() );
        else
	    padding = 1;
        if (m_render && m_render->isTable()) {
            static_cast<RenderTable *>(m_render)->setCellPadding(padding);
	    if (!m_render->needsLayout())
	        m_render->setNeedsLayout(true);
        }
        break;
    case ATTR_COLS:
    {
        // ###
#if 0
        int c;
        c = attr->val()->toInt();
        addColumns(c-totalCols);
#endif
        break;

    }
    case ATTR_ALIGN:
        setChanged();
        break;
    case ATTR_VALIGN:
        if (!attr->value().isEmpty())
            addCSSProperty(CSS_PROP_VERTICAL_ALIGN, attr->value().lower());
        else
            removeCSSProperty(CSS_PROP_VERTICAL_ALIGN);
        break;
    case ATTR_NOSAVE:
	break;
    default:
        HTMLElementImpl::parseAttribute(attr);
    }
}

void HTMLTableElementImpl::attach()
{
    updateFrame();
    HTMLElementImpl::attach();
    if ( m_render && m_render->isTable() )
        static_cast<RenderTable *>(m_render)->setCellPadding( padding );
}

void HTMLTableElementImpl::close()
{
    ElementImpl* firstBody = tFirstBody();
    if (firstBody && !firstBody->closed())
        firstBody->close();
    HTMLElementImpl::close();
}

void HTMLTableElementImpl::updateFrame()
{
    int v = m_solid ? CSS_VAL_SOLID : CSS_VAL_OUTSET;
    if ( frame & Above )
         addCSSProperty(CSS_PROP_BORDER_TOP_STYLE, v);
    else
        addCSSProperty(CSS_PROP_BORDER_TOP_STYLE, CSS_VAL_NONE);
    if ( frame & Below )
        addCSSProperty(CSS_PROP_BORDER_BOTTOM_STYLE, v);
    else
        addCSSProperty(CSS_PROP_BORDER_BOTTOM_STYLE, CSS_VAL_NONE);
    if ( frame & Lhs )
        addCSSProperty(CSS_PROP_BORDER_LEFT_STYLE, v);
    else
        addCSSProperty(CSS_PROP_BORDER_LEFT_STYLE, CSS_VAL_NONE);
    if ( frame & Rhs )
        addCSSProperty(CSS_PROP_BORDER_RIGHT_STYLE, v);
    else
        addCSSProperty(CSS_PROP_BORDER_RIGHT_STYLE, CSS_VAL_NONE);
}

// --------------------------------------------------------------------------

void HTMLTablePartElementImpl::parseAttribute(AttributeImpl *attr)
{
    switch(attr->id())
    {
    case ATTR_BGCOLOR:
        if (attr->val())
            addHTMLColor(CSS_PROP_BACKGROUND_COLOR, attr->value() );
        else
            removeCSSProperty(CSS_PROP_BACKGROUND_COLOR);
        break;
    case ATTR_BACKGROUND:
    {
        if (attr->val()) {
            TQString url = tdehtml::parseURL( attr->value() ).string();
            url = getDocument()->completeURL( url );
            addCSSProperty(CSS_PROP_BACKGROUND_IMAGE,  "url('"+url+"')" );
        }
        else
            removeCSSProperty(CSS_PROP_BACKGROUND_IMAGE);
        break;
    }
    case ATTR_BORDERCOLOR:
    {
        if(!attr->value().isEmpty()) {
            addHTMLColor(CSS_PROP_BORDER_COLOR, attr->value());
            addCSSProperty(CSS_PROP_BORDER_TOP_STYLE, CSS_VAL_SOLID);
            addCSSProperty(CSS_PROP_BORDER_BOTTOM_STYLE, CSS_VAL_SOLID);
            addCSSProperty(CSS_PROP_BORDER_LEFT_STYLE, CSS_VAL_SOLID);
            addCSSProperty(CSS_PROP_BORDER_RIGHT_STYLE, CSS_VAL_SOLID);
        }
        break;
    }
    case ATTR_ALIGN:
    {
        DOMString v = attr->value();
        if ( strcasecmp( attr->value(), "middle" ) == 0 || strcasecmp( attr->value(), "center" ) == 0 )
            addCSSProperty(CSS_PROP_TEXT_ALIGN, CSS_VAL__TDEHTML_CENTER);
        else if (strcasecmp(attr->value(), "absmiddle") == 0)
            addCSSProperty(CSS_PROP_TEXT_ALIGN, CSS_VAL_CENTER);
        else if (strcasecmp(attr->value(), "left") == 0)
            addCSSProperty(CSS_PROP_TEXT_ALIGN, CSS_VAL__TDEHTML_LEFT);
        else if (strcasecmp(attr->value(), "right") == 0)
            addCSSProperty(CSS_PROP_TEXT_ALIGN, CSS_VAL__TDEHTML_RIGHT);
        else
            addCSSProperty(CSS_PROP_TEXT_ALIGN, v);
        break;
    }
    case ATTR_VALIGN:
    {
        if (!attr->value().isEmpty())
            addCSSProperty(CSS_PROP_VERTICAL_ALIGN, attr->value().lower());
        else
            removeCSSProperty(CSS_PROP_VERTICAL_ALIGN);
        break;
    }
    case ATTR_HEIGHT:
        if (!attr->value().isEmpty())
            addCSSLength(CSS_PROP_HEIGHT, attr->value());
        else
            removeCSSProperty(CSS_PROP_HEIGHT);
        break;
    case ATTR_NOSAVE:
	break;
    default:
        HTMLElementImpl::parseAttribute(attr);
    }
}

// -------------------------------------------------------------------------

HTMLTableSectionElementImpl::HTMLTableSectionElementImpl(DocumentImpl *doc,
                                                         ushort tagid, bool implicit)
    : HTMLTablePartElementImpl(doc)
{
    _id = tagid;
    m_implicit = implicit;
}

HTMLTableSectionElementImpl::~HTMLTableSectionElementImpl()
{
}

NodeImpl::Id HTMLTableSectionElementImpl::id() const
{
    return _id;
}

// these functions are rather slow, since we need to get the row at
// the index... but they aren't used during usual HTML parsing anyway
HTMLElementImpl *HTMLTableSectionElementImpl::insertRow( long index, int& exceptioncode )
{
    HTMLTableRowElementImpl *r = 0L;
    HTMLCollectionImpl rows(const_cast<HTMLTableSectionElementImpl*>(this), HTMLCollectionImpl::TSECTION_ROWS);
    int numRows = rows.length();
    //kdDebug(6030) << k_funcinfo << "index=" << index << " numRows=" << numRows << endl;
    if ( index < -1 || index > numRows ) {
        exceptioncode = DOMException::INDEX_SIZE_ERR; // per the DOM
    }
    else
    {
        r = new HTMLTableRowElementImpl(docPtr());
        if ( numRows == index || index == -1 )
            appendChild(r, exceptioncode);
        else {
            NodeImpl *n;
            if(index < 1)
                n = firstChild();
            else
                n = rows.item(index);
            insertBefore(r, n, exceptioncode );
        }
    }
    return r;
}

void HTMLTableSectionElementImpl::deleteRow( long index, int &exceptioncode )
{
    HTMLCollectionImpl rows(const_cast<HTMLTableSectionElementImpl*>(this), HTMLCollectionImpl::TSECTION_ROWS);
    int numRows = rows.length();
    if ( index == -1 ) index = numRows - 1;
    if( index >= 0 && index < numRows )
        HTMLElementImpl::removeChild(rows.item(index), exceptioncode);
    else
        exceptioncode = DOMException::INDEX_SIZE_ERR;
}


int HTMLTableSectionElementImpl::numRows() const
{
    HTMLCollectionImpl rows(const_cast<HTMLTableSectionElementImpl*>(this), HTMLCollectionImpl::TSECTION_ROWS);
    return rows.length();
}

// -------------------------------------------------------------------------

NodeImpl::Id HTMLTableRowElementImpl::id() const
{
    return ID_TR;
}

long HTMLTableRowElementImpl::rowIndex() const
{
    int rIndex = 0;

    NodeImpl *table = parentNode();
    if ( !table )
	return -1;
    table = table->parentNode();
    if ( !table || table->id() != ID_TABLE )
	return -1;

    HTMLTableSectionElementImpl *head = static_cast<HTMLTableElementImpl *>(table)->tHead();
    HTMLTableSectionElementImpl *foot = static_cast<HTMLTableElementImpl *>(table)->tFoot();

    if ( head ) {
        const NodeImpl *row = head->firstChild();
        while ( row ) {
            if ( row == this )
                return rIndex;
            rIndex++;
            row = row->nextSibling();
        }
    }

    NodeImpl *node = table->firstChild();
    while ( node ) {
        if ( node != foot && node != head && (node->id() == ID_THEAD || node->id() == ID_TFOOT || node->id() == ID_TBODY) ) {
	    HTMLTableSectionElementImpl* section = static_cast<HTMLTableSectionElementImpl *>(node);
	    const NodeImpl *row = section->firstChild();
	    while ( row ) {
		if ( row == this )
		    return rIndex;
		rIndex++;
		row = row->nextSibling();
	    }
	}
	node = node->nextSibling();
    }
    const NodeImpl *row = foot->firstChild();
    while ( row ) {
	if ( row == this )
	    return rIndex;
	rIndex++;
	row = row->nextSibling();
    }
    // should never happen
    return -1;
}

long HTMLTableRowElementImpl::sectionRowIndex() const
{
    int rIndex = 0;
    const NodeImpl *n = this;
    do {
        n = n->previousSibling();
        if (n && n->id() == ID_TR)
            rIndex++;
    }
    while (n);

    return rIndex;
}

HTMLElementImpl *HTMLTableRowElementImpl::insertCell( long index, int &exceptioncode )
{
    HTMLTableCellElementImpl *c = 0L;
    NodeListImpl *children = childNodes();
    int numCells = children ? children->length() : 0;
    if ( index < -1 || index > numCells )
        exceptioncode = DOMException::INDEX_SIZE_ERR; // per the DOM
    else
    {
        c = new HTMLTableCellElementImpl(docPtr(), ID_TD);
        if(numCells == index || index == -1)
            appendChild(c, exceptioncode);
        else {
            NodeImpl *n;
            if(index < 1)
                n = firstChild();
            else
                n = children->item(index);
            insertBefore(c, n, exceptioncode);
        }
    }
    delete children;
    return c;
}

void HTMLTableRowElementImpl::deleteCell( long index, int &exceptioncode )
{
    NodeListImpl *children = childNodes();
    int numCells = children ? children->length() : 0;
    if ( index == -1 ) index = numCells-1;
    if( index >= 0 && index < numCells )
        HTMLElementImpl::removeChild(children->item(index), exceptioncode);
    else
        exceptioncode = DOMException::INDEX_SIZE_ERR;
    delete children;
}

// -------------------------------------------------------------------------

HTMLTableCellElementImpl::HTMLTableCellElementImpl(DocumentImpl *doc, int tag)
  : HTMLTablePartElementImpl(doc)
{
  _col = -1;
  _row = -1;
  cSpan = rSpan = 1;
  _id = tag;
  rowHeight = 0;
  m_solid = false;
}

HTMLTableCellElementImpl::~HTMLTableCellElementImpl()
{
}

long HTMLTableCellElementImpl::cellIndex() const
{
    int index = 0;
    for (const NodeImpl * node = previousSibling(); node; node = node->previousSibling()) {
        if (node->id() == ID_TD || node->id() == ID_TH)
            index++;
    }

    return index;
}

void HTMLTableCellElementImpl::parseAttribute(AttributeImpl *attr)
{
    switch(attr->id())
    {
    case ATTR_BORDER:
        // euhm? not supported by other browsers as far as I can see (Dirk)
        //addCSSLength(CSS_PROP_BORDER_WIDTH, attr->value());
        break;
    case ATTR_ROWSPAN:
        // ###
        rSpan = attr->val() ? attr->val()->toInt() : 1;
        // limit this to something not causing an overflow with short int
        if(rSpan < 1 || rSpan > 1024) rSpan = 1;
        break;
    case ATTR_COLSPAN:
        // ###
        cSpan = attr->val() ? attr->val()->toInt() : 1;
        // limit this to something not causing an overflow with short int
        if(cSpan < 1 || cSpan > 1024) cSpan = 1;
        break;
    case ATTR_NOWRAP:
        if (attr->val() != 0)
	    addCSSProperty(CSS_PROP_WHITE_SPACE, CSS_VAL__TDEHTML_NOWRAP);
        else
	    removeCSSProperty(CSS_PROP_WHITE_SPACE);
        break;
    case ATTR_WIDTH:
        if (!attr->value().isEmpty())
            addCSSLength( CSS_PROP_WIDTH, attr->value() );
        else
            removeCSSProperty(CSS_PROP_WIDTH);
        break;
    case ATTR_NOSAVE:
	break;
    default:
        HTMLTablePartElementImpl::parseAttribute(attr);
    }
}

void HTMLTableCellElementImpl::attach()
{
    HTMLElementImpl* p = static_cast<HTMLElementImpl*>(parentNode());
    while(p && p->id() != ID_TABLE)
        p = static_cast<HTMLElementImpl*>(p->parentNode());

    if(p) {
        HTMLTableElementImpl* table = static_cast<HTMLTableElementImpl*>(p);
        if (table->rules == HTMLTableElementImpl::None) {
            addCSSProperty(CSS_PROP_BORDER_TOP_STYLE, CSS_VAL_NONE);
            addCSSProperty(CSS_PROP_BORDER_BOTTOM_STYLE, CSS_VAL_NONE);
            addCSSProperty(CSS_PROP_BORDER_LEFT_STYLE, CSS_VAL_NONE);
            addCSSProperty(CSS_PROP_BORDER_RIGHT_STYLE, CSS_VAL_NONE);
        }
        else {
            addCSSProperty(CSS_PROP_BORDER_WIDTH, "1px");
            int v = (table->m_solid || m_solid) ? CSS_VAL_SOLID : CSS_VAL_INSET;
            addCSSProperty(CSS_PROP_BORDER_TOP_STYLE, v);
            addCSSProperty(CSS_PROP_BORDER_BOTTOM_STYLE, v);
            addCSSProperty(CSS_PROP_BORDER_LEFT_STYLE, v);
            addCSSProperty(CSS_PROP_BORDER_RIGHT_STYLE, v);

            if (!m_solid)
                addCSSProperty(CSS_PROP_BORDER_COLOR, CSS_VAL_INHERIT);
        }
    }

    HTMLTablePartElementImpl::attach();
}

// -------------------------------------------------------------------------

HTMLTableColElementImpl::HTMLTableColElementImpl(DocumentImpl *doc, ushort i)
    : HTMLTablePartElementImpl(doc)
{
    _id = i;
    _span = 1;
}

NodeImpl::Id HTMLTableColElementImpl::id() const
{
    return _id;
}


void HTMLTableColElementImpl::parseAttribute(AttributeImpl *attr)
{
    switch(attr->id())
    {
    case ATTR_SPAN:
        _span = attr->val() ? attr->val()->toInt() : 1;
        if (_span < 1) _span = 1;
        break;
    case ATTR_WIDTH:
        if (!attr->value().isEmpty())
            addCSSLength(CSS_PROP_WIDTH, attr->value(), false, true );
        else
            removeCSSProperty(CSS_PROP_WIDTH);
        break;
    case ATTR_VALIGN:
        if (!attr->value().isEmpty())
            addCSSProperty(CSS_PROP_VERTICAL_ALIGN, attr->value().lower());
        else
            removeCSSProperty(CSS_PROP_VERTICAL_ALIGN);
        break;
    default:
        HTMLTablePartElementImpl::parseAttribute(attr);
    }

}

// -------------------------------------------------------------------------

NodeImpl::Id HTMLTableCaptionElementImpl::id() const
{
    return ID_CAPTION;
}


void HTMLTableCaptionElementImpl::parseAttribute(AttributeImpl *attr)
{
    switch(attr->id())
    {
    case ATTR_ALIGN:
        if (!attr->value().isEmpty())
            addCSSProperty(CSS_PROP_CAPTION_SIDE, attr->value().lower());
        else
            removeCSSProperty(CSS_PROP_CAPTION_SIDE);
        break;
    default:
        HTMLElementImpl::parseAttribute(attr);
    }

}
