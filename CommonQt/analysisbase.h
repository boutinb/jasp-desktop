//
// Copyright (C) 2013-2025 University of Amsterdam
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public
// License along with this program.  If not, see
// <http://www.gnu.org/licenses/>.
//

#ifndef ANALYSISBASE_H
#define ANALYSISBASE_H

#include <QObject>
#include <QQuickItem>
#include <json/json.h>
#include "appinfo.h"
#include "utils.h"
#include "columntype.h"
#include "utilities/qutils.h"
#include "commotqtglobals.h"

class AnalysisFormBase;

class COMMONQT_EXPORTS AnalysisBase : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QQuickItem		*	formItem				READ formItem										NOTIFY formItemChanged			)
	Q_PROPERTY(QString				qmlError				READ qmlError			WRITE setQmlError			NOTIFY qmlErrorChanged			)

public:
    struct ParentKey
    {
        std::string name, key;
        std::vector<std::string> value;
        ParentKey(const std::string & _name, const std::string & _key, const std::vector<std::string>& _value)
            : name(_name), key(_key), value(_value) {}
    };

	explicit AnalysisBase(QObject *parent = nullptr, Version moduleVersion = AppInfo::version);
	AnalysisBase(QObject *parent, AnalysisBase* duplicateMe);

	virtual				bool				isOwnComputedColumn(const std::string &col)					const	{ return false; }
	virtual				void				refresh()															{}
	virtual				void				run()																{}
	virtual				void				reloadForm()														{}
	virtual				void				exportResults()														{}
	virtual				bool				isDuplicate()												const	{ return false;				}
	virtual				bool				wasUpgraded()												const	{ return false;				}
	virtual				bool				needsRefresh()												const	{ return false;				}
	virtual				const std::string & module()													const	{ return emptyString;		}
	virtual				const std::string & name()														const	{ return emptyString;		}
	virtual				const std::string & title()														const	{ return emptyString;		}
	virtual				void				setTitle(const std::string& titel)									{}
	virtual				void				preprocessMarkdownHelp(const QString& md)					const	{}
	virtual				QString				helpFile()															{ return "";				}
	virtual				const stringvec   & upgradeMsgsForOption(const std::string& name)				const	{ return emptyStringVec;	}
	virtual				const Json::Value & resultsMeta()												const 	{ return Json::Value::null;	}
	virtual				const Json::Value & getRSource(const std::string& name)							const 	{ return Json::Value::null;	}
    virtual				void				initialized(AnalysisFormBase* form, bool isNewAnalysis)					{}
	virtual				std::string			qmlFormPath(bool addFileProtocol = true,
											bool ignoreReadyForUse = false)								const;
	virtual Q_INVOKABLE	QString				helpFile()													const	{ return ""; }
	virtual Q_INVOKABLE void				createForm(QQuickItem* parentItem=nullptr);
	virtual				void				destroyForm();
	virtual				bool				isColumnFreeOrMine(const QString & columnName)				const	{ return false; }

						const Json::Value &	boundValues()												const	{ return _boundValues;		}
						const Json::Value &	orgBoundValues()											const	{ return _orgBoundValues;	}
						const Json::Value &	boundValue(const std::string& name,
                                                         const QVector<AnalysisBase::ParentKey>& parentKeys = {});

                        void				setBoundValue(const std::string& name, const Json::Value& value, const Json::Value& meta, const QVector<AnalysisBase::ParentKey>& parentKeys = {});
						void				setBoundValues(const Json::Value& boundValues);
						void				setOrgBoundValues(const Json::Value& orgBoundValues)				{ _orgBoundValues = orgBoundValues; }
						const Json::Value	optionsMeta()												const	{ return _boundValues.get(".meta", Json::nullValue);	}
						void				clearOptions()														{ _boundValues.clear();		}

						const Version	  &	moduleVersion()												const	{ return _moduleVersion;	}

						QQuickItem		  *	formItem()													const;

						const QString	  &	qmlError()													const;
						void				setQmlError(const QString &newQmlError);
						void				sendRScript(const QString & script, const QString & controlName, bool whiteListedVersion)		{ emit sendRScriptSignal(script, controlName, whiteListedVersion, tq(module())); }
						void				sendFilter(	const QString & name)																{ emit sendFilterSignal(name, tq(module())); }


public slots:
	virtual void	boundValueChangedHandler()																	{}
	virtual void	requestColumnCreationHandler(			const std::string & columnName, columnType colType)	{}
	virtual void	requestComputedColumnCreationHandler(	const std::string & columnName)						{}
	virtual void	requestComputedColumnDestructionHandler(const std::string & columnName)						{}
	virtual void	onUsedVariablesChanged()																	{}
	

signals:
	void			sendRScriptSignal(QString script, QString controlName, bool whiteListedVersion, QString module);
	void			sendFilterSignal( QString  name,  QString module);
	void			formItemChanged();
	void			qmlErrorChanged();
	void			boundValuesChanged();


protected:
    Json::Value&	_getParentBoundValue(const QVector<AnalysisBase::ParentKey> & parentKeys, QVector<std::string>& parentNames, bool & found, bool createAnyway = false);
    std::string		_displayParentKeys(const QVector<AnalysisBase::ParentKey> & parentKeys) const;


    AnalysisFormBase*   _analysisForm		= nullptr;
    QQuickItem	*       _parentItem         = nullptr;
    QString             _qmlError;
    Version             _moduleVersion;

private:
	Json::Value		_boundValues		= Json::objectValue,
					_orgBoundValues		= Json::objectValue;



protected:
	static const std::string	emptyString; ///< Otherwise we return references to a temporary object (std::string(""))
	static const stringvec		emptyStringVec;
};

#endif // ANALYSISBASE_H
