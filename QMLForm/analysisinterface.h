#ifndef ANALYSISINTERFACE_H
#define ANALYSISINTERFACE_H

#include <string>
#include <QObject>
#include <json/json.h>

class AnalysisForm;

class AnalysisInterface : public QObject
{
	Q_OBJECT
public:
	AnalysisInterface();

	virtual bool isOwnComputedColumn(const std::string &col) = 0;

	virtual void sendRScript(QString script, QString controlName, bool whiteListedVersion) = 0;
	virtual void refresh() = 0;
	virtual void exportResults() = 0;
	virtual bool isDuplicate() = 0;
	virtual void requestColumnCreationHandler() = 0;
	virtual void requestComputedColumnCreationHandler() = 0;
	virtual void requestComputedColumnDestructionHandler() = 0;
	virtual bool setBoundValue(const std::string &name, const Json::Value &value, const Json::Value &meta, const QVector<DUP_JASPControl::ParentKey> &parentKeys) = 0;
	virtual Json::Value & boundValues() = 0;
	virtual Json::Value & optionsFromJASPFile() = 0;
	virtual Json::Value resultsMeta() = 0;
	virtual Json::Value getRSource() = 0;
	virtual void setOptionsBound(bool b) = 0;
	virtual std::string upgradeMsgsForOption(std::string name) = 0;
	virtual void boundValueChangedHandler() = 0;
	virtual void hasVolatileNotesChanged() = 0;
	virtual void needsRefreshChanged() = 0;
	virtual void initialized(AnalysisForm* form, bool b) = 0;
	virtual bool wasUpgraded() = 0;
	virtual bool needsRefresh() = 0;
	virtual bool hasVolatileNotes() = 0;
	virtual QString titleQ() = 0;
	virtual void preprocessMarkdownHelp(const QString& md) = 0;

};

#endif // ANALYSISINTERFACE_H
