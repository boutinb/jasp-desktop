#include <QtQml/QQmlEngineExtensionPlugin>
#include <QQmlEngine>
#include <QQmlContext>
#include "preferencesmodelbase.h"
#include "jasptheme.h"
#include "controls/jaspcontrol.h"
#include <qdebug.h>
#include "knownissues.h"
#include "utilities/qmlutils.h"
#include "models/term.h"
#include "jaspcontrol.h"
#include "altnavpostfixassignmentstrategy.h"
#include "altnavcontrol.h"
#include "tempfiles.h"
#include "log.h"

//![plugin]
class JASPQmlPlugin : public QQmlEngineExtensionPlugin
{
    Q_OBJECT
	Q_PLUGIN_METADATA(IID "org.jasp-stats.JASPQmlPlugin")


	void initializeEngine(QQmlEngine *engine, const char *uri) override
	{
		QQmlEngineExtensionPlugin::initializeEngine(engine, uri);

		QLocale::setDefault(QLocale(QLocale::English)); // make decimal points == .

		int sessionId = engine->rootContext()->contextProperty("sessionId").toInt();
		TempFiles::initGlobals(sessionId);

		QmlUtils::setGlobalPropertiesInQMLContext(engine->rootContext());
		PreferencesModelBase* prefModel = engine->rootContext()->contextProperty("preferencesModel").value<PreferencesModelBase*>();
		if (!prefModel)
		{
			prefModel = new PreferencesModelBase();
			engine->rootContext()->setContextProperty("preferencesModel",		prefModel);
		}

		ALTNavControl::ctrl()->enableAlTNavigation(prefModel->ALTNavModeActive());
		connect(prefModel,	&PreferencesModelBase::ALTNavModeActiveChanged,	ALTNavControl::ctrl(),	&ALTNavControl::enableAlTNavigation);

		if (engine->rootContext()->contextProperty("jaspTheme").isNull())
		{
			JaspTheme* defaultJaspTheme = new JaspTheme();
			defaultJaspTheme->setIconPath("/default/");
			engine->rootContext()->setContextProperty("jaspTheme",				defaultJaspTheme	);
		}

        engine->rootContext()->setContextProperty("INTERACTION_SEPARATOR",	Term::separator);

        qmlRegisterUncreatableType<JASPControl>(					"JASP",		1, 0, "JASP",					"Impossible to create JASP Object");
        qmlRegisterUncreatableType<ALTNavPostfixAssignmentStrategy>("JASP",		1, 0, "AssignmentStrategy",		"Can't make it"	);

		qmlRegisterUncreatableMetaObject(JASPControl::staticMetaObject, // static meta object
										 "JASP.Controls",        // import statement
										 0, 1,                   // major and minor version of the import
										 "JASP",                 // name in QML
										 "Error: only enums");
		if (!KnownIssues::issues())
			new KnownIssues(this);

	}
};
//![plugin]

#include "plugin.moc"


