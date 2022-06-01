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

#include "DUP_listmodelavailableinterface.h"
#include "DUP_listmodelassignedinterface.h"
#include "DUP_jasplistcontrol.h"
#include "log.h"

void DUP_ListModelAvailableInterface::initTerms(const DUP_Terms &terms, const RowControlsValues&)
{
	beginResetModel();
	
	_allTerms = _allSortedTerms = terms;
	_setTerms(terms, _allSortedTerms);

	if (currentSortType() != SortType::None)
		DUP_Sortable::sortItems();

	removeTermsInAssignedList();
	endResetModel();
}

QVariant DUP_ListModelAvailableInterface::requestInfo(const DUP_Term &term, DUP_VariableInfo::InfoType info) const
{
	return DUP_VariableInfoConsumer::requestInfo(term, info);
}

void DUP_ListModelAvailableInterface::sortItems(SortType sortType)
{
	beginResetModel();

	switch(sortType)
	{
	case SortType::None:
	{
		DUP_Terms allowed, forbidden;

		for (const DUP_Term &term : _allTerms)
		{
			if ( ! isAllowed(term))	forbidden.add(term);
			else					allowed.add(term);
		}

		_allTerms.clear();
		_allTerms.add(allowed);
		_allTerms.add(forbidden);
		_allSortedTerms = _allTerms;
		break;
	}

	case SortType::SortByName:
	{
		QList<QString> sortedTerms = _allSortedTerms.asQList();
		std::sort(sortedTerms.begin(), sortedTerms.end(),
				  [&](const QString& a, const QString& b) {
						return a.compare(b, Qt::CaseInsensitive) < 0;
					});
		_allSortedTerms = DUP_Terms(sortedTerms);
		break;
	}

	case SortType::SortByType:
	{
		QList<QString>				termsList = _allSortedTerms.asQList();
		QList<QPair<QString, int> > termsTypeList;

		for (const QString& term : termsList)
			termsTypeList.push_back(QPair<QString, int>(term, requestInfo(term, DUP_VariableInfo::VariableType).toInt()));

		std::sort(termsTypeList.begin(), termsTypeList.end(),
				  [&](const QPair<QString, int>& a, const QPair<QString, int>& b) {
						return a.second - b.second > 0;
					});

		QList<QString> sortedTerms;

		for (const auto& term : termsTypeList)
			sortedTerms.push_back(term.first);

		_allSortedTerms = DUP_Terms(sortedTerms);
		break;
	}

	default:
		Log::log() << "Unimplemented sort in ListModelAvailableInterface::sortItems!";
		break;
	}

	DUP_Terms orgTerms = terms();
	_setTerms(orgTerms); // This will reorder the terms

	endResetModel();
}

void DUP_ListModelAvailableInterface::sourceTermsReset()
{
	resetTermsFromSources();
}

void DUP_ListModelAvailableInterface::sourceNamesChanged(QMap<QString, QString> map)
{
	DUP_ListModelDraggable::sourceNamesChanged(map);

	QMap<QString, QString>	allTermsChangedMap;
	QMapIterator<QString, QString> it(map);

	while (it.hasNext())
	{
		it.next();
		const QString& oldName = it.key(), newName = it.value();

		QSet<int> allIndexes = _allTerms.replaceVariableName(oldName.toStdString(), newName.toStdString());

		if (allIndexes.size() > 0)
			allTermsChangedMap[oldName] = newName;
	}

	if (allTermsChangedMap.size() > 0)
		emit namesChanged(allTermsChangedMap);
}

void DUP_ListModelAvailableInterface::sourceColumnsChanged(QStringList columns)
{
	DUP_ListModelDraggable::sourceColumnsChanged(columns);

	QStringList changedColumns;

	for (const QString& column : columns)
	{
		if (_allTerms.contains(column))
			changedColumns.push_back(column);
	}

	if (changedColumns.size() > 0)
		emit columnsChanged(changedColumns);
}

int DUP_ListModelAvailableInterface::sourceColumnTypeChanged(QString name)
{
	int index = DUP_ListModelDraggable::sourceColumnTypeChanged(name);

	if (index == -1 && _allTerms.contains(name))
		emit columnTypeChanged(name);

	return index;
}

bool DUP_ListModelAvailableInterface::sourceLabelsChanged(QString columnName, QMap<QString, QString> changedLabels)
{
	bool change = DUP_ListModelDraggable::sourceLabelsChanged(columnName, changedLabels);

	if (!change && _allTerms.contains(columnName))
		emit labelsChanged(columnName, changedLabels);

	return change;
}

bool DUP_ListModelAvailableInterface::sourceLabelsReordered(QString columnName)
{
	bool change = DUP_ListModelDraggable::sourceLabelsReordered(columnName);

	if (!change && _allTerms.contains(columnName))
		emit labelsReordered(columnName);

	return change;
}

void DUP_ListModelAvailableInterface::removeTermsInAssignedList()
{
	beginResetModel();
	
	DUP_Terms newTerms = _allSortedTerms;
	
	for (DUP_ListModelAssignedInterface* modelAssign : assignedModel())
	{
		DUP_Terms assignedTerms = modelAssign->terms();
		if (assignedTerms.discardWhatIsntTheseTerms(_allSortedTerms))
			modelAssign->initTerms(assignedTerms); // initTerms call removeTermsInAssignedList
		else if (!modelAssign->copyTermsWhenDropped())
			newTerms.remove(assignedTerms);
	}

	_setTerms(newTerms, _allSortedTerms);
	
	endResetModel();
}

void DUP_ListModelAvailableInterface::addAssignedModel(DUP_ListModelAssignedInterface *assignedModel)
{
	_assignedModels.push_back(assignedModel);

	connect(assignedModel,	&DUP_ListModelAssignedInterface::destroyed,				this,						&DUP_ListModelAvailableInterface::removeAssignedModel		);
	connect(this,			&DUP_ListModelAvailableInterface::availableTermsReset,	assignedModel,				&DUP_ListModelAssignedInterface::availableTermsResetHandler	);
	connect(this,			&DUP_ListModelAvailableInterface::namesChanged,			assignedModel,				&DUP_ListModelAssignedInterface::sourceNamesChanged			);
	connect(this,			&DUP_ListModelAvailableInterface::columnsChanged,		assignedModel,				&DUP_ListModelAssignedInterface::sourceColumnsChanged		);
	connect(this,			&DUP_ListModelAvailableInterface::columnTypeChanged,	assignedModel,				&DUP_ListModelAssignedInterface::sourceColumnTypeChanged	);
	connect(this,			&DUP_ListModelAvailableInterface::labelsChanged,		assignedModel,				&DUP_ListModelAssignedInterface::sourceLabelsChanged		);
	connect(this,			&DUP_ListModelAvailableInterface::labelsReordered,		assignedModel,				&DUP_ListModelAssignedInterface::sourceLabelsReordered		);
	connect(listView(),		&DUP_JASPListControl::containsVariablesChanged,			assignedModel->listView(),	&DUP_JASPListControl::setContainsVariables					);
	connect(listView(),		&DUP_JASPListControl::containsInteractionsChanged,		assignedModel->listView(),	&DUP_JASPListControl::setContainsInteractions				);
}

void DUP_ListModelAvailableInterface::removeAssignedModel(DUP_ListModelDraggable* assignedModel)
{
	_assignedModels.removeAll(qobject_cast<DUP_ListModelAssignedInterface*>(assignedModel));
}

