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

#ifndef DUP_LISTMODELDRAGGABLE_H
#define DUP_LISTMODELDRAGGABLE_H

#include "DUP_listmodel.h"
#include "DUP_jaspcontrol.h"

class DUP_RowControls;

class DUP_ListModelDraggable : public DUP_ListModel
{
	Q_OBJECT
	
public:
	DUP_ListModelDraggable(DUP_JASPListControl* listView);
	~DUP_ListModelDraggable();

	bool copyTermsWhenDropped() const						{ return _copyTermsWhenDropped; }
	DUP_JASPControl::DropMode dropMode() const				{ return _dropMode; }
	
	void setDropMode(DUP_JASPControl::DropMode dropMode)	{ _dropMode = dropMode; }
	void setCopyTermsWhenDropped(bool copy)					{ _copyTermsWhenDropped = copy; }
	
	virtual DUP_Terms termsFromIndexes(const QList<int> &indexes)					const;
	virtual DUP_Terms canAddTerms(const DUP_Terms& terms)								const;
	virtual DUP_Terms addTerms(const DUP_Terms& terms, int dropItemIndex = -1, const RowControlsValues& rowValues = RowControlsValues());
	virtual void removeTerms(const QList<int>& indexes);
	virtual void moveTerms(const QList<int>& indexes, int dropItemIndex = -1);

signals:
	void destroyed(DUP_ListModelDraggable * me);

protected:
	bool						_copyTermsWhenDropped;
	bool						_addNewAvailableTermsToAssignedModel	= false;
	bool						_allowAnalysisOwnComputedColumns		= true;
	DUP_JASPControl::DropMode		_dropMode								= DUP_JASPControl::DropMode::DropNone;
		
	bool						isAllowed(const DUP_Term &term) const;
};

#endif // LISTMODELDRAGGABLE_H
