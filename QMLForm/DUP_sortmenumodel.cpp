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

#include "DUP_sortmenumodel.h"
#include "DUP_listmodelavailableinterface.h"
#include "qquick/jasptheme.h"

QMap<DUP_Sortable::SortType, QString> DUP_SortMenuModel::_labels; //Only set this constructor because otherwise it might cause a crash on Windows

DUP_SortMenuModel::DUP_SortMenuModel(QObject* parent, const QVector<DUP_Sortable::SortType> &menuEntries) : QAbstractListModel(parent)
{
	if(_labels.size() == 0)
		_labels =
		{
			{ DUP_Sortable::SortType::None,			tr("None") },
			{ DUP_Sortable::SortType::SortByName,	tr("Sort by name") },
			{ DUP_Sortable::SortType::SortByNameAZ, tr("Sort by name A-Z") },
			{ DUP_Sortable::SortType::SortByNameZA, tr("Sort by name Z-A") },
			{ DUP_Sortable::SortType::SortByType,	tr("Sort by type") },
			{ DUP_Sortable::SortType::SortByDate,	tr("Sort by date") },
			{ DUP_Sortable::SortType::SortBySize,	tr("Sort by size") }
		};


	_sortable = dynamic_cast<DUP_Sortable*>(parent);
	if (!_sortable)
	{
		QString errormsg(tr("SortMenuModel not called with a sortable parent"));
		throw new std::runtime_error(errormsg.toStdString().c_str());
	}

	for (const DUP_Sortable::SortType& sortType : menuEntries)
		_menuEntries.push_back(new SortMenuItem(sortType));

	_sortable->setSortModel(this);
}

QVariant DUP_SortMenuModel::data(const QModelIndex &index, int role) const
{
	if (index.row() >= rowCount())
		return QVariant();

	SortMenuItem* entry = _menuEntries.at(index.row());

	switch(role)
	{
	case DisplayRole:				return _labels[entry->sortType];
	case MenuImageSourceRole:		return index.row() == _currentEntry ? JaspTheme::currentIconPath() + "check-mark.png" : "";
	case IsEnabledRole:				return true;
	default:						return QVariant();
	}
}


QHash<int, QByteArray> DUP_SortMenuModel::roleNames() const
{
	static const auto roles = QHash<int, QByteArray>{
		{	DisplayRole,            "displayText"		},
		{	MenuImageSourceRole,    "menuImageSource"	},
		{	IsEnabledRole,			"isEnabled"			}
	};

	return roles;
}

void DUP_SortMenuModel::clickSortItem(int index)
{
	SortMenuItem* entry = _menuEntries[index];
	_sortable->sortItems(entry->sortType);

	_currentEntry = index;
}

void DUP_SortMenuModel::sortItems()
{
	SortMenuItem* entry = _menuEntries[_currentEntry];
	_sortable->sortItems(entry->sortType);
}

DUP_Sortable::SortType DUP_SortMenuModel::currentSortType()
{
	SortMenuItem* entry = _menuEntries[_currentEntry];
	return entry->sortType;
}

bool DUP_SortMenuModel::isAscending()
{
	SortMenuItem* entry = _menuEntries[_currentEntry];
	return entry->ascending;
}

void DUP_SortMenuModel::setCurrentEntry(DUP_Sortable::SortType sortType)
{
	int index = 0;
	for (const SortMenuItem* menuItem : _menuEntries)
	{
		if (menuItem->sortType == sortType)
			_currentEntry = index;
		index++;
	}
}
