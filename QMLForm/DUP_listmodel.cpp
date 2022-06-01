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

#include <QAbstractTableModel>

#include "DUP_listmodel.h"
#include "DUP_jasplistcontrol.h"
#include "DUP_analysisform.h"
#include "DUP_rowcontrols.h"
#include "DUP_sourceitem.h"
#include "log.h"

DUP_ListModel::DUP_ListModel(DUP_JASPListControl* listView)
	: QAbstractTableModel(listView)
	, _listView(listView)
{
	setInfoProvider(listView->form());

	// Connect all apecific signals to a general signal
	connect(this,	&DUP_ListModel::modelReset,				this,	&DUP_ListModel::termsChanged);
	connect(this,	&DUP_ListModel::rowsRemoved,			this,	&DUP_ListModel::termsChanged);
	connect(this,	&DUP_ListModel::rowsMoved,				this,	&DUP_ListModel::termsChanged);
	connect(this,	&DUP_ListModel::rowsInserted,			this,	&DUP_ListModel::termsChanged);
	connect(this,	&DUP_ListModel::dataChanged,			this,	&DUP_ListModel::dataChangedHandler);
	connect(this,	&DUP_ListModel::namesChanged,			this,	&DUP_ListModel::termsChanged);
	connect(this,	&DUP_ListModel::columnTypeChanged,		this,	&DUP_ListModel::termsChanged);
}

QHash<int, QByteArray> DUP_ListModel::roleNames() const
{
	static QHash<int, QByteArray>	roles = QAbstractTableModel::roleNames();
	static bool						setMe = true;

	if(setMe)
	{
		roles[TypeRole]						= "type";
		roles[SelectedRole]					= "selected";
		roles[SelectableRole]				= "selectable";
		roles[ColumnTypeRole]				= "columnType";
		roles[ColumnTypeIconRole]			= "columnTypeIcon";
		roles[ColumnTypeDisabledIconRole]	= "columnTypeDisabledIcon";
		roles[NameRole]						= "name";
		roles[RowComponentRole]				= "rowComponent";
		roles[ValueRole]					= "value";
		roles[VirtualRole]					= "virtual";
		roles[DeletableRole]				= "deletable";

		setMe = false;
	}

	return roles;
}

void DUP_ListModel::refresh()
{
	beginResetModel(); 
	endResetModel();
}

void DUP_ListModel::addControlError(const QString &error) const
{
	_listView->addControlError(error);
}

void DUP_ListModel::initTerms(const DUP_Terms &terms, const RowControlsValues& allValuesMap)
{
	_initTerms(terms, allValuesMap, true);
}

void DUP_ListModel::_initTerms(const DUP_Terms &terms, const RowControlsValues& allValuesMap, bool setupControlConnections)
{
	beginResetModel();
	_rowControlsValues = allValuesMap;
	_setTerms(terms);
	endResetModel();

	if (setupControlConnections) _connectAllSourcesControls();
}

void DUP_ListModel::_connectAllSourcesControls()
{
	for (DUP_SourceItem* sourceItem : listView()->sourceItems())
		_connectSourceControls(sourceItem->listModel(), sourceItem->usedControls());
}

void DUP_ListModel::_connectSourceControls(DUP_ListModel* sourceModel, const QSet<QString>& controls)
{
	// Connect option changes from controls in sourceModel that influence the terms of this model
	if (!sourceModel || controls.size() == 0) return;

	const DUP_Terms& terms = sourceModel->terms();

	for (const QString& controlName : controls)
	{
		for (const DUP_Term& term : terms)
		{
			DUP_JASPControl* control = sourceModel->getRowControl(term.asQString(), controlName);
			if (control)
			{
				DUP_BoundControl* boundControl = control->boundControl();
				if (boundControl && !_rowControlsConnected.contains(boundControl))
				{
					connect(control, &DUP_JASPControl::boundValueChanged, this, &DUP_ListModel::sourceTermsReset);
					_rowControlsConnected.push_back(boundControl);
				}
			}
			else
				Log::log() << "Cannot find control " << controlName << " in model " << name() << std::endl;
		}
	}
}

DUP_Terms DUP_ListModel::getSourceTerms()
{
	DUP_Terms termsAvailable;

	listView()->applyToAllSources([&](DUP_SourceItem *sourceItem, const DUP_Terms& terms)
	{
		_connectSourceControls(sourceItem->listModel(), sourceItem->usedControls());
		termsAvailable.add(terms);
	});
	
	return termsAvailable;
}

DUP_ListModel *DUP_ListModel::getSourceModelOfTerm(const DUP_Term &term)
{
	DUP_ListModel* result = nullptr;

	listView()->applyToAllSources([&](DUP_SourceItem *sourceItem, const DUP_Terms& terms)
	{
		if (terms.contains(term))
			result = sourceItem->listModel();
	});

	return result;
}

