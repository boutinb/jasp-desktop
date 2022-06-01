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

#include "DUP_terms.h"

#include <sstream>

#include <QDataStream>
#include <QIODevice>
#include <QSet>
#include "DUP_qutils.h"

using namespace std;

DUP_Terms::DUP_Terms(const QList<QList<QString> > &terms, DUP_Terms *parent)
{
	_parent = parent;
	set(terms);
}

DUP_Terms::DUP_Terms(const QList<QString> &terms, DUP_Terms *parent)
{
	_parent = parent;
	set(terms);
}

DUP_Terms::DUP_Terms(const std::vector<std::vector<string> > &terms, DUP_Terms *parent)
{
	_parent = parent;
	set(terms);
}

DUP_Terms::DUP_Terms(const std::vector<string> &terms, DUP_Terms *parent)
{
	_parent = parent;
	set(terms);
}

DUP_Terms::DUP_Terms(const QList<DUP_Term> &terms, DUP_Terms *parent)
{
	_parent = parent;
	set(terms);
}

DUP_Terms::DUP_Terms(DUP_Terms *parent)
{
	_parent = parent;
}

void DUP_Terms::set(const std::vector<DUP_Term> &terms)
{
	_terms.clear();

	for(const DUP_Term &term : terms)
		add(term);
}

void DUP_Terms::set(const std::vector<string> &terms)
{
	_terms.clear();

	for(const DUP_Term &term : terms)
		add(term);
}

void DUP_Terms::set(const std::vector<std::vector<string> > &terms)
{
	_terms.clear();

	for(const DUP_Term &term : terms)
		add(term);
}

void DUP_Terms::set(const QList<DUP_Term> &terms)
{
	_terms.clear();

	for(const DUP_Term &term : terms)
		add(term);
}

void DUP_Terms::set(const DUP_Terms &terms)
{
	_terms.clear();
	_hasDuplicate = terms.hasDuplicate();

	for(const DUP_Term &term : terms)
		add(term);
}

void DUP_Terms::set(const QList<QList<QString> > &terms)
{
	_terms.clear();

	for(const QList<QString> &term : terms)
		add(DUP_Term(term));
}

void DUP_Terms::set(const QList<QString> &terms)
{
	_terms.clear();

	for(const QString &term : terms)
		add(DUP_Term(term));
}

void DUP_Terms::setSortParent(const DUP_Terms &parent)
{
	_parent = &parent;
}

void DUP_Terms::removeParent() {
	_parent = nullptr;
}

void DUP_Terms::add(const DUP_Term &term, bool isUnique)
{
	if (!isUnique || _hasDuplicate)
	{
		if (!_hasDuplicate && contains(term)) _hasDuplicate = true;
		_terms.push_back(term);
	}
	else if (_parent != nullptr)
	{
		vector<DUP_Term>::iterator itr = _terms.begin();
		int result = -1;

		for (; itr != _terms.end(); itr++)
		{
			result = termCompare(term, *itr);
			if (result >= 0)
				break;
		}

		if (result > 0)
			_terms.insert(itr, term);
		else if (result < 0)
			_terms.push_back(term);
	}
	else
	{
		if ( ! contains(term))
			_terms.push_back(term);
	}
}

void DUP_Terms::insert(int index, const DUP_Term &term)
{
	if (_parent == nullptr)
	{
		vector<DUP_Term>::iterator itr = _terms.begin();

		for (int i = 0; i < index; i++)
			itr++;

		_terms.insert(itr, term);
	}
	else
	{
		add(term);
	}
}

void DUP_Terms::insert(int index, const DUP_Terms &terms)
{
	if (_parent == nullptr)
	{
		vector<DUP_Term>::iterator itr = _terms.begin();

		for (int i = 0; i < index; i++)
			itr++;

		_terms.insert(itr, terms.begin(), terms.end());
	}
	else
	{
		add(terms);
	}
}

void DUP_Terms::add(const DUP_Terms &terms)
{
	_hasDuplicate = _hasDuplicate || terms.hasDuplicate();

	for(const DUP_Term & term : terms)
		add(term);
}

const DUP_Term& DUP_Terms::at(size_t index) const
{
	return _terms.at(index);
}

bool DUP_Terms::contains(const DUP_Term &term) const
{
	return std::find(_terms.begin(), _terms.end(), term) != _terms.end();
}

bool DUP_Terms::contains(const std::string & component)
{
	return contains(tq(component));
}

int DUP_Terms::indexOf(const QString &component) const
{
	int i = 0;
	for(const DUP_Term &term : _terms)
	{
		if (term.contains(component))
			return i;
		i++;
	}

	return -1;
}

