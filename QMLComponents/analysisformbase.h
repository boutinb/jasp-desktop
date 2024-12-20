//
// Copyright (C) 2013-2024 University of Amsterdam
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

#ifndef ANALYSISFORMBASE_H
#define ANALYSISFORMBASE_H

#include <QQuickItem>
#include "utils.h"

class AnalysisBase;

class AnalysisFormBase : public QQuickItem
{
	Q_OBJECT

public:

	explicit AnalysisFormBase(QQuickItem *parent = nullptr) : QQuickItem(parent)	{}

	virtual stringset	usedVariables()												{ return stringset(); }
	virtual void		cleanUpForm()												{}
	virtual bool		hasError()													{ return false; }
	virtual bool		runOnChange()												{ return false; }
	virtual bool		showRButton()										const	{ return false; }
	virtual void		setMustBe(		std::set<std::string>						mustBe)			{}
	virtual void		setMustContain(	std::map<std::string,std::set<std::string>> mustContain)	{}
	virtual void		setHasVolatileNotes(bool hasVolatileNotes)					{}
	virtual bool		formCompleted()										const	{ return false;	}
	virtual Q_INVOKABLE bool initialized()									const	{ return false; }
	virtual QString		generateRSyntax(bool useHtml = false)				const	{ return QString(); }

	const QString		rSyntaxControlName = "__RSyntaxTextArea";

public slots:
	virtual void	setAnalysis(AnalysisBase * analysis)							{}
	virtual void	setShowRButton(bool showRButton)								{}
	virtual void	setDeveloperMode(bool developerMode)							{}
	virtual void	runScriptRequestDone(const QString& result, const QString& requestId, bool hasError) {}


signals:
	void	rSourceChanged(const QString& name);
	void	refreshTableViewModels();
	void	needsRefreshChanged();
	void	titleChanged();
	void	languageChanged();
};

#endif // ANALYSISFORMBASE_H


