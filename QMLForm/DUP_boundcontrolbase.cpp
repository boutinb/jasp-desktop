//
// Copyright (C) 2013-2020 University of Amsterdam
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

#include "DUP_boundcontrolbase.h"
#include "DUP_jaspcontrol.h"
#include "DUP_jasplistcontrol.h"
#include "DUP_analysisform.h"
#include "log.h"
#include "DUP_listmodel.h"
#include "DUP_rowcontrols.h"

DUP_BoundControlBase::DUP_BoundControlBase(DUP_JASPControl* control) : _control(control)
{
}


//To do: define these fields (isRCode, shouldEncode, etc) somewhere centrally through an enum or something
Json::Value DUP_BoundControlBase::createMeta()
{ 
	Json::Value meta(Json::objectValue);
	
	if (_isColumn || _control->encodeValue())	
		meta["shouldEncode"] = true;
	
	for(const std::string & key : _isRCode)
		if(key.empty())		meta	 ["isRCode"] = true;
		else				meta[key]["isRCode"] = true;
	
	return meta;
}

void DUP_BoundControlBase::setBoundValue(const Json::Value &value, bool emitChange)
{
	if (value == boundValue()) return;

	DUP_AnalysisForm* form = _control->form();

	if (form && _control->isBound())
	{
		if (_isColumn && value.isString())
		{
			const Json::Value & orgValue = boundValue();
			std::string			newName  = value.asString(),
								orgName  = orgValue.asString();
			
			if (newName.empty() && !orgName.empty())
				emit _control->requestComputedColumnDestruction(orgName);
			
			else if (newName != orgName)
			{
				if (_isComputedColumn)	emit _control->requestComputedColumnCreation(newName);
				else					emit _control->requestColumnCreation(newName, _columnType);

				if (!orgName.empty())
					emit _control->requestComputedColumnDestruction(orgName);
			}
		}

		form->setBoundValue(getName(), value, createMeta(), _control->getParentKeys());
	}
	else
		emitChange = false;
	
	if (emitChange)	
		emit _control->boundValueChanged(_control);
}

void DUP_BoundControlBase::setIsRCode(std::string key)
{
	_isRCode.insert(key);
}

const Json::Value &DUP_BoundControlBase::boundValue()
{
	DUP_AnalysisForm* form = _control->form();

	if (form && form->analysisObj())	return form->analysisObj()->boundValue(getName(), _control->getParentKeys());
	else								return Json::Value::null;
}

void DUP_BoundControlBase::setIsColumn(bool isComputed, columnType type)
{
	_isColumn = true;
	_isComputedColumn = isComputed;
	_columnType = type;

	DUP_AnalysisForm* form = _control->form();
	if (form)	form->addColumnControl(_control, isComputed);
}


const std::string &DUP_BoundControlBase::getName()
{
	if (_name.empty())
		_name = _control->name().toStdString();

	return _name;
}

void DUP_BoundControlBase::_readTableValue(const Json::Value &value, const std::string& key, bool hasMultipleTerms, DUP_Terms& terms, DUP_ListModel::RowControlsValues& allControlValues)
{
	for (const Json::Value& row : value)
	{
		std::vector<std::string> term;
		const Json::Value& keyValue = row[key];
		if (hasMultipleTerms)
		{
			if (keyValue.isArray())
			{
				for (const Json::Value& component : keyValue)
					term.push_back(component.asString());
				terms.add(DUP_Term(term));
			}
			else
				Log::log() << "Key (" << key << ") bind value is not an array in " << _name << ": " << value.toStyledString() << std::endl;
		}
		else
		{
			if (keyValue.isString())
			{
				term.push_back(keyValue.asString());
				terms.add(DUP_Term(term));
			}
			else
				Log::log() << "Key (" << key << ") bind value is not a string in " << _name << ": " << value.toStyledString() << std::endl;
		}

		QMap<QString, Json::Value> controlMap;
		for (auto itr = row.begin(); itr != row.end(); ++itr)
		{
			const std::string& name = itr.key().asString();
			if (name != key)
				controlMap[tq(name)] = *itr;
		}

		allControlValues[DUP_Term(term).asQString()] = controlMap;
	}
}

void DUP_BoundControlBase::_setTableValue(const DUP_Terms& terms, const QMap<QString, DUP_RowControls*>& allControls, const std::string& key, bool hasMultipleTerms)
{
	Json::Value boundValue(Json::arrayValue);
	for (const DUP_Term& term : terms)
	{
		Json::Value rowValues(Json::objectValue);
		if (hasMultipleTerms)
		{
			Json::Value keyValue(Json::arrayValue);
			for (const std::string& comp : term.scomponents())
				keyValue.append(comp);
			rowValues[key] = keyValue;
		}
		else
		{
			Json::Value keyValue(term.asString());
			rowValues[key] = keyValue;
		}

		DUP_RowControls* rowControls = allControls.value(term.asQString());
		if (rowControls)
		{
			const QMap<QString, DUP_JASPControl*>& controlsMap = rowControls->getJASPControlsMap();
			QMapIterator<QString, DUP_JASPControl*> it(controlsMap);
			while (it.hasNext())
			{
				it.next();
				DUP_JASPControl* control = it.value();
				DUP_BoundControl* boundControl = control->boundControl();
				if (boundControl)
				{
					const QString& name = it.key();
					const Json::Value& value = boundControl->boundValue();
					rowValues[fq(name)] = value;
				}
			}
		}
		boundValue.append(rowValues);
	}

	setBoundValue(boundValue);
}

