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

#ifndef DUP_TERMS_H
#define DUP_TERMS_H

#include <vector>
#include <string>
#include <set>

#include <QString>
#include <QList>
#include <QByteArray>

#include "DUP_term.h"

///
/// Terms is a list of Term. They are used in VariablesList
/// Some extra functionalities are added to deal with terms with interactions, in order for example to remove all terms that contain some component
/// (a component is most of the time a Variable name, but it can be a factor of level name).
/// Terms may have a parent Terms. This is used mainly in the available Variables List: this list has some variables that can be assigned to another (assigned) list.
/// The variable is then removed from the Available list and added to the assigned list. But if this variable is set back to the available list, it should get the same
/// order as before being set to the assigned list. For this we keep the original terms, and set it as parent of the 'functional' terms of the available list. When a variable
/// is set back to the available list, we can know with the parent terms where it was before being moved.
///
class DUP_Terms
{
public:
	DUP_Terms(const QList<QList<QString> >						& terms,	DUP_Terms *parent = nullptr);
	DUP_Terms(const QList<QString>								& terms,	DUP_Terms *parent = nullptr);
	DUP_Terms(const std::vector<std::vector<std::string> >		& terms,	DUP_Terms *parent = nullptr);
	DUP_Terms(const std::vector<std::string>					& terms,	DUP_Terms *parent = nullptr);
	DUP_Terms(const QList<DUP_Term>								& terms,	DUP_Terms *parent = nullptr);
	DUP_Terms(																DUP_Terms *parent = nullptr);

	void set(const QList<QList<QString> >					& terms);
	void set(const QList<QString>							& terms);
	void set(const std::vector<DUP_Term>					& terms);
	void set(const std::vector<std::string>					& terms);
	void set(const std::vector<std::vector<std::string> >	& terms);
	void set(const QList<DUP_Term>							& terms);
	void set(const DUP_Terms								& terms);
	void set(const QByteArray								& array);

	void removeParent();
	void setSortParent(const DUP_Terms &parent);

	void add(const DUP_Term &term, bool isUnique = true);
	void add(const DUP_Terms &terms);

	void insert(int index, const DUP_Term &term);
	void insert(int index, const DUP_Terms &terms);

	size_t size() const;
	const std::vector<DUP_Term> &terms() const;

	typedef std::vector<DUP_Term>::const_iterator const_iterator;
	typedef std::vector<DUP_Term>::iterator iterator;

	const_iterator begin() const;
	const_iterator end() const;

	void remove(const DUP_Term &term);
	void remove(const DUP_Terms &terms);
	void remove(size_t pos, size_t n = 1);
	void replace(int pos, const DUP_Term& term);
	bool discardWhatDoesntContainTheseComponents(	const DUP_Terms &terms);
	bool discardWhatDoesContainTheseComponents(		const DUP_Terms &terms);
	bool discardWhatDoesContainTheseTerms(			const DUP_Terms &terms);
	bool discardWhatIsntTheseTerms(					const DUP_Terms &terms, DUP_Terms *discarded = nullptr);

	QSet<int> replaceVariableName(const std::string & oldName, const std::string & newName);

	void clear();

	const DUP_Term &at(size_t index)								const;
	bool contains(const DUP_Term		&	term)					const;
	bool contains(const QString		&	component);
	bool contains(const std::string &	component);
	int	 indexOf(const QString		&	component)				const;

	std::vector<std::string>				asVector()			const;
	std::set<std::string>					asSet()				const;
	std::vector<std::vector<std::string> >	asVectorOfVectors()	const;
	QList<QString>							asQList()			const;
	QList<QList<QString> >					asQListOfQLists()	const;

	DUP_Term	sortComponents(const DUP_Term &term)	const;
	DUP_Terms	sortComponents(const DUP_Terms &terms)	const;

	DUP_Terms crossCombinations()					const;
	DUP_Terms wayCombinations(int ways)				const;
	DUP_Terms ffCombinations(const DUP_Terms &terms);

	std::string asString() const;
	bool hasDuplicate() const	{ return _hasDuplicate; }

	bool operator==(const DUP_Terms &terms) const;
	bool operator!=(const DUP_Terms &terms) const;
	const DUP_Term& operator[](size_t index) const { return at(index); }

private:

	int		rankOf(const QString &component)						const;
	int		termCompare(const DUP_Term& t1, const DUP_Term& t2)				const;
	bool	termLessThan(const DUP_Term &t1, const DUP_Term &t2)			const;

	const DUP_Terms			*	_parent;
	std::vector<DUP_Term>		_terms;
	bool					_hasDuplicate = false;
};

#endif // DUP_TERMS_H
