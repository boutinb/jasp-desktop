//
// Copyright (C) 2013-2018 University of Amsterdam
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

#ifndef DUP_ANALYSISFORM_H
#define DUP_ANALYSISFORM_H

#include <QMap>
#include <QQuickItem>

#include "DUP_boundcontrol.h"
#include "DUP_variableinfo.h"
#include "DUP_listmodel.h"
#include "DUP_jaspcontrol.h"
#include "DUP_qutils.h"
#include "analysisinterface.h"

#include <queue>

class DUP_ExpanderButtonBase;

///
/// The backend for the `Form{}` used in all JASP's well, qml forms
/// This is directly a QQuickItem and connects to Analyses to inform it when the options change.
/// Each element placed inside the Form will be a derivative of JASPControl
/// A JASPControl can be also a BoundControl: it is bound in the sense that any changes to its values will be propagated to Analysis and stored there through `Analysis::setBoundValue`
/// Each time a JASPControl is initialized, it is registered by the form, and when the form is initialized it sets up the JASPControls either by their default values, 
/// or by the values stored in a JASP file (when a JASP file is loaded).
/// The interface errors and warnings are managed by this class.
class DUP_AnalysisForm : public QQuickItem, public DUP_VariableInfoProvider
{
	Q_OBJECT
	Q_PROPERTY(QString		errors				READ errors											NOTIFY errorsChanged			)
	Q_PROPERTY(QString		warnings			READ warnings										NOTIFY warningsChanged			)
	Q_PROPERTY(bool			needsRefresh		READ needsRefresh									NOTIFY needsRefreshChanged		)
	Q_PROPERTY(bool			hasVolatileNotes	READ hasVolatileNotes								NOTIFY hasVolatileNotesChanged	)
	Q_PROPERTY(bool			runOnChange			READ runOnChange		WRITE setRunOnChange		NOTIFY runOnChangeChanged		)
	Q_PROPERTY(QString		info				READ info				WRITE setInfo				NOTIFY infoChanged				)
	Q_PROPERTY(QString		helpMD				READ helpMD											NOTIFY helpMDChanged			)
	Q_PROPERTY(QVariant		analysis			READ analysis			WRITE setAnalysis			NOTIFY analysisChanged			)

public:
	explicit					DUP_AnalysisForm(QQuickItem * = nullptr);
				void			bindTo();
				void			unbind();

				void			runRScript(QString script, QString controlName, bool whiteListedVersion);
				
				void			itemChange(QQuickItem::ItemChange change, const QQuickItem::ItemChangeData &value) override;

				void			setMustBe(		std::set<std::string>						mustBe);
				void			setMustContain(	std::map<std::string,std::set<std::string>> mustContain);

				bool			runOnChange()	{ return _runOnChange; }
				void			setRunOnChange(bool change);
				void			blockValueChangeSignal(bool block, bool notifyOnceUnblocked = true);

public slots:
				void			runScriptRequestDone(const QString& result, const QString& requestId);
				void			setInfo(QString info);
				void			setAnalysis(QVariant analysis);
				void			boundValueChangedHandler(DUP_JASPControl* control);

signals:
				void			sendRScript(QString script, int key);
				void			formCompleted();
				void			refreshTableViewModels();
				void			errorMessagesItemChanged();
				void			languageChanged();
				void			needsRefreshChanged();
				void			hasVolatileNotesChanged();
				void			runOnChangeChanged();
				void			infoChanged();
				void			helpMDChanged();
				void			errorsChanged();
				void			warningsChanged();
				void			analysisChanged();
				void			rSourceChanged(const QString& name);

protected:
				QVariant		requestInfo(const DUP_Term &term, DUP_VariableInfo::InfoType info)	const override;

public:
	DUP_ListModel			*	getModel(const QString& modelName)								const	{ return _modelMap.count(modelName) > 0 ? _modelMap[modelName] : nullptr;	} // Maps create elements if they do not exist yet
	void					addModel(DUP_ListModel* model)												{ if (!model->name().isEmpty())	_modelMap[model->name()] = model;			}
	DUP_JASPControl			*	getControl(const QString& name)											{ return _controls.contains(name) ? _controls[name] : nullptr;				}
	void					addListView(DUP_JASPListControl* listView, DUP_JASPListControl* sourceListView);
	DUP_ExpanderButtonBase	*	nextExpander(DUP_ExpanderButtonBase* expander)								{ return _nextExpanderMap[expander];										}
	void					addControl(DUP_JASPControl* control);

