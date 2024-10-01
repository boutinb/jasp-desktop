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


#include "ribbonmodel.h"
#include "appdirs.h"
#include "messageforwarder.h"
#include "log.h"
#include "qquick/datasetview.h"
#include "mainwindow.h"

using namespace Modules;

RibbonModel * RibbonModel::_singleton = nullptr;

RibbonModel::RibbonModel() : QAbstractListModel(DynamicModules::dynMods())
{
	if(_singleton) throw std::runtime_error("RibbonModel can only be instantiated once!");
	_singleton = this;	

	_buttonNames = std::vector<stringvec>(2); //Analyses & Data

	connect(DynamicModules::dynMods(), &DynamicModules::dynamicModuleAdded,			this, &RibbonModel::addRibbonButtonModelFromDynamicModule	);
	connect(DynamicModules::dynMods(), &DynamicModules::dynamicModuleUninstalled,	this, &RibbonModel::removeDynamicRibbonButtonModel			);
	connect(DynamicModules::dynMods(), &DynamicModules::dynamicModuleReplaced,		this, &RibbonModel::dynamicModuleReplaced					);
	connect(DynamicModules::dynMods(), &DynamicModules::dynamicModuleChanged,		this, &RibbonModel::dynamicModuleChanged					);
	connect(PreferencesModel::prefs(), &PreferencesModel::languageCodeChanged,		this, &RibbonModel::refreshButtons							);
}

void RibbonModel::loadModules(std::vector<std::string> commonModulesToLoad, std::vector<std::string> extraModulesToLoad)
{
	DynamicModules::dynMods()->initializeInstalledModules();	
	DynamicModules::dynMods()->insertCommonModuleNames(std::set<std::string>(commonModulesToLoad.begin(), commonModulesToLoad.end()));

	addSpecialRibbonButtonsEarly();

	auto loadModulesFromBundledOrUserData = [&](bool common)
	{
		for(const std::string & moduleName : (common ? commonModulesToLoad : extraModulesToLoad))
		{
			if (!moduleName.empty())
			{
				if(DynamicModules::dynMods()->moduleIsInstalledByUser(moduleName)) //Only load bundled if the user did not install a newer/other version
					addRibbonButtonModelFromDynamicModule((*DynamicModules::dynMods())[moduleName]);
				else
				{
					try {
						std::string moduleLibrary = DynamicModules::bundledModuleLibraryPath(moduleName);
						
						//Check if the module pkg actually exists in the module library and otherwise show a friendly warning instead of confusing stuff about icons: https://github.com/jasp-stats/INTERNAL-jasp/issues/1287
						if(!QFileInfo::exists(tq(moduleLibrary + "/" + moduleName)))
						{
							Log::log() << "Module " << moduleName << " is missing!\nLooked at: '" << tq(moduleLibrary + "/" + moduleName) << " and it cant be loaded because it isnt there" << std::endl;
							
							MessageForwarder::showWarning(
								tr("Module missing"), 
								tr(	"It seems the bundled module %1 wasn't correctly installed, and thus cannot be loaded.\n"
									"If you installed this version of JASP via an official installer let the JASP team know.").arg(tq(moduleName)));
						}
						else
							DynamicModules::dynMods()->initializeModuleFromDir(moduleLibrary, true, common);
					} 
					catch (std::runtime_error & e) 
					{
						QString titleWarn = tr("Loading bundled module %1 failed").arg(tq(moduleName)),
								bodyWarn  = tr("Loading of the bundled module %1 failed with the following error:\n\n%2").arg(tq(moduleName)).arg(tq(e.what()));
						
						Log::log() << titleWarn << "\n" << bodyWarn << std::endl;
						
						MessageForwarder::showWarning(titleWarn, bodyWarn);
					}
				}
			}
		}
	};
	
	loadModulesFromBundledOrUserData(true);
	loadModulesFromBundledOrUserData(false);
	
	for(const std::string & modName : DynamicModules::dynMods()->moduleNames())
		if(!isModuleName(modName)) //Was it already added from commonModulesToLoad or extraModulesToLoad?
			addRibbonButtonModelFromDynamicModule((*DynamicModules::dynMods())[modName]);

	addSpecialRibbonButtonsLate();

	if(PreferencesModel::prefs()->modulesRemember())
	{
		QStringList enabledModules = PreferencesModel::prefs()->modulesRemembered();

		for(const QString & enabledModule : enabledModules)
		{
			std::string mod = enabledModule.toStdString();

			if(_buttonModelsByName.count(mod) > 0 && _buttonModelsByName[mod]->remember())
				_buttonModelsByName[mod]->setEnabled(true);
		}
	}
}

