/* This file is part of the KDE project
 * Copyright (C) 2012 Timothy Pearson <kb9vqf@pearsoncomputing.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation version 2.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include <config.h>
#include "kfile_elf.h"

#include <kprocess.h>
#include <klocale.h>
#include <kgenericfactory.h>
#include <kstringvalidator.h>
#include <kdebug.h>

#include <tqdict.h>
#include <tqvalidator.h>
#include <tqcstring.h>
#include <tqfile.h>
#include <tqdatetime.h>

#include "tdelficon.h"

#if !defined(__osf__)
#include <inttypes.h>
#else
typedef unsigned long uint32_t;
typedef unsigned short uint16_t;
#endif

TQString elf_get_resource(libr_file *handle, char *section_name)
{
	size_t buffer_size = 0;
	char *buffer = NULL;
	TQString result;

	/* Get the resource from the ELF binary */
	if(!libr_size(handle, section_name, &buffer_size))
	{
		kdWarning() << "failed to obtain ELF resource size: " << libr_errmsg() << endl;
		return result;
	}
	/* Get the resource from the ELF file */
	buffer = (char *) malloc(buffer_size+1);
	buffer[buffer_size] = 0;
	if(!libr_read(handle, section_name, buffer))
	{
		kdWarning() << "failed to obtain ELF resource: " << libr_errmsg() << endl;
		goto fail;
	}
	result = buffer;

fail:
	free(buffer);

	return result;
}

typedef KGenericFactory<KElfPlugin> ElfFactory;

K_EXPORT_COMPONENT_FACTORY(kfile_elf, ElfFactory( "kfile_elf" ))

KElfPlugin::KElfPlugin(TQObject *parent, const char *name,
                       const TQStringList &args)

    : KFilePlugin(parent, name, args)
{
    KFileMimeTypeInfo* info = addMimeTypeInfo( "application/x-executable" );

    KFileMimeTypeInfo::GroupInfo* group = 0L;
    KFileMimeTypeInfo::GroupInfo* group2 = 0L;

    group = addGroupInfo(info, "Technical", i18n("Embedded Metadata"));
    group2 = addGroupInfo(info, "Icon", i18n("Embedded Icon(s)"));

    KFileMimeTypeInfo::ItemInfo* item;

    item = addItemInfo(group, "Name", i18n("Internal Name"), TQVariant::String);
    item = addItemInfo(group, "Description", i18n("Description"), TQVariant::String);
    item = addItemInfo(group, "License", i18n("License"), TQVariant::String);
    item = addItemInfo(group, "Copyright", i18n("Copyright"), TQVariant::String);
    item = addItemInfo(group, "Authors", i18n("Author(s)"), TQVariant::String);
    item = addItemInfo(group, "Product", i18n("Product"), TQVariant::String);
    item = addItemInfo(group, "Organization", i18n("Organization"), TQVariant::String);
    item = addItemInfo(group, "Version", i18n("Version"), TQVariant::String);
    item = addItemInfo(group, "DateTime", i18n("Creation Date/Time"), TQVariant::String);
    item = addItemInfo(group, "Notes", i18n("Comments"), TQVariant::String);

    item = addItemInfo(group2, "EmbeddedIcon", i18n("Icon Name(s)"), TQVariant::String);
}


bool KElfPlugin::readInfo( KFileMetaInfo& info, uint what)
{
	libr_icon *icon = NULL;
	libr_file *handle = NULL;
	libr_access_t access = LIBR_READ;

	if((handle = libr_open(const_cast<char*>(info.path().ascii()), access)) == NULL)
	{
		kdWarning() << "failed to open file" << info.path() << endl;
	}

	KFileMetaInfoGroup group = appendGroup(info, "Technical");
	KFileMetaInfoGroup group2 = appendGroup(info, "Icon");

	appendItem(group, "Name", elf_get_resource(handle, ".metadata_name"));
	appendItem(group, "Description", elf_get_resource(handle, ".metadata_description"));
	appendItem(group, "License", elf_get_resource(handle, ".metadata_license"));
	appendItem(group, "Copyright", elf_get_resource(handle, ".metadata_copyright"));
	appendItem(group, "Authors", elf_get_resource(handle, ".metadata_authors"));
	appendItem(group, "Product", elf_get_resource(handle, ".metadata_product"));
	appendItem(group, "Organization", elf_get_resource(handle, ".metadata_organization"));
	appendItem(group, "Version", elf_get_resource(handle, ".metadata_version"));
	appendItem(group, "DateTime", elf_get_resource(handle, ".metadata_datetime"));
	appendItem(group, "Notes", elf_get_resource(handle, ".metadata_notes"));

	TQString iconListing;

	iconentry *entry = NULL;
	iconlist icons;
	if(!get_iconlist(handle, &icons))
	{
		// Failed to obtain a list of ELF icons
	}
	else {
		while((entry = get_nexticon(&icons, entry)) != NULL)
		{
			if (iconListing.isEmpty()) {
				iconListing = entry->name;
			}
			else {
				iconListing = iconListing.append("<p>").append(entry->name);
			}
			break;
		}
	}

	appendItem(group2, "EmbeddedIcon", iconListing);

	libr_close(handle);

	return true;
}

#include "kfile_elf.moc"
