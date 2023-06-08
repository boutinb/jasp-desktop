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

#ifndef DATASETTABLEMODEL_H
#define DATASETTABLEMODEL_H

#include "datasettableproxy.h"


///
/// Makes sure that the data from DataSetPackage is properly filtered (and possible sorted) and then passed on as a normal table-model to QML
class DataSetTableModel : public DataSetTableProxy
{
	Q_OBJECT
	Q_PROPERTY(int	columnsFilteredCount	READ columnsFilteredCount							NOTIFY columnsFilteredCountChanged)
	Q_PROPERTY(bool showInactive			READ showInactive			WRITE setShowInactive	NOTIFY showInactiveChanged)

public:
	explicit				DataSetTableModel(bool showInactive = true);
	bool					filterAcceptsRow(int source_row, const QModelIndex & source_parent)	const override;

				int			columnsFilteredCount()					const				{ return DataSetPackage::pkg()->columnsFilteredCount();								}
	Q_INVOKABLE bool		isColumnNameFree(QString name)								{ return DataSetPackage::pkg()->isColumnNameFree(name);								}
	Q_INVOKABLE	QVariant	columnTitle(int column)					const				{ return DataSetPackage::pkg()->getColumnTitle(column);								}
	Q_INVOKABLE QVariant	columnIcon(int column)					const				{ return DataSetPackage::pkg()->getColumnIcon(column);								}
	Q_INVOKABLE QString		columnName(int column)					const;
	Q_INVOKABLE void		setColumnName(int col, QString name)	const;
	Q_INVOKABLE QVariant	getColumnTypesWithIcons()				const				{ return DataSetPackage::pkg()->getColumnTypesWithIcons();				}
	Q_INVOKABLE bool		columnUsedInEasyFilter(int column)		const				{ return DataSetPackage::pkg()->isColumnUsedInEasyFilter(column);					}
	Q_INVOKABLE void		resetAllFilters()											{		 DataSetPackage::pkg()->resetAllFilters();									}
	Q_INVOKABLE int			setColumnTypeFromQML(int columnIndex, int newColumnType)	{ return DataSetPackage::pkg()->setColumnTypeFromQML(columnIndex, newColumnType);	}
	Q_INVOKABLE void		resizeData(int row, int col)								{		 DataSetPackage::pkg()->resizeData(row, col);								}

	//the following column-int passthroughs will fail once columnfiltering is added...
	columnType				getColumnType(size_t column)			const				{ return DataSetPackage::pkg()->getColumnType(column);								}
	std::string				getColumnName(size_t col)				const				{ return DataSetPackage::pkg()->getColumnName(col);									}
	int						getColumnIndex(const std::string& col)	const				{ return DataSetPackage::pkg()->getColumnIndex(col);								}
	QStringList				getColumnLabelsAsStringList(int col)	const;
	QStringList				getColumnValuesAsStringList(int col)	const				{ return DataSetPackage::pkg()->getColumnValuesAsStringList(col);					}
	QList<QVariant>			getColumnValuesAsDoubleList(int col)	const				{ return DataSetPackage::pkg()->getColumnValuesAsDoubleList(col);					}
	size_t					getMaximumColumnWidthInCharacters(int index) const			{ return DataSetPackage::pkg()->getMaximumColumnWidthInCharacters(index);			}
	bool					synchingData()							const				{ return DataSetPackage::pkg()->synchingData();										}

	void					resetModelOneCell();
	void					pasteSpreadsheet(size_t row, size_t col, const std::vector<std::vector<QString>> & cells, QStringList newColNames = QStringList());
	void					columnInsert(	size_t column	);
	void					columnDelete(	size_t column	);
	void					rowInsert(		size_t row		);
	void					rowDelete(		size_t row		);

	bool					insertRows(		int row,		int count, const QModelIndex & aparent = QModelIndex()) override;
	bool					insertColumns(	int column,		int count, const QModelIndex & aparent = QModelIndex()) override;
	bool					removeRows(		int row,		int count, const QModelIndex & aparent = QModelIndex()) override;
	bool					removeColumns(	int column,		int count, const QModelIndex & aparent = QModelIndex()) override;


				bool		showInactive()							const				{ return _showInactive;	}

signals:
				void		columnsFilteredCountChanged();
				void		showInactiveChanged(bool showInactive);
				void		columnTypeChanged(QString colName);
				void		labelChanged(QString columnName, QString originalLabel, QString newLabel);
				void		labelsReordered(QString columnName);

				void		renameColumnDialog(int columnIndex);

public slots:
				void		setShowInactive(bool showInactive);
				//void		onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles) { if( roles.count(int(DataSetPackage::specialRoles::filter)) > 0) invalidateFilter(); }


private:
	bool					_showInactive;

};

#endif // DATASETTABLEMODEL_H
