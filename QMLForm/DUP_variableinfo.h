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

#ifndef DUP_VARIABLEINFO_H
#define DUP_VARIABLEINFO_H

#include <QVariant>
#include <QIcon>

class DUP_Term;
///
/// This is a mechanism to give some extra information about a variable in a VariablesList.
/// There is a provider and a consumer.
/// The provider is in fact always the form. It knows from which dataset comes the variable, and can tell which type it has, and which labels it uses.
/// The consumer is usually the VariablesList, so that it can set for example the right type icon for each variable.
///
class DUP_VariableInfo
{
public:
	enum InfoType { VariableType, Labels, VariableTypeName, VariableTypeIcon, VariableTypeDisabledIcon, VariableTypeInactiveIcon };
};

class DUP_VariableInfoProvider
{
	friend class DUP_VariableInfoConsumer;


protected:
	virtual QVariant requestInfo(const DUP_Term &term, DUP_VariableInfo::InfoType info) const = 0;
};

class DUP_VariableInfoConsumer
{
public:
	DUP_VariableInfoConsumer() {}

	void setInfoProvider(DUP_VariableInfoProvider *provider)
	{
		_provider = provider;
	}

	QVariant requestInfo(const DUP_Term &term, DUP_VariableInfo::InfoType info) const
	{
		if (_provider != nullptr)	return _provider->requestInfo(term, info);
		else						return QVariant();
	}

	QVariant requestLabel(const DUP_Term &term) const
	{
		return requestInfo(term, DUP_VariableInfo::Labels);
	}

private:
	DUP_VariableInfoProvider *_provider = nullptr;
};

#endif // DUP_VARIABLEINFO_H