bool DUP_Terms::contains(const QString & component)
{
	for(const DUP_Term &term : _terms)
	{
		if (term.contains(component))
			return true;
	}

	return false;
}

vector<string> DUP_Terms::asVector() const
{
	vector<string> items;

	for(const DUP_Term &term : _terms)
		items.push_back(term.asString());

	return items;
}

std::set<std::string> DUP_Terms::asSet() const
{
	std::set<std::string> items;

	for(const DUP_Term &term : _terms)
		for(std::string termComp : term.scomponents())
			items.insert(termComp);

	return items;
}

vector<vector<string> > DUP_Terms::asVectorOfVectors() const
{
	vector<vector<string> > items;

	for(const DUP_Term &term : _terms)
	{
		vector<string> components = term.scomponents();
		items.push_back(components);
	}

	return items;
}

QList<QString> DUP_Terms::asQList() const
{
	QList<QString> items;

	for(const DUP_Term &term : _terms)
		items.append(term.asQString());

	return items;
}

QList<QList<QString> > DUP_Terms::asQListOfQLists() const
{
	QList<QList<QString> > items;

	for(const DUP_Term &term : _terms)
	{
		QList<QString> components = term.components();
		items.append(components);
	}

	return items;
}

DUP_Terms DUP_Terms::sortComponents(const DUP_Terms &terms) const
{
	QList<DUP_Term> ts;

	for (const DUP_Term &term : terms)
		ts.append(sortComponents(term));

	return DUP_Terms(ts);
}

DUP_Terms DUP_Terms::crossCombinations() const
{
	if (_terms.size() <= 1)
		return DUP_Terms(asVector());

	DUP_Terms t;

	for (uint r = 1; r <= _terms.size(); r++)
	{
		vector<bool> v(_terms.size());
		fill(v.begin() + r, v.end(), true);

		do {

			vector<string> combination;

			for (uint i = 0; i < _terms.size(); i++) {
				if (!v[i])
					combination.push_back(_terms.at(i).asString());
			}

			t.add(DUP_Term(combination));

		} while (std::next_permutation(v.begin(), v.end()));
	}

	return t;
}

DUP_Terms DUP_Terms::wayCombinations(int ways) const
{
	DUP_Terms t;

	for (int r = ways; r <= ways; r++)
	{
		vector<bool> v(_terms.size());
		std::fill(v.begin() + r, v.end(), true);

		do {

			vector<string> combination;

			for (uint i = 0; i < _terms.size(); ++i) {
				if (!v[i])
					combination.push_back(_terms.at(i).asString());
			}

			t.add(DUP_Term(combination));

		} while (std::next_permutation(v.begin(), v.end()));
	}

	return t;
}

DUP_Terms DUP_Terms::ffCombinations(const DUP_Terms &terms)
{
	// full factorial combinations

	DUP_Terms combos = terms.crossCombinations();

	DUP_Terms newTerms;

	newTerms.add(*this);
	newTerms.add(combos);

	for (uint i = 0; i < _terms.size(); i++)
	{
		for (uint j = 0; j < combos.size(); j++)
		{
			QStringList term = _terms.at(i).components();
			QStringList newTerm = combos.at(j).components();

			term.append(newTerm);
			newTerms.add(DUP_Term(term));
		}
	}

	newTerms.add(terms);

	return newTerms;
}

string DUP_Terms::asString() const
{
	if (_terms.size() == 0)
		return "";

	stringstream ss;

	ss << _terms.at(0).asString();

	for (size_t i = 1; i < _terms.size(); i++)
		ss << ", " << _terms.at(i).asString();

	return ss.str();
}

bool DUP_Terms::operator==(const DUP_Terms &terms) const
{
	return _terms == terms._terms;
}

bool DUP_Terms::operator!=(const DUP_Terms &terms) const
{
	return _terms != terms._terms;
}

void DUP_Terms::set(const QByteArray & array)
{
	QDataStream stream(array);

	if (stream.atEnd())
		throw exception();

	int count;
	stream >> count;

	clear();

	while ( ! stream.atEnd())
	{
		QStringList variable;
		stream >> variable;
		add(DUP_Term(variable));
	}
}

DUP_Term DUP_Terms::sortComponents(const DUP_Term &term) const
{
	QStringList components = term.components();
	std::sort(components.begin(), components.end(), [this](const QString &a, const QString &b) { return rankOf(a) < rankOf(b); });
	return DUP_Term(components);
}

int DUP_Terms::rankOf(const QString &component) const
{
	if (_parent == nullptr)
		return 0;

	int index = 0;

	for(const DUP_Term& compare : _parent->terms())
	{
		if (compare.asQString() == component)
			break;
		index++;
	}

	return index;
}

