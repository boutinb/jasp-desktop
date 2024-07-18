#ifndef DATASETVIEW_H
#define DATASETVIEW_H

#include <QObject>
#include <QQuickItem>
#include <QAbstractItemModel>
#include <vector>
#include <stack>
#include <QSGFlatColorMaterial>

#include <map>
#include <QtQml>
#include "utilities/qutils.h"
#include "data/expanddataproxymodel.h"

#include <QItemSelectionModel>
#include <QItemSelection>

//#define DATASETVIEW_DEBUG_VIEWPORT
//#define DATASETVIEW_DEBUG_CREATION

#define SHOW_ITEMS_PLEASE
#define ADD_LINES_PLEASE

struct ItemContextualized
{
	ItemContextualized(QQmlContext * context = nullptr, QQuickItem * item = nullptr) 
	: item(item), context(context) 
	{}
	
	~ItemContextualized()
	{
		if(item)	item	-> deleteLater();
		if(context)	context	-> deleteLater();
		
		context =  nullptr;
		item	=  nullptr;
	}
	
	QQuickItem	* item		= nullptr;
	QQmlContext * context	= nullptr;
};

typedef std::map<int, std::map<int, ItemContextualized *>>  ItemCxsByColRow;
typedef std::map<int, ItemContextualized *>                 ItemCxsByIndex;

/// Custom QQuickItem to render data tables witch caching and only displaying the necessary cells and lines
/// Supports scaling the data into millions of columns and rows without any noticable slowdowns (the model could slow it down though)
/// Contains custom rendering code for the lines to make sure they are always a single pixel wide.
/// Caching is a bit flawed at the moment though so when changing data in the model it is best to turn that off.
/// It also uses pools of header-, rowheader- and general-items when they go out of view to avoid the overhead of recreating them all the time.
class DataSetView : public QQuickItem
{
	Q_OBJECT

	Q_PROPERTY( QAbstractItemModel	*	model					READ model					WRITE setModel					NOTIFY modelChanged					)
	Q_PROPERTY( int						itemHorizontalPadding	READ itemHorizontalPadding	WRITE setItemHorizontalPadding	NOTIFY itemHorizontalPaddingChanged )
	Q_PROPERTY( int						itemVerticalPadding		READ itemVerticalPadding	WRITE setItemVerticalPadding	NOTIFY itemVerticalPaddingChanged	)
	Q_PROPERTY( double					viewportX				READ viewportX				WRITE setViewportX				NOTIFY viewportXChanged				)
	Q_PROPERTY( double					viewportY				READ viewportY				WRITE setViewportY				NOTIFY viewportYChanged				)
	Q_PROPERTY( double					viewportW				READ viewportW				WRITE setViewportW				NOTIFY viewportWChanged				)
	Q_PROPERTY( double					viewportH				READ viewportH				WRITE setViewportH				NOTIFY viewportHChanged				)
	Q_PROPERTY( QQmlComponent		*	itemDelegate			READ itemDelegate			WRITE setItemDelegate			NOTIFY itemDelegateChanged			)
	Q_PROPERTY( QQmlComponent		*	rowNumberDelegate		READ rowNumberDelegate		WRITE setRowNumberDelegate		NOTIFY rowNumberDelegateChanged		)
	Q_PROPERTY( QQmlComponent		*	columnHeaderDelegate	READ columnHeaderDelegate	WRITE setColumnHeaderDelegate	NOTIFY columnHeaderDelegateChanged	)
	Q_PROPERTY( QQuickItem			*	leftTopCornerItem		READ leftTopCornerItem		WRITE setLeftTopCornerItem		NOTIFY leftTopCornerItemChanged		)
	Q_PROPERTY( QQuickItem			*	extraColumnItem			READ extraColumnItem		WRITE setExtraColumnItem		NOTIFY extraColumnItemChanged		)
	Q_PROPERTY( QQmlComponent		*	editDelegate			READ editDelegate			WRITE setEditDelegate			NOTIFY editDelegateChanged			)
	Q_PROPERTY( double					headerHeight			READ headerHeight											NOTIFY headerHeightChanged			)
	Q_PROPERTY( double					rowNumberWidth			READ rowNumberWidth			WRITE setRowNumberWidth			NOTIFY rowNumberWidthChanged		)
	Q_PROPERTY( bool					cacheItems				READ cacheItems				WRITE setCacheItems				NOTIFY cacheItemsChanged			)
	Q_PROPERTY( bool					expandDataSet			READ expandDataSet			WRITE setExpandDataSet			NOTIFY expandDataSetChanged			)
	Q_PROPERTY( QQuickItem			*	tableViewItem			READ tableViewItem			WRITE setTableViewItem												)
	Q_PROPERTY( QItemSelectionModel *	selection				READ selectionModel											NOTIFY selectionModelChanged		)
	Q_PROPERTY(	QPoint					selectionMin			READ selectionMin											NOTIFY selectionMinChanged			)
	Q_PROPERTY(	QPoint					selectionMax			READ selectionMax											NOTIFY selectionMaxChanged			)
	Q_PROPERTY(	QPoint					editCoordinates			READ editCoordinates										NOTIFY editCoordinatesChanged		)
	Q_PROPERTY(	bool					editing					READ editing				WRITE setEditing				NOTIFY editingChanged				)
	Q_PROPERTY( bool					mainData				READ mainData				WRITE setMainData				NOTIFY mainDataChanged				)
	Q_PROPERTY( int						maxColWidth				READ maxColWidth			WRITE setMaxColWidth			NOTIFY maxColWidthChanged			)
	
public:
	friend ExpandDataProxyModel;

