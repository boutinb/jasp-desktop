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
#include "gui/preferencesmodel.h"

#include <QItemSelectionModel>
#include <QItemSelection>

//#define DATASETVIEW_DEBUG_VIEWPORT
//#define DATASETVIEW_DEBUG_CREATION

#define SHOW_ITEMS_PLEASE
#define ADD_LINES_PLEASE

struct ItemContextualized
{
	ItemContextualized(QQmlContext * context = nullptr, QQuickItem * item = nullptr) : item(item), context(context) {}

	QQuickItem * item;
	QQmlContext * context;
};


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
	Q_PROPERTY( QQuickItem			*	tableViewItem			READ tableViewItem			WRITE setTableViewItem												)
	Q_PROPERTY( QItemSelectionModel *	selection				READ selectionModel											NOTIFY selectionModelChanged		)
	Q_PROPERTY(	QModelIndex				selectionStart			READ selectionStart			WRITE setSelectionStart			NOTIFY selectionStartChanged		)
	Q_PROPERTY(	QModelIndex				selectionEnd			READ selectionEnd			WRITE setSelectionEnd			NOTIFY selectionEndChanged			)
	Q_PROPERTY(	bool					editing					READ editing				WRITE setEditing				NOTIFY editingChanged				)
	
public:
							DataSetView(QQuickItem *parent = nullptr);

	static DataSetView *	lastInstancedDataSetView()					{ return _lastInstancedDataSetView; }

	void					setModel(QAbstractItemModel * model);
	
	QAbstractItemModel	*	model()								const	{ return _model;					}
	QItemSelectionModel	*	selectionModel()					const	{ return _selectionModel;			}

	int						itemHorizontalPadding()				const	{ return _itemHorizontalPadding;	}
	int						itemVerticalPadding()				const	{ return _itemVerticalPadding;		}
	int						headerHeight()						const	{ return _dataRowsMaxHeight;		}
	int						rowNumberWidth()					const	{ return _rowNumberMaxWidth;		}

	double					viewportX()							const	{ return _viewportX;				}
	double					viewportY()							const	{ return _viewportY;				}
	double					viewportW()							const	{ return _viewportW;				}
	double					viewportH()							const	{ return _viewportH;				}

	QModelIndex				selectionTopLeft()					const;

	QQmlComponent		*	itemDelegate()						const	{ return _itemDelegate;				}
	QQmlComponent		*	rowNumberDelegate()					const	{ return _rowNumberDelegate;		}
	QQmlComponent		*	columnHeaderDelegate()				const	{ return _columnHeaderDelegate;		}
	QQuickItem			*	leftTopCornerItem()					const	{ return _leftTopItem;				}
	QQuickItem			*	extraColumnItem()					const	{ return _extraColumnItem;			}
	QQuickItem			*	tableViewItem()						const	{ return _tableViewItem;			}
	QQmlComponent		*	editDelegate()						const	{ return _editDelegate;				}

	bool					cacheItems()						const	{ return _cacheItems;				}
	QModelIndex				selectionStart()					const	{ return _selectionStart;			}
	QModelIndex				selectionEnd()						const	{ return _selectionEnd;				}
	bool					editing()							const	{ return _editing;		}

	Q_INVOKABLE QQuickItem*	getColumnHeader(int col)					{ return _columnHeaderItems.count(col) 	> 0	? _columnHeaderItems[col]->item : nullptr;	}
	Q_INVOKABLE QQuickItem*	getRowHeader(	int row)					{ return _rowNumberItems.count(row) 	> 0 ? _rowNumberItems[row]->item	: nullptr;	}


	
	
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

	void resetItems();

	GENERIC_SET_FUNCTION(HeaderHeight,		_dataRowsMaxHeight, headerHeightChanged,		double)
	GENERIC_SET_FUNCTION(RowNumberWidth,	_rowNumberMaxWidth, rowNumberWidthChanged,		double)
	 
	
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

	void		headerHeightChanged();
	void		rowNumberWidthChanged();

	void		cacheItemsChanged();
	
	void		selectionStartChanged(	QModelIndex selectionStart);
	void		selectionEndChanged(	QModelIndex selectionEnd);
	void		editingChanged(bool shiftSelectActive);

	void		selectionBudgesUp();
	void		selectionBudgesDown();
	void		selectionBudgesLeft();
	void		selectionBudgesRight();

	void		showComputedColumn(QString name);
	
