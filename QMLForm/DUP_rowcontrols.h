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

#ifndef DUP_ROWCOMPONENTS_H
#define DUP_ROWCOMPONENTS_H

#include <QQmlComponent>
#include <QQuickItem>
#include <json/json.h>

class DUP_JASPListControl;
class DUP_ListModel;
class DUP_JASPControl;
class DUP_Term;

class DUP_RowControls : public QObject
{
Q_OBJECT

public:
	
	DUP_RowControls(
			DUP_ListModel* parent
			, QQmlComponent* components
			, const QMap<QString, Json::Value>& rowValues);

	void										init(int row, const DUP_Term& key, bool isNew);
	void										setContext(int row, const QString& key);
	QQmlComponent*								getComponent()								const	{ return _rowComponent; }
	QQuickItem*									getRowObject()								const	{ return _rowObject;			}
	const QMap<QString, DUP_JASPControl*>&			getJASPControlsMap()						const	{ return _rowJASPControlMap;	}
	DUP_JASPControl*								getJASPControl(const QString& name)					{ return _rowJASPControlMap.contains(name) ? _rowJASPControlMap[name] : nullptr; }
	bool										addJASPControl(DUP_JASPControl* control);

private:

	void										_setupControls(bool reuseBoundValue = false);

	DUP_ListModel*								_parentModel;
	QQmlComponent*							_rowComponent = nullptr;
	QQuickItem*								_rowObject;
	QMap<QString, DUP_JASPControl*>				_rowJASPControlMap;
	QQmlContext*							_context;
	QMap<QString, Json::Value>				_initialValues;
};

#endif // ROWCOMPONENTS_H