	Q_INVOKABLE void		clearFormErrors();
	Q_INVOKABLE void		clearFormWarnings();
	Q_INVOKABLE void		exportResults();
	Q_INVOKABLE void		addFormError(const QString& message);
	Q_INVOKABLE void		refreshAnalysis();
	Q_INVOKABLE bool		initialized()	const	{ return _initialized; }

	void		addControlError(DUP_JASPControl* control, QString message, bool temporary = false, bool warning = false);
	void		clearControlError(DUP_JASPControl* control);
	void		cleanUpForm();
	bool		hasError();

	bool		isOwnComputedColumn(const std::string& col)			const;

	bool		needsRefresh()		const;
	bool		hasVolatileNotes()	const;

	QString		info()				const { return _info; }
	QString		helpMD()			const;
	QString		metaHelpMD()		const;
	QString		errors()			const {	return msgsListToString(_formErrors);	}
	QString		warnings()			const { return msgsListToString(_formWarnings);	}

	std::vector<std::vector<std::string> >	getValuesFromRSource(const QString& sourceID, const QStringList& searchPath);
	void		addColumnControl(DUP_JASPControl* control, bool isComputed);

	bool		setBoundValue(const std::string& name, const Json::Value& value, const Json::Value& meta, const QVector<DUP_JASPControl::ParentKey>& parentKeys = {});
	std::set<std::string> usedVariables();
	QVariant	analysis()			const { return QVariant::fromValue(_analysis);	}
	AnalysisInterface*	analysisObj()		const { return _analysis;				}

	void		sortControls(QList<DUP_JASPControl*>& controls);

protected:
	QString		msgsListToString(const QStringList & list) const;

private:
	Json::Value& _getParentBoundValue(const QVector<DUP_JASPControl::ParentKey>& parentKeys);
	void		_setUpControls();
	void		_setUpModels();
	void		_setUp();
	void		_orderExpanders();
	QString		_getControlLabel(QString controlName);
	void		_addLoadingError(QStringList wrongJson);
	void		setControlIsDependency(	QString controlName, bool isDependency);
	void		setControlMustContain(	QString controlName, QStringList containThis);
	void		setControlIsDependency(	std::string controlName, bool isDependency)					{ setControlIsDependency(tq(controlName), isDependency);	}
	void		setControlMustContain(	std::string controlName, std::set<std::string> containThis)	{ setControlMustContain(tq(controlName), tql(containThis)); }
	void		setAnalysisUp();
	std::vector<std::vector<std::string> > _getValuesFromJson(const Json::Value& jsonValues, const QStringList& searchPath);

private slots:
	   void		formCompletedHandler();
	   void		knownIssuesUpdated();

protected:
	AnalysisInterface							*	_analysis			= nullptr;

	QMap<QString, DUP_JASPControl* >				_controls;

	///Ordered on dependencies within QML, aka an assigned variables list depends on the available list it is connected to.
	QVector<DUP_JASPControl*>						_dependsOrderedCtrls;
	QMap<QString, DUP_ListModel* >					_modelMap;
	QVector<DUP_ExpanderButtonBase*>				_expanders;
	QMap<DUP_ExpanderButtonBase*, DUP_ExpanderButtonBase*>	_nextExpanderMap;
	QMap<DUP_JASPControl*, DUP_ExpanderButtonBase*>		_controlExpanderMap;
	bool										_removed = false;
	std::set<std::string>						_mustBe;
	std::map<std::string,std::set<std::string>>	_mustContain;
	
private:
	QStringList									_formErrors,
												_formWarnings;

	QQmlComponent*								_controlErrorMessageComponent = nullptr;
	QList<QQuickItem*>							_controlErrorMessageCache;
	bool										_runOnChange	= true,
												_formCompleted = false,
												_initialized = false;
	QString										_info;
	int											_signalValueChangedBlocked = 0;
	bool										_signalValueChangedWasEmittedButBlocked = false;
	
	std::queue<std::tuple<QString, QString, bool>>	_waitingRScripts; //Sometimes signals are blocked, and thus rscripts. But they shouldnt just disappear right?
};

#endif // ANALYSISFORM_H
