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

#ifndef ANALYSISFORMBASE_H
#define ANALYSISFORMBASE_H

#include <QQuickItem>
#include "utils.h"
#include "commotqtglobals.h"

class AnalysisBase;
class COMMONQT_EXPORTS AnalysisFormBase : public QQuickItem
{
    Q_OBJECT

public:
	explicit AnalysisFormBase(QQuickItem * parent = nullptr) : QQuickItem(parent) {} ;

	Q_INVOKABLE virtual bool		initialized()									const			= 0;
				virtual void		cleanUpForm()													= 0;
				virtual bool		hasError()														= 0;
				virtual bool		runOnChange()													= 0;
				virtual stringset	usedVariables()													= 0;
				virtual void		setMustBe(		stringset						mustBe)			= 0;
				virtual void		setMustContain(	std::map<std::string,stringset> mustContain)	= 0;
				virtual void		setHasVolatileNotes(bool hasVolatileNotes)						= 0;
				virtual bool		formCompleted()									const			= 0;
				virtual QString		generateRSyntax(bool useHtml = false)			const			= 0;

				const QString	rSyntaxControlName = "__RSyntaxTextArea";


public slots:
                virtual void            setAnalysis(        AnalysisBase *	analysis)                   = 0;
                virtual void			setShowRButton(		bool			showRButton)                = 0;
                virtual void			setDeveloperMode(	bool			developerMode)              = 0;
                virtual void            runScriptRequestDone(const QString	&	result, const QString & requestId, bool hasError)   = 0;
                virtual void            filterByNameDone(	const QString	&	name,	const QString & error)                      = 0;

signals:
    void	rSourceChanged(const QString& name);
    void	refreshTableViewModels();
    void	titleChanged();
    void	needsRefreshChanged();
	void	languageChanged();

};


#endif // ANALYSISFORMBASE_H
