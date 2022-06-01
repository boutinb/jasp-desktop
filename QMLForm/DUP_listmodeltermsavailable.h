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

#ifndef DUP_LISTMODELTERMSAVAILABLE_H
#define DUP_LISTMODELTERMSAVAILABLE_H

#include "DUP_listmodelavailableinterface.h"

class DUP_ListModelTermsAvailable : public DUP_ListModelAvailableInterface
{
	Q_OBJECT
public:
	DUP_ListModelTermsAvailable(DUP_JASPListControl* listView) : DUP_ListModelAvailableInterface(listView) {}
		
	void	resetTermsFromSources(bool updateAssigned = true)	override;
};

#endif // LISTMODELTERMSAVAILABLE_H
