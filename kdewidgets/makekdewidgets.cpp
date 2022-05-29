/* Copyright (C) 2004-2005 ian reinhart geiser <geiseri@sourcextreme.com> */

#include "config-prefix.h"

#include <kaboutdata.h>
#include <kcomponentdata.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <kmacroexpander.h>
#include <kdebug.h>
#include <kconfiggroup.h>
#include <klocale.h>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QTextStream>

static const char classHeader[] = 	"/**\n"
                                "* This file was autogenerated by makekdewidgets. Any changes will be lost!\n"
                                "* The generated code in this file is licensed under the same license that the\n"
                                "* input file.\n"
                                                "*/\n"
                                "#include <QtGui/qicon.h>\n"
                                "#include <QtDesigner/container.h>\n"
                                "#include <QtDesigner/customwidget.h>\n"
                                "#include <QtCore/qplugin.h>\n"
                                "#include <QtCore/qdebug.h>\n";

static const char collClassDef[] = "class %CollName : public QObject, public QDesignerCustomWidgetCollectionInterface\n"
                                "{\n"
                                "	Q_OBJECT\n"
                                "	Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)\n"
                                "public:\n"
                                "	%CollName(QObject *parent = 0);\n"
                                "	virtual ~%CollName();\n"
                                "	QList<QDesignerCustomWidgetInterface*> customWidgets() const { return m_plugins; } \n"
                                "	\n"
                                "private:\n"
                                "	QList<QDesignerCustomWidgetInterface*> m_plugins;\n"
                                "};\n\n"
                                "Q_EXPORT_PLUGIN2(%CollName, %CollName)\n\n";

static const char collClassImpl[] = "%CollName::%CollName(QObject *parent)\n"
                                "	: QObject(parent)"
                                "{\n"
                                "	%CollInit\n"
                                "	(void) new KComponentData(\"%CollName\");\n"
                                "%CollectionAdd\n"
                                "}\n"
                                "\n"
                                "%CollName::~%CollName()\n"
                                "{\n"
                                "	%CollDestroy\n"
                                "}\n";


static const char classDef[] =  "class %PluginName : public QObject, public QDesignerCustomWidgetInterface\n"
                                "{\n"
                                "	Q_OBJECT\n"
                                "	Q_INTERFACES(QDesignerCustomWidgetInterface)\n"
                                "public:\n"
                                "	%PluginName(QObject *parent = 0) :\n\t\tQObject(parent), mInitialized(false) {}\n"
                                "	virtual ~%PluginName() {}\n"
                                "	\n"
                                "	bool isContainer() const { return %IsContainer; }\n"
                                "	bool isInitialized() const { return mInitialized; }\n"
                                "	QIcon icon() const { return QIcon(QLatin1String(\"%IconName\")); }\n"
                                "	QString codeTemplate() const { return QLatin1String(\"%CodeTemplate\");}\n"
                                "	QString domXml() const { return %DomXml; }\n"
                                "	QString group() const { return QLatin1String(\"%Group\"); }\n"
                                "	QString includeFile() const { return QLatin1String(\"%IncludeFile\"); }\n"
                                "	QString name() const { return QLatin1String(\"%Class\"); }\n"
                                "	QString toolTip() const { return QLatin1String(\"%ToolTip\"); }\n"
                                "	QString whatsThis() const { return QLatin1String(\"%WhatsThis\"); }\n\n"
                                "	QWidget* createWidget( QWidget* parent ) \n\t{%CreateWidget\n\t}\n"
                                "	void initialize(QDesignerFormEditorInterface *core) \n\t{%Initialize\n\t}\n"
                                "\n"
                                "private:\n"
                                "	bool mInitialized;\n"
                                "};\n\n";

static QString denamespace ( const QString &str );
static QString buildCollClass( KConfig &input, const QStringList& classes );
static QString buildWidgetClass( const QString &name, KConfig &input, const QString &group );
static QString buildWidgetInclude( const QString &name, KConfig &input );
static void buildFile( QTextStream &stream, const QString& group, const QString& fileName, const QString& pluginName );

int main( int argc, char **argv ) {
    new KComponentData( "makekdewidgets" );

    KLocalizedString description = ki18n( "Builds Qt widget plugins from an ini style description file." );
    const char version[] = "0.4";

    KCmdLineOptions options;
    options.add("+file", ki18n( "Input file" ) );
    options.add("o <file>", ki18n( "Output file" ) );
    options.add("n <plugin name>", ki18n( "Name of the plugin class to generate" ), "WidgetsPlugin" );
    options.add("g <group>", ki18n( "Default widget group name to display in designer" ), "Custom" );


    KAboutData about( "makekdewidgets", 0, ki18n( "makekdewidgets" ), version, description, KAboutData::License_GPL, ki18n("(C) 2004-2005 Ian Reinhart Geiser"), KLocalizedString(), 0, "geiseri@kde.org" );
    about.addAuthor( ki18n("Ian Reinhart Geiser"), KLocalizedString(), "geiseri@kde.org" );
    about.addAuthor( ki18n("Daniel Molkentin"), KLocalizedString(), "molkentin@kde.org" );
    KCmdLineArgs::init( argc, argv, &about );
    KCmdLineArgs::addCmdLineOptions( options );
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if ( args->count() < 1 ) {
        args->usage();
        return ( 1 );
    }

    QFileInfo fi( args->arg( args->count() - 1 ) );

    QString outputFile = args->getOption( "o" );
    QString pluginName = args->getOption( "n" );
    QString group = args->getOption( "g" );
    QString fileName = fi.absoluteFilePath();

    if ( args->isSet( "o" ) ) {
        QFile output( outputFile );
        if ( output.open( QIODevice::WriteOnly ) ) {
            QTextStream ts( &output );
            buildFile( ts, group, fileName , pluginName );
            QString mocFile = output.fileName();
            mocFile.replace(".cpp", ".moc");
            ts << QString( "#include <%1>\n" ).arg(mocFile) << endl;
        }
        output.close();
    } else {
        QTextStream ts( stdout, QIODevice::WriteOnly );
        buildFile( ts, group, fileName , pluginName );
    }
}