							DataSetView(QQuickItem *parent = nullptr);

							static DataSetView *	mainDataViewer()								{ return _mainDataSetView;}

	void					setModel(QAbstractItemModel * model);
	
	QAbstractItemModel	*	model()								const	{ return _model->sourceModel();		}
	QItemSelectionModel	*	selectionModel()					const	{ return _selectionModel;			}

	int						itemHorizontalPadding()				const	{ return _itemHorizontalPadding;	}
	int						itemVerticalPadding()				const	{ return _itemVerticalPadding;		}
	int						headerHeight()						const	{ return _dataRowsMaxHeight;		}
	int						rowNumberWidth()					const	{ return _rowNumberMaxWidth;		}

	double					viewportX()							const	{ return _viewportX;				}
	double					viewportY()							const	{ return _viewportY;				}
	double					viewportW()							const	{ return _viewportW;				}
	double					viewportH()							const	{ return _viewportH;				}

	QQmlComponent		*	itemDelegate()						const	{ return _itemDelegate;				}
	QQmlComponent		*	rowNumberDelegate()					const	{ return _rowNumberDelegate;		}
	QQmlComponent		*	columnHeaderDelegate()				const	{ return _columnHeaderDelegate;		}
	QQuickItem			*	leftTopCornerItem()					const	{ return _leftTopItem;				}
	QQuickItem			*	extraColumnItem()					const	{ return _extraColumnItem;			}
	QQuickItem			*	tableViewItem()						const	{ return _tableViewItem;			}
	QQmlComponent		*	editDelegate()						const	{ return _editDelegate;				}

	bool					cacheItems()						const	{ return _cacheItems;				}
	bool					expandDataSet()						const	{ return _model ? _model->expandDataSet() : false;			}
	QPoint					selectionMin()						const;
	QPoint					selectionMax()						const;
	bool					editing()							const	{ return _editing;					}
	bool					mainData()							const	{ return _mainData;					}

	Q_INVOKABLE QQuickItem*	getColumnHeader(int col)					{ return _columnHeaderItems.count(col) 	> 0	? _columnHeaderItems[col]->item : nullptr;	}
	Q_INVOKABLE QQuickItem*	getRowHeader(	int row)					{ return _rowNumberItems.count(row) 	> 0 ? _rowNumberItems[row]->item	: nullptr;	}