void DUP_ListModel::setRowComponent(QQmlComponent* rowComponent)
{
	_rowComponent = rowComponent;
}

void DUP_ListModel::setUpRowControls()
{
	if (_rowComponent == nullptr)
		return;

	int row = 0;
	for (const DUP_Term& term : terms())
	{
		const QString& key = term.asQString();
		if (!_rowControlsMap.contains(key))
		{
			bool hasOptions = _rowControlsValues.contains(key);
			DUP_RowControls* rowControls = new DUP_RowControls(this, _rowComponent, _rowControlsValues[key]);
			_rowControlsMap[key] = rowControls;
			rowControls->init(row, term, !hasOptions);
		}
		else
			_rowControlsMap[key]->setContext(row, key);
		row++;
	}
}

DUP_JASPControl *DUP_ListModel::getRowControl(const QString &key, const QString &name) const
{
	DUP_JASPControl* control = nullptr;

	DUP_RowControls* rowControls = _rowControlsMap.value(key);
	if (rowControls)
	{
		const QMap<QString, DUP_JASPControl*>& controls = rowControls->getJASPControlsMap();
		if (controls.contains(name))
			control = controls[name];
	}

	return control;
}

bool DUP_ListModel::addRowControl(const QString &key, DUP_JASPControl *control)
{
	return _rowControlsMap.contains(key) ? _rowControlsMap[key]->addJASPControl(control) : false;
}

QStringList DUP_ListModel::termsTypes()
{
	QSet<QString> types;

	for (const DUP_Term& term : terms())
	{
		columnType type = columnType(requestInfo(term, DUP_VariableInfo::VariableType).toInt());
		if (type != columnType::unknown)
			types.insert(columnTypeToQString(type));
	}

	return types.values();
}

int DUP_ListModel::searchTermWith(QString searchString)
{
	int result = -1;
	const DUP_Terms& myTerms = terms();
	int startIndex = 0;
	if (_selectedItems.length() > 0)
	{
		startIndex = _selectedItems.first();
		if (searchString.length() == 1)
			startIndex++;
	}

	if (searchString.length() > 0)
	{
		QString searchStringLower = searchString.toLower();
		for (size_t i = 0; i < myTerms.size(); i++)
		{
			size_t index = (size_t(startIndex) + i) % myTerms.size();
			const DUP_Term& term = myTerms.at(index);
			if (term.asQString().toLower().startsWith(searchStringLower))
			{
				result = int(index);
				break;
			}
		}
	}

	return result;
}

void DUP_ListModel::_addSelectedItemType(int _index)
{
	QString type = data(index(_index, 0), DUP_ListModel::ColumnTypeRole).toString();
	if (!type.isEmpty())
		_selectedItemsTypes.insert(type);
}

void DUP_ListModel::selectItem(int _index, bool _select)
{
	bool changed = false;
	if (_select)
	{
		if (data(index(_index, 0), DUP_ListModel::SelectableRole).toBool())
		{
			int i = 0;
			for (; i < _selectedItems.length(); i++)
			{
				if (_selectedItems[i] == _index)
					break;
				else if (_selectedItems[i] > _index)
				{
					_selectedItems.insert(i, _index);
					_addSelectedItemType(_index);
					changed = true;
					break;
				}
			}
			if (i == _selectedItems.length())
			{
				_selectedItems.append(_index);
				_addSelectedItemType(_index);
				changed = true;
			}
		}
	}
	else
	{
		if (_selectedItems.removeAll(_index) > 0)
		{
			_selectedItemsTypes.clear();
			for (int i : _selectedItems)
			{
				QString type = data(index(i, 0), DUP_ListModel::ColumnTypeRole).toString();
				if (!type.isEmpty())
					_selectedItemsTypes.insert(type);
			}
			changed = true;
		}
	}

	if (changed)
	{
		emit dataChanged(index(_index, 0), index(_index, 0), { DUP_ListModel::SelectedRole });
		emit selectedItemsChanged();
	}
}

void DUP_ListModel::clearSelectedItems(bool emitSelectedChange)
{
	QList<int> selected = _selectedItems;

	_selectedItems.clear();
	_selectedItemsTypes.clear();

	for (int i : selected)
		emit dataChanged(index(i,0), index(i,0), { DUP_ListModel::SelectedRole });

	if (selected.length() > 0 && emitSelectedChange)
		emit selectedItemsChanged();
}

void DUP_ListModel::setSelectedItem(int _index)
{
	if (_selectedItems.size() == 1 && _selectedItems[0] == _index) return;

	clearSelectedItems(false);
	selectItem(_index, true);
}

