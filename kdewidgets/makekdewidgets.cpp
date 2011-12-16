/* Copyright (C) 2004-2005 ian reinhart geiser <geiseri@sourcextreme.com> */
#include <kaboutdata.h>
#include <kinstance.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <kmacroexpander.h>
#include <kdebug.h>

#include <tqtextstream.h>
#include <tqfileinfo.h>
#include <tqfile.h>
#include <tqbuffer.h>
#include <tqimage.h>

static const char description[] = I18N_NOOP( "Builds Qt widget plugins from an ini style description file." );
static const char version[] = "0.2";
static const char classHeader[] = 	"/**\n"
					"* This file was autogenerated by makekdewidgets. Any changes will be lost!\n"
					"* The generated code in this file is licensed under the same license that the\n"
					"* input file.\n"
                                   	"*/\n"
					"#include <tqwidgetplugin.h>\n";
static const char classDef[] = "#ifndef EMBED_IMAGES\n"
                                "#include <kstandarddirs.h>\n"
                                "#endif\n"
                                "\n"
                                "class %PluginName : public TQWidgetPlugin\n"
                                "{\n"
                                "public:\n"
                                "	%PluginName();\n"
                                "	\n"
                                "	virtual ~%PluginName();\n"
                                "	\n"
                                "	virtual TQStringList keys() const\n"
                                "	{\n"
                                "		TQStringList result;\n"
                                "		for (WidgetInfos::ConstIterator it = m_widgets.begin(); it != m_widgets.end(); ++it)\n"
                                "			result << it.key();\n"
                                "		return result;\n"
                                "	}\n"
                                "	\n"
                                "	virtual TQWidget *create(const TQString &key, TQWidget *parent = 0, const char *name = 0);\n"
                                "	\n"
                                "	virtual TQIconSet iconSet(const TQString &key) const\n"
                                "	{\n"
                                "#ifdef EMBED_IMAGES\n"
                                "		TQPixmap pix(m_widgets[key].iconSet);\n"
                                "#else\n"
                                "		TQPixmap pix(locate( \"data\", \n"
				"			TQString::tqfromLatin1(\"%PluginNameLower/pics/\") + m_widgets[key].iconSet));\n"
                                "#endif\n"
                                "		return TQIconSet(pix);\n"
                                "	}\n"
                                "	\n"
                                "	virtual bool isContainer(const TQString &key) const { return m_widgets[key].isContainer; }\n"
                                "	\n"
                                "	virtual TQString group(const TQString &key) const { return m_widgets[key].group; }\n"
                                "	\n"
                                "	virtual TQString includeFile(const TQString &key) const { return m_widgets[key].includeFile; }\n"
                                "	\n"
                                "	virtual TQString toolTip(const TQString &key) const { return m_widgets[key].toolTip; }\n"
                                "	\n"
                                "	virtual TQString whatsThis(const TQString &key) const { return m_widgets[key].whatsThis; }\n"
                                "private:\n"
                                "	struct WidgetInfo\n"
                                "	{\n"
                                "		TQString group;\n"
                                "#ifdef EMBED_IMAGES\n"
                                "		TQPixmap iconSet;\n"
                                "#else\n"
                                "		TQString iconSet;\n"
                                "#endif\n"
                                "		TQString includeFile;\n"
                                "		TQString toolTip;\n"
                                "		TQString whatsThis;\n"
                                "		bool isContainer;\n"
                                "	};\n"
                                "	typedef TQMap<TQString, WidgetInfo> WidgetInfos;\n"
                                "	WidgetInfos m_widgets;\n"
                                "};\n"
                                "%PluginName::%PluginName()\n"
                                "{\n"
                                "        WidgetInfo widget;\n";