	Q_INVOKABLE	bool		clipBoardPasteIsCells()				const;
	
	GENERIC_SET_FUNCTION(ViewportX,		_viewportX,		viewportXChanged,	double	)
	GENERIC_SET_FUNCTION(ViewportY,		_viewportY,		viewportYChanged,	double	)
	GENERIC_SET_FUNCTION(ViewportW,		_viewportW,		viewportWChanged,	double	)
	GENERIC_SET_FUNCTION(ViewportH,		_viewportH,		viewportHChanged,	double	)

	void setItemHorizontalPadding(	int newHorizontalPadding)	{ if(newHorizontalPadding	!= _itemHorizontalPadding)	{ _itemHorizontalPadding	= newHorizontalPadding;		emit itemHorizontalPaddingChanged();	update(); } }
	void setItemVerticalPadding(	int newVerticalPadding)		{ if(newVerticalPadding		!= _itemVerticalPadding)	{ _itemVerticalPadding		= newVerticalPadding;		emit itemVerticalPaddingChanged();		update(); } }

	void setRowNumberDelegate(		QQmlComponent	* newDelegate);
	void setColumnHeaderDelegate(	QQmlComponent	* newDelegate);
	void setItemDelegate(			QQmlComponent	* newDelegate);
	void setLeftTopCornerItem(		QQuickItem		* newItem);
	void setExtraColumnItem(		QQuickItem		* newItem);
	void setEditDelegate(			QQmlComponent	* editDelegate);
	void setTableViewItem(			QQuickItem		* tableViewItem) { _tableViewItem = tableViewItem; }
	void setCacheItems(				bool			  cacheItems);
	void setExpandDataSet(			bool			  expandDataSet);

	void resetItems();

	GENERIC_SET_FUNCTION(HeaderHeight,		_dataRowsMaxHeight, headerHeightChanged,		double)
	GENERIC_SET_FUNCTION(RowNumberWidth,	_rowNumberMaxWidth, rowNumberWidthChanged,		double)
	 
	
	void setMainData(bool newMainData);
	
	QPoint editCoordinates() const;
	
	int maxColWidth() const;
	void setMaxColWidth(int newMaxColWidth);
	
signals:
	void		modelChanged();
	void		selectionModelChanged();
	void		itemHorizontalPaddingChanged();
	void		itemVerticalPaddingChanged();

	void		viewportXChanged();
	void		viewportYChanged();
	void		viewportWChanged();
	void		viewportHChanged();

	void		rowNumberDelegateChanged();
	void		columnHeaderDelegateChanged();
	void		itemDelegateChanged();
	void		leftTopCornerItemChanged();
	void		extraColumnItemChanged();
	void		editDelegateChanged(QQmlComponent * editDelegate);

	void		itemSizeChanged();
	void		selectionChanged();

	void		headerHeightChanged();
	void		rowNumberWidthChanged();

	void		cacheItemsChanged();
	void		expandDataSetChanged();
	
	void		selectionMinChanged();
	void		selectionMaxChanged();
	
	void		editingChanged(bool shiftSelectActive);

	void		selectionBudgesUp();
	void		selectionBudgesDown();
	void		selectionBudgesLeft();
	void		selectionBudgesRight();

	void		undoChanged();
	
	void		mainDataChanged();
	
	void		editCoordinatesChanged();
	
	void		maxColWidthChanged();
	
public slots:
	void		calculateCellSizes()	{ calculateCellSizesAndClear(false); }
	void		aContentSizeChanged()	{ _recalculateCellSizes = true; }
	void		viewportChanged();
	void		viewportChangedDelayed();
	void		myParentChanged(QQuickItem *);

	void		reloadTextItems();
	void		reloadRowNumbers();
	void		reloadColumnHeaders();

	void		modelDataChanged(const QModelIndex &, const QModelIndex &, const QVector<int> &);
	void		modelHeaderDataChanged(Qt::Orientation, int, int);
	void		modelAboutToBeReset();
	void		modelWasReset();
	void		setExtraColumnX();
	