void DUP_ListModel::selectAllItems()
{
	int nbTerms = rowCount();
	if (nbTerms == 0) return;

	_selectedItems.clear();
	_selectedItemsTypes.clear();

	for (int i = 0; i < nbTerms; i++)
	{
		if (data(index(i, 0), DUP_ListModel::SelectableRole).toBool())
		{
			_selectedItems.append(i);
			_addSelectedItemType(i);
		}
	}

	emit dataChanged(index(0, 0), index(nbTerms - 1, 0), { DUP_ListModel::SelectedRole });
	emit selectedItemsChanged();
}

void DUP_ListModel::sourceTermsReset()
{
	_initTerms(getSourceTerms(), RowControlsValues(), false);
}

int DUP_ListModel::rowCount(const QModelIndex &) const
{
	return int(terms().size());
}

QVariant DUP_ListModel::data(const QModelIndex &index, int role) const
{
	int row = index.row();
	const DUP_Terms& myTerms = terms();
	size_t row_t = size_t(row);
	if (row_t >= myTerms.size())
		return QVariant();

	switch (role)
	{
	case Qt::DisplayRole:
	case DUP_ListModel::NameRole:			return QVariant(myTerms.at(row_t).asQString());
	case DUP_ListModel::SelectableRole:		return !myTerms.at(row_t).asQString().isEmpty();
	case DUP_ListModel::SelectedRole:		return _selectedItems.contains(row);
	case DUP_ListModel::RowComponentRole:
	{
		QString term = myTerms.at(row_t).asQString();
		return _rowControlsMap.contains(term) ? QVariant::fromValue(_rowControlsMap[term]->getRowObject()) : QVariant();
	}
	case DUP_ListModel::TypeRole:			return listView()->containsVariables() ? "variable" : "";
	case DUP_ListModel::ColumnTypeRole:
	case DUP_ListModel::ColumnTypeIconRole:
	case DUP_ListModel::ColumnTypeDisabledIconRole:
	{
		const DUP_Term& term = myTerms.at(row_t);
		if (!listView()->containsVariables() || term.size() != 1)	return "";
		if (role == DUP_ListModel::ColumnTypeRole)						return requestInfo(term, DUP_VariableInfo::VariableTypeName);
		else if (role == DUP_ListModel::ColumnTypeIconRole)				return requestInfo(term, DUP_VariableInfo::VariableTypeIcon);
		else if (role == DUP_ListModel::ColumnTypeDisabledIconRole)		return requestInfo(term, DUP_VariableInfo::VariableTypeDisabledIcon);
		break;
	}
	}

	return QVariant();
}

const QString &DUP_ListModel::name() const
{
	return _listView->name();
}

DUP_Terms DUP_ListModel::filterTerms(const DUP_Terms& terms, const QStringList& filters)
{
	if (filters.empty())	return terms;

	DUP_Terms result = terms;

	const QString typeIs = "type=";
	const QString controlIs = "control=";

	QString useTheseVariableTypes, useThisControl;

	for (const QString& filter : filters)
	{
		if (filter.startsWith(typeIs))		useTheseVariableTypes	= filter.right(filter.length() - typeIs.length());
		if (filter.startsWith(controlIs))	useThisControl			= filter.right(filter.length() - controlIs.length());
	}

	if (!useTheseVariableTypes.isEmpty())
	{
		result.clear();
		QStringList typesStr = useTheseVariableTypes.split("|");
		QList<columnType> types;

		for (const QString& typeStr : typesStr)
		{
			columnType type = columnTypeFromQString(typeStr, columnType::unknown);
			if (type != columnType::unknown)
				types.push_back(type);
		}

		for (const DUP_Term& term : terms)
		{
			columnType type = columnType(requestInfo(term, DUP_VariableInfo::VariableType).toInt());
			if (types.contains(type))
				result.add(term);
		}
	}

	if (!useThisControl.isEmpty())
	{
		DUP_Terms controlTerms;
		for (const DUP_Term& term : result)
		{
			DUP_RowControls* rowControls = _rowControlsMap.value(term.asQString());
			if (rowControls)
			{
				DUP_JASPControl* control = rowControls->getJASPControl(useThisControl);

				if (control)	controlTerms.add(control->property("value").toString());
				else			Log::log() << "Could not find control " << useThisControl << " in list view " << name() << std::endl;
			}
		}
		result = controlTerms;
	}

	if (filters.contains("levels"))
	{
		DUP_Terms allLabels;
		for (const DUP_Term& term : result)
		{
			DUP_Terms labels = requestInfo(term, DUP_VariableInfo::Labels).toStringList();
			if (labels.size() > 0)	allLabels.add(labels);
			else					allLabels.add(term);
		}

		result = allLabels;
	}

	return result;
}

