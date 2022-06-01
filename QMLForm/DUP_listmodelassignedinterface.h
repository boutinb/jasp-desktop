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

#ifndef DUP_LISTMODELASSIGNEDINTERFACE_H
#define DUP_LISTMODELASSIGNEDINTERFACE_H

#include "DUP_listmodeldraggable.h"
#include "DUP_listmodelavailableinterface.h"

class DUP_ListModelAssignedInterface : public DUP_ListModelDraggable
{
	Q_OBJECT
public:
	DUP_ListModelAssignedInterface(DUP_JASPListControl* listView);
	
	void							refresh()													override;

	virtual void					setAvailableModel(DUP_ListModelAvailableInterface *availableModel);
	DUP_ListModelAvailableInterface*	availableModel() const										{ return _availableModel; }
	
public slots:
	virtual void availableTermsResetHandler(DUP_Terms termsAdded, DUP_Terms termsRemoved)				{}
			int  sourceColumnTypeChanged(QString name)												override;
			bool sourceLabelsChanged(QString columnName, QMap<QString, QString> changedLabels)		override;
			bool sourceLabelsReordered(QString columnName)											override;
protected:
	DUP_ListModelAvailableInterface*			_availableModel;
};

#endif // LISTMODELASSIGNEDINTERFACE_H
