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

#include "DUP_expanderbuttonbase.h"
#include "DUP_analysisform.h"

DUP_ExpanderButtonBase::DUP_ExpanderButtonBase(QQuickItem *parent)
	: DUP_JASPControl(parent)
{
	_controlType = ControlType::Expander;
}

DUP_JASPControl* DUP_ExpanderButtonBase::_findFirstControl(QObject* obj)
{
	DUP_JASPControl* result = nullptr;

	for (QObject* child : obj->children())
	{
		result = qobject_cast<DUP_JASPControl*>(child);

		if (!result)
			result = _findFirstControl(child);

		if (result)
			break;
	}

	return result;
}

void DUP_ExpanderButtonBase::setUp()
{
	if (!form())
		return;

	DUP_ExpanderButtonBase* nextExpander = form()->nextExpander(this);

	if (nextExpander)
		setProperty("nextExpander", QVariant::fromValue(nextExpander));

	QQuickItem* childControlsAreaItem = childControlsArea();

	if (childControlsAreaItem)
	{
		DUP_JASPControl* firstControl = _findFirstControl(childControlsAreaItem);
		if (firstControl)
			setProperty("firstControl", QVariant::fromValue(firstControl));
	}

	setInitialized();
}
