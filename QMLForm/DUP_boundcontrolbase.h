//
// Copyright (C) 2013-2021 University of Amsterdam
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

#ifndef DUP_BOUNDCONTROLBASE_H
#define DUP_BOUNDCONTROLBASE_H

#include "DUP_boundcontrol.h"
#include "columntype.h"
#include "DUP_listmodel.h"

class DUP_JASPControl;
class DUP_Terms;

class DUP_BoundControlBase : public DUP_BoundControl
{
public:
	DUP_BoundControlBase(DUP_JASPControl* control);
	virtual				~DUP_BoundControlBase()	{}

	Json::Value					createJson()														override { return Json::nullValue;		}
	Json::Value					createMeta()														override;
	void						bindTo(const Json::Value& value)									override { _orgValue = value; setBoundValue(value, false); }
	const Json::Value&			boundValue()														override;
	void						resetBoundValue()													override { bindTo(_orgValue); }
	void						setBoundValue(const Json::Value& value, bool emitChange = true)		override;
	void						setIsRCode(std::string key = "");
	void						setIsColumn(bool isComputed, columnType type = columnType::unknown);
	

protected:
	inline const std::string&	getName();

	void						_readTableValue(const Json::Value& value, const std::string& key, bool hasMultipleTerms, DUP_Terms& terms, DUP_ListModel::RowControlsValues& allControlValues);
	void						_setTableValue(const DUP_Terms& terms, const QMap<QString, DUP_RowControls*>& allControls, const std::string& key, bool hasMultipleTerms);

	DUP_JASPControl*				_control			= nullptr;
	bool						_isComputedColumn	= false,
								_isColumn			= false;
	std::set<std::string>		_isRCode;
	Json::Value					_orgValue;
	std::string					_name;
	columnType					_columnType			= columnType::unknown;
};

#endif // BOUNDCONTROLBASE_H
