import QtQuick
import JASP.Widgets
import JASP.Controls
import QtQuick.Controls as QC

WavyWindow
{
	id:				aboutWindow

	visible:				aboutModel.visible
	onVisibleChanged:		aboutModel.visible = visible
	onCloseModel:			{ aboutModel.visible = false }

	title:					qsTr("About JASP")


	property int labelwidth:	10 + Math.max(	jaspVersionLabel.implicitWidth,
												buildDateLabel	.implicitWidth,
												sourceLabel		.implicitWidth,
												downloadLabel	.implicitWidth,
												citationLabel	.implicitWidth)

	Text { text: aboutModel.copyrightMessage; color: jaspTheme.textEnabled	}

	Row
	{
		Label { text: "<b>" + qsTr("Version:") + "</b>";		color: labelcolor; width: aboutWindow.labelwidth; id: jaspVersionLabel }
		Text	 { text: aboutModel.version;	color: jaspTheme.textEnabled											}
	}

	Row
	{
		Label { text: "<b>" + qsTr("Built on:") + "</b>";		color: labelcolor; width: aboutWindow.labelwidth; id: buildDateLabel	}
		Text	 { text: aboutModel.buildDate;	color: jaspTheme.textEnabled											}
	}

	Row
	{
		Label { text: "<b>" + qsTr("Source:") + "</b>";		color: labelcolor; width: aboutWindow.labelwidth; id: sourceLabel		}
		Text
		{
			text:		"<u>" + qsTr("Access the sources here") + "</u>"
			color:		jaspTheme.blue

			MouseArea
			{
				id:				mouseAreaSources
				anchors.fill:	parent
				onClicked:		Qt.openUrlExternally(aboutModel.commitUrl)
				cursorShape:	Qt.PointingHandCursor
			}
		}
	}

	Row
	{
		Label { text: "<b>" + qsTr("Download:") + "</b>";		color:	labelcolor; width: aboutWindow.labelwidth; id:	downloadLabel	}
		Text
		{
			text:			"<u>" + aboutModel.downloadUrl + "</u>"
			color:			jaspTheme.blue

			MouseArea
			{
				id:				mouseAreaDownload
				anchors.fill:	parent
				onClicked:		Qt.openUrlExternally(aboutModel.downloadUrl)
				cursorShape:	Qt.PointingHandCursor

			}
		}
	}

	Row
	{
		width:				parent.width

		Label
		{
			id:				citationLabel
			width:			aboutWindow.labelwidth
			text:			"<b>" + qsTr("Citation:") + "</b>"
			color:			labelcolor
		}

		Column
		{
			width:			Math.min(parent.width - aboutWindow.labelwidth, 450 * jaspTheme.uiScale)

			QC.TextArea
			{
				id:				citationText
				textFormat:		Text.StyledText
				text:			aboutModel.citation
				color:			jaspTheme.textEnabled
				leftPadding:	0
				topPadding:		0
				bottomPadding:	0
				wrapMode:		TextEdit.Wrap
				font:			jaspTheme.fontCode
				width:			parent.width

				selectByMouse:	true
				readOnly:		true

				onPressed:		(event)=>{ if (event.button === Qt.RightButton)	contextMenu.popup() }

				QC.Menu
				{
					id:		contextMenu
					width:	120

					QC.Action { text: qsTr("Select All");		onTriggered: citationText.selectAll();	}
					QC.Action { text: qsTr("Copy Selection");	onTriggered: citationText.copy();		} //citationText.deselect(); is not really necessary right?
				}
			}

			Text
			{
				text:			"<u>" + qsTr("BibTeX") + "</u>"
				color:			jaspTheme.blue

				MouseArea
				{
					id:				mouseAreaBibTex
					anchors.fill:	parent
					onClicked:		Qt.openUrlExternally(aboutModel.citationUrl)
					cursorShape:	Qt.PointingHandCursor
				}
			}
		}
	}

	Text
	{
		id:				warrantyText
		text:			aboutModel.warranty
		textFormat:		Text.StyledText
		opacity:		0.5
		color:			jaspTheme.textEnabled
	}

	Text
	{
		id:				openSourceText
		text:			"<u>" + qsTr("Open Source Components") + "</u>"
		color:			jaspTheme.blue

		MouseArea
		{
			id:				mouseAreaOpenSource
			anchors.fill:	parent
			onClicked:		Qt.openUrlExternally(aboutModel.openSourceUrl)
			cursorShape:	Qt.PointingHandCursor
		}
	}
}

