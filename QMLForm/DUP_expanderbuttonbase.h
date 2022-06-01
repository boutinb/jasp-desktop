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

#ifndef DUP_EXPANDERBUTTONBASE_H
#define DUP_EXPANDERBUTTONBASE_H

#include "DUP_jaspcontrol.h"

#include <QObject>

class DUP_ExpanderButtonBase : public DUP_JASPControl
{
	Q_OBJECT
public:
	explicit DUP_ExpanderButtonBase(QQuickItem *parent = nullptr);

	void	setUp() override;

protected:
	DUP_JASPControl* _findFirstControl(QObject* obj);
};

#endif // EXPANDERBUTTONBASE_H
