//
// Copyright (C) 2018 University of Amsterdam
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

#include "computer.h"
#include <QDir>
#include "messageforwarder.h"
#include "log.h"

Computer::Computer(FileMenu *parent): FileMenuObject(parent)
{
	setListModel(new ComputerListModel(this));

	qRegisterMetaType<FileEvent::FileMode>("FileEvent::FileMode");

	connect(this, &Computer::browseOpenSignal, this, &Computer::browseOpen, Qt::QueuedConnection);
	connect(this, &Computer::browseSaveSignal, this, &Computer::browseSave, Qt::QueuedConnection);
}

FileEvent *Computer::browseOpen(const QString &path)
{
	QString browsePath;
	if (path == "")
		browsePath = _computerListModel->getMostRecent();
	else
		browsePath = path;

	QString filter = tr("All Data Sets %1").arg("(*.jasp *.csv *.txt *.tsv *.sav *.zsav  *.ods *.xls *.xlsx *.dta *.por *.sas7bdat *.sas7bcat *.xpt);;")
					+ tr("JASP Files %1").arg("(*.jasp);;")
					+ tr("CSV Text Files %1").arg("(*.csv *.txt *.tsv);;")
					+ tr("Spreadsheet Files %1").arg("(*.ods *.xls *.xlsx);;")
					+ tr("SPSS Files %1").arg("(*.sav *.zsav *.por)") + ";;"
					+ tr("Stata Files %1").arg("(*.dta);;")
					+ tr("SAS Files %1").arg("(*.sas7bdat *.sas7bcat *.xpt)");

	if (mode() == FileEvent::FileSyncData)
		filter = "Data Sets (*.csv *.txt *.tsv *.sav *.ods *.xls *.xlsx)";

	Log::log() << "Now calling MessageForwarder::browseOpenFile(\"Open\", \"" << browsePath.toStdString() << "\", \"" << filter.toStdString() << "\")" << std::endl;
	QString finalPath = MessageForwarder::browseOpenFile("Open", browsePath, filter);
	Log::log() << "Chosen path: \"" << finalPath.toStdString() << "\"" << std::endl;

	FileEvent *event = new FileEvent(this, mode());

	if (finalPath != "")
	{
		event->setPath(finalPath);
		emit dataSetIORequest(event);
	}
	else
	{
		event->setComplete(false);
	}

	return event;
}

FileEvent *Computer::browseSave(const QString &path, FileEvent::FileMode mode)
{
	QString caption, filter;
	QString browsePath = path;
	if (path == "")
		browsePath = _computerListModel->getMostRecent();

	if (!_fileName.isEmpty())
		browsePath += QDir::separator() + _fileName;

	switch(mode)
	{
	case FileEvent::FileExportResults:
		caption = tr("Export Result as HTML or PDF");
		filter = tr("HTML Files") += " (*.html);;" + tr("Portable Document Format") + " (*.pdf)";
		browsePath += ".html";
		break;

	case FileEvent::FileGenerateData:
	case FileEvent::FileExportData:
		caption	= tr("Export Data as CSV");
		filter	= tr("CSV Files") += " (*.csv *.txt *.tsv)";
		browsePath += ".csv";
		break;

	case FileEvent::FileSyncData:
		caption = tr("Sync Data");
		filter  = tr("Data Files") += " (*.csv *.txt *.tsv *.sav *.ods *.xls *.xlsx)";
		browsePath += ".csv";
		break;

	case FileEvent::FileSave:
		caption = tr("Save");
		filter  = tr("JASP Files") + " (*.jasp)";
		browsePath += ".jasp";
		break;

	default:
		throw std::runtime_error("Wrong FileEvent type for saving!");
	}


	QString extension,
			finalPath = MessageForwarder::browseSaveFile(caption, browsePath, filter, &extension);

	FileEvent *event = new FileEvent(this, mode);

	if (finalPath != "")
	{
		// Default file extensions if not specified
		if		(mode == FileEvent::FileSave			&&	 !finalPath.endsWith(".jasp", Qt::CaseInsensitive)	)	finalPath.append(QString(".jasp"));
		else if	(mode == FileEvent::FileExportResults	&&	(!finalPath.endsWith(".html", Qt::CaseInsensitive) &&
															 !finalPath.endsWith(".pdf",  Qt::CaseInsensitive))	)	finalPath.append(QString(".html"));
		else if	(mode == FileEvent::FileExportData		&&	(!finalPath.endsWith(".csv",  Qt::CaseInsensitive) &&
															 !finalPath.endsWith(".txt",  Qt::CaseInsensitive) &&
															 !finalPath.endsWith(".tsv",  Qt::CaseInsensitive))	)	finalPath.append(QString(".csv"));
		event->setPath(finalPath);
		emit dataSetIORequest(event);
	}
	else
		event->setComplete(false);

	return event;

}

void Computer::addRecentFolder(const QString &path)
{
    _computerListModel->addRecentFolder(path);
}

void Computer::analysesExportResults()
{
    setMode(FileEvent::FileExportResults);
    browseMostRecent();
}

void Computer::setFileName(const QString &filename)
{
	_fileName = filename;
}

void Computer::clearFileName()
{
	_fileName = "";
}

void Computer::browsePath(QString path)
{

	Log::log() << "void Computer::browsePath(\"" << path.toStdString() << "\") called, now sending out signal to show " << (mode() == FileEvent::FileOpen || mode() == FileEvent::FileSyncData ? "Open " : "Save ") << "file dialog." << std::endl;

	if (mode() == FileEvent::FileOpen || mode() == FileEvent::FileSyncData)
		emit browseOpenSignal(path);
	else
		emit browseSaveSignal(path, mode());
}

void Computer::browseMostRecent()
{
	QString mostrecent = _computerListModel->getMostRecent();
	browsePath(mostrecent);
}


void Computer::setListModel(ComputerListModel * listModel)
{
	if (_computerListModel == listModel)
		return;

	_computerListModel = listModel;

	connect(_computerListModel, &ComputerListModel::browsePath, this, &Computer::browsePath);

	emit listModelChanged(_computerListModel);
}
