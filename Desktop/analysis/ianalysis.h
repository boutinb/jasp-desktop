#ifndef IANALYSIS_H
#define IANALYSIS_H

#include <QObject>
#include <json/json.h>
#include "jaspcontrol.h"

class AnalysisForm;

class IAnalysis : public QObject
{
	Q_OBJECT
public:
	explicit IAnalysis(QObject *parent = nullptr) : QObject(parent) {}

	virtual bool isOwnComputedColumn(const std::string &col)				const	{ return false; }
	virtual void refresh()															{}
	virtual void run()																{}
	virtual void reload()															{}
	virtual void exportResults()													{}
	virtual bool isDuplicate()												const	{ return false;				}
	virtual bool wasUpgraded()												const	{ return false;				}
	virtual bool needsRefresh()												const	{ return false;				}
	virtual const std::string & module()									const	{ return emptyString;		}
	virtual const std::string & name()										const	{ return emptyString;		}
	virtual const std::string & title()										const	{ return emptyString;		}
	virtual void setTitle(const std::string& titel)									{}
	virtual void preprocessMarkdownHelp(const QString& md)					const	{}
	virtual QString helpFile()														{ return "";				}
	virtual std::string upgradeMsgsForOption(std::string name)						{ return "";				}
	virtual const Json::Value & boundValues()								const	{ return Json::Value::null;	}
	virtual const Json::Value & optionsFromJASPFile()						const	{ return Json::Value::null;	}
	virtual const Json::Value & resultsMeta()								const 	{ return Json::Value::null;	}
	virtual const Json::Value & getRSource(const std::string& name)			const 	{ return Json::Value::null;	}
	virtual void initialized(AnalysisForm* form, bool isNewAnalysis)				{}
	virtual const Json::Value&	boundValue(const std::string& name, const QVector<JASPControl::ParentKey>& parentKeys = {}) { return Json::Value::null; }
	virtual bool setBoundValue(const std::string &name, const Json::Value &value, const Json::Value &meta, const QVector<JASPControl::ParentKey> &parentKeys) { return false; }

public slots:
	virtual void boundValueChangedHandler()														{}
	virtual void requestColumnCreationHandler(const std::string&columnName, columnType colType) {}
	virtual void requestComputedColumnCreationHandler(const std::string& columnName)			{}
	virtual void requestComputedColumnDestructionHandler(const std::string& columnName)			{}

signals:
	void sendRScript(QString script, QString controlName, bool whiteListedVersion);


private:
	static const std::string emptyString;

};

#endif // IANALYSIS_H