void RibbonModel::addRibbonButtonModelFromDynamicModule(Modules::DynamicModule * module)
{
	addRibbonButtonModel(new RibbonButton(this, module), size_t(RowType::Analyses));
}

void RibbonModel::addSpecialRibbonButtonsEarly()
{
	//_entriesInsert and _entriesDelete are destroyed by the menumodel destructor when the button gets destroyed.
	_entriesInsert = new AnalysisEntries(
	{
		new AnalysisEntry([&](){ emit this->dataInsertRowBefore(	-1);				},	"insert-row-before",			[&](){ return fq(tr("Insert row above"));},								true,		"menu-row-insert-before"),
		new AnalysisEntry([&](){ emit this->dataInsertRowAfter(		-1);				},	"insert-row-after",				[&](){ return fq(tr("Insert row below"));},								true,		"menu-row-insert-after"),
		new AnalysisEntry(),
		new AnalysisEntry([&](){ emit this->dataInsertColumnBefore(	-1,false,false);	},	"insert-column-before",			[&](){ return fq(tr("Insert column before"));},							true,		"menu-column-insert-before"),
		new AnalysisEntry([&](){ emit this->dataInsertColumnAfter(	-1,false,false);	},	"insert-column-after",			[&](){ return fq(tr("Insert column after"));},							true,		"menu-column-insert-after"),
		new AnalysisEntry(),
		new AnalysisEntry([&](){ emit this->dataInsertColumnBefore(	-1,true	,false);	},	"insert-c-column-before",		[&](){ return fq(tr("Insert constructor column before"));},				true,		"menu-column-insert-before"),
		new AnalysisEntry([&](){ emit this->dataInsertColumnAfter(	-1,true	,false);	},	"insert-c-column-after",		[&](){ return fq(tr("Insert constructor column after"));},				true,		"menu-column-insert-after"),
		new AnalysisEntry([&](){ emit this->dataInsertColumnBefore(	-1,true	,true);		},	"insert-r-column-before",		[&](){ return fq(tr("Insert R column before"));},						true,		"menu-column-insert-before"),
		new AnalysisEntry([&](){ emit this->dataInsertColumnAfter(	-1,true	,true);		},	"insert-r-column-after",		[&](){ return fq(tr("Insert R column after"));},						true,		"menu-column-insert-after"),
	});
	
	_entriesDelete = new AnalysisEntries(
	{
		new AnalysisEntry([&](){ emit this->dataRemoveColumn();			},					"delete-column",				[&](){ return fq(tr("Delete column"));},								true,		"menu-column-remove"),
		new AnalysisEntry([&](){ emit this->dataRemoveRow();			},					"delete-row",					[&](){ return fq(tr("Delete row"));},									true,		"menu-row-remove"),
		new AnalysisEntry([&](){ emit this->cellsClear();				},					"clear-cells",					[&](){ return fq(tr("Clear cells"));},									true,		"menu-cells-clear")
	});

	_entriesSynchOn = new AnalysisEntries(
	{
		new AnalysisEntry([&]() { MainWindow::singleton()->startDataEditorHandler(); },		"open-datafile",				[](){ return fq(tr("Open datafile with default spreadsheet editor"));},	true, ""),
		new AnalysisEntry([&]() { emit setDataSynchronisation(false);	},					"stop-externaledit",			[](){ return fq(tr("Turn external data synchronisation off"));},		true, "")
	});

	_analysesButton			= new RibbonButton(this, "Analyses",				[&](){ return fq(tr("Analyses"));},					"JASP_logo_green.svg",		false, [&](){ emit finishCurrentEdit(); emit showStatistics(); },	[&](){return tr("Switch JASP to analyses mode");},								true);
	_dataSwitchButton		= new RibbonButton(this, "Data",					[&](){ return fq(tr("Edit Data"));},				"data-button.svg",			false, [&](){ emit showData(); },									[&](){return tr("Switch JASP to data editing mode");},							false, false, false);
	_dataNewButton			= new RibbonButton(this, "Data-New",				[&](){ return fq(tr("New Data"));},					"data-button-new.svg",		false, [&](){ emit showNewData();	 },								[&](){return tr("Open a workspace without data");},								true, false, false);
	_dataResizeButton		= new RibbonButton(this, "Data-Resize",				[&](){ return fq(tr("Resize Data"));},				"data-button-resize.svg",	false, [&](){ emit resizeData(); },									[&](){return tr("Resize your dataset");},										false);
	_insertButton			= new RibbonButton(this, "Data-Insert",				[&](){ return fq(tr("Insert"));},					"data-button-insert.svg",	_entriesInsert,														[&](){return tr("Insert empty columns or rows");});
	_removeButton			= new RibbonButton(this, "Data-Remove",				[&](){ return fq(tr("Remove"));},					"data-button-erase.svg",	_entriesDelete,														[&](){return tr("Remove columns or rows");});
	_synchroniseOnButton	= new RibbonButton(this, "Data-Synch-On",			[&](){ return fq(tr("Synchronisation"));},			"data-button-sync-off.svg",	true, [&](){ emit setDataSynchronisation(true); },					[&](){return tr("Turn external data synchronisation on");},						false);
	_synchroniseOffButton	= new RibbonButton(this, "Data-Synch-Off",			[&](){ return fq(tr("Synchronisation"));},			"data-button-sync-on.svg",	_entriesSynchOn,													[&](){return tr("Turn external data synchronisation off");},					true);
	_undoButton				= new RibbonButton(this, "Data-Undo",				[&](){ return fq(tr("Undo"));},						"menu-undo.svg",			true,  [&](){ emit dataUndo(); },									[&](){return tr("Undo changes, %1+Z").arg(getShortCutKey());},					true, false, false);
	_redoButton				= new RibbonButton(this, "Data-Redo",				[&](){ return fq(tr("Redo"));},						"menu-redo.svg",			true,  [&](){ emit dataRedo(); },									[&](){return tr("Redo changes, %1+shift+Z or %1+Y").arg(getShortCutKey());},	true, false, false);

	_dataNewButton->setActive(true);
	connect(this, &RibbonModel::dataLoadedChanged,							_dataSwitchButton,	[=](bool loaded)			{ _dataSwitchButton	->setEnabled(loaded);				});
	connect(this, &RibbonModel::dataLoadedChanged,							_dataNewButton,		[=](bool loaded)			{ _dataNewButton	->setEnabled(!loaded);				});
	connect(MainWindow::singleton(), &MainWindow::dataAvailableChanged,		_dataSwitchButton,	[=](bool dataAvailable)		{ _dataSwitchButton	->setActive(dataAvailable);			});

	connect(this, &RibbonModel::dataLoadedChanged,		_insertButton,			&RibbonButton::setEnabled);
	connect(this, &RibbonModel::dataLoadedChanged,		_removeButton,			&RibbonButton::setEnabled);
	connect(this, &RibbonModel::dataLoadedChanged,		_dataResizeButton,		&RibbonButton::setEnabled);
	connect(this, &RibbonModel::synchronisationChanged, _synchroniseOnButton,	[=](bool synching){ _synchroniseOnButton->setEnabled(!synching); });
	connect(this, &RibbonModel::synchronisationChanged, _synchroniseOffButton,	[=](bool synching){ _synchroniseOffButton->setEnabled(synching); });

	{
		DataSetView * view = DataSetView::mainDataViewer();

		auto setUnAndRedoButtonLambda = [&,view]()
		{
			_undoButton->setActive(!view->undoText().isEmpty());
			_redoButton->setActive(!view->redoText().isEmpty());

			_undoButton->setToolTipF([&,view](){return tr("Undo %2 (%1+Z)")					.arg(getShortCutKey()).arg(view->undoText());});
			_redoButton->setToolTipF([&,view](){return tr("Redo %2 (%1+shift+Z or %1+Y)")	.arg(getShortCutKey()).arg(view->redoText());});
		};



		connect(view, &DataSetView::undoChanged,			setUnAndRedoButtonLambda);
		connect(this, &RibbonModel::dataLoadedChanged,		setUnAndRedoButtonLambda);
	}


	addRibbonButtonModel(_dataSwitchButton,			size_t(RowType::Analyses));
	addRibbonButtonModel(_dataNewButton,			size_t(RowType::Analyses));
	addRibbonButtonModel(new RibbonButton(this),	size_t(RowType::Analyses));

	addRibbonButtonModel(_analysesButton,			size_t(RowType::Data));
	addRibbonButtonModel(new RibbonButton(this),	size_t(RowType::Data));
	addRibbonButtonModel(_synchroniseOnButton,		size_t(RowType::Data));
	addRibbonButtonModel(_synchroniseOffButton,		size_t(RowType::Data));
	addRibbonButtonModel(_dataResizeButton,			size_t(RowType::Data));
	addRibbonButtonModel(_insertButton,				size_t(RowType::Data));
	addRibbonButtonModel(_removeButton,				size_t(RowType::Data));
	addRibbonButtonModel(_undoButton,				size_t(RowType::Data));
	addRibbonButtonModel(_redoButton,				size_t(RowType::Data));
}