void buildFile( QTextStream &ts, const QString& group, const QString& fileName, const QString& pluginName ) {
    KConfig input( fileName, KConfig::NoGlobals );
    KConfigGroup cg(&input, "Global" );
    ts << classHeader << endl;

    QStringList includes = cg.readEntry( "Includes", QStringList() );
    QStringList classes = input.groupList();
    classes.removeAll( "Global" );

    foreach ( const QString &myInclude, classes )
      includes += buildWidgetInclude( myInclude, input );

    foreach ( const QString &myInclude, includes)
        ts << "#include <" << myInclude << ">" << endl;

    ts << QLatin1String("\n\n");

    // Autogenerate widget defs here
    foreach ( const QString &myClass, classes )
        ts << buildWidgetClass( myClass, input, group ) << endl;

    ts << buildCollClass( input, classes );

}

QString denamespace ( const QString &str ) {
    QString denamespaced = str;
    denamespaced.remove("::");
    return denamespaced;
}

QString buildCollClass( KConfig &_input, const QStringList& classes ) {
    KConfigGroup input(&_input, "Global");
    QHash<QString, QString> defMap;
    defMap.insert( "CollName", input.readEntry( "PluginName" ) );
    defMap.insert( "CollInit", input.readEntry( "Init", "" ) );
    defMap.insert( "CollDestroy", input.readEntry( "Destroy", "" ) );
    QString genCode;

    foreach ( const QString &myClass, classes )
    {
      genCode += QString("\t\tm_plugins.append( new %1(this) );\n").arg(denamespace( myClass ) +"Plugin");
    }

    defMap.insert( "CollectionAdd", genCode  );

    QString str = KMacroExpander::expandMacros(collClassDef, defMap);
    str += KMacroExpander::expandMacros(collClassImpl, defMap);
    return str;
}

QString buildWidgetClass( const QString &name, KConfig &_input, const QString &group ) {
    KConfigGroup input(&_input, name);
    QHash<QString, QString> defMap;

    const QString widgetIconName = denamespace( name ).toLower();
    QString widgetIconPath = QString::fromLatin1(DATA_INSTALL_DIR "/kdewidgets/pics/%1.png").arg( widgetIconName );

    defMap.insert( "Group", input.readEntry( "Group", group ).replace( '\"', "\\\"" ) );
    defMap.insert( "IconSet", input.readEntry( "IconSet", QString(name.toLower() + ".png") ).replace( ':', '_' ) );
    defMap.insert( "Pixmap", name.toLower().replace( ':', '_' ) + "_xpm" );
    defMap.insert( "IncludeFile", input.readEntry( "IncludeFile", QString(name.toLower() + ".h") ).remove( ':' ) );
    defMap.insert( "ToolTip", input.readEntry( "ToolTip", QString(name + " Widget") ).replace( '\"', "\\\"" ) );
    defMap.insert( "WhatsThis", input.readEntry( "WhatsThis", QString(name + " Widget") ).replace( '\"', "\\\"" ) );
    defMap.insert( "IsContainer", input.readEntry( "IsContainer", "false" ) );
    defMap.insert( "IconName", input.readEntry( "IconName",  widgetIconPath ) );
    defMap.insert( "Class", name );
    defMap.insert( "PluginName", denamespace( name ) + QLatin1String( "Plugin" ) );

    // FIXME: ### make this more useful, i.e. outsource to separate file
    QString domXml = input.readEntry("DomXML", QString());
    // If domXml is empty then we shoud call base class function
    if ( domXml.isEmpty() ) {
        domXml = QLatin1String("QDesignerCustomWidgetInterface::domXml()");
    }
    else {
        // Wrap domXml value into QLatin1String
        domXml = QString(QLatin1String("QLatin1String(\"%1\")")).arg(domXml.replace( '\"', "\\\"" ));
    }
    defMap.insert( "DomXml", domXml  );
    defMap.insert( "CodeTemplate", input.readEntry( "CodeTemplate" ) );
    defMap.insert( "CreateWidget", input.readEntry( "CreateWidget",
      QString( "\n\t\treturn new %1%2;" )
         .arg( input.readEntry( "ImplClass", name ) )
         .arg( input.readEntry( "ConstructorArgs", "( parent )" ) ) ) );
    defMap.insert( "Initialize", input.readEntry( "Initialize", "\n\t\tQ_UNUSED(core);\n\t\tif (mInitialized) return;\n\t\tmInitialized=true;" ) );

    return KMacroExpander::expandMacros( classDef, defMap );
}

QString buildWidgetInclude( const QString &name, KConfig &_input ) {
    KConfigGroup input(&_input, name);
    return input.readEntry( "IncludeFile", QString(name.toLower() + ".h") );
}
