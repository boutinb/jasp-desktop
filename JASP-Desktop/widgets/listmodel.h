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

#ifndef LISTMODEL_H
#define LISTMODEL_H

#include <QAbstractListModel>
#include <QQmlComponent>

#include "common.h"
#include "analysis/options/variableinfo.h"
#include "analysis/options/terms.h"

class QMLListView;
class RowControls;
class Option;

class ListModel : public QAbstractTableModel, public VariableInfoConsumer
{
	Q_OBJECT
public:
	enum ListModelRoles
	{
        NameRole = Qt::UserRole + 1,
		TypeRole,
		SelectedRole,
		ColumnTypeRole,
		RowComponentsRole
    };
	typedef QMap<QString, QMap<QString, Option*> > RowControlsOptions;

	ListModel(QMLListView* listView);
	
			QHash<int, QByteArray>	roleNames()													const override;
			int						rowCount(const QModelIndex &parent = QModelIndex())			const override;
			int						columnCount(const QModelIndex &parent = QModelIndex())		const override { return 1; }
			QVariant				data(const QModelIndex &index, int role = Qt::DisplayRole)	const override;

	virtual void					endResetModel();

			QMLListView*			listView() const								{ return _listView; }
			const QString &			name() const;
	virtual const Terms &			terms(const QString& what = QString())			{ return _terms; }
			bool					areTermsVariables() const						{ return _areTermsVariables; }
			bool					areTermsInteractions() const					{ return _areTermsInteractions; }
	virtual QString					getItemType(const Term& term) const				{ return _itemType; }
			void					setTermsAreVariables(bool areVariables)			{ _areTermsVariables = areVariables; }
			void					setTermsAreInteractions(bool interactions)		{ _areTermsInteractions = interactions; }
			void					setItemType(QString type)						{ _itemType = type; }
			void					addError(const QString& error) const;
	virtual void					refresh();
	virtual void					initTerms(const Terms &terms, const RowControlsOptions& allOptionsMap = RowControlsOptions());
	virtual Terms					getSourceTerms();
	QMap<ListModel*, Terms> 		getSourceTermsPerModel();

			void					setRowComponents(QVector<QQmlComponent*> &rowComponents);
	virtual void					setUpRowControls();
	const QMap<QString, RowControls*>& getRowControls() const { return _rowControlsMap; }

	Q_INVOKABLE int					searchTermWith(QString searchString);
	Q_INVOKABLE void				selectItem(int _index, bool _select);
	Q_INVOKABLE void				clearSelectedItems(bool emitSelectedChange = true);
	Q_INVOKABLE void				setSelectedItem(int _index);
	Q_INVOKABLE void				selectAllItems();
	Q_INVOKABLE QList<int>			selectedItems() { return _selectedItems; }
	Q_INVOKABLE QList<QString>		selectedItemsTypes() { return _selectedItemsTypes.toList(); }


signals:
			void modelChanged(Terms* added = nullptr, Terms* removed = nullptr);
			void selectedItemsChanged();

public slots:	
	virtual void sourceTermsChanged(Terms* termsAdded, Terms* termsRemoved);

private:
			void _addSelectedItemType(int _index);

protected:
	QMLListView*	_listView = nullptr;
	QString			_itemType;
	Terms			_terms;
	QList<int>		_selectedItems;
	QSet<QString>	_selectedItemsTypes;
	bool			_areTermsVariables;
	bool			_areTermsInteractions = false;
	QMap<QString, RowControls* >	_rowControlsMap;
	QVector<QQmlComponent *>		_rowComponents;
	RowControlsOptions				_rowControlsOptions;

};

#endif // LISTMODEL_H