DUP_Terms DUP_ListModel::termsEx(const QStringList &filters)
{
	return filterTerms(terms(), filters);
}

void DUP_ListModel::sourceNamesChanged(QMap<QString, QString> map)
{
	QMap<QString, QString>	changedNamesMap;
	QSet<int>				changedIndexes;

	QMapIterator<QString, QString> it(map);
	while (it.hasNext())
	{
		it.next();
		const QString& oldName = it.key(), newName = it.value();
		QSet<int> indexes = _terms.replaceVariableName(oldName.toStdString(), newName.toStdString());
		if (indexes.size() > 0)
		{
			changedNamesMap[oldName] = newName;
			changedIndexes += indexes;
		}
	}

	for (int i : changedIndexes)
	{
		QModelIndex ind = index(i, 0);
		emit dataChanged(ind, ind);
	}

	if (changedNamesMap.size() > 0)
		emit namesChanged(changedNamesMap);
}

int DUP_ListModel::sourceColumnTypeChanged(QString name)
{
	int i = terms().indexOf(name);
	if (i >= 0)
	{
		QModelIndex ind = index(i, 0);
		emit dataChanged(ind, ind);
		emit columnTypeChanged(name);
	}

	return i;
}

bool DUP_ListModel::sourceLabelsChanged(QString columnName, QMap<QString, QString> changedLabels)
{
	bool change = false;
	if (_columnsUsedForLabels.contains(columnName))
	{
		if (changedLabels.size() == 0)
		{
			// The changed labels are not specified. Requery the source.
			sourceTermsReset();
			change = true;
		}
		else
		{
			QMap<QString, QString> newChangedValues;
			QMapIterator<QString, QString> it(changedLabels);
			while (it.hasNext())
			{
				it.next();
				if (terms().contains(it.key()))
				{
					change = true;
					newChangedValues[it.key()] = it.value();
				}
			}
			sourceNamesChanged(newChangedValues);
		}
	}
	else
	{
		change = terms().contains(columnName);
		if (change)
			emit labelsChanged(columnName, changedLabels);
	}

	return change;
}

bool DUP_ListModel::sourceLabelsReordered(QString columnName)
{
	bool change = false;
	if (_columnsUsedForLabels.contains(columnName))
	{
		sourceTermsReset();
		change = true;
	}
	else
	{
		change = terms().contains(columnName);
		if (change)
			emit labelsReordered(columnName);
	}

	return change;
}

void DUP_ListModel::sourceColumnsChanged(QStringList columns)
{
	QStringList changedColumns;

	for (const QString& column : columns)
	{
		if (_columnsUsedForLabels.contains(column))
			sourceLabelsChanged(column);
		else if (terms().contains(column))
			changedColumns.push_back(column);
	}

	if (changedColumns.size() > 0)
	{
		emit columnsChanged(changedColumns);

		if (listView()->isBound())
			listView()->form()->refreshAnalysis();
	}
}

void DUP_ListModel::dataChangedHandler(const QModelIndex &, const QModelIndex &, const QVector<int> &roles)
{
	if (roles.isEmpty() || roles.size() > 1 || roles[0] != DUP_ListModel::SelectedRole)
		emit termsChanged();
}

void DUP_ListModel::_setTerms(const DUP_Terms &terms, const DUP_Terms& parentTerms)
{
	_terms.removeParent();
	_setTerms(terms);
	_terms.setSortParent(parentTerms);
}

void DUP_ListModel::_setTerms(const std::vector<DUP_Term> &terms)
{
	_terms.set(terms);
	setUpRowControls();
}

void DUP_ListModel::_setTerms(const DUP_Terms &terms)
{
	_terms.set(terms);
	setUpRowControls();
}

void DUP_ListModel::_removeTerms(const DUP_Terms &terms)
{
	_terms.remove(terms);
	setUpRowControls();
}

void DUP_ListModel::_removeTerm(int index)
{
	_terms.remove(size_t(index));
	setUpRowControls();
}

void DUP_ListModel::_removeTerm(const DUP_Term &term)
{
	_terms.remove(term);
	setUpRowControls();
}

void DUP_ListModel::_removeLastTerm()
{
	if (_terms.size() > 0)
		_terms.remove(_terms.size() - 1);
}

void DUP_ListModel::_addTerms(const DUP_Terms &terms)
{
	_terms.add(terms);
	setUpRowControls();
}

void DUP_ListModel::_addTerm(const QString &term, bool isUnique)
{
	_terms.add(term, isUnique);
	setUpRowControls();
}

void DUP_ListModel::_replaceTerm(int index, const DUP_Term &term)
{
	_terms.replace(index, term);
	setUpRowControls();
}