static const char widgetDef[] = "	widget.group = TQString::tqfromLatin1(\"%Group\");\n"
                                 "#ifdef EMBED_IMAGES\n"
                                 "	widget.iconSet = TQPixmap(%Pixmap);\n"
                                 "#else\n"
                                 "	widget.iconSet = TQString::tqfromLatin1(\"%IconSet\");\n"
                                 "#endif\n"
                                 "	widget.includeFile = TQString::tqfromLatin1(\"%IncludeFile\");\n"
                                 "	widget.toolTip = TQString::tqfromLatin1(\"%ToolTip\");\n"
                                 "	widget.whatsThis = TQString::tqfromLatin1(\"%WhatsThis\");\n"
                                 "	widget.isContainer = %IsContainer;\n"
                                 "	m_widgets.insert(TQString::tqfromLatin1(\"%Class\"), widget);\n";
static const char endCtor[] = "	%Init\n"
                               "}\n"
                               "%PluginName::~%PluginName()\n"
                               "{\n"
                               "	%Destroy\n"
                               "}\n"
                               "TQWidget *%PluginName::create(const TQString &key, TQWidget *parent, const char *name)\n"
                               "{\n";
static const char widgetCreate[] = "         if (key == TQString::tqfromLatin1(\"%Class\"))\n"
                                    "                return new %ImplClass%ConstructorArgs;\n";
static const char endCreate[] = "	return 0;\n"
                                 "}\n"
                                 "KDE_Q_EXPORT_PLUGIN(%PluginName)\n";


static KCmdLineOptions options[] =
    {
        { "+file", I18N_NOOP( "Input file" ), 0 },
        { "o <file>", I18N_NOOP( "Output file" ), 0 },
        { "n <plugin name>", I18N_NOOP( "Name of the plugin class to generate" ), "WidgetsPlugin" },
        { "g <group>", I18N_NOOP( "Default widget group name to display in designer" ), "Custom" },
        { "p <pixmap dir>", I18N_NOOP( "Embed pixmaps from a source directory" ), 0 },
        KCmdLineLastOption
    };

static TQString buildWidgetDef( const TQString &name, KConfig &input, const TQString &group );
static TQString buildWidgetCreate( const TQString &name, KConfig &input );
static TQString buildWidgetInclude( const TQString &name, KConfig &input );
static void buildFile( TQTextStream &stream, const TQString& group, const TQString& fileName, const TQString& pluginName, const TQString& iconPath );
static TQString buildPixmap( const TQString &name, KConfig &input, const TQString &iconPath );

int main( int argc, char **argv ) {
    new KInstance( "makekdewidgets" );

    KAboutData about( "makekdewidgets", I18N_NOOP( "makekdewidgets" ), version, description, KAboutData::License_GPL, "(C) 2004-2005 ian reinhart geiser", 0, 0, "geiseri@kde.org" );
    about.addAuthor( "ian reinhart geiser", 0, "geiseri@kde.org" );
    KCmdLineArgs::init( argc, argv, &about );
    KCmdLineArgs::addCmdLineOptions( options );
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if ( args->count() < 1 ) {
        args->usage();
        return ( 1 );
    }

    TQFileInfo fi( args->arg( args->count() - 1 ) );

    TQString outputFile = args->getOption( "o" );
    TQString pluginName = args->getOption( "n" );
    TQString group = args->getOption( "g" );
    TQString iconPath = "";
    if ( args->isSet( "p" ) )
        iconPath = args->getOption( "p" );
    TQString fileName = fi.absFilePath();

    if ( args->isSet( "o" ) ) {
        TQFile output( outputFile );
        if ( output.open( IO_WriteOnly ) ) {
            TQTextStream ts( &output );
            buildFile( ts, group, fileName , pluginName, iconPath );
        }
        output.close();
    } else {
        TQTextStream ts( stdout, IO_WriteOnly );
        buildFile( ts, group, fileName , pluginName, iconPath );
    }
}

