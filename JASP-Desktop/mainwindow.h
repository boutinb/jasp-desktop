//
// Copyright (C) 2013-2017 University of Amsterdam
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

#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QMainWindow>

#include "dataset.h"

#include "datasettablemodel.h"
#include "enginesync.h"
#include "analyses.h"
#include "widgets/progresswidget.h"

#include "analysisforms/analysisform.h"
#include "asyncloader.h"
#include "asyncloaderthread.h"
#include "activitylog.h"
#include "fileevent.h"
#include "resultsjsinterface.h"
#include "customwebenginepage.h"

class ResultsJsInterface;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

	friend class ResultsJsInterface;
public:
	explicit MainWindow(QWidget *parent = 0);
	void open(QString filepath);
	virtual ~MainWindow();


protected:
	virtual void resizeEvent(QResizeEvent *event) OVERRIDE;
	virtual void dragEnterEvent(QDragEnterEvent *event) OVERRIDE;
	virtual void dropEvent(QDropEvent *event) OVERRIDE;
	virtual void closeEvent(QCloseEvent *event) OVERRIDE;

private:
	Ui::MainWindow *ui;

	ResultsJsInterface *_resultsJsInterface;
	AnalysisForm *_currentOptionsWidget;
	DataSetPackage *_package;
	DataSetTableModel *_tableModel;
	Analysis *_currentAnalysis;

	int _scrollbarWidth = 0;

	OnlineDataManager *_odm;

	Analyses *_analyses;
	EngineSync* _engineSync;

	void refreshAnalysesUsingColumns(std::vector<std::string> &changedColumns
									, std::vector<std::string> &missingColumns
									, std::map<std::string, std::string> &changeNameColumns);


	void packageChanged(DataSetPackage *package);
	void packageDataChanged(DataSetPackage *package
							, std::vector<std::string> &changedColumns
							, std::vector<std::string> &missingColumns
							, std::map<std::string, std::string> &changeNameColumns
							);


	bool closeRequestCheck(bool &isSaving);

	AsyncLoader _loader;
	AsyncLoaderThread _loaderThread;
	ProgressWidget *_progressIndicator;

	bool _inited;
	bool _applicationExiting = false;

	AnalysisForm* loadForm(Analysis *analysis);
	AnalysisForm* loadForm(const std::string name);
	void showForm(Analysis *analysis);
	void closeCurrentOptionsWidget();
	void removeAnalysis(Analysis *analysis);

	QWidget *_buttonPanel;
	QVBoxLayout *_buttonPanelLayout;
	QPushButton *_okButton;
	QPushButton *_runButton;

	std::map<std::string, AnalysisForm *> _analysisForms;

	int _tableViewWidthBeforeOptionsMadeVisible;

	bool _resultsViewLoaded = false;
	bool _openedUsingArgs = false;
	QString _openOnLoadFilename;
	QSettings _settings;
	ActivityLog *_log;
	QString _fatalError;
	QString _currentFilePath;

	CustomWebEnginePage* _customPage;

	QString escapeJavascriptString(const QString &str);
	void getAnalysesUserData();
	Json::Value getResultsMeta();

	void startDataEditor(QString path);
	void checkUsedModules();
	void resultsPageLoaded(bool success, int ppi);
	void analysisUnselectedHandler();
	void setPackageModified();
	void analysisSelectedHandler(int id);
	void saveTextToFileHandler(const QString &filename, const QString &data);
	void analysisChangedDownstreamHandler(int id, QString options);
	void analysisSaveImageHandler(int id, QString options);
	void removeAnalysisRequestHandler(int id);

signals:
	void updateAnalysesUserData(QString userData);

private slots:
	void analysisResultsChangedHandler(Analysis* analysis);
	void analysisImageSavedHandler(Analysis* analysis);

	void removeAllAnalyses();
	void refreshAllAnalyses();
	void refreshAnalysesUsingColumn(QString col);
	void resetTableView();

	void tabChanged(int index);
	void helpToggled(bool on);

	void dataSetIORequest(FileEvent *event);
	void dataSetIOCompleted(FileEvent *event);
	void populateUIfromDataSet();
	void itemSelected(const QString &item);

	void adjustOptionsPanelWidth();
	void splitterMovedHandler(int, int);

	void hideOptionsPanel();
	void showOptionsPanel();
	void showDataPanel();
	void hideDataPanel();
	void showVariablesPage();
	void startDataEditorHandler();
	void startDataEditorEventCompleted(FileEvent *event);

	void analysisOKed();
	void analysisRunned();

	void updateMenuEnabledDisabledStatus();

	void saveKeysSelected();
	void openKeysSelected();
	void syncKeysSelected();
	void refreshKeysSelected();

	void illegalOptionStateChanged();
	void fatalError();

	void helpFirstLoaded(bool ok);
	void requestHelpPage(const QString &pageName);

    void emptyValuesChangedHandler();
};

#endif // MAINWIDGET_H
