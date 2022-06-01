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

#ifndef DUP_TERM_H
#define DUP_TERM_H

#include <vector>
#include <string>

#include <QString>
#include <QStringList>

///
/// A term is a basic element of a VariablesList
/// It is usually just a string, but in case of interactions, it is a vector of strings, a component being one part of an interaction.
///
class DUP_Term
{
public:
	DUP_Term(const std::vector<std::string> components);
	DUP_Term(const std::string				component);
	DUP_Term(const QStringList				components);
	DUP_Term(const QString					component);

	const QStringList			& components()	const;
	const QString				& asQString()	const;

	std::vector<std::string>	scomponents()	const;
	std::string					asString()		const;

	typedef QStringList::const_iterator const_iterator;
	typedef QStringList::iterator		iterator;

	bool contains(		const QString	& component)	const;
	bool containsAll(	const DUP_Term		& term)			const;
	bool containsAny(	const DUP_Term		& term)			const;

	iterator begin();
	iterator end();

	const QString &at(int i) const;

	bool operator==(const DUP_Term &other) const;
	bool operator!=(const DUP_Term &other) const;

	size_t size() const;

	bool replaceVariableName(const std::string & oldName, const std::string & newName);

	static const char* separator;
	static DUP_Term	readTerm(std::string str);
	static DUP_Term	readTerm(QString str);

private:
	void initFrom(const QStringList components);
	void initFrom(const QString		component);

	QStringList		_components;
	QString			_asQString;

};

#endif // DUP_TERM_H
