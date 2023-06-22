#include "expanddataproxymodel.h"
#include "datasettablemodel.h"
#include "log.h"

ExpandDataProxyModel::ExpandDataProxyModel(QObject *parent)
	: QObject{parent}
{
	_undoStack = new QUndoStack(this);
	connect(_undoStack, &QUndoStack::indexChanged, this, &ExpandDataProxyModel::undoChanged) ;
}

int ExpandDataProxyModel::rowCount(bool includeVirtuals) const
{
	if (!_sourceModel)
		return 0;
	return _sourceModel->rowCount() + (includeVirtuals && _expandDataSet ? EXTRA_ROWS : 0);
}

int ExpandDataProxyModel::columnCount(bool includeVirtuals) const
{
	if (!_sourceModel)
		return 0;
	return _sourceModel->columnCount() + (includeVirtuals && _expandDataSet ? EXTRA_COLS : 0);
}

QVariant ExpandDataProxyModel::data(int row, int col, int role) const
{
	if (!_sourceModel || role == -1) // Role not defined
		return QVariant();

	if (col < _sourceModel->columnCount() && row < _sourceModel->rowCount())
		return _sourceModel->data(_sourceModel->index(row, col), role);

	if (role == getRole("selected"))
		return false;
	else if (role == getRole("lines"))
	{
		if (col == columnCount() - 1)
			col = _sourceModel->columnCount() - 1;
		else if (col >= _sourceModel->columnCount())
			col = _sourceModel->columnCount() - 2;
		if (col < 0) col = 0;
		if (row == rowCount() - 1)
			row = _sourceModel->rowCount() - 1;
		else if (row >= _sourceModel->rowCount())
			row = _sourceModel->rowCount() - 2;
		if (row < 0) row = 0;

		return _sourceModel->data(_sourceModel->index(row, col), role);
	}
	else if (role == getRole("value"))
		return "";
	else if (role == getRole("itemInputValue"))
		return "string";

	return QVariant();
}

QVariant ExpandDataProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	if (!_sourceModel || role == -1) // Role not defined
		return QVariant();

	if (orientation == Qt::Orientation::Horizontal)
	{
		if (section < _sourceModel->columnCount())
			return _sourceModel->headerData(section, orientation, role);
		else
		{
			if (role == getRole("columnIsComputed"))
				return false;
			else if (role == getRole("computedColumnIsInvalidated"))
				return false;
			else if (role == getRole("filter"))
				return false;
			else if (role == getRole("computedColumnError"))
				return "";
			else if (role == getRole("columnType"))
				return int(columnType::unknown);
			else if (role == getRole("maxColString"))
				return "XXXXXXXXXXX";
			else if (role == Qt::DisplayRole)
				return "";
		}
	}
	else if (orientation == Qt::Orientation::Vertical)
	{
		if (section < _sourceModel->rowCount())
			return _sourceModel->headerData(section, orientation, role);
		else
			return section + 1;
	}

	return QVariant();
}

Qt::ItemFlags ExpandDataProxyModel::flags(int row, int column) const
{
	if (!_sourceModel)
		return Qt::NoItemFlags;

	if (column < _sourceModel->columnCount() && row < _sourceModel->rowCount())
		return _sourceModel->flags(index(row, column));

	return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

QModelIndex ExpandDataProxyModel::index(int row, int column, const QModelIndex &parent) const
{
	if (!_sourceModel)
		return QModelIndex();

	return _sourceModel->index(row, column, parent);
}

bool ExpandDataProxyModel::filtered(int row, int column) const
{
	if (!_sourceModel)
		return false;

	if (column < _sourceModel->columnCount() && row < _sourceModel->rowCount())
	{
		QModelIndex ind(_sourceModel->index(row, column));
		return _sourceModel->data(ind, getRole("filter")).toBool();
	}

	return true;
}

bool ExpandDataProxyModel::isRowVirtual(int row) const
{
	if (!_sourceModel)
		return false;

	return row >= _sourceModel->rowCount();
}

bool ExpandDataProxyModel::isColumnVirtual(int col) const
{
	if (!_sourceModel)
		return false;

	return col >= _sourceModel->columnCount();
}

void ExpandDataProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
	if(_sourceModel != sourceModel)
		_sourceModel = sourceModel;

	_setRolenames();
}

void ExpandDataProxyModel::_setRolenames()
{
	_roleNameToRole.clear();

	if(_sourceModel == nullptr) return;

	auto roleNames = _sourceModel->roleNames();

	for(auto rn : roleNames.keys())
		_roleNameToRole[roleNames[rn].toStdString()] = rn;
}

int ExpandDataProxyModel::getRole(const std::string &roleName) const
{
	auto it = _roleNameToRole.find(roleName);
	if (it == _roleNameToRole.end())
		return -1;
	else
		return it->second;
}

void ExpandDataProxyModel::undo()
{
	_undoStack->undo();
}