	bool		isSelected(			int row, int col);
	void		pollSelectScroll(	int row, int column);
	void		setEditing(bool shiftSelectActive);
	bool		relaxForSelectScroll();

	bool		isVirtual		(const QPoint& point);
	bool		isColumnHeader	(const QPoint& p) { return p.x() >= 0	&& p.y() == -1; }
	bool		isRowHeader		(const QPoint& p) { return p.x() == -1	&& p.y() >= 0;	}
	bool		isCell			(const QPoint& p) { return p.x() >= 0	&& p.y() >= 0;	}

	void		cut(	QPoint where = QPoint(-1,-1)) { _copy(where, true);  }
	void		copy(	QPoint where = QPoint(-1,-1)) { _copy(where, false); }
	void		paste(	QPoint where = QPoint(-1,-1));

	void		select(						int row, int column,	bool shiftPressed,			bool ctrlCmdPressed);
	void		selectionClear();
	void		selectHover(				int row, int column);
	QString		columnInsertBefore(			int col = -1,	bool computed = false, bool R = false);
	QString		columnInsertAfter(			int col = -1,	bool computed = false, bool R = false);
	void		columnComputedInsertAfter(	int col = -1,	bool R=true);
	void		columnComputedInsertBefore(	int col = -1,	bool R=true);
	void		columnsDeleteSelected();
	void		columnsDelete(				int col);
	void		columnReverseValues(		int col = -1);
	void		columnautoSortByValues(		int col = -1);
	void		rowInsertBefore(			int row = -1);
	void		rowInsertAfter(				int row = -1);
	void		rowsDelete(					int row);
	void		rowsDeleteSelected();
	void		cellsClear();

	void		columnsAboutToBeInserted(	const QModelIndex &parent, int first, int last);
	void		columnsAboutToBeRemoved(	const QModelIndex &parent, int first, int last);
	void		rowsAboutToBeInserted(		const QModelIndex &parent, int first, int last);
	void		rowsAboutToBeRemoved(		const QModelIndex &parent, int first, int last);
	void		columnsInserted(			const QModelIndex &parent, int first, int last);
	void		columnsRemoved(				const QModelIndex &parent, int first, int last);
	void		rowsInserted(				const QModelIndex &parent, int first, int last);
	void		rowsRemoved(				const QModelIndex &parent, int first, int last);

	int			rowCount()								{ return _model->rowCount();	}
	int			columnCount()							{ return _model->columnCount(); }

	
	void		selectAll();
	void		undo()									{ _model->undo(); }
	void		redo()									{ _model->redo(); }
	QString		undoText()								{ return _model->undoText(); }
	QString		redoText()								{ return _model->redoText(); }

	void		clearEdit();
	void		edit(int row, int column);
	void		destroyEditItem(bool restoreItem=true);
	void		commitEdit(				int row, int column, QVariant editedValue);
	void		onDataModeChanged(bool dataMode);
	void		commitLastEdit();

	void		setColumnType(int columnIndex, int newColumnType);
	void		resizeData(int rows, int columns);
	void		currentSelectedColumnHandler(const QModelIndex &current, const QModelIndex &previous);

protected:
	void		_copy(QPoint where, bool clear);
	void		calculateCellSizesAndClear(bool clearStorage);
	void		determineCurrentViewPortIndices();
    void		storeAllItems();
    void		storeOutOfViewItems();
	void		buildNewLinesAndCreateNewItems();
	void		columnIndexSelectedApply(int columnIndex, std::function<void (int)> applyThis);
	void		columnIndexSelectedApply(int columnIndex, std::function<void (intset)> applyThis);

#ifdef ADD_LINES_PLEASE
	QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data) override;
