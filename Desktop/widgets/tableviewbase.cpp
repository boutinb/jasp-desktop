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

#include "log.h"
#include <QTimer>
#include "tableviewbase.h"
#include "listmodeljagsdatainput.h"
#include "../analysis/analysisform.h"
#include "listmodelfiltereddataentry.h"
#include "listmodelmultinomialchi2test.h"
#include "listmodelfactorlevels.h"
#include "listmodelcustomcontrasts.h"
#include "listmodelgridinput.h"
#include "boundcontrolcontraststableview.h"
#include "boundcontrolfilteredtableview.h"
#include "boundcontrolgridtableview.h"
#include "sourceitem.h"

TableViewBase::TableViewBase(QQuickItem* parent)
	: JASPListControl(parent)
{
	_controlType = ControlType::TableView;
}

void TableViewBase::setUpModel()
{
	switch (modelType())
	{
	case ModelType::MultinomialChi2Model	: _tableModel = new ListModelMultinomialChi2Test(	this );	break;
	case ModelType::JAGSDataInputModel		: _tableModel = new ListModelJAGSDataInput(			this );	break;
	case ModelType::CustomContrasts			: _tableModel = new ListModelCustomContrasts(		this );	break;
	case ModelType::FilteredDataEntryModel	: _tableModel = new ListModelFilteredDataEntry(		this );	break;
	case ModelType::GridInput				: _tableModel = new ListModelGridInput(				this ); break;
	case ModelType::Simple					: _tableModel = new ListModelTableViewBase(			this );	break;
	}

	JASPListControl::setUpModel();

	connect(_tableModel, &ListModelTableViewBase::columnCountChanged,	this, &TableViewBase::columnCountChanged);
	connect(_tableModel, &ListModelTableViewBase::rowCountChanged,		this, &TableViewBase::rowCountChanged);
	connect(_tableModel, &ListModelTableViewBase::variableCountChanged,	this, &TableViewBase::variableCountChanged);
}

void TableViewBase::setUp()
{
	switch (modelType())
	{
	case ModelType::CustomContrasts			: _boundControl = new BoundControlContrastsTableView(this); break;
	case ModelType::FilteredDataEntryModel	: _boundControl = new BoundControlFilteredTableView(this);	break;
	case ModelType::GridInput				: _boundControl = new BoundControlGridTableView(this);		break;
	case ModelType::MultinomialChi2Model	:
	case ModelType::JAGSDataInputModel		:
	case ModelType::Simple					:
	default									: _boundControl = new BoundControlTableView(this);
	}

	JASPListControl::setUp();

	if (modelType() == ModelType::GridInput && hasNativeSource()) setUpdateSource(true);

	setInitialValuesControl();
	connect(this,	&TableViewBase::initialValuesSourceChanged, this, &TableViewBase::setInitialValuesControl);

	// form is not always known in the constructor, so all references to form (and dataset) must be done here
	if (form())
		connect(form(),		&AnalysisForm::refreshTableViewModels,			this, &TableViewBase::refreshMe	);
	_tableModel->setup();
}

void TableViewBase::addColumn(int col, bool left)
{
	if (!_tableModel) return;

	if (_sourceItems.length() > 0 && updateSource())
	{
		SourceItem* source = _sourceItems[0];
		QAbstractItemModel* nativeModel = source->nativeModel();
		if (!left) col++;
		nativeModel->insertColumns(col, 1);
	}
	else
		_tableModel->addColumn();
}

void TableViewBase::removeColumn(int col)
{
	if (!_tableModel) return;

	if (_sourceItems.length() > 0 && updateSource())
	{
		SourceItem* source = _sourceItems[0];
		QAbstractItemModel* nativeModel = source->nativeModel();
		nativeModel->removeColumns(col, 1);
	}
	else
		_tableModel->removeColumn(col);
}

void TableViewBase::addRow()
{
	if (_tableModel)
		_tableModel->addRow();
}

void TableViewBase::removeRow(int row)
{
	if (_tableModel)
		_tableModel->removeRow(row);
}

