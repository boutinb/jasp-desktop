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

#ifndef DUP_SORTMENUMODEL_H
#define DUP_SORTMENUMODEL_H

#include <QAbstractListModel>
#include "DUP_sortable.h"

class DUP_ListModelAvailableInterface;

class DUP_SortMenuModel : public QAbstractListModel
{
	Q_OBJECT

public:
	enum {
		DisplayRole,
		MenuImageSourceRole,
		IsEnabledRole
	};

	DUP_SortMenuModel(QObject *parent, const QVector<DUP_Sortable::SortType> &menuEntries);

	int										rowCount(const QModelIndex &parent = QModelIndex())			const override	{	return _menuEntries.length();	}
	QVariant								data(const QModelIndex &index, int role = Qt::DisplayRole)	const override;
	virtual QHash<int, QByteArray>			roleNames()													const override;

	Q_INVOKABLE void						clickSortItem(int index);
	void									sortItems();
	DUP_Sortable::SortType						currentSortType();
	bool									isAscending();
	void									setCurrentEntry(DUP_Sortable::SortType sortType);

private:
	static QMap<DUP_Sortable::SortType, QString> _labels;

	struct SortMenuItem
	{
		DUP_Sortable::SortType	sortType;
		bool				ascending;

		SortMenuItem(DUP_Sortable::SortType _sortType, bool _ascending = true) : sortType(_sortType), ascending(_ascending) {}
	};

	QVector<SortMenuItem*>	_menuEntries;
	int						_currentEntry = 0;
	DUP_Sortable*				_sortable;
};

#endif // SORTMENUMODEL_H