int DUP_Terms::termCompare(const DUP_Term &t1, const DUP_Term &t2) const
{
	if (_parent == nullptr)
		return 1;

	if (t1.size() < t2.size())
		return 1;
	if (t1.size() > t2.size())
		return -1;

	for (uint i = 0; i < t1.size(); i++)
	{
		int t1Rank = rankOf(t1.at(i));
		int t2Rank = rankOf(t2.at(i));

		if (t1Rank < t2Rank)
			return 1;
		if (t1Rank > t2Rank)
			return -1;
	}

	return 0;
}

bool DUP_Terms::termLessThan(const DUP_Term &t1, const DUP_Term &t2) const
{
	return termCompare(t1, t2) > 0;
}

void DUP_Terms::remove(const DUP_Terms &terms)
{
	for(const DUP_Term &term : terms)
	{
		vector<DUP_Term>::iterator itr = find(_terms.begin(), _terms.end(), term);
		if (itr != _terms.end())
			_terms.erase(itr);
	}
}

void DUP_Terms::remove(size_t pos, size_t n)
{
	vector<DUP_Term>::iterator itr = _terms.begin();

	for (size_t i = 0; i < pos && itr != _terms.end(); i++)
		itr++;

	for (; n > 0 && itr != _terms.end(); n--)
		_terms.erase(itr);
}

void DUP_Terms::replace(int pos, const DUP_Term &term)
{
	size_t pos_t = size_t(pos);
	if (pos_t <_terms.size())
	{
		remove(pos_t);
		insert(pos, term);
	}
}

bool DUP_Terms::discardWhatDoesntContainTheseComponents(const DUP_Terms &terms)
{
	bool changed = false;

	_terms.erase(
		std::remove_if(
			_terms.begin(),
			_terms.end(),
			[&](DUP_Term& existingTerm)
			{
				for (const string &str : existingTerm.scomponents())
					if (! terms.contains(str))
					{
						changed = true;
						return true;
					}

				return false;
			}
		),
		_terms.end()
	);

	return changed;
}

bool DUP_Terms::discardWhatDoesContainTheseComponents(const DUP_Terms &terms)
{
	bool changed = false;

	_terms.erase(
		std::remove_if(
			_terms.begin(),
			_terms.end(),
			[&](DUP_Term& existingTerm)
			{
				for (const DUP_Term &term : terms)
					for (const QString &component : term.components())
						if (existingTerm.contains(component))
						{
							changed			= true;
							return true;
						}


				return false;
			}),
		_terms.end()
	);

	return changed;
}

bool DUP_Terms::discardWhatDoesContainTheseTerms(const DUP_Terms &terms)
{
	bool changed = false;

	_terms.erase(
		std::remove_if(
			_terms.begin(),
			_terms.end(),
			[&](const DUP_Term& existingTerm)
			{
				for (const DUP_Term &term : terms)
					if (existingTerm.containsAll(term))
					{
						changed = true;
						return true;
					}

				return false;
			}),
		_terms.end()
	);

	return changed;
}

bool DUP_Terms::discardWhatIsntTheseTerms(const DUP_Terms &terms, DUP_Terms *discarded)
{
	bool changed = false;

	_terms.erase(
		std::remove_if(
			_terms.begin(),
			_terms.end(),
			[&](DUP_Term& term)
			{
				if (!term.asString().empty() && !terms.contains(term))
				{
					if (discarded != nullptr)
						discarded->add(term);

					changed = true;
					return true;
				}

				return false;
			}),
		_terms.end()
	);

	return changed;
}

void DUP_Terms::clear()
{
	_terms.clear();
}

size_t DUP_Terms::size() const
{
	return _terms.size();
}

const std::vector<DUP_Term> &DUP_Terms::terms() const
{
	return _terms;
}

DUP_Terms::const_iterator DUP_Terms::begin() const
{
	return _terms.begin();
}

DUP_Terms::const_iterator DUP_Terms::end() const
{
	return _terms.end();
}

void DUP_Terms::remove(const DUP_Term &term)
{
	vector<DUP_Term>::iterator itr = std::find(_terms.begin(), _terms.end(), term);
	if (itr != end())
		_terms.erase(itr);
}

QSet<int> DUP_Terms::replaceVariableName(const std::string & oldName, const std::string & newName)
{
	QSet<int> change;

	int i = 0;
	for(DUP_Term & t : _terms)
	{
		if (t.replaceVariableName(oldName, newName))
			change.insert(i);
		i++;
	}

	return change;
}
