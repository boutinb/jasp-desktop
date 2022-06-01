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

#include "DUP_listmodelassignedinterface.h"
#include "DUP_variableslistbase.h"
#include "DUP_analysisform.h"

DUP_ListModelAssignedInterface::DUP_ListModelAssignedInterface(DUP_JASPListControl* listView)
	: DUP_ListModelDraggable(listView)
  , _availableModel(nullptr)
{
	_needsSource = false;
}

void DUP_ListModelAssignedInterface::refresh()
{
	bool doRefresh = true;
	QList<int> toRemove;
	for (int i = 0; i < rowCount(); i++)
	{
		QString term = data(index(i, 0)).toString();
		if (!isAllowed(term))
			toRemove.push_back(i);
	}

	if (toRemove.count() > 0)
	{
		DUP_VariablesListBase* qmlListView = dynamic_cast<DUP_VariablesListBase*>(listView());
		if (qmlListView)
		{
			qmlListView->moveItems(toRemove, _availableModel);
			doRefresh = false;
		}
	}

	if (doRefresh)
		DUP_ListModelDraggable::refresh();
}

void DUP_ListModelAssignedInterface::setAvailableModel(DUP_ListModelAvailableInterface *source)
{
	_availableModel = source;
}

int DUP_ListModelAssignedInterface::sourceColumnTypeChanged(QString name)
{
	int index = DUP_ListModelDraggable::sourceColumnTypeChanged(name);
	DUP_VariablesListBase* qmlListView = dynamic_cast<DUP_VariablesListBase*>(listView());

	if (qmlListView && index >= 0 && index < int(terms().size()))
	{
		if (!isAllowed(terms().at(size_t(index))))
		{
			QList<int> indexes = {index};
			qmlListView->moveItems(indexes, _availableModel);
			DUP_ListModelDraggable::refresh();
		}
		// Force the analysis to be rerun
		emit qmlListView->boundValueChanged(qmlListView);
	}

	return index;
}

bool DUP_ListModelAssignedInterface::sourceLabelsChanged(QString columnName, QMap<QString, QString> changedLabels)
{
	bool change = DUP_ListModelDraggable::sourceLabelsChanged(columnName, changedLabels);

	if (change && listView() && listView()->form())
		listView()->form()->refreshAnalysis();

	return change;
}

bool DUP_ListModelAssignedInterface::sourceLabelsReordered(QString columnName)
{
	bool change = DUP_ListModelDraggable::sourceLabelsReordered(columnName);

	if (change && listView() && listView()->form())
		listView()->form()->refreshAnalysis();

	return change;
}