void RibbonModel::addSpecialRibbonButtonsLate()
{
	addRibbonButtonModel(new RibbonButton(this, "R", [&](){return fq(tr("R console")); }, "Rlogo.svg", false, [&](){ emit showRCommander(); }, [&](){ return tr("Execute R code in a console");}, false, true), size_t(RowType::Analyses));
}

void RibbonModel::setDataMode(bool data)
{
	if(data)	showData();
	else		showStatistics();
}

void RibbonModel::dynamicModuleChanged(Modules::DynamicModule * dynMod)
{
	Log::log() << "void RibbonModel::dynamicModuleChanged(" << dynMod->toString() << ")" << std::endl;

	for(const auto & nameButton : _buttonModelsByName)
		if(nameButton.second->dynamicModule() == dynMod)
			nameButton.second->reloadDynamicModule(dynMod);
}

void RibbonModel::addRibbonButtonModel(RibbonButton* model, size_t row)
{
	if(isModuleName(model->name()))
		removeRibbonButtonModel(model->name());

	if(_currentRow == row)
		beginInsertRows(QModelIndex(), rowCount(), rowCount());

	_buttonNames[row].push_back(model->name());
	_buttonModelsByName[model->name()] = model;

	if(_currentRow == row)
		endInsertRows();

	connect(model, &RibbonButton::iChanged,				this, &RibbonModel::ribbonButtonModelChanged);
}

