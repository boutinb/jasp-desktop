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

#include "filemenu.h"

#include <QFileInfo>
#include "utilities/settings.h"
#include "utilities/messageforwarder.h"
#include "log.h"
#include "data/datasetpackage.h"
#include "mainwindow.h"

FileMenu::FileMenu(QObject *parent) : QObject(parent)
{	
	_mainWindow				= qobject_cast<MainWindow*>(parent);
	_recentFiles			= new RecentFiles			(this);
	_currentDataFile		= new CurrentDataFile		(this);
	_computer				= new Computer				(this);
	_OSF					= new OSF					(this);
	_database				= new DatabaseFileMenu				(this);
	_dataLibrary			= new DataLibrary			(this);
	_actionButtons			= new ActionButtons			(this);
	_resourceButtons		= new ResourceButtons		(this);
	_resourceButtonsVisible	= new ResourceButtonsVisible(this, _resourceButtons);

	

	connect(&_watcher,			&QFileSystemWatcher::fileChanged,			this,			&FileMenu::dataFileModifiedHandler	);
	connect(_actionButtons,		&ActionButtons::buttonClicked,				this,			&FileMenu::actionButtonClicked		);
	connect(_actionButtons,		&ActionButtons::selectedActionChanged,		this,			&FileMenu::setFileoperation			);
	connect(_resourceButtons,	&ResourceButtons::selectedButtonChanged,	this,			&FileMenu::resourceButtonClicked	);
	connect(_currentDataFile,	&CurrentDataFile::setCheckAutomaticSync,	_mainWindow,	&MainWindow::setCheckAutomaticSync	);

	_actionButtons->setEnabled(ActionButtons::Open,				true);
	_actionButtons->setEnabled(ActionButtons::Save,				false);
	_actionButtons->setEnabled(ActionButtons::SaveAs,			false);
	_actionButtons->setEnabled(ActionButtons::ExportResults,	false);
	_actionButtons->setEnabled(ActionButtons::ExportData,		false);
	_actionButtons->setEnabled(ActionButtons::SyncData,			false);
	_actionButtons->setEnabled(ActionButtons::Close,			false);
	_actionButtons->setEnabled(ActionButtons::Preferences,		true);
	_actionButtons->setEnabled(ActionButtons::Contact,			true);
	_actionButtons->setEnabled(ActionButtons::About,			true);

	setResourceButtonsVisibleFor(_fileoperation);
}

void FileMenu::setFileoperation(ActionButtons::FileOperation fo)
{
	if(_fileoperation == fo)
		return;

	_fileoperation = fo;
	emit fileoperationChanged();

	setResourceButtonsVisibleFor(fo);
}

void FileMenu::setResourceButtonsVisibleFor(ActionButtons::FileOperation fo)
{
	_resourceButtons->setOnlyTheseButtonsVisible(_actionButtons->resourceButtonsForButton(fo));
}

void FileMenu::setMode(FileEvent::FileMode mode)
{
	_mode = mode;
	_OSF->setCurrentFileName(getDefaultOutFileName());

	emit modeChanged(_mode);
}

void FileMenu::setOnlineDataManager(OnlineDataManager *odm)
{
	_odm = odm;
	_OSF->setOnlineDataManager(odm);
}

FileEvent *FileMenu::open(const QString &path)
{
	FileEvent *event = new FileEvent(this, FileEvent::FileOpen);
	event->setPath(path);
	dataSetIORequestHandler(event);

	return event;
}

FileEvent *FileMenu::open(const Json::Value & dbJson)
{
	FileEvent *event = new FileEvent(this, FileEvent::FileOpen);
	event->setDatabase(dbJson);
	event->setFileType(Utils::FileType::database);
	dataSetIORequestHandler(event);

	return event;
}

FileEvent *FileMenu::saveAs()
{
	return _computer->browseSave();
}

FileEvent *FileMenu::save()
{
	FileEvent *event = nullptr;

	if (_currentFileType != Utils::FileType::jasp || DataSetPackage::pkg()->currentFileIsExample())
	{
		event = saveAs();
		if (event->isCompleted())
			return event;
	}
	else
	{
		event = new FileEvent(this, FileEvent::FileSave);
		if (!event->setPath(_currentFilePath))
		{
			MessageForwarder::showWarning(tr("File Types"), event->getLastError());
			event->setComplete(false, tr("Failed to open file from OSF"));
			return event;
		}
	}

	dataSetIORequestHandler(event);

	return event;
}