void TableViewBase::reset()
{
	if (_tableModel)
		_tableModel->reset();
}

void TableViewBase::itemChanged(int col, int row, QString value, QString type)
{
	if (!_tableModel || _tableModel->data(_tableModel->index(row, col)).toString() == value) return;

	if (_tableModel->valueOk(value, col, row))
	{
		if (_sourceItems.length() > 0 && updateSource())
		{
			QAbstractItemModel* nativeModel = _sourceItems[0]->nativeModel();
			nativeModel->setData(nativeModel->index(row, col), value);
		}
		else
			_tableModel->itemChanged(col, row, value, type);
	}
	else
		QTimer::singleShot(0, _tableModel, &ListModelTableViewBase::refreshModel);
	}

void TableViewBase::setInitialValuesControl()
{
	if (_initialValuesControl)
		disconnect(_initialValuesControl->model(), &ListModel::termsChanged, _tableModel, &ListModelTableViewBase::initialValuesChanged);

	QString initialValuesSourceName = initialValuesSource().toString();
	if (!initialValuesSourceName.isEmpty() && form())
	{
		_initialValuesControl = qobject_cast<JASPListControl*>(form()->getControl(initialValuesSourceName));
		addDependency(_initialValuesControl);
		connect(_initialValuesControl->model(), &ListModel::termsChanged, _tableModel, &ListModelTableViewBase::initialValuesChanged);
		_tableModel->initialValuesChanged();
	}
}

void TableViewBase::rScriptDoneHandler(const QString & result)
{
	if(_tableModel)
		_tableModel->rScriptDoneHandler(result);
}

JASPControl::ItemType TableViewBase::itemTypePerItem(int col, int row) const
{
	if (col >= 0 && _itemTypePerColumn.length() > col)	return _itemTypePerColumn[col];
	if (row >= 0 && _itemTypePerRow.length() > row)		return _itemTypePerRow[row];

	return _itemType;
}

void TableViewBase::refreshMe()
{
	if(_tableModel)
		_tableModel->refreshModel();
}

void TableViewBase::termsChangedHandler()
{
	if (_boundControl)
		_boundControl->resetBoundValue();
}

QVariant TableViewBase::defaultValue() const
{
	// Force the QVariant to have the right type
	switch (itemType())
	{
	case JASPControl::ItemType::Integer:
	{
		if (_defaultValue.type() == QVariant::Int)		return _defaultValue;
		if (_defaultValue.canConvert(QVariant::Int))	return _defaultValue.toInt();
		break;
	}
	case JASPControl::ItemType::Double:
	{
		if (_defaultValue.type() == QVariant::Double)	return _defaultValue;
		if (_defaultValue.canConvert(QVariant::Double))	return _defaultValue.toDouble();
		break;
	}
	case JASPControl::ItemType::String:
	{
		if (_defaultValue.type() == QVariant::String)	return _defaultValue;
		if (_defaultValue.canConvert(QVariant::String))	return _defaultValue.toString();
		break;
	}
	}

	return _defaultValue;
}

std::vector<std::string> TableViewBase::usedVariables() const
{
	std::vector<std::string> result;

	if (_tableModel && _tableModel->areColumnNamesVariables())
	{
		for (int i = 0; i < _tableModel->columnCount(); i++)
			result.push_back(fq(_tableModel->headerData(i, Qt::Horizontal).toString()));
	}

	return result;
}

void TableViewBase::setItemTypePerRow(QVariantList list)
{
	QList<JASPControl::ItemType> typeList;
	for (const QVariant& t : list) typeList.append(JASPControl::ItemType(t.toInt()));

	if (typeList != _itemTypePerRow)
	{
		_itemTypePerRow = typeList;
		emit itemTypePerRowChanged();
	}
}

void TableViewBase::setItemTypePerColumn(QVariantList list)
{
	QList<JASPControl::ItemType> typeList;
	for (const QVariant& t : list) typeList.append(JASPControl::ItemType(t.toInt()));

	if (typeList != _itemTypePerColumn)
	{
		_itemTypePerColumn = typeList;
		emit itemTypePerColumnChanged();
	}
}
