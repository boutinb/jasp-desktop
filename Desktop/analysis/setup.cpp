#include "setup.h"
#include <QQmlEngine>
#include <QQmlApplicationEngine>
#include "preferencesmodelbase.h"
#include "jasptheme.h"
#include "controls/jaspcontrol.h"
#include "knownissues.h"
#include "analysisform.h"
#include "utilities/qmlutils.h"


void Setup::applicationAvailable()
{
	// custom code that does not require QQmlEngine
}

void Setup::qmlEngineAvailable(QQmlEngine *engine)
{
	// custom code that needs QQmlEngine, register QML types, add import paths,...
	static QStringList originalImportPaths = engine->importPathList();

	QStringList newImportPaths = originalImportPaths;

	newImportPaths.append(":/jasp-stats.org/imports");
	newImportPaths.append("qrc:///components");
	//newImportPaths.append(_dynamicModules->importPaths());

	engine->setImportPathList(newImportPaths);


	QmlUtils::setGlobalPropertiesInQMLContext(engine->rootContext());

	PreferencesModelBase* prefModel = PreferencesModelBase::preferences();
	engine->rootContext()->setContextProperty("preferencesModel",		prefModel);

	JaspTheme* defaultJaspTheme = JaspTheme::currentTheme();
	engine->rootContext()->setContextProperty("jaspTheme",			defaultJaspTheme);

	qmlRegisterUncreatableMetaObject(JASPControl::staticMetaObject, // static meta object
									 "JASP.Controls",        // import statement
									 0, 1,                   // major and minor version of the import
									 "JASP",                 // name in QML
									 "Error: only enums");
	if (!KnownIssues::issues())
		new KnownIssues();


}

void Setup::cleanupTestCase()
{
	// custom code to clean up before destruction starts
}