void FileMenu::sync()
{
	
	if(DataSetPackage::pkg()->databaseJson() != Json::nullValue)
	{
		FileEvent *event = new FileEvent(this, FileEvent::FileSyncData);
	
		event->setDatabase(DataSetPackage::pkg()->databaseJson());
		dataSetIORequestHandler(event);
	}
	else
	{
		QString path = _currentDataFile->getCurrentFilePath();
	
		if (path.isEmpty())
		{
			if(!MessageForwarder::showYesNo(tr("No associated data file"),
						tr("JASP has no associated data file to be synchronized with.\nDo you want to search for such a data file on your computer?\nNB: You can also set this data file via menu File/Sync Data.")))
				return;
	
			path =  MessageForwarder::browseOpenFile(tr("Find Data File"), "", tr("Data File").arg("*.csv *.txt *.tsv *.sav *.zsav *.ods *.dta *.por *.sas7bdat *.sas7bcat *.xpt"));
		}
	
		_mainWindow->setCheckAutomaticSync(false);
		setSyncRequest(path);
	}	
}

void FileMenu::close()
{
	FileEvent *event = new FileEvent(this, FileEvent::FileClose);
	dataSetIORequestHandler(event);

	setMode(FileEvent::FileOpen);
	_actionButtons->setSelectedAction(ActionButtons::FileOperation::Open);
}

void FileMenu::setCurrentDataFile(const QString &path)
{
	QString currentPath = _currentDataFile->getCurrentFilePath();
	if (!currentPath.isEmpty())
		_watcher.removePath(currentPath);

	bool setCurrentPath = true;
	if (!path.isEmpty())
	{
		if (checkSyncFileExists(path))
		{
			bool sync = DataSetPackage::pkg()->synchingExternally();
			if (sync)
				_watcher.addPath(path);
		}
		else
			setCurrentPath = false;
	}

	if (setCurrentPath)
		_currentDataFile->setCurrentFilePath(path);
}

void FileMenu::setDataFileWatcher(bool watch)
{
	QString path = _currentDataFile->getCurrentFilePath();
	if (path.isEmpty())
		return;

	if (!(watch && !_currentDataFile->isOnlineFile(path)))
		_watcher.removePath(path);

	else if(_watcher.addPath(path))
		dataFileModifiedHandler(path);

}


QString FileMenu::getDefaultOutFileName()
{
	QString path				= getCurrentFilePath(),
			DefaultOutFileName	= "";

	if (path != "")
	{
		QString name	= QFileInfo(path).completeBaseName(),
				ext		= QFileInfo(path).suffix();
		switch (_mode)
		{
		case FileEvent::FileSave:			ext = "jasp";	break;
		case FileEvent::FileExportResults:	ext = "html";	break;
		case FileEvent::FileExportData:
		case FileEvent::FileGenerateData:	ext = "csv";	break;
		default:											break;
		}
		DefaultOutFileName = name + "." + ext;
	}

	return DefaultOutFileName;	
}

void FileMenu::enableButtonsForOpenedWorkspace(bool enableSaveButton)
{
	_actionButtons->setEnabled(ActionButtons::Save,				enableSaveButton);
	_actionButtons->setEnabled(ActionButtons::SaveAs,			true);
	_actionButtons->setEnabled(ActionButtons::ExportResults,	true);
	_actionButtons->setEnabled(ActionButtons::ExportData,		true);
	_actionButtons->setEnabled(ActionButtons::SyncData,			true);
	_actionButtons->setEnabled(ActionButtons::Close,			true);
}

