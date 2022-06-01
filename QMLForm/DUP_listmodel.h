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

#ifndef DUP_LISTMODEL_H
#define DUP_LISTMODEL_H

#include <QAbstractListModel>
#include <QQmlComponent>

#include "DUP_variableinfo.h"
#include "DUP_terms.h"
#include <json/json.h>

class DUP_JASPListControl;
class DUP_RowControls;
class DUP_JASPControl;
class DUP_BoundControl;

class DUP_ListModel : public QAbstractTableModel, public DUP_VariableInfoConsumer
{
	Q_OBJECT
	typedef QMap<QString, DUP_RowControls*> rowControlMap;

public:
	enum ListModelRoles
	{
        NameRole = Qt::UserRole + 1,
		TypeRole,
		SelectedRole,
		SelectableRole,
		ColumnTypeRole,
		ColumnTypeIconRole,
		ColumnTypeDisabledIconRole,
		RowComponentRole,
		ValueRole,
		VirtualRole,
		DeletableRole
    };
	typedef QMap<QString, QMap<QString, Json::Value> > RowControlsValues;

	DUP_ListModel(DUP_JASPListControl* listView);
	
			QHash<int, QByteArray>	roleNames()													const override;
			int						rowCount(const QModelIndex &parent = QModelIndex())			const override;
			int						columnCount(const QModelIndex &parent = QModelIndex())		const override { return 1; }
			QVariant				data(const QModelIndex &index, int role = Qt::DisplayRole)	const override;

			DUP_JASPListControl*		listView()													const		{ return _listView; }
			const QString &			name() const;
			DUP_Terms					termsEx(const QStringList& what);
			const DUP_Terms &			terms()														const		{ return _terms;	}
	virtual DUP_Terms					filterTerms(const DUP_Terms& terms, const QStringList& filters);
			bool					needsSource()												const		{ return _needsSource;			}
			void					setNeedsSource(bool needs)												{ _needsSource = needs;			}
	virtual QString					getItemType(const DUP_Term& term)								const		{ return _itemType; }
			void					setItemType(QString type)												{ _itemType = type; }
			void					addControlError(const QString& error)						const;
	virtual void					refresh();
	virtual void					initTerms(const DUP_Terms &terms, const RowControlsValues& allValuesMap = RowControlsValues());
			DUP_Terms					getSourceTerms();
			DUP_ListModel*				getSourceModelOfTerm(const DUP_Term& term);
			void					setColumnsUsedForLabels(const QStringList& columns)						{ _columnsUsedForLabels = columns; }
			void					setRowComponent(QQmlComponent* rowComponents);
	virtual void					setUpRowControls();
	const rowControlMap	&			getAllRowControls()											const		{ return _rowControlsMap;				}
	DUP_RowControls*					getRowControls(const QString& key)							const		{ return _rowControlsMap.value(key);	}
	virtual DUP_JASPControl	*			getRowControl(const QString& key, const QString& name)		const;
	virtual bool					addRowControl(const QString& key, DUP_JASPControl* control);
			QStringList				termsTypes();

	Q_INVOKABLE int					searchTermWith(QString searchString);
	Q_INVOKABLE void				selectItem(int _index, bool _select);
	Q_INVOKABLE void				clearSelectedItems(bool emitSelectedChange = true);
	Q_INVOKABLE void				setSelectedItem(int _index);
	Q_INVOKABLE void				selectAllItems();
	Q_INVOKABLE QList<int>			selectedItems()															{ return _selectedItems; }
    Q_INVOKABLE QList<QString>		selectedItemsTypes()													{ return QList<QString>(_selectedItemsTypes.begin(), _selectedItemsTypes.end()); }


signals:
			void termsChanged();		// Used to signal all kinds of changes in the model. Do not call it directly
			void namesChanged(QMap<QString, QString> map);
			void columnTypeChanged(QString name);
			void labelsChanged(QString columnName, QMap<QString, QString> = {});
			void labelsReordered(QString columnName);
			void columnsChanged(QStringList columns);
			void selectedItemsChanged();
			void oneTermChanged(const QString& oldName, const QString& newName);

public slots:	
	virtual void sourceTermsReset();
	virtual void sourceNamesChanged(QMap<QString, QString> map);
	virtual int  sourceColumnTypeChanged(QString colName);
	virtual bool sourceLabelsChanged(QString columnName, QMap<QString, QString> changedLabels = {});
	virtual bool sourceLabelsReordered(QString columnName);
	virtual void sourceColumnsChanged(QStringList columns);

			void dataChangedHandler(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>());

protected:
			void	_setTerms(const DUP_Terms& terms);
			void	_setTerms(const DUP_Terms& terms, const DUP_Terms& parentTerms);
			void	_setTerms(const std::vector<DUP_Term>& terms);
			void	_removeTerms(const DUP_Terms& terms);
			void	_removeTerm(int index);
			void	_removeTerm(const DUP_Term& term);
			void	_removeLastTerm();
			void	_addTerms(const DUP_Terms& terms);
			void	_addTerm(const QString& term, bool isUnique = true);
			void	_replaceTerm(int index, const DUP_Term& term);
			void	_connectAllSourcesControls();

			QString							_itemType;
			bool							_needsSource			= true;
			QMap<QString, DUP_RowControls* >	_rowControlsMap;
			QQmlComponent *					_rowComponent			= nullptr;
			RowControlsValues				_rowControlsValues;
			QList<DUP_BoundControl *>			_rowControlsConnected;
			QList<int>						_selectedItems;
			QSet<QString>					_selectedItemsTypes;
			QStringList						_columnsUsedForLabels;

private:
			void	_addSelectedItemType(int _index);
			void	_initTerms(const DUP_Terms &terms, const RowControlsValues& allValuesMap, bool setupRowConnections = true);
			void	_connectSourceControls(DUP_ListModel* sourceModel, const QSet<QString>& controls);

			DUP_JASPListControl*				_listView = nullptr;
			DUP_Terms							_terms;

};

#endif // LISTMODEL_H
