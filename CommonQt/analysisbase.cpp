//
// Copyright (C) 2013-2025 University of Amsterdam
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

#include "analysisbase.h"
#include "analysisformbase.h"
#include "log.h"
#include "utilities/qmlutils.h"

const std::string AnalysisBase::emptyString;
const stringvec AnalysisBase::emptyStringVec;

AnalysisBase::AnalysisBase(QObject* parent, Version moduleVersion)
	: QObject(parent)
	, _moduleVersion(moduleVersion)
{
}

AnalysisBase::AnalysisBase(QObject* parent, AnalysisBase* duplicateMe)
	: QObject(parent)
	, _moduleVersion(duplicateMe->moduleVersion())
	, _boundValues(duplicateMe->boundValues())
{
}

QQuickItem* AnalysisBase::formItem() const
{
	return _analysisForm;
}

void AnalysisBase::destroyForm()
{
	Log::log() << "Analysis(" << this << ")::destroyForm() called" << std::endl;

	if(_analysisForm)
	{
		Log::log(false) << " it has a AnalysisForm " << _analysisForm << " so let's destroy it" << std::endl;

		_analysisForm->setParent(		nullptr);
		_analysisForm->setParentItem(	nullptr);

		delete _analysisForm;
		_analysisForm = nullptr;

		emit formItemChanged();
	}
	else
		Log::log(false) << " it has no AnalysisForm." << std::endl;
}

void AnalysisBase::createForm(QQuickItem* parentItem)
{
	Log::log() << "Analysis(" << this << ")::createForm() called with parentItem " << parentItem << std::endl;

	setQmlError("");
	
	if(parentItem)
		_parentItem = parentItem;

	if(_analysisForm)
	{
		Log::log() << "It already has a form, so we destroy it." << std::endl;
		
		if (!parentItem) 
			parentItem = _analysisForm->parentItem();
		
		destroyForm();
	}
	else if(!parentItem)
		parentItem = _parentItem;

	try
	{
		Log::log()  << std::endl << "Loading QML form from: " << qmlFormPath(false, false) << std::endl;

		QObject * newForm = instantiateQml(QUrl::fromLocalFile(tq(qmlFormPath(false, false))), module(), qmlContext(parentItem));

		Log::log() << "Created a form, got pointer " << newForm << std::endl;

        _analysisForm = qobject_cast<AnalysisFormBase *>(newForm);

		if(!_analysisForm)
			throw std::logic_error("QML file '" + qmlFormPath(false, false) + "' didn't spawn into AnalysisForm, but into: " + (newForm ? fq(newForm->objectName()) : "null"));

		_analysisForm->setAnalysis(this);
		_analysisForm->setParent(this);
		_analysisForm->setParentItem(parentItem);

		emit formItemChanged();
	}
	catch(qmlLoadError & e)
	{
		setQmlError(e.what());
		_analysisForm = nullptr;
	}
	catch(std::exception & e)
	{
		setQmlError(e.what());
		_analysisForm = nullptr;
	}
}

std::string AnalysisBase::qmlFormPath(bool, bool) const
{
	return module() + "/qml/"  + name();
}

const QString &AnalysisBase::qmlError() const
{
	return _qmlError;
}

void AnalysisBase::setQmlError(const QString &newQmlError)
{
	if (_qmlError == newQmlError)
		return;
	_qmlError = newQmlError;
	emit qmlErrorChanged();
}

