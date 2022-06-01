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

#ifndef DUP_LISTMODELAVAILABLEINTERFACE_H
#define DUP_LISTMODELAVAILABLEINTERFACE_H

#include "DUP_listmodeldraggable.h"
#include "DUP_terms.h"
#include "DUP_variableinfo.h"
#include "DUP_sortmenumodel.h"
#include "DUP_sortable.h"

class DUP_ListModelAssignedInterface;

class DUP_ListModelAvailableInterface: public DUP_ListModelDraggable, public DUP_VariableInfoProvider, public DUP_Sortable
{
	Q_OBJECT
public:
	DUP_ListModelAvailableInterface(DUP_JASPListControl* listView)
		: DUP_ListModelDraggable(listView) {}
	
	virtual const DUP_Terms& allTerms()																						const { return _allSortedTerms; }
			void initTerms(const DUP_Terms &terms, const RowControlsValues& _rowControlsValues = RowControlsValues())	override;
	virtual void resetTermsFromSources(bool updateAssigned = true)			= 0;
	virtual void removeTermsInAssignedList();
	
			QVariant requestInfo(const DUP_Term &term, DUP_VariableInfo::InfoType info)			const override;

			void sortItems(SortType sortType)											override;

			void										addAssignedModel(DUP_ListModelAssignedInterface* model);
			const QList<DUP_ListModelAssignedInterface*>&	assignedModel()	const			{ return _assignedModels; }

signals:
			void availableTermsReset(DUP_Terms termsAdded, DUP_Terms termsRemoved);

public slots:
			void sourceTermsReset()															override;
			void sourceNamesChanged(QMap<QString, QString> map)								override;
			void sourceColumnsChanged(QStringList columns)									override;
			int  sourceColumnTypeChanged(QString name)										override;
			bool sourceLabelsChanged(QString columnName, QMap<QString, QString> = {})		override;
			bool sourceLabelsReordered(QString columnName)									override;
			void removeAssignedModel(DUP_ListModelDraggable* model);

protected:
	DUP_Terms								_allTerms;
	DUP_Terms								_allSortedTerms;

	QList<DUP_ListModelAssignedInterface*>	_assignedModels;
};

#endif // LISTMODELTERMSAVAILABLEINTERFACE_H