public slots:
	void		calculateCellSizes()	{ calculateCellSizesAndClear(false); }
	void		aContentSizeChanged()	{ _recalculateCellSizes = true; }
	void		viewportChanged();
	void		myParentChanged(QQuickItem *);

	void		reloadTextItems();
	void		reloadRowNumbers();
	void		reloadColumnHeaders();

	void		modelDataChanged(const QModelIndex &, const QModelIndex &, const QVector<int> &);
	void		modelHeaderDataChanged(Qt::Orientation, int, int);
	void		modelAboutToBeReset();
	void		modelWasReset();
	void		setExtraColumnX();
	
	void		setSelectionStart(	QModelIndex selectionStart	);
	void		setSelectionEnd(	QModelIndex selectionEnd	);	
	void		pollSelectScroll(	QModelIndex mouseIndex		);
	void		setEditing(bool shiftSelectActive);
	bool		relaxForSelectScroll();


	void		cut(	bool includeHeader = false) { _copy(includeHeader, true);  }
	void		copy(	bool includeHeader = false) { _copy(includeHeader, false); }
	void		paste(	bool includeHeader = false);

	void		columnSelect(				int col = -1);
	QString		columnInsertBefore(			int col = -1, bool computed = false, bool R = false);
	QString		columnInsertAfter(			int col = -1, bool computed = false, bool R = false);
	void		columnComputedInsertAfter(	int col = -1,	bool R=true);
	void		columnComputedInsertBefore(	int col = -1,	bool R=true);

	void		columnsDelete();
	void		rowSelect(					int row = -1);
	void		rowInsertBefore(			int row = -1);
	void		rowInsertAfter(				int row = -1);
	void		rowsDelete();

	void		columnsAboutToBeInserted(	const QModelIndex &parent, int first, int last);
	void		columnsAboutToBeRemoved(	const QModelIndex &parent, int first, int last);
	void		rowsAboutToBeInserted(		const QModelIndex &parent, int first, int last);
	void		rowsAboutToBeRemoved(		const QModelIndex &parent, int first, int last);
	void		columnsInserted(			const QModelIndex &parent, int first, int last);
	void		columnsRemoved(				const QModelIndex &parent, int first, int last);
	void		rowsInserted(				const QModelIndex &parent, int first, int last);
	void		rowsRemoved(				const QModelIndex &parent, int first, int last);



	void		selectAll();

	void		edit(QModelIndex here);
	void		destroyEditItem(bool restoreItem=true);
	void		editFinished(			QModelIndex here, QVariant editedValue);
	void		commitEdit(QModelIndex here, QVariant editedValue);
	void		onDataModeChanged(bool dataMode);
	void		contextMenuClickedAtIndex(QModelIndex index);

	
protected:
	void		_copy(bool includeHeader, bool clear);
	void		calculateCellSizesAndClear(bool clearStorage);
	void		setRolenames();
	void		determineCurrentViewPortIndices();
	void		storeOutOfViewItems();
	void		buildNewLinesAndCreateNewItems();

#ifdef ADD_LINES_PLEASE
	QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *data) override;
#endif
	float extraColumnWidth() { return !_extraColumnItem ? 0 : 2 + _extraColumnItem->width(); }

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
	QQmlContext *	setStyleDataColumnHeader(	QQmlContext * previousContext, QString text, int column, bool isComputed, bool isInvalidated, bool isFiltered,  QString computedError, int columnType);


	void			addLine(float x0, float y0, float x1, float y1);

	QSizeF			getTextSize(const QString& text)	const;
	QSizeF			getColumnSize(int col);
	QSizeF			getRowHeaderSize();

protected:
	QAbstractItemModel									*	_model					= nullptr;
	QItemSelectionModel									*	_selectionModel			= nullptr;
	std::vector<QSizeF>										_cellSizes;							//[col]
	std::vector<double>										_colXPositions,						//[col][row]
															_dataColsMaxWidth;
	std::stack<ItemContextualized*>							_textItemStorage,
															_rowNumberStorage,
															_columnHeaderStorage;
	std::map<int, ItemContextualized *>						_rowNumberItems,
															_columnHeaderItems;
	std::map<int, std::map<int, ItemContextualized *>>		_cellTextItems;						//[col][row]
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
	std::map<std::string, int>								_roleNameToRole;
	std::map<size_t, std::map<size_t, unsigned char>>		_storedLineFlags;
	std::map<size_t, std::map<size_t, QString>>				_storedDisplayText;
	static DataSetView									*	_lastInstancedDataSetView;
	
	bool		_cacheItems				= true,
				_recalculateCellSizes	= false,
				_ignoreViewpoint		= true,
				_linesWasChanged		= false,
				_editing				= false;
	double		_dataRowsMaxHeight,
				_dataWidth				= -1,
				_rowNumberMaxWidth		= 0,
				_viewportX				= 0,
				_viewportY				= 0, 
				_viewportW				= 1, 
				_viewportH				= 1;
	int			_itemHorizontalPadding	= 8,
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
				_prevEditCol			= -1;
	size_t		_linesActualSize		= 0;
	long		_selectScrollMs			= 0;
	QModelIndex _selectionStart,
				_selectionEnd;
};



#endif // DATASETVIEW_H