void buildFile( TQTextStream &ts, const TQString& group, const TQString& fileName, const TQString& pluginName, const TQString& iconPath ) {
    KConfig input( fileName, true, false );
    input.setGroup( "Global" );
    TQMap<TQString, TQString> MainMap;
    MainMap.insert( "PluginName", input.readEntry( "PluginName", pluginName ) );
    MainMap.insert( "PluginNameLower", input.readEntry( "PluginName", pluginName ).lower() );
    MainMap.insert( "Init", input.readEntry( "Init", "" ) );
    MainMap.insert( "Destroy", input.readEntry( "Destroy", "" ) );
    ts << classHeader << endl;
    TQStringList includes = input.readListEntry( "Includes", ',' );
    for ( uint idx = 0; idx < includes.count(); ++idx )
        ts << "#include <" << includes[ idx ] << ">" << endl;
    TQStringList classes = input.groupList();
    classes.remove( classes.find( "Global" ) );
    // Autogenerate widget includes here
    for ( uint idx = 0; idx < classes.count(); ++idx )
        ts << buildWidgetInclude( classes[ idx ], input ) << endl;
    // Generate embedded icons
    if ( !iconPath.isEmpty() ) {
        for ( uint idx = 0; idx < classes.count(); ++idx )
            ts << buildPixmap( classes[ idx ], input, iconPath ) << endl;
        ts << "#define EMBED_IMAGES" << endl;
    }
    // Generate the main class code.
    ts << KMacroExpander::expandMacros( classDef, MainMap ) << endl;
    // Autogenerate widget defs here
    for ( uint idx = 0; idx < classes.count(); ++idx )
        ts << buildWidgetDef( classes[ idx ], input, group ) << endl;
    ts << KMacroExpander::expandMacros( endCtor, MainMap ) << endl;
    // Autogenerate create code here...
    for ( uint idx = 0; idx < classes.count(); ++idx )
        ts << buildWidgetCreate( classes[ idx ], input ) << endl;
    ts << KMacroExpander::expandMacros( endCreate, MainMap ) << endl;

}

TQString buildWidgetDef( const TQString &name, KConfig &input, const TQString &group ) {
    input.setGroup( name );
    TQMap<TQString, TQString> defMap;
    defMap.insert( "Group", input.readEntry( "Group", group ).replace( "\"", "\\\"" ) );
    defMap.insert( "IconSet", input.readEntry( "IconSet", name.lower() + ".png" ).replace( ":", "_" ) );
    defMap.insert( "Pixmap", name.lower().replace( ":", "_" ) + "_xpm" );
    defMap.insert( "IncludeFile", input.readEntry( "IncludeFile", name.lower() + ".h" ).remove( ":" ) );
    defMap.insert( "ToolTip", input.readEntry( "ToolTip", name + " Widget" ).replace( "\"", "\\\"" ) );
    defMap.insert( "WhatsThis", input.readEntry( "WhatsThis", name + " Widget" ).replace( "\"", "\\\"" ) );
    defMap.insert( "IsContainer", input.readEntry( "IsContainer", "false" ) );
    defMap.insert( "Class", name );
    return KMacroExpander::expandMacros( widgetDef, defMap );
}

TQString buildWidgetCreate( const TQString &name, KConfig &input ) {
    input.setGroup( name );
    TQMap<TQString, TQString> createMap;
    createMap.insert( "ImplClass", input.readEntry( "ImplClass", name ) );
    createMap.insert( "ConstructorArgs", input.readEntry( "ConstructorArgs", "(parent, name)" ) );
    createMap.insert( "Class", name );
    return KMacroExpander::expandMacros( widgetCreate, createMap );
}

TQString buildWidgetInclude( const TQString &name, KConfig &input ) {
    input.setGroup( name );
    return "#include <" + input.readEntry( "IncludeFile", name.lower() + ".h" ) + ">";
}

TQString buildPixmap( const TQString &name, KConfig &input, const TQString &iconPath ) {
    input.setGroup( name );
    TQString cleanName = name.lower().replace( ":", "_" );
    TQString iconName = input.readEntry( "IconSet", cleanName + ".png" );

    TQFileInfo fi( iconPath + "/" + iconName );
    TQImage pix( fi.absFilePath() );
    TQCString xpm;
    TQBuffer buff( xpm );
    buff.open( IO_WriteOnly );
    TQImageIO io( &buff, "XPM" );
    io.setFileName( cleanName + "_xpm" );
    io.setImage( pix );
    io.write();
    buff.close();
    return xpm;
}
