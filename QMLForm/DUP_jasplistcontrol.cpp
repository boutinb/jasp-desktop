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

#include "DUP_jasplistcontrol.h"
#include "DUP_jaspcontrol.h"
#include "DUP_listmodel.h"
#include "DUP_rowcontrols.h"
#include "DUP_sourceitem.h"
#include "DUP_analysisform.h"
#include "DUP_listmodelassignedinterface.h"
#include "log.h"

#include <QQmlContext>


DUP_JASPListControl::DUP_JASPListControl(QQuickItem *parent)
	: DUP_JASPControl(parent)
{
	_hasUserInteractiveValue = false;
}

void DUP_JASPListControl::setUpModel()
{
	if (model() && form())	form()->addModel(model());

	emit modelChanged();
}

void DUP_JASPListControl::_setupSources()
{
	for (DUP_SourceItem* sourceItem : _sourceItems)
	{
		if (sourceItem->listModel())
		{
			DUP_JASPListControl* sourceControl = sourceItem->listModel()->listView();
			disconnect(sourceControl, &DUP_JASPListControl::containsVariablesChanged,		this, &DUP_JASPListControl::setContainsVariables);
			disconnect(sourceControl, &DUP_JASPListControl::containsInteractionsChanged,	this, &DUP_JASPListControl::setContainsInteractions);
		}
		delete sourceItem;
	}
	_sourceItems.clear();

	_sourceItems = DUP_SourceItem::readAllSources(this);

	for (DUP_SourceItem* sourceItem : _sourceItems)
	{
		if (sourceItem->listModel())
		{
			DUP_JASPListControl* sourceControl = sourceItem->listModel()->listView();
			connect(sourceControl, &DUP_JASPListControl::containsVariablesChanged,		this, &DUP_JASPListControl::setContainsVariables);
			connect(sourceControl, &DUP_JASPListControl::containsInteractionsChanged,	this, &DUP_JASPListControl::setContainsInteractions);
		}
	}

	setContainsVariables();
	setContainsInteractions();
}

void DUP_JASPListControl::setContainsVariables()
{
	bool containsVariables = false;

	DUP_ListModelAssignedInterface* assignedModel = qobject_cast<DUP_ListModelAssignedInterface*>(model());
	if (assignedModel && assignedModel->availableModel())
		containsVariables = assignedModel->availableModel()->listView()->containsVariables();

	if (!containsVariables)
	{
		for (DUP_SourceItem* sourceItem : _sourceItems)
		{
			if (sourceItem->isColumnsModel())	containsVariables = true;
			else if (sourceItem->listModel())
			{
				if (sourceItem->listModel()->listView()->containsVariables() && sourceItem->controlName().isEmpty() && !sourceItem->modelUse().contains("levels"))
					containsVariables = true;
			}
		}
	}

	if (_containsVariables != containsVariables)
	{
		_containsVariables = containsVariables;
		emit containsVariablesChanged();
	}
}

void DUP_JASPListControl::setContainsInteractions()
{
	bool containsInteractions = false;

	if (_termsAreInteractions)
		containsInteractions = true;

	if (!containsInteractions)
	{
		DUP_ListModelAssignedInterface* assignedModel = qobject_cast<DUP_ListModelAssignedInterface*>(model());
		if (assignedModel && assignedModel->availableModel())
			containsInteractions = assignedModel->availableModel()->listView()->containsInteractions();
	}

	if (!containsInteractions)
	{
		for (DUP_SourceItem* sourceItem : _sourceItems)
		{
			if (sourceItem->listModel())
			{
				DUP_JASPListControl* sourceControl = sourceItem->listModel()->listView();
				if (sourceControl->containsInteractions() || sourceItem->combineWithOtherModels())
					containsInteractions = true;
			}
		}
	}

	if (_containsInteractions != containsInteractions)
	{
		_containsInteractions = containsInteractions;
		emit containsInteractionsChanged();
	}
}

void DUP_JASPListControl::setUp()
{
	if (!model())	setUpModel();
	DUP_JASPControl::setUp();

	DUP_ListModel* listModel = model();
	if (!listModel)	return;

	listModel->setRowComponent(rowComponent());
	_setupSources();

	connect(this,		&DUP_JASPListControl::sourceChanged,	this,	&DUP_JASPListControl::sourceChangedHandler);
	connect(listModel,	&DUP_ListModel::termsChanged,			this,	&DUP_JASPListControl::termsChangedHandler);
	connect(listModel,	&DUP_ListModel::termsChanged,			[this]() { emit countChanged(); });
}

void DUP_JASPListControl::cleanUp()
{
	try
	{
		DUP_ListModel* _model = model();

		if (_model)
		{
			_model->disconnect();
			for (DUP_RowControls* rowControls : _model->getAllRowControls().values())
				for (DUP_JASPControl* control : rowControls->getJASPControlsMap().values())
					control->cleanUp();
		}

		DUP_JASPControl::cleanUp();
	}
	catch (...) {}
}

DUP_Terms DUP_JASPListControl::_getCombinedTerms(DUP_SourceItem* sourceToCombine)
{
	DUP_Terms result = sourceToCombine->getTerms();
	DUP_Terms termsToBeCombinedWith;
	for (DUP_SourceItem* sourceItem : _sourceItems)
		if (sourceItem != sourceToCombine)
			termsToBeCombinedWith.add(sourceItem->getTerms());

	DUP_Terms termsToCombine = sourceToCombine->getTerms();
	for (const DUP_Term& termToCombine : termsToCombine)
	{
		for (const DUP_Term& termToBeCombined : termsToBeCombinedWith)
		{
			QStringList components = termToCombine.components();
			components.append(termToBeCombined.components());
			result.add(DUP_Term(components));
		}
	}

	return result;
}

void DUP_JASPListControl::applyToAllSources(std::function<void(DUP_SourceItem *sourceItem, const DUP_Terms& terms)> applyThis)
{
	for (DUP_SourceItem* sourceItem : _sourceItems)
		applyThis(sourceItem, sourceItem->combineWithOtherModels() ? _getCombinedTerms(sourceItem) : sourceItem->getTerms());
}

bool DUP_JASPListControl::hasNativeSource() const
{
	return _sourceItems.size() == 1 && _sourceItems[0]->isNativeModel();
}

bool DUP_JASPListControl::addRowControl(const QString &key, DUP_JASPControl *control)
{
	return model() ? model()->addRowControl(key, control) : false;
}

bool DUP_JASPListControl::hasRowComponent() const
{
	return rowComponent() != nullptr;
}

DUP_JASPControl *DUP_JASPListControl::getChildControl(QString key, QString name)
{
	return getRowControl(key, name);
}

DUP_JASPControl *DUP_JASPListControl::getRowControl(const QString &key, const QString &name) const
{
	return model() ? model()->getRowControl(key, name) : nullptr;
}

QString DUP_JASPListControl::getSourceType(QString name)
{
	return model() ? model()->getItemType(name) : "";
}

int DUP_JASPListControl::count()
{
	return model() ? model()->rowCount() : 0;
}

std::vector<std::string> DUP_JASPListControl::usedVariables() const
{
	if (containsVariables() && isBound() && model())	return model()->terms().asVector();
	else												return {};
}

void DUP_JASPListControl::sourceChangedHandler()
{
	if (!model())	return;

	_setupSources();
	model()->sourceTermsReset();
}