void RibbonModel::dynamicModuleReplaced(Modules::DynamicModule * oldModule, Modules::DynamicModule * module)
{
	for(const auto & nameButton : _buttonModelsByName)
		if(nameButton.second->dynamicModule() == oldModule || nameButton.first == oldModule->name())
			nameButton.second->reloadDynamicModule(module);
}

QVariant RibbonModel::data(const QModelIndex &index, int role) const
{
	if (index.row() >= rowCount())
		return QVariant();

	size_t row = size_t(index.row());

	switch(role)
	{
	case DisplayRole:													//Displayed in + panel
	case ModuleTitleRole:	return ribbonButtonModelAt(row)->titleQ();	//Displayed in ribbon. Could obviously be merged with the above
	case RibbonRole:		return QVariant::fromValue(ribbonButtonModelAt(row));
	case EnabledRole:		return ribbonButtonModelAt(row)->enabled();
	case ActiveRole:		return ribbonButtonModelAt(row)->active();
	case CommonRole:		return ribbonButtonModelAt(row)->isCommon();
	case ModuleNameRole:	return ribbonButtonModelAt(row)->nameQ();
	case ModuleRole:		return QVariant::fromValue(ribbonButtonModelAt(row)->dynamicModule());
	case BundledRole:		return ribbonButtonModelAt(row)->isBundled();
	case VersionRole:		return ribbonButtonModelAt(row)->version();
	case SpecialRole:		return ribbonButtonModelAt(row)->isSpecial();
	case ClusterRole:		//To Do!?
	default:				return QVariant();
	}
}


QHash<int, QByteArray> RibbonModel::roleNames() const
{
	static const auto roles = QHash<int, QByteArray>{
		{ ClusterRole,		"clusterMenu"		},
		{ DisplayRole,		"displayText"		},
		{ RibbonRole,		"ribbonButton"		},
		{ EnabledRole,		"ribbonEnabled"		},
		{ CommonRole,		"isCommon"			},
		{ ModuleNameRole,	"moduleName"		},
		{ ModuleTitleRole,	"moduleTitle"		},
		{ ModuleRole,		"dynamicModule"		},
		{ ActiveRole,		"active"			},
		{ BundledRole,		"isBundled"			},
		{ VersionRole,		"moduleVersion"		},
		{ SpecialRole,		"isSpecial"			}
	};

	return roles;
}

RibbonButton* RibbonModel::ribbonButtonModel(std::string name) const
{
	if(_buttonModelsByName.count(name) > 0)
		return _buttonModelsByName.at(name);

	return nullptr;
}

