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

#ifndef DUP_SOURCEITEM_H
#define DUP_SOURCEITEM_H

#include <QObject>
#include <QVector>
#include <QVariant>
#include <QMap>
#include <QSet>
#include <QAbstractItemModel>

#include "DUP_jasplistcontrol.h"

class DUP_SourceItem : public QObject
{
	Q_OBJECT
public:
	struct ConditionVariable
	{
		QString name,
				controlName,
				propertyName;
		bool	addQuotes = false;

		ConditionVariable(const QString& _name, const QString& _controlName, const QString& _propertyName, bool _addQuotes = false)
			: name(_name), controlName(_controlName), propertyName(_propertyName), addQuotes(_addQuotes) {}
		ConditionVariable(const ConditionVariable& source)
			: name(source.name), controlName(source.controlName), propertyName(source.propertyName), addQuotes(source.addQuotes) {}
		ConditionVariable() {}
	};

	DUP_SourceItem(
			  DUP_JASPListControl* _listControl
			, QMap<QString, QVariant>& map
			, const DUP_JASPListControl::LabelValueMap& _values
			, const QVector<DUP_SourceItem*> _rSources
			, QAbstractItemModel* _nativeModel = nullptr
			, const QVector<DUP_SourceItem*>& _discardSources = QVector<DUP_SourceItem*>()
			, const QVector<QMap<QString, QVariant> >& _conditionVariables = QVector<QMap<QString, QVariant> >()
			);

	DUP_SourceItem(DUP_JASPListControl* _listControl, const DUP_JASPListControl::LabelValueMap& _values);

	DUP_SourceItem(DUP_JASPListControl* _listControl, const QString& sourceName, const QString& sourceUse);

	DUP_SourceItem(DUP_JASPListControl* _listControl = nullptr);

	DUP_ListModel*				listModel()							{ return _listModel;				}
	const QString&			controlName()				const	{ return _controlName;				}
	const QStringList&		modelUse()					const	{ return _modelUse;					}
	bool					combineWithOtherModels()	const	{ return _combineWithOtherModels;	}
	const QSet<QString>&	usedControls()				const	{ return _usedControls;				}
	bool					isColumnsModel()			const	{ return _isColumnsModel;			}
	bool					isNativeModel()				const	{ return _nativeModel != nullptr;	}
	QAbstractItemModel*		nativeModel()						{ return _nativeModel;				}
	DUP_Terms					getTerms();

	static QVector<DUP_SourceItem*>				readAllSources(DUP_JASPListControl* _listControl);

private:
	static QString							_readSourceName(const QString& sourceNameExt, QString& sourceControl, QString& sourceUse);
	static QString							_readRSourceName(const QString& sourceNameExt, QString& sourceUse);
	static QMap<QString, QVariant>			_readSource(DUP_JASPListControl* _listControl, const QVariant& source, DUP_JASPListControl::LabelValueMap& sourceValues, QVector<DUP_SourceItem*>& rSources, QAbstractItemModel*& _nativeModel);
	static DUP_JASPListControl::LabelValueMap	_readValues(DUP_JASPListControl* _listControl, const QVariant& _values);
	static DUP_SourceItem*						_readRSource(DUP_JASPListControl* listControl, const QVariant& rSource);
	static QList<QVariant>					_getListVariant(QVariant var);

	void									_setUp();
	DUP_Terms									_readAllTerms();

private slots:
	void									_connectModels();
	void									_resetModel();
	void									_dataChangedHandler(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>());
	void									_rSourceChanged(const QString& name);

private:
	DUP_JASPListControl		*			_listControl			= nullptr;
	QString							_name,
									_controlName;
	QStringList						_modelUse;
	QVector<DUP_SourceItem*>			_discardSources;
	QVector<DUP_SourceItem*>			_rSources;
	DUP_JASPListControl::LabelValueMap	_values;
	bool							_isValuesSource			= false;
	bool							_isRSource				= false;
	DUP_ListModel			*			_listModel				= nullptr;
	QAbstractItemModel	*			_nativeModel			= nullptr;
	int								_nativeModelRole		= Qt::DisplayRole;
	bool							_isColumnsModel			= false;
	bool							_combineWithOtherModels	= false;
	QString							_conditionExpression;
	QVector<ConditionVariable>		_conditionVariables;
	QSet<QString>					_usedControls;
};

#endif // SOURCEITEM_H