void FileMenu::dataSetIOCompleted(FileEvent *event)
{
	if (event->operation() == FileEvent::FileSave || event->operation() == FileEvent::FileOpen)
	{
		if (event->isSuccessful())
		{
			//  don't add examples to the recent list
			if (!event->isReadOnly())
			{
				_recentFiles->pushRecentFilePath(event->path());
				_computer->addRecentFolder(event->path());
			}

			if(event->operation() == FileEvent::FileSave || (event->operation() == FileEvent::FileOpen && !event->isReadOnly()))
			{
				QString datafile = event->dataFilePath();
				if (datafile.isEmpty())
					datafile = QString::fromStdString(DataSetPackage::pkg()->dataFilePath());
				setCurrentDataFile(datafile);

				if	(	event->operation() == FileEvent::FileOpen
					&& !event->isReadOnly()
					&& event->type() == FileTypeBase::jasp
					&& !DataSetPackage::pkg()->dataFileReadOnly()
					&& DataSetPackage::pkg()->dataSet()->dataFileSynch()
				)
					DataSetPackage::pkg()->setSynchingExternally(true);
			}
			
			// all this stuff is a hack
			QFileInfo info(event->path());
			_computer->setFileName(info.completeBaseName());

			_currentFilePath		= event->path();
			_currentFileType		= event->type();
			_OSF->setProcessing(false);
		}
	}
	else if (event->operation() == FileEvent::FileSyncData)
	{
		if (event->isSuccessful())		setCurrentDataFile(event->dataFilePath());
		else
			Log::log() << "Sync failed: " << event->getLastError().toStdString() << std::endl;
	}
	else if (event->operation() == FileEvent::FileClose)
	{
		_computer->clearFileName();
		_currentFilePath		= "";
		_currentFileType		= Utils::FileType::unknown;
		clearSyncData();
	}

	_resourceButtons->setButtonEnabled(ResourceButtons::CurrentFile, !_currentDataFile->getCurrentFilePath().isEmpty());

	if (event->isSuccessful())
	{
		switch(event->operation())
		{
		case FileEvent::FileOpen:
		case FileEvent::FileSave:
			enableButtonsForOpenedWorkspace((!event->isReadOnly() && event->type() == Utils::FileType::jasp) || event->operation() == FileEvent::FileSave);
			break;

		case FileEvent::FileClose:
			_actionButtons->setEnabled(ActionButtons::Save,				false);
			_actionButtons->setEnabled(ActionButtons::SaveAs,			false);
			_actionButtons->setEnabled(ActionButtons::ExportResults,	false);
			_actionButtons->setEnabled(ActionButtons::ExportData,		false);
			_actionButtons->setEnabled(ActionButtons::SyncData,			false);
			_actionButtons->setEnabled(ActionButtons::Close,			false);
			setMode(FileEvent::FileOpen);
			break;

		default:
			//Do nothing?
			break;
		}
	}
}

void FileMenu::syncDataFile(const QString& path, bool waitForExistence)
{
	bool autoSync = DataSetPackage::pkg()->synchingExternally();
	if (autoSync)
		setSyncRequest(path, waitForExistence);
}

void FileMenu::refresh()
{
	_resourceButtons->refresh();
	_actionButtons->refresh();
	_dataLibrary->refres();
}

void FileMenu::dataFileModifiedHandler(QString path)
{
	_mainWindow->setCheckAutomaticSync(false);
	syncDataFile(path, true);
}

void FileMenu::dataSetIORequestHandler(FileEvent *event)
{
	connect(event, &FileEvent::completed,		this, &FileMenu::dataSetIOCompleted			);

	emit dataSetIORequest(event);

	if(event->operation() != FileEvent::FileClose)
		setVisible(false); //If we just did something we are now done with the filemenu right? Except if we just closed a file
}

void FileMenu::analysisAdded(Analysis *analysis)
{
	_actionButtons->setEnabled(ActionButtons::Close,			true);
	_actionButtons->setEnabled(ActionButtons::SaveAs,			true);
	_actionButtons->setEnabled(ActionButtons::ExportResults,	true);
}

void FileMenu::setSyncFile(FileEvent *event)
{
	if (event->isSuccessful())
		setCurrentDataFile(event->path());
}