#endif
	float extraColumnWidth() { return !_extraColumnItem || expandDataSet() ? 0 : 2 + _extraColumnItem->width(); }

	QQuickItem *	createTextItem(int row, int col);
	void			storeTextItem(int row, int col, bool cleanUp = true);
	void			setTextItemInfo(int row, int col, QQuickItem * textItem);

	QQuickItem	*	createRowNumber(	int row);
	void			storeRowNumber(		int row);

	QQuickItem	*	createColumnHeader(	int col);
	void			storeColumnHeader(	int col);

	QQuickItem	*	createleftTopCorner();
	
	void			updateExtraColumnItem();
	void			positionEditItem(	int row, int col);

	QQmlContext *	setStyleDataItem(			QQmlContext * previousContext, bool active, size_t col, size_t row, bool emptyValLabel = true);
	QQmlContext *	setStyleDataRowNumber(		QQmlContext * previousContext, QString text, int row);
	QQmlContext *	setStyleDataColumnHeader(	QQmlContext * previousContext, QString text, int column, bool isComputed, bool isInvalidated, bool isFiltered,  QString computedError, int columnType, int computedColumnType);


	void			addLine(float x0, float y0, float x1, float y1);

	QSizeF			getTextSize(const QString& text)	const;
	QSizeF			getColumnSize(int col);
	QSizeF			getRowHeaderSize();
	void			clearCaches();

protected:
	QItemSelectionModel									*	_selectionModel			= nullptr;
	ExpandDataProxyModel								*	_model					= nullptr;
	std::vector<QSizeF>										_cellSizes;							//[col]
	std::vector<double>										_colXPositions,						//[col][row]
															_dataColsMaxWidth;
	std::stack<ItemContextualized*>							_textItemStorage,
															_rowNumberStorage,
															_columnHeaderStorage;
    ItemCxsByIndex                  						_rowNumberItems,
															_columnHeaderItems;
    ItemCxsByColRow                                 		_cellTextItems;						//[col][row]
	std::vector<float>										_lines;
	QQuickItem											*	_leftTopItem			= nullptr,
														*	_extraColumnItem		= nullptr,
														*	_tableViewItem			= nullptr;
	QQmlComponent										*	_itemDelegate			= nullptr,
														*	_rowNumberDelegate		= nullptr,
														*	_columnHeaderDelegate	= nullptr,
														*	_leftTopCornerDelegate	= nullptr,
														*	_styleDataCreator		= nullptr,
														*	_editDelegate			= nullptr;
	ItemContextualized									*	_editItemContextual		= nullptr;
	QSGFlatColorMaterial									_material;
	std::map<size_t, std::map<size_t, unsigned char>>		_storedLineFlags;
	std::map<size_t, std::map<size_t, QString>>				_storedDisplayText;
	static DataSetView									*	_mainDataSetView;
	bool													_cacheItems				= false,
															_recalculateCellSizes	= false,
															_ignoreViewpoint		= true,
															_linesWasChanged		= false,
															_editing				= false,
															_mainData				= false;
	double													_dataRowsMaxHeight,
															_dataWidth				= -1,
															_rowNumberMaxWidth		= 0,
															_viewportX				= 0,
															_viewportY				= 0, 
															_viewportW				= 1, 
															_viewportH				= 1;
	int														_itemHorizontalPadding	= 8,
															_itemVerticalPadding	= 8,
															_previousViewportColMin = -1,
															_previousViewportColMax = -1,
															_previousViewportRowMin = -1,
															_previousViewportRowMax = -1,
															_viewportMargin			=  1,
															_currentViewportColMin	= -1,
															_currentViewportColMax	= -1,
															_currentViewportRowMin	= -1,
															_currentViewportRowMax	= -1,
															_prevEditRow			= -1,
															_prevEditCol			= -1,
															_maxColWidth			= -1;
	size_t													_linesActualSize		= 0;
	long													_selectScrollMs			= 0;
	std::vector<Json::Value>								_copiedColumns;
	QString													_lastJaspCopyIntoClipboard;
	std::vector<qstringvec>									_lastJaspCopyValues,
															_lastJaspCopyLabels;
	std::vector<boolvec>									_lastJaspCopySelect;
	QTimer												*	_delayViewportChangedTimer	= nullptr;
};



#endif // DATASETVIEW_H