// This method tries to find the parent keys in _boundValues Json object
// If found, it sets the path to this reference to parentNames and returns a reference of the sub Json object
Json::Value& AnalysisBase::_getParentBoundValue(const QVector<AnalysisBase::ParentKey>& parentKeys, QVector<std::string>& parentNames, bool& found, bool createAnyway)
{
	found = (parentKeys.size() == 0);
	Json::Value* parentBoundValue = &_boundValues;

	// A parentKey has 3 properties: <name>, <key> and <value>: it assumes that the json boundValue is an abject, one of its member is <name>,
	// whom value is an array of json objects. These objects have a member <key>, and one of them has for value <value>.
	// if there are several parentKeys, it repeats this operation
	//
	// {
	//		anOptionName : "one"
	//		...
	//		<name>: [
	//			{
	//				<key>: "anothervalue"
	//				anotherKey: "xxx"
	//			},
	//			{
	//				<key>: "<value>"
	//				anotherKey: "yyy" // parentBoundValue gets a reference to this Json object
	//			}
	//		]
	// }

	for (const auto& parent : parentKeys)
	{
		found = false;
		if (createAnyway && !parentBoundValue->isMember(parent.name))	(*parentBoundValue)[parent.name] = Json::Value(Json::arrayValue);

		if (parentBoundValue->isMember(parent.name))
		{
			Json::Value* parentBoundValues = &(*parentBoundValue)[parent.name];
			if (parentBoundValues->isObject() && parentBoundValues->isMember("value") && parentBoundValues->isMember("types"))
				parentBoundValues = &(*parentBoundValues)["value"]; // If the control contain variable names, the option values are inside the 'value' member

			if (!parentBoundValues->isNull() && parentBoundValues->isArray())
			{
				for (Json::Value & boundValue : (*parentBoundValues))
				{
					if (boundValue.isMember(parent.key))
					{
						Json::Value* keyValue = &(boundValue[parent.key]);
						if (keyValue->isObject() && keyValue->isMember("value") && keyValue->isMember("types"))
							keyValue = &((*keyValue)["value"]); // The key can be also a variable name

						// The value can be a string or an array of strings (for interaction terms)
						if (keyValue->isString() && parent.value.size() == 1)
						{
							if (keyValue->asString() == parent.value[0])	found = true;
						}
						else if (keyValue->isArray() && keyValue->size() == parent.value.size())
						{
							found = true;
							size_t i = 0;
							for (const Json::Value& compVal : *keyValue)
							{
								if (!compVal.isString() || compVal.asString() != parent.value[i]) found = false;
								i++;
							}
						}
						if (found)
						{
							parentBoundValue = &boundValue;
							parentNames.append(parent.name);
							break;
						}
					}
				}
			}

			if (!found && createAnyway && (parentBoundValues->isNull() || parentBoundValues->isArray()))
			{
				// A control can be created before the parent control is completed.
				Json::Value row(Json::objectValue);
				if (parent.value.size() == 1)
					row[parent.key] = parent.value[0];
				else
				{
					Json::Value newValue(Json::arrayValue);
					for (size_t i = 0; i < parent.value.size(); i++)
						newValue.append(parent.value[i]);
					row[parent.key] = newValue;
				}
				parentBoundValue = &(parentBoundValues->append(row));
				found = true;
			}
		}
	}

	return *parentBoundValue;
}

std::string AnalysisBase::_displayParentKeys(const QVector<AnalysisBase::ParentKey> & parentKeys) const
{
	std::string keys;
	bool firstKey = true;
	for (const auto& parentKey : parentKeys)
	{
		std::string parentValue;
		bool firstValue = true;
		for (const std::string& v : parentKey.value)
		{
			if (!firstValue) parentValue += "*";
			parentValue += v;
			firstValue = false;
		}

		if (!firstKey) keys += " - ";
		keys += ("key: " + parentKey.key + " name: " + parentKey.name + " value: " + parentValue);
		firstKey = false;
	}

	return keys;
}

void AnalysisBase::setBoundValue(const std::string &name, const Json::Value &value, const Json::Value &meta, const QVector<AnalysisBase::ParentKey>& parentKeys)
{
	bool found = false;
	QVector<std::string> parents;
	Json::Value& parentBoundValue = _getParentBoundValue(parentKeys, parents, found, true);

	if (found && parentBoundValue.isObject())
	{
		parentBoundValue[name] = value;

		if ((meta.isObject() || meta.isArray()) && meta.size() > 0)
		{
			Json::Value* metaBoundValue = &_boundValues[".meta"];
			for (const std::string& parent : parents)
				metaBoundValue = &((*metaBoundValue)[parent]);
			(*metaBoundValue)[name] = meta;
		}
	}
	else
		Log::log() << "Could not find parent keys " << _displayParentKeys(parentKeys) << " in options: " << _boundValues.toStyledString() << std::endl;

	if (_analysisForm->initialized())
		emit boundValuesChanged();
}

void AnalysisBase::setBoundValues(const Json::Value &boundValues)
{
	_boundValues = boundValues;
}

const Json::Value &AnalysisBase::boundValue(const std::string &name, const QVector<AnalysisBase::ParentKey> &parentKeys)
{
	bool found = false;
	QVector<std::string> parentNames;
	Json::Value& parentBoundValue = _getParentBoundValue(parentKeys, parentNames, found);


	if (found && !parentBoundValue.isNull() && parentBoundValue.isObject())	return parentBoundValue[name];
	else																	return Json::Value::null;
}
