#include "helpmodel.h"
#include "appdirs.h"
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include "stringutils.h"
#include "gui/preferencesmodel.h"
#include "log.h"

HelpModel::HelpModel(QObject * parent) : QObject(parent)
{
	setPagePath("index");
	connect(this,						&HelpModel::pagePathChanged,				this, &HelpModel::generateJavascript);
	connect(PreferencesModel::prefs(),	&PreferencesModel::currentThemeNameChanged, this, &HelpModel::setThemeCss,			Qt::QueuedConnection);
	connect(PreferencesModel::prefs(),	&PreferencesModel::resultFontChanged,		this, &HelpModel::setFont,				Qt::QueuedConnection);
	connect(this,						&HelpModel::markdownChanged,				this, &HelpModel::loadMarkdown);
}

void HelpModel::runJavaScript(QString renderFunc, QString content)
{
#ifdef JASP_DEBUG
	Log::log() << "Help is sending content: '" << content << "'" << std::endl;
#endif

	content.replace("\\", "\\\\");  //The order here is important, replace the escaped escape characters first. Helps with latex, see: https://github.com/jasp-stats/INTERNAL-jasp/issues/2530
	content.replace("\"", "\\\"");
	content.replace("\r\n", "\\n");
	content.replace("\r", "\\n");
	content.replace("\n", "\\n");



	runJavaScriptSignal(renderFunc + "(\"" + content + "\");");
}

void HelpModel::setVisible(bool visible)
{
	if (_visible == visible)
		return;

	_visible = visible;
	
	if(!_visible)
	{
		_analysis = nullptr;
		_pagePath = "";	
		_markdown = "";
	}
	
	emit visibleChanged(_visible);

}

void HelpModel::loadingSucceeded()
{
	setThemeCss(PreferencesModel::prefs()->currentThemeName());
	setFont();
	generateJavascript();
}

void HelpModel::setMarkdown(QString markdown)
{
	if (_markdown == markdown)
		return;
	
	if(markdown != "")
		_pagePath = "";
	
	_markdown = markdown;

	if(_analysis != nullptr)
		_analysis->dynamicModule()->preprocessMarkdownHelp(_markdown);
	
	emit markdownChanged(_markdown);
}

void HelpModel::setPagePath(QString pagePath)
{	
    _pagePath = pagePath;
	emit pagePathChanged(_pagePath);
}

QString	HelpModel::indexURL()
{
	return "file:" + AppDirs::help() + "/index.html";
}

void HelpModel::generateJavascript()
{
	if(markdown() != "")
	{
		loadMarkdown(markdown());
		return;
	}

	QString renderFunc = "";
	QString content = "";

	//Try to load laguage specific translated help file first.
	if (!loadHelpContent(_pagePath, false, renderFunc, content))
	{
		//Fall back to English
		if (!loadHelpContent(_pagePath, true, renderFunc, content))
		{
			content = tr("Coming Soon!\n========\n\nThere is currently no help available for this analysis.\n\nAdditional documentation will be available in future releases of JASP.");
#ifdef JASP_DEBUG
			content += " (" + _pagePath + ")";
#endif
		}
	}

	runJavaScript(renderFunc, content);
}

QString HelpModel::convertPagePathToLower(const QString & pagePath)
{
	std::string pagePathStd		= pagePath.toStdString();
	auto		slashPos		= pagePathStd.find_last_of('/');

	if(slashPos == std::string::npos)
		return pagePath.toLower();

	slashPos++;

	std::string	lastSegment		= pagePathStd.substr(slashPos),
				firstSegment	= pagePathStd.substr(0, slashPos);

	return QString::fromStdString(firstSegment + stringUtils::toLower(lastSegment));
}

void HelpModel::showOrTogglePage(QString pagePath)
{
	_analysis = nullptr;
	
    if((pagePath == _pagePath) && _visible)
		setVisible(false);
	else
	{
		setPagePath(pagePath);
		setVisible(true);
	}
}

void HelpModel::showOrToggleParticularPageForAnalysis(Analysis * analysis, QString helpPage)
{
	if(!analysis)
	{
		setAnalysis(nullptr);
		setVisible(false);
		return;
	}
	
	QString renderFunc, 
			contentMD,
			pagePath = helpPage == "" ? analysis->helpFile() : analysis->fullHelpPath(helpPage);
	
	if(analysis == _analysis && pagePath == _pagePath && _visible)
	{
		setVisible(false);
		return;
	}
	else 
	{
		_analysis = analysis;
		emit analysisChanged();
		
		if((loadHelpContent(pagePath, false, renderFunc, contentMD) || loadHelpContent(pagePath, true, renderFunc, contentMD)) && renderFunc == "window.render")
		{
			//If we get here the file exists and it is a markdown file.
			setMarkdown(contentMD);
			setVisible(true);
			
			//Set pagepath here because setMarkdown erases it
			_pagePath = pagePath; //dont trigger generateJavascript through emit pagePathChanged!
		}
		else
			setPagePath(pagePath);
		
		return;
	}
}


void HelpModel::reloadPage()
{
	QString curPage = _pagePath;

	if(curPage != "" && curPage != "index")
	{
		setPagePath("index");
		setPagePath(curPage);
	}
}


void HelpModel::setThemeCss(QString themeName)
{
	runJavaScript("window.setTheme", themeName);
}

void HelpModel::setFont()
{
	QString fontFamily = PreferencesModel::prefs()->resultFont(true);
	runJavaScript("window.setFont", fontFamily);
}

///Temporary function for https://github.com/jasp-stats/INTERNAL-jasp/issues/1215 
bool HelpModel::pageExists(QString pagePath)
{
	 QString renderFunc, content;
	 
	 //We will simply have to use loadHelpContent to make sure we follow the same logic.
	 
	 return loadHelpContent(pagePath, false, renderFunc, content) || loadHelpContent(pagePath, true, renderFunc, content);
}

bool HelpModel::loadHelpContent(const QString & pagePath, bool ignorelanguage, QString &renderFunc, QString &content)
{

	QString _localname = ignorelanguage ? "" : LanguageModel::currentTranslationSuffix();
	bool found = false;

	renderFunc = "window.render";
	content = "";

	QFile fileMD, fileHTML;
	QFileInfo pathMd(pagePath + _localname + ".md");

	bool relative = pathMd.isRelative();

	if(relative) //This is probably a file in resources then
	{
		fileMD.setFileName(  AppDirs::help() + "/" + pagePath + _localname + ".md");
		fileHTML.setFileName(AppDirs::help() + "/" + pagePath + _localname + ".html");
	}
	else
	{
		//We got an absolute path, this probably means it comes from a (dynamic) module.

		fileMD.setFileName(	 pagePath + _localname + ".md");
		fileHTML.setFileName(pagePath + _localname + ".html");
	}

	if (fileHTML.exists())
	{
		fileHTML.open(QFile::ReadOnly);
		content = QString::fromUtf8(fileHTML.readAll());
		fileHTML.close();

		renderFunc = "window.renderHtml";
		found = true;

	}
	else if (fileMD.exists())
	{
		fileMD.open(QFile::ReadOnly);
		content = QString::fromUtf8(fileMD.readAll());
		fileMD.close();
		found = true;
	}

	return found;
}

void HelpModel::loadMarkdown(QString md)
{
	//Log::log() << "loadMarkdown got:\n" << md << std::endl;

	setVisible(true);
	runJavaScript("window.render", md);
}

void HelpModel::setAnalysis(Analysis *newAnalysis)
{
	if (_analysis == newAnalysis)
		return;
	_analysis = newAnalysis;
	emit analysisChanged();
}