void ExpandDataProxyModel::redo()
{
	_undoStack->redo();
}

void ExpandDataProxyModel::columnDataTypeChanged(QString colName)
{
	// A column type has been changed: either it was explicitly set, and then it is already in the undoStack,
	// or it is due a data change command, then this change must be added in the undoStack as child of this command.
	// TODO
}

void ExpandDataProxyModel::removeRows(int start, int count)
{
	if (!_sourceModel)
		return;

	if (count > 1)
		_startMacro(QObject::tr("Removes %1 rows from %2").arg(count).arg(start + 1));

	for (int i = 0; i < count; i++)
		removeRow(start + i);
	_endMacro();
}

void ExpandDataProxyModel::removeColumns(int start, int count)
{
	if (!_sourceModel)
		return;

	if (count > 1)
		_startMacro(QObject::tr("Removes %1 columns from %2").arg(count).arg(start + 1));

	for (int i = 0; i < count; i++)
		removeColumn(start + i);

	_endMacro();
}

void ExpandDataProxyModel::removeRow(int row)
{
	if (!_sourceModel)
		return;

	if (row >= 0 && row < _sourceModel->rowCount())
		_pushCommand(new RemoveRowCommand(this, row));
}

void ExpandDataProxyModel::removeColumn(int col)
{
	if (!_sourceModel)
		return;

	if (col >= 0 && col < _sourceModel->columnCount())
		_pushCommand(new RemoveColumnCommand(this, col));
}

void ExpandDataProxyModel::insertRow(int row)
{
	if (!_sourceModel)
		return;

	_pushCommand(new InsertRowCommand(this, row));
}

void ExpandDataProxyModel::insertColumn(int col, bool computed, bool R)
{
	if (!_sourceModel)
		return;

	_pushCommand(new InsertColumnCommand(this, col, computed, R));
}

void ExpandDataProxyModel::_pushCommand(UndoModelCommand *command)
{
	if (!_parentCommand) // Push to the stack only when no macro is started: in this case the command is autmatically added to the _parentCommand
		_undoStack->push(command);
}

void ExpandDataProxyModel::_startMacro(const QString& text)
{
	if (_parentCommand)
		Log::log() << "Macro started though last one is not finished!" << std::endl;
	_parentCommand = new UndoModelCommand(this);
	if (!text.isEmpty())
		_parentCommand->setText(text);
}

void ExpandDataProxyModel::_endMacro(UndoModelCommand* command)
{
	if (command)
	{
		if (_parentCommand)
			_parentCommand->setText(command->text());
		else
			_undoStack->push(command); // Case when macro was not started
	}
	if (_parentCommand)
		_undoStack->push(_parentCommand);

	_parentCommand = nullptr;
}

void ExpandDataProxyModel::_expandIfNecessary(int row, int col)
{
	QUndoCommand* parentCommand = nullptr;

	if (!_sourceModel || row < 0 || col < 0 || row >= rowCount() || col >= columnCount())
		return;

	if (col >= _sourceModel->columnCount() || row >= _sourceModel->rowCount())
		_startMacro();

	for (int colNr = _sourceModel->columnCount(); colNr <= col; colNr++)
		insertColumn(colNr, false, false);
	for (int rowNr = _sourceModel->rowCount(); rowNr <= row; rowNr++)
		insertRow(rowNr);

}

void ExpandDataProxyModel::setData(int row, int col, const QVariant &value, int role)
{
	if (!_sourceModel || row < 0 || col < 0)
		return;

	_expandIfNecessary(row, col);
	_endMacro(new SetDataCommand(this, row, col, value, role));
}

void ExpandDataProxyModel::pasteSpreadsheet(int row, int col, const std::vector<std::vector<QString>> & cells, QStringList newColNames)
{
	if (!_sourceModel || row < 0 || col < 0)
		return;

	_expandIfNecessary(row + cells.size() > 0 ? cells[0].size() : 0, col + cells.size());
	_endMacro(new PasteSpreadsheetCommand(this, row, col, cells, newColNames));
}

int ExpandDataProxyModel::setColumnType(int columnIndex, int columnType)
{
	_pushCommand(new SetColumnTypeCommand(this, columnIndex, columnIndex));

	return int(DataSetPackage::pkg()->getColumnType(columnIndex));
}

SetDataCommand::SetDataCommand(ExpandDataProxyModel *proxyModel, int row, int col, const QVariant &value, int role)
	: UndoModelCommand(proxyModel), _newValue(value), _row(row), _col(col), _role(role)
{
	setText(QObject::tr("Set value to %1 at row %2 column %3").arg(_newValue.toString()).arg(_row).arg(_col));
}

void SetDataCommand::undo()
{
	sourceModel()->setData(_proxyModel->index(_row, _col), _oldValue, _role);
}

void SetDataCommand::redo()
{
	_oldValue = _proxyModel->data(_row, _col);
	sourceModel()->setData(_proxyModel->index(_row, _col), _newValue, _role);
}

