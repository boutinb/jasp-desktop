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

#include "datasettablemodel.h"
#include "utilities/qutils.h"
#include "log.h"

DataSetTableModel::DataSetTableModel(bool showInactive) 
: DataSetTableProxy(DataSetPackage::pkg()->dataSubModel()), _showInactive(showInactive)
{
	
	connect(DataSetPackage::pkg(),	&DataSetPackage::columnsFilteredCountChanged,	this, &DataSetTableModel::columnsFilteredCountChanged	);
	connect(DataSetPackage::pkg(),	&DataSetPackage::columnDataTypeChanged,			this, &DataSetTableModel::columnTypeChanged				);
	connect(DataSetPackage::pkg(),	&DataSetPackage::labelChanged,					this, &DataSetTableModel::labelChanged					);
	connect(DataSetPackage::pkg(),	&DataSetPackage::labelsReordered,				this, &DataSetTableModel::labelsReordered				);
	//connect(this,		&DataSetTableModel::dataChanged,				this, &DataSetTableModel::onDataChanged,				Qt::QueuedConnection);

	setFilterRole(int(DataSetPackage::specialRoles::filter));
}


void DataSetTableModel::setShowInactive(bool showInactive)
{
	if (_showInactive == showInactive)
		return;

	_showInactive = showInactive;
	emit showInactiveChanged(_showInactive);
	invalidate();
	beginResetModel();
	endResetModel();
}

bool DataSetTableModel::filterAcceptsRow(int source_row, const QModelIndex & source_parent)	const
{
	return (_showInactive || DataSetPackage::pkg()->getRowFilter(source_row));
}

QStringList DataSetTableModel::getColumnLabelsAsStringList(int col) const
{
	QStringList labels = DataSetPackage::pkg()->getColumnLabelsAsStringList(col);
	QStringList notUsedLabels = labels;
	int max = rowCount();

	for (int i = 0; i < max; i++)
	{
		QString value = data(index(i, col)).toString();
		notUsedLabels.removeAll(value);
		if (notUsedLabels.count() == 0) break;
	}

	// The order of the labels must be kept.
	for (const QString& notUsedLabel : notUsedLabels)
		labels.removeAll(notUsedLabel);

	return labels;
}

QString DataSetTableModel::columnName(int column) const
{
	//map to source might be needed here once we start filtering columns
	return tq(getColumnName(column));
}

void DataSetTableModel::setColumnName(int col, QString name) const
{
	return DataSetPackage::pkg()->setColumnName(col, fq(name));
}

void DataSetTableModel::pasteSpreadsheet(size_t row, size_t col, const std::vector<std::vector<QString> > & cells, QStringList newColNames)
{
	QModelIndex idx = mapToSource(index(row, col));
	DataSetPackage::pkg()->pasteSpreadsheet(idx.row(), idx.column(), cells, newColNames);
}

void DataSetTableModel::setColumnComputed(int column, bool R)
{
	beginResetModel();
	int sourceColumn = std::min(DataSetPackage::pkg()->dataColumnCount()-1, column); //The mapping isnt made yet and I think insertColumn returns the actual columnIndex in datasetpackage not in filterproxy //mapToSource(index(0, column)).column();
	DataSetPackage::pkg()->dataSet()->column(sourceColumn)->setCodeType(R ? computedColumnType::rCode : computedColumnType::constructorCode);
	endResetModel();
}