void RibbonModel::removeRibbonButtonModel(std::string moduleName)
{
	if(!isModuleName(moduleName))
		return;

	for(size_t row=0; row<size_t(RowType::Data); row++)
	{
		int indexRemoved = -1;

		for(int i=_buttonNames.size() - 1; i >= 0; i--)
			if(_buttonNames[row][i] == moduleName)
			{
				indexRemoved = i;
				break;
			}

		if(indexRemoved != -1)
		{
			if(row == _currentRow)
				beginRemoveRows(QModelIndex(), indexRemoved, indexRemoved);

			delete _buttonModelsByName[moduleName];
			_buttonModelsByName.erase(moduleName);

			_buttonNames[row].erase(_buttonNames[row].begin() + indexRemoved);

			if(row == _currentRow)
				endRemoveRows();
		}
	}
}

void RibbonModel::analysisClicked(QString analysisFunction, QString analysisQML, QString analysisTitle, QString module)
{
	RibbonButton * button = ribbonButtonModel(fq(module));
	
	if(button->isSpecial())		button->runSpecial(analysisFunction);
	else						emit analysisClickedSignal(analysisFunction, analysisQML, analysisTitle, module);
}

void RibbonModel::setCurrentRow(int currentRow)
{
	size_t cur = currentRow;
	if (_currentRow == cur)
		return;

	beginResetModel();
	_currentRow = cur;
	endResetModel();

	emit currentRowChanged();
	emit dataModeChanged(dataMode());
}

void RibbonModel::setHighlightedModuleIndex(int highlightedModuleIndex)
{
	if (_highlightedModuleIndex == highlightedModuleIndex)
		return;

	_highlightedModuleIndex = highlightedModuleIndex;
	emit highlightedModuleIndexChanged(_highlightedModuleIndex);
}

void RibbonModel::setModuleEnabled(int ribbonButtonModelIndex, bool enabled)
{
	if(ribbonButtonModelIndex < 0)
		return;

	RibbonButton * ribbonButtonModel = ribbonButtonModelAt(size_t(ribbonButtonModelIndex));

	if(ribbonButtonModel->enabled() != enabled)
	{
		ribbonButtonModel->setEnabled(enabled);
		emit dataChanged(index(ribbonButtonModelIndex), index(ribbonButtonModelIndex));
		emit invalidateFilterModel();
	}
}

QStringList RibbonModel::getModulesEnabled() const
{
	QStringList list;
	
	for(auto & nameButton : _buttonModelsByName)
		if(nameButton.second->enabled())
			list.append(nameButton.second->nameQ());
	
	return list;
}


Modules::AnalysisEntry *RibbonModel::getAnalysis(std::string moduleName, const std::string & analysisName)
{
	if(moduleName == "Common")
		moduleName = getModuleNameFromAnalysisName(analysisName);

	RibbonButton			* ribbonButton	= ribbonButtonModel(moduleName);

	if (ribbonButton)
		return ribbonButton->getEntry(analysisName);
	else
	{
		std::string strippedModuleName = stringUtils::stripNonAlphaNum(moduleName);

		if(strippedModuleName != moduleName)
			return getAnalysis(strippedModuleName, analysisName);
		else if(strippedModuleName.size() < 4 || strippedModuleName.substr(0, 4) != "jasp") //Cause we renamed everything to jasp*
			return getAnalysis("jasp" + strippedModuleName, analysisName);
	}

	return nullptr;
}

std::string RibbonModel::getModuleNameFromAnalysisName(const std::string & analysisName)
{
	// This function is needed for old JASP file: they still have a reference to the common module that does not exist anymore.
	for (const std::string & moduleName : _buttonNames[size_t(RowType::Analyses)])
		for (const std::string & name : _buttonModelsByName[moduleName]->getAllEntries())
			if (name == analysisName)
				return moduleName;

	return "Common";
}

void RibbonModel::toggleModuleEnabled(int ribbonButtonModelIndex)
{
	if(ribbonButtonModelIndex < 0)
		return;

	RibbonButton * ribbonButtonModel = ribbonButtonModelAt(size_t(ribbonButtonModelIndex));

	ribbonButtonModel->setEnabled(!ribbonButtonModel->enabled());

	emit dataChanged(index(ribbonButtonModelIndex), index(ribbonButtonModelIndex));
}

int RibbonModel::ribbonButtonModelIndex(RibbonButton * model)	const
{
	for(auto & keyval : _buttonModelsByName)
		if(keyval.second == model)
			for(size_t i=0; i<_buttonNames[_currentRow].size(); i++)
				if(_buttonNames[_currentRow][i] == keyval.first)
					return int(i);
	return -1;
}


void RibbonModel::ribbonButtonModelChanged(RibbonButton* model)
{
	int row = ribbonButtonModelIndex(model);
	if(row > -1)
		emit dataChanged(index(row), index(row));
}

void RibbonModel::refreshButtons()
{
	beginResetModel();
	endResetModel();	
}