InsertColumnCommand::InsertColumnCommand(ExpandDataProxyModel *proxyModel, int column, bool computed, bool R)
	: UndoModelCommand(proxyModel), _col(column), _computed(computed), _R(R)
{
	setText(_computed ? QObject::tr("Insert computed column %1").arg(_col) : QObject::tr("Insert column %1").arg(_col));
}

void InsertColumnCommand::undo()
{
	sourceModel()->removeColumn(_col);
}

void InsertColumnCommand::redo()
{
	DataSetTableModel * dataSetTable = dynamic_cast<DataSetTableModel *>(sourceModel());

	if (dataSetTable)
		dataSetTable->insertColumnSpecial(_col, _computed, _R);
}

InsertRowCommand::InsertRowCommand(ExpandDataProxyModel *proxyModel, int row)
	: UndoModelCommand(proxyModel), _row(row)
{
	setText(QObject::tr("Insert row %1").arg(_row));
}

void InsertRowCommand::undo()
{
	sourceModel()->removeRow(_row);
}

void InsertRowCommand::redo()
{
	sourceModel()->insertRow(_row);
}

RemoveColumnCommand::RemoveColumnCommand(ExpandDataProxyModel *proxyModel, int col)
	: UndoModelCommand(proxyModel), _col(col)
{
	setText(QObject::tr("Remove column %1").arg(_col));
}

void RemoveColumnCommand::undo()
{
	sourceModel()->insertColumn(_col);
	DataSetPackage::pkg()->setColumn(_col, _serializedColumn);
}

void RemoveColumnCommand::redo()
{
	_serializedColumn = DataSetPackage::pkg()->getColumn(_col);
	sourceModel()->removeColumn(_col);
}

RemoveRowCommand::RemoveRowCommand(ExpandDataProxyModel *proxyModel, int row)
	: UndoModelCommand(proxyModel), _row(row)
{
	setText(QObject::tr("Remove row %1").arg(_row));
}

void RemoveRowCommand::undo()
{
	sourceModel()->insertRow(_row);

	for (int i = 0; i < sourceModel()->columnCount() && i < _values.count(); i++)
		sourceModel()->setData(sourceModel()->index(_row, i), _values[i], 0);
}

void RemoveRowCommand::redo()
{
	_values.clear();
	for (int i = 0; i < sourceModel()->columnCount(); i++)
		_values.push_back(sourceModel()->data(sourceModel()->index(_row, i)));

	sourceModel()->removeRow(_row);
}

PasteSpreadsheetCommand::PasteSpreadsheetCommand(ExpandDataProxyModel *proxyModel, int row, int col, const std::vector<std::vector<QString> > &cells, const QStringList &newColNames)
	: UndoModelCommand(proxyModel), _row(row), _col(col), _newCells(cells), _newColNames(newColNames)
{
	setText(QObject::tr("Paste spreadsheet at row %1 column %2").arg(_row).arg(_col));
}

void PasteSpreadsheetCommand::undo()
{
	DataSetTableModel* dataSetTable = qobject_cast<DataSetTableModel*>(_proxyModel->sourceModel());

	if (dataSetTable)
		dataSetTable->pasteSpreadsheet(_row, _col, _oldCells, _newColNames);
}

void PasteSpreadsheetCommand::redo()
{
	_oldCells.clear();
	for (int c = 0; c < _newCells.size(); c++)
	{
		_oldCells.push_back(std::vector<QString>());
		for (int r = 0; r < _newCells[c].size(); r++)
			_oldCells[c].push_back(sourceModel()->data(sourceModel()->index(_row + r, _col + c)).toString());
	}

	DataSetTableModel* dataSetTable = qobject_cast<DataSetTableModel*>(_proxyModel->sourceModel());

	if (dataSetTable)
		dataSetTable->pasteSpreadsheet(_row, _col, _newCells, _newColNames);
}

UndoModelCommand::UndoModelCommand(ExpandDataProxyModel *proxyModel)
	: QUndoCommand{proxyModel->parentCommand()}, _proxyModel{proxyModel}
{
}

QAbstractItemModel *UndoModelCommand::sourceModel() const
{
	return _proxyModel->sourceModel();
}

SetColumnTypeCommand::SetColumnTypeCommand(ExpandDataProxyModel *proxyModel, int col, int colType)
	: UndoModelCommand(proxyModel),_col(col), _newColType(colType)
{
	setText(QObject::tr("Set type %1 to column %2").arg(columnTypeToQString(columnType(colType))).arg(col));
}

void SetColumnTypeCommand::undo()
{
	DataSetPackage::pkg()->setColumnTypeFromQML(_col, _oldColType);
}

void SetColumnTypeCommand::redo()
{
	_oldColType = int(DataSetPackage::pkg()->getColumnType(_col));
	DataSetPackage::pkg()->setColumnTypeFromQML(_col, _newColType);
}