void FileMenu::dataColumnAdded(QString columnName)
{
	if(_currentDataFile->getCurrentFilePath() != "" && checkSyncFileExists(_currentDataFile->getCurrentFilePath()))
	{
		//Ok a column was added to the data but we already have a sync file so we should re-generate the data!

		FileEvent * event = new FileEvent(this, FileEvent::FileGenerateData);

		connect(event, &FileEvent::completed, this, &FileMenu::setSyncFile);
		event->setPath(_currentDataFile->getCurrentFilePath());

		dataSetIORequestHandler(event);
	}
}

void FileMenu::analysesExportResults()
{
	_computer->analysesExportResults();
}

void FileMenu::actionButtonClicked(const ActionButtons::FileOperation action)
{	
	switch (action)
	{
	case ActionButtons::FileOperation::Open:				setMode(FileEvent::FileOpen);			break;
	case ActionButtons::FileOperation::SaveAs:				setMode(FileEvent::FileSave);			break;
	case ActionButtons::FileOperation::ExportResults:		setMode(FileEvent::FileExportResults);	break;
	case ActionButtons::FileOperation::ExportData:  		setMode(FileEvent::FileExportData);		break;
	case ActionButtons::FileOperation::SyncData:			setMode(FileEvent::FileSyncData);		break;
	case ActionButtons::FileOperation::Close:				close();								break;
	case ActionButtons::FileOperation::Save:
		if (getCurrentFileType() == Utils::FileType::jasp && ! DataSetPackage::pkg()->currentFileIsExample())
			save();
		else
			setMode(FileEvent::FileSave);			
		break;

	case ActionButtons::FileOperation::About:
		setVisible(false);
		showAboutRequest();
		break;

	case ActionButtons::FileOperation::Contact:
		setVisible(false);
		showContactRequest();
		break;

	default:
		break;
	}
}

void FileMenu::resourceButtonClicked(const int buttonType)
{
	if (buttonType == ResourceButtons::OSF)
		_OSF->attemptToConnect();
}

void FileMenu::showAboutRequest()
{
	emit showAbout();
}

void FileMenu::showContactRequest()
{
	emit showContact();
}

void FileMenu::setSyncRequest(const QString& path, bool waitForExistence)
{
	if (path.isEmpty())
		return;

	if (checkSyncFileExists(path, waitForExistence))
	{
		FileEvent *event = new FileEvent(this, FileEvent::FileSyncData);
		event->setPath(path);

		dataSetIORequestHandler(event);
	}
}

bool FileMenu::checkSyncFileExists(const QString &path, bool waitForExistence)
{
	if (path.startsWith("http"))
		return true;

	auto checkExists = [](const QString& path) { return QFileInfo::exists(path); };
	auto checkNotEmpty = [](const QString& path) { return Utils::getFileSize(path.toStdString()) > 0; };

	auto checkFn = [&](std::function<bool(const QString&)> fn)
	{
		if (!waitForExistence) return fn(path);

		for (int i = 0; i < 10; i++)
		{
			if (fn(path))	return true;
			Utils::sleep(100);
		}
		return false;
	};

	bool result = checkFn(checkExists) && checkFn(checkNotEmpty);

	if (!result)
	{
		Log::log() << "Could not find a valid Sync file in " << path << std::endl;
		clearSyncData();
	}

	return result;
}


void FileMenu::clearSyncData()
{
	setDataFileWatcher(false); // must be done before setting the current to empty.
	_currentDataFile->setCurrentFilePath(QString());
}

bool FileMenu::clearOSFFromRecentList(QString path)
{
	return OnlineDataManager::determineProvider(path) != OnlineDataManager::OSF;
}

void FileMenu::setVisible(bool visible)
{
	if (_visible == visible)
		return;

	_visible = visible;
	emit visibleChanged(_visible);

	if(!_visible)
	{
		_resourceButtons->setSelectedButton(ResourceButtons::None);
		_actionButtons->setSelectedAction(ActionButtons::None);
	}
}

void FileMenu::showFileOpenMenu()
{
	setVisible(true);
	_actionButtons->setSelectedAction(ActionButtons::Open);
}

void FileMenu::showPreferences()
{
	setVisible(true);
	_actionButtons->setSelectedAction(ActionButtons::Preferences);
}

void FileMenu::exportResultsInteractive()
{
	actionButtonClicked(ActionButtons::ExportResults);
	_computer->browseMostRecent();
}
