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

#ifndef SORTABLE_H
#define SORTABLE_H

class DUP_SortMenuModel;

class DUP_Sortable
{

public:
	enum SortType {
		None = 0,
		SortByName,
		SortByNameAZ,
		SortByNameZA,
		SortByType,
		SortByDate,
		SortBySize
	};

	virtual ~DUP_Sortable();

	virtual void sortItems(SortType sortType) = 0;
			void sortItems();

			void setSortModel(DUP_SortMenuModel* menu) { _sortMenuModel = menu; }
			SortType currentSortType();

private:
	DUP_SortMenuModel* _sortMenuModel = nullptr;
};

#endif // SORTABLE_H
