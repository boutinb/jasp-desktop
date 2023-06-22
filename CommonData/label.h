#ifndef LABEL_H
#define LABEL_H

#include <string>
#include <json/json.h>
#include "datasetbasenode.h"

class Column;
class DatabaseInterface;

/// A label
/// 
/// Label is a class that stores the value of a column if it is not a Scale (a Nominal Int, Nominal Text, or Ordinal).
/// The original value can be an integer, float or string, this is stored in a json
///
/// Internally for all non-scalar columns they are stored as ints in Column::_ints, the value of a Label corresponds to that
/// Beyond that there are some extra attributes like a description or whether it is currently allowed by the generated filter.
///
/// The order of the labels in R is determined by their order in Column::_labels,
/// and Column makes sure (_dbUpdateLabelOrder) the order is stored in the database when it is changed.
class Label : public DataSetBaseNode
{
public:	
	static const int MAX_LABEL_DISPLAY_LENGTH = 64; //we can store the rest in description if necessary

								Label(Column * column);
								Label(Column * column, int value);
								Label(Column * column, const std::string & label, int value, bool filterAllows = true, const std::string & description = "", const Json::Value & originalValue = Json::nullValue, int order = -1, int id = -1);

			void				dbDelete();
			void				dbCreate();
			void				dbLoad(int labelId = -1);
			void				dbUpdate();

			Label			&	operator=(const Label &label);

			int					id()						const	{ return _id;				}
	const	std::string		&	description()				const	{ return _description;		}
			std::string			label(bool shorten = false)	const	{ return !shorten || _label.size() <= MAX_LABEL_DISPLAY_LENGTH ? _label : _label.substr(0, MAX_LABEL_DISPLAY_LENGTH);	}
			int					value()						const	{ return _value;			}
			int					order()						const	{ return _order;			}
			bool				filterAllows()				const	{ return _filterAllows;		}
	const	Json::Value		&	originalValue()				const	{ return _originalValue;	}

			std::string			originalValueAsString(bool fancyEmptyValue = false)		const;
			std::string			str() const;

			void				setValue(			int value);
			void				setOrder(			int order);
			void				setId(				int id) { _id = id; }
			bool				setLabel(			const std::string & label);
			void				setOriginalValue(	const Json::Value & originalValue);
			void				setDescription(		const std::string & description);
			void				setFilterAllows(	bool allowFilter);
			void				setInformation(Column * column, int id, int order, const std::string &label, int value, bool filterAllows, const std::string & description, const Json::Value & originalValue);

			Json::Value			serialize()	const;

			DatabaseInterface	& db();
	const	DatabaseInterface	& db() const;

private:

	Column		*	_column;

	Json::Value		_originalValue	= Json::nullValue;	///< Could contain integers, floats or strings. Arrays and objects are undefined.

	int				_id				= -1,	///< Database id
					_order			= -1,	///< Should correspond to its position in Column::_labels
					_value			= -1;	///< value of label, should always map to Column::_ints
	std::string		_label,					///< What to display in the dataview
					_description;			///< Extended information for tooltip in dataview and of course in the variableswindow
	bool			_filterAllows	= true;	///< Used in generating filters for when users disable and enable certain labels/levels
};

#endif // LABEL_H
