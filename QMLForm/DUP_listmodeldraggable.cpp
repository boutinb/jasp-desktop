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

#include "DUP_listmodeldraggable.h"
#include "DUP_analysisform.h"
#include "DUP_jasplistcontrol.h"

DUP_ListModelDraggable::DUP_ListModelDraggable(DUP_JASPListControl* listView)
	: DUP_ListModel(listView)
	, _copyTermsWhenDropped(false)	
{
	_allowAnalysisOwnComputedColumns = listView->property("allowAnalysisOwnComputedColumns").toBool();
	_addNewAvailableTermsToAssignedModel = listView->property("addAvailableVariablesToAssigned").toBool();
}

DUP_ListModelDraggable::~DUP_ListModelDraggable()
{
	emit destroyed(this);
}

DUP_Terms DUP_ListModelDraggable::termsFromIndexes(const QList<int> &indexes) const
{
	DUP_Terms result;
	const DUP_Terms& myTerms = terms();
	for (int index : indexes)
	{
		size_t index_t = size_t(index);
		if (index_t < myTerms.size())
		{
			const DUP_Term& term = myTerms.at(index_t);
			result.add(term);
		}
	}
	
	return result;
}

void DUP_ListModelDraggable::removeTerms(const QList<int> &indices)
{
	beginResetModel();

	DUP_Terms termsToRemove;

	for (int index : indices)
		if (index < rowCount())
			termsToRemove.add(terms().at(size_t(index)));

	_removeTerms(termsToRemove);

	endResetModel();
}


void DUP_ListModelDraggable::moveTerms(const QList<int> &indexes, int dropItemIndex)
{
	DUP_JASPControl::DropMode _dropMode = dropMode();
	if (indexes.length() == 0 || _dropMode == DUP_JASPControl::DropMode::DropNone)
		return;	

	beginResetModel();
	DUP_Terms terms = termsFromIndexes(indexes);
	removeTerms(indexes); // Remove first before adding: we cannot add terms that already exist
	for (int index : indexes)
	{
		if (index < dropItemIndex)
			dropItemIndex--;
	}
	DUP_Terms removedTerms = addTerms(terms, dropItemIndex);
	if (removedTerms.size() > 0)
	{
		addTerms(removedTerms);
	}
	
	endResetModel();
}

DUP_Terms DUP_ListModelDraggable::addTerms(const DUP_Terms& terms, int dropItemIndex, const RowControlsValues&)
{
	if (terms.size() > 0)
	{
		beginResetModel();
		_addTerms(terms);
		endResetModel();
	}

	return DUP_Terms();
}

DUP_Terms DUP_ListModelDraggable::canAddTerms(const DUP_Terms& terms) const
{
	DUP_Terms result;
	for (const DUP_Term &term : terms)
	{
		if (isAllowed(term))
			result.add(term);
	}

	return result;
}

bool DUP_ListModelDraggable::isAllowed(const DUP_Term &term) const
{
	if (!_allowAnalysisOwnComputedColumns)
	{
		if (listView()->form()->isOwnComputedColumn(term.asString()))
			return false;
	}

	int variableTypesAllowed = listView()->variableTypesAllowed();

	if (variableTypesAllowed == 0xff || term.size() > 1)
		return true;
	
	QVariant	v				= requestInfo(term, DUP_VariableInfo::VariableType);
	int			variableType	= v.toInt();

	return variableType == 0 || variableType & variableTypesAllowed;
}
