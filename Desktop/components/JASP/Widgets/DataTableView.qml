import QtQuick
import QtQuick.Controls
import JASP.Controls		as JaspControls
import QtQml.Models


FocusScope
{
	id: __myRoot

	signal doubleClicked()

	Rectangle
	{
		color:			jaspTheme.white
		anchors.fill:	parent
		z:				-1
		border.width:	1
		border.color:	jaspTheme.uiBorder

		JASPDataView
		{
			focus:					__myRoot.focus

			id:						dataTableView
			anchors.top:			parent.top
			anchors.left:			parent.left
			anchors.right:			parent.right
			anchors.bottom:			dataStatusBar.top

			itemHorizontalPadding:	8 * jaspTheme.uiScale
			itemVerticalPadding:	8 * jaspTheme.uiScale

			model:					dataSetModel
			cacheItems:				!ribbonModel.dataModeb

			doubleClickWorkaround:	!ribbonModel.dataMode
			//flickableInteractive:	!ribbonModel.dataMode
			onDoubleClicked:		__myRoot.doubleClicked()

			function showCopyPasteMenu(fromItem, globalPos, indexClicked)
			{
				console.log("showCopyPasteMenu!")

				view.contextMenuClickedAtIndex(indexClicked);

				var ctrlCmd = MACOS ? qsTr("Cmd") : qsTr("Ctrl");

				var copyPasteMenuModel =
				[
					{ text: qsTr("Select All"),									func: function() { dataTableView.view.selectAll() },		icon: "menu-select-all" },

					{ text: "---" },

					{ text: qsTr("Cut             (%1+X)").arg(ctrlCmd),		func: function() { dataTableView.view.cut(false) },			icon: "menu-data-cut"	},
					{ text: qsTr("Copy            (%1+C)").arg(ctrlCmd),		func: function() { dataTableView.view.copy(false) },		icon: "menu-data-copy"	},
					{ text: qsTr("Paste           (%1+V)").arg(ctrlCmd),		func: function() { dataTableView.view.paste(false) },		icon: "menu-data-paste" },

					{ text: "---" },

					{ text: qsTr("Header cut      (%1+Shift+X)").arg(ctrlCmd),	func: function() { dataTableView.view.cut(true) }   ,       icon: "menu-data-paste" },
					{ text: qsTr("Header copy     (%1+Shift+C)").arg(ctrlCmd),	func: function() { dataTableView.view.copy(true) }  ,       icon: "menu-data-copy"	},
					{ text: qsTr("Header paste    (%1+Shift+V)").arg(ctrlCmd),	func: function() { dataTableView.view.paste(true) } ,       icon: "menu-data-cut"	},

					{ text: "---" },

					{ text: qsTr("Select column"),								func: function() { dataTableView.view.columnSelect(			indexClicked.column) }, icon: "menu-column-select"			},
					{ text: qsTr("Insert column before"),						func: function() { dataTableView.view.columnInsertBefore(	indexClicked.column) }, icon: "menu-column-insert-before"	},
					{ text: qsTr("Insert column after"),						func: function() { dataTableView.view.columnInsertAfter(	indexClicked.column) }, icon: "menu-column-insert-after"	},
					{ text: qsTr("Delete column"),								func: function() { dataTableView.view.columnsDelete() },							icon: "menu-column-remove"			},

					{ text: "---" },

					{ text: qsTr("Select row"),									func: function() { dataTableView.view.rowSelect(			indexClicked.row) },	icon: "menu-row-select"				},
					{ text: qsTr("Insert row before"),							func: function() { dataTableView.view.rowInsertBefore(		indexClicked.row) },	icon: "menu-row-insert-before"			},
					{ text: qsTr("Insert row after"),							func: function() { dataTableView.view.rowInsertAfter(		indexClicked.row) },	icon: "menu-row-insert-after"			},
					{ text: qsTr("Delete row"),									func: function() { dataTableView.view.rowsDelete(); },								icon: "menu-row-remove"				}

				]

				var copyPasteMenuText = []
				var copyPasteMenuFunctions = []
				var copyPasteMenuIcons = []

				for (var i = 0; i < copyPasteMenuModel.length; i++)
				{
					var menu = copyPasteMenuModel[i]
					copyPasteMenuText.push(menu["text"])
					if (menu.hasOwnProperty("func"))
						copyPasteMenuFunctions.push(menu["func"])
					else
						copyPasteMenuFunctions.push(function() {})
					if (menu.hasOwnProperty("icon"))
						copyPasteMenuIcons.push(jaspTheme.iconPath + menu["icon"] + ".svg")
					else
						copyPasteMenuIcons.push("")
				}

				var props = {
					"hasIcons": true,
					"model":		copyPasteMenuText,
					"functionCall": function (index)
					{
						copyPasteMenuFunctions[index]();

						customMenu.hide()
					},
					"icons": copyPasteMenuIcons
				};

				//customMenu.scrollOri.x	= __JASPDataViewRoot.contentX;
				//customMenu.scrollOri.y	= 0;

				var fromItemPos = fromItem.mapFromGlobal(globalPos.x, globalPos.y)

				customMenu.show(fromItem, props, fromItemPos.x, fromItemPos.y);

				//customMenu.menuScroll.x	= Qt.binding(function() { return -1 * (__JASPDataViewRoot.contentX - customMenu.scrollOri.x); });
				//customMenu.menuScroll.y	= 0;
				//customMenu.menuMinIsMin	= true
				//customMenu.menuMaxPos.x	= __JASPDataViewRoot.width + __JASPDataViewRoot.x
			}


			editDelegate:
				TextInput
				{
					id:						editItem
					text:					itemText
					color:					itemActive ? jaspTheme.textEnabled : jaspTheme.textDisabled
					font:					jaspTheme.font
					verticalAlignment:		Text.AlignVCenter
					onEditingFinished:		dataTableView.view.commitEdit(index, text);
					z:						10
					readOnly:				!itemEditable

					Component.onCompleted:	{ focusTimer.start(); }
					Timer
					{
						id:					focusTimer
						interval:			10
						repeat:				false
						onTriggered:
						{
							editItem.forceActiveFocus()
							dataTableView.moveItemIntoView(editItem);
						}
					}

					Keys.onPressed: (event) =>
					{
						var rowI			= rowIndex
						var colI			= columnIndex
						var controlPressed	= Boolean(event.modifiers & Qt.ControlModifier);
						var shiftPressed	= Boolean(event.modifiers & Qt.ShiftModifier  );
						var arrowPressed	= false;
						var arrowIndex;

						switch(event.key)
						{
						case Qt.Key_C:
							if(controlPressed)
							{
								theView.copy(shiftPressed);
								event.accepted = true;
							}
							break;

						case Qt.Key_X:
							if(controlPressed)
							{
								theView.cut(shiftPressed);
								event.accepted = true;
							}
							break;

						case Qt.Key_V:
							if(controlPressed)
							{
								theView.paste(shiftPressed);
								event.accepted = true;
							}
							break;

						case Qt.Key_A:
							if(controlPressed)
							{
								theView.selectAll();
								event.accepted = true;
							}
							break;

						case Qt.Key_Home:	mainWindowRoot.changeFocusToFileMenu(); break;

						case Qt.Key_Up:		if(rowI > 0)										{ arrowPressed = true; arrowIndex   = dataTableView.model.index(rowI - 1,	colI);		} break;
						case Qt.Key_Down:	if(rowI	< dataTableView.model.rowCount()    - 1)	{ arrowPressed = true; arrowIndex   = dataTableView.model.index(rowI + 1,	colI);		} break;
						case Qt.Key_Left:	if(colI	> 0 && editItem.cursorPosition <= 0)		{ arrowPressed = true; arrowIndex   = dataTableView.model.index(rowI,		colI - 1);	} break;
						case Qt.Key_Right:	if(colI	< dataTableView.model.columnCount() - 1 && editItem.cursorPosition >= editItem.text.length)	{ arrowPressed = true; arrowIndex = dataTableView.model.index(rowI, colI + 1);} break;

						case Qt.Key_Backtab: if(colI > 0)									{ arrowPressed = true; arrowIndex = dataTableView.model.index(rowI, colI - 1);	shiftPressed = false; } break;
						case Qt.Key_Tab:	 if(colI < dataTableView.model.columnCount())	{ arrowPressed = true; arrowIndex = dataTableView.model.index(rowI, colI + 1);	} break;


						}

						if(arrowPressed)
						{
							if(!shiftPressed)
								dataTableView.view.selectionStart	= arrowIndex;
							else
							{
								dataTableView.view.selectionEnd  = arrowIndex;
								dataTableView.view.edit(arrowIndex);
							}

							event.accepted = true;
						}

						if(!itemEditable)
							event.accepted = true; //textinput steals the focus otherwise and we cant move with arrows after pressing anything

					}


					Rectangle
					{
						id:					highlighter
						color:				jaspTheme.itemHighlight
						z:					-1
						visible:			ribbonModel.dataMode
						anchors
						{
							fill:			 parent
							topMargin:		-dataTableView.itemVerticalPadding
							leftMargin:		-dataTableView.itemHorizontalPadding
							rightMargin:	-dataTableView.itemHorizontalPadding
							bottomMargin:	-dataTableView.itemVerticalPadding
						}

						MouseArea
						{
							z:					1234
							anchors.fill:		parent
							acceptedButtons:	Qt.RightButton

							onPressed:
								if(mouse.buttons & Qt.RightButton)
									dataTableView.showCopyPasteMenu(editItem, mapToGlobal(mouse.x, mouse.y), dataSetModel.index(rowIndex, columnIndex));
						}
					}
				}

			itemDelegate:
				Text
				{
					text:				itemText
					textFormat:			Text.RichText
					color:				itemActive ? jaspTheme.textEnabled : jaspTheme.textDisabled
					font:				jaspTheme.font
					verticalAlignment:	Text.AlignVCenter
					
					MouseArea
					{
						z:					1234
						hoverEnabled:		true
						anchors.fill:		itemHighlight
						acceptedButtons:	Qt.LeftButton | Qt.RightButton
						
						onPressed:	(mouse) => 
						{		
							if(ribbonModel.dataMode)
							{
								var shiftPressed = Boolean(mouse.modifiers & Qt.ShiftModifier);

								if(Boolean(mouse.buttons & Qt.RightButton))
								{
									forceActiveFocus();
									dataTableView.showCopyPasteMenu(itemHighlight, mapToGlobal(mouse.x, mouse.y), dataSetModel.index(rowIndex, columnIndex));
								}
								else
								{
									if(!shiftPressed)	dataTableView.view.selectionStart   = dataTableView.view.model.index(rowIndex, columnIndex);
									else				dataTableView.view.selectionEnd		= dataTableView.view.model.index(rowIndex, columnIndex);
								}
							}
						}
											
						onPositionChanged:	(mouse) =>
						{
							if(ribbonModel.dataMode && Boolean(mouse.modifiers & Qt.ShiftModifier))
							{
								var idx = dataTableView.view.model.index(rowIndex, columnIndex)
								dataTableView.view.pollSelectScroll(idx)
								dataTableView.view.selectionEnd = idx
							}
						}

					}
					
					Rectangle
					{
						id:				itemHighlight
						visible:		ribbonModel.dataMode && (dataTableView.selection.hasSelection, dataTableView.selection.isSelected(dataTableView.view.model.index(rowIndex, columnIndex)))
						
						color:			jaspTheme.itemHighlight
						opacity:		1.0
						z:				-1

						anchors
						{
							fill:			 parent
							topMargin:		-dataTableView.itemVerticalPadding
							leftMargin:		-dataTableView.itemHorizontalPadding
							rightMargin:	-dataTableView.itemHorizontalPadding
							bottomMargin:	-dataTableView.itemVerticalPadding
						}
					}
				}

			leftTopCornerItem:
				JaspControls.RectangularButton
				{
					id:				filterToggleButton
					width:			dataTableView.rowNumberWidth
					toolTip:		filterWindow.opened ? qsTr("Hide filter") : qsTr("Show filter")
					iconSource:		jaspTheme.iconPath + "filter.png"
					onClicked:		filterWindow.toggle()
					border.width:	0
				}

			/*extraColumnItem:
				JaspControls.RectangularButton
				{
					id:				addColumnButton
					width:			height
					toolTip:		qsTr("Add computed column")
					iconSource:		jaspTheme.iconPath + "/addition-sign.svg"
					onClicked:		createComputeDialog.open()
					border.width:	0
				}*/

			rowNumberDelegate:
				Rectangle
				{
					//gradient: Gradient{	GradientStop { position: 0.0;	color: "#EEEEEE" }	GradientStop { position: 0.75;	color: "#EEEEEE" }
					//					GradientStop { position: 0.77;	color: "#DDDDDD" }	GradientStop { position: 1.0;	color: "#DDDDDD" }	}
					color:	jaspTheme.uiBackground
					Text {
						text:				rowNumber
						font:				jaspTheme.font
						anchors.centerIn:	parent
						color:				jaspTheme.textEnabled
					}

				}

			columnHeaderDelegate: Rectangle
			{
				id:		headerRoot
				color:	jaspTheme.uiBackground

							property real	iconTextPadding:	10
				readonly	property int	__iconDim:			baseBlockDim * preferencesModel.uiScale

				Image
				{
					id:						colIcon
					anchors.verticalCenter: parent.verticalCenter
					anchors.left:			parent.left
					anchors.margins:		4

					source: jaspTheme.iconPath + dataSetModel.getColumnTypesWithIcons()[columnType]
					width:	headerRoot.__iconDim
					height: headerRoot.__iconDim

					sourceSize {	width:	width * 2
									height:	height * 2 }


					function setColumnType(newColumnType)
					{
						dataSetModel.setColumnTypeFromQML(columnIndex, newColumnType)

						if(labelModel.chosenColumn === columnIndex && columnType === columnTypeScale)
							labelModel.visible = false;
					}


					MouseArea
					{
						anchors.fill:		parent
						onClicked:
						{
							var functionCall      = function (index)
							{
								// FIXME:
								var columnType = [columnTypeScale, columnTypeOrdinal, columnTypeNominal][index];

								if (columnType !== undefined)
									colIcon.setColumnType(columnType);

								customMenu.hide()
							}

							var props = {
								"model":		columnTypesModel,
								"functionCall": functionCall
							};

							customMenu.scrollOri.x	= dataTableView.contentX;
							customMenu.scrollOri.y	= 0;

							customMenu.toggle(dataTableView, props, headerRoot.x - contentX, headerRoot.y + headerRoot.height - dataTableView.contentY);

							customMenu.menuScroll.x	= Qt.binding(function() { return -1 * (dataTableView.contentX - customMenu.scrollOri.x); });
							customMenu.menuScroll.y	= 0;
							customMenu.menuMinIsMin	= true
							customMenu.menuMaxPos.x	= dataTableView.width + dataTableView.x
						}

						hoverEnabled:		true
						ToolTip.visible:	containsMouse
						ToolTip.text:		qsTr("Click here to change column type")
						ToolTip.timeout:	3000
						ToolTip.delay:		500
						cursorShape:		Qt.PointingHandCursor
					}
				}

				Image
				{
					id:			colIsComputed

					width:		visible ? headerRoot.__iconDim : 0
					height:		headerRoot.__iconDim
					visible:	columnIsComputed

					anchors.left:			colIcon.right
					anchors.verticalCenter: parent.verticalCenter
					anchors.margins:		visible ? 1 : 0

					source:				jaspTheme.iconPath + "/computed.png"
					sourceSize {	width:	headerRoot.__iconDim * 2
									height:	headerRoot.__iconDim * 2 }

					MouseArea
					{
						anchors.fill:	parent
						onClicked:		computeColumnWindow.open(dataSetModel.headerData(columnIndex, Qt.Horizontal))

						hoverEnabled:		true
						ToolTip.visible:	containsMouse
						ToolTip.text:		qsTr("Click here to change the columns formulas")
						ToolTip.timeout:	3000
						ToolTip.delay:		500

						cursorShape:		Qt.PointingHandCursor

					}
				}

				Text
				{
					id:				headerTextItem

					text:			headerText
					font:			jaspTheme.font
					color:			jaspTheme.textEnabled
					textFormat:		Text.RichText

					horizontalAlignment:		Text.AlignHCenter

					anchors.horizontalCenter:	headerRoot.horizontalCenter
					anchors.verticalCenter:		headerRoot.verticalCenter
				}

				LoadingIndicator
				{
					id:			colIsInvalidated


					width:		columnIsInvalidated ? headerRoot.__iconDim : 0
					visible:	columnIsInvalidated

					anchors.right:			colFilterOn.left
					anchors.verticalCenter:	parent.verticalCenter
					anchors.margins:		visible ? 1 : 0
				}

				Image
				{
					id:						colFilterOn

					width:					columnIsFiltered ? headerRoot.__iconDim : 0
					height:					headerRoot.__iconDim

					source:					jaspTheme.iconPath + "filter.png"
					sourceSize {	width:	headerRoot.__iconDim * 2
									height:	headerRoot.__iconDim * 2 }

					anchors.right:			colHasError.left
					anchors.margins:		columnIsFiltered ? 1 : 0
					anchors.verticalCenter:	parent.verticalCenter
				}

				Image
				{
					id:			colHasError

					width:		columnError.length > 0 ? headerRoot.__iconDim : 0
					height:		headerRoot.__iconDim
					visible:	columnError.length > 0 // && !columnIsInvalidated

					source:					jaspTheme.iconPath + "/error.png"
					sourceSize {	width:	headerRoot.__iconDim * 2
									height:	headerRoot.__iconDim * 2 }

					anchors.right:			parent.right
					anchors.verticalCenter:	parent.verticalCenter
					anchors.margins:		visible ? 1 : 0

					MouseArea
					{
						anchors.fill:		parent
						onClicked:			computeColumnWindow.open(dataSetModel.headerData(columnIndex, Qt.Horizontal))

						hoverEnabled:		true
						ToolTip.visible:	containsMouse && columnError.length > 0
						ToolTip.text:		columnError
						ToolTip.timeout:	3000
						ToolTip.delay:		500
						cursorShape:		Qt.PointingHandCursor

					}

				}

				MouseArea
				{
					anchors.left:	colIsComputed.right
					anchors.top:	parent.top
					anchors.bottom: parent.bottom
					anchors.right:	colHasError.left

					onClicked:
						if(columnIndex >= 0)
						{

							if(columnType  !== columnTypeScale)
							{
								var changedIndex		= labelModel.chosenColumn	!== columnIndex
								labelModel.chosenColumn	= columnIndex;
								labelModel.visible		= changedIndex ? true : !labelModel.visible;
							}
							else
								dataSetModel.renameColumnDialog(columnIndex);

							if(dataSetModel.columnUsedInEasyFilter(columnIndex))
							{
								filterWindow.showEasyFilter = true
								filterWindow.open()
							}
						}

					hoverEnabled:		true
					ToolTip.visible:	containsMouse && (columnType !== columnTypeScale)
					ToolTip.text:		qsTr("Click here to change labels") + (columnIsFiltered ? qsTr(" or inspect filter") : "" )
					ToolTip.timeout:	3000
					ToolTip.delay:		500
					cursorShape:		(columnType !== columnTypeScale) || dataSetModel.columnUsedInEasyFilter(columnIndex) ? Qt.PointingHandCursor : Qt.ArrowCursor
				}
			}
		}

		Rectangle
		{
			id:				dataStatusBar
			objectName:		"dataStatusBar"
			anchors.left:	parent.left
			anchors.right:	parent.right
			anchors.bottom: parent.bottom

			color:			jaspTheme.grayMuchLighter
			border.color:	jaspTheme.grayLighter
			border.width:	1

			height:			datafiltertatusText.text.length > 0 ? datafiltertatusText.contentHeight + (16 * preferencesModel.uiScale) : 0

			Text
			{
				id:						datafiltertatusText
				text:					filterModel.statusBarText
				font:					jaspTheme.font
				color:					jaspTheme.textEnabled
				anchors.left:			parent.left
				anchors.verticalCenter:	parent.verticalCenter
				anchors.leftMargin:		8 * preferencesModel.uiScale
			}
		}
	}
}
