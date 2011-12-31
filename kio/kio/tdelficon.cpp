#include "tdelficon.h"

#include <cstring>

/*
 * Obtain an existing icon resource list
 */
int get_iconlist(libr_file *file_handle, iconlist *icons)
{
	if(icons == NULL)
	{
		/* Need to be able to return SOMETHING */
		return false;
	}
	/* Obtain the icon resource list */
	icons->buffer = libr_malloc(file_handle, ICON_SECTION, &(icons->size));
	if(icons->buffer == NULL)
		return false;
	return true;
}

/*
 * Get the next entry in an icon resource list
 */
iconentry *get_nexticon(iconlist *icons, iconentry *last_entry)
{
	size_t i;
	
	/* The icon list is needed both for the data buffer and for a call-specific iconentry instance */ 
	if(icons == NULL)
		return NULL;
	/* If this is the first call (last_entry == NULL) then return the first entry */
	if(last_entry == NULL)
		icons->entry.offset = sizeof(uint32_t)+sizeof(UUID);
	else
		icons->entry.offset += icons->entry.entry_size;
	/* Check to see if we've run out of entries */
	if(icons->entry.offset >= icons->size)
		return NULL;
	i = icons->entry.offset;
	memcpy(&(icons->entry.entry_size), &(icons->buffer[i]), sizeof(uint32_t));
	i += sizeof(uint32_t);
	icons->entry.type = (libr_icontype_t)icons->buffer[i];
	i += sizeof(unsigned char);
	switch(icons->entry.type)
	{
		case LIBR_SVG:
			icons->entry.icon_size = 0;
			icons->entry.name = &(icons->buffer[i]);
			break;
		case LIBR_PNG:
			memcpy(&(icons->entry.icon_size), &(icons->buffer[i]), sizeof(uint32_t));
			i += sizeof(uint32_t);
			icons->entry.name = &(icons->buffer[i]);
			break;
		default:
			/* Invalid entry type */
			return NULL;
	}
	return &(icons->entry);
}