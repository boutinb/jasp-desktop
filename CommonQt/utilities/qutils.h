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

#ifndef QUTILS_H
#define QUTILS_H

#include <QJSValueIterator>
#include <QStringList>
#include <QQuickItem>
#include <QProcess>
#include <QJSValue>
#include <QString>
#include <QVector>
#include <QRegularExpression>
#include <QMap>
#include <QDir>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <json/json.h>
#include <QItemSelection>
#include "commonqtglobals.h"

/// This file collect a set of useful functions for interop between Qt and stdlib, like `fq` and `tq` for easily converting to and fro normal strings and whatnot
/// These could have been collected into a class but because we use `fq` and `tq` in so many places that would probably not have made our life easier anyway.

typedef std::vector<QString>				qstringvec;

enum Encryption { NoEncryption, SimpleCryptEncryption };

typedef std::set<QString>					qstringset;
typedef std::vector<QString>				qstringvec;

//fq -> fromQt, tq -> toQt
//tql -> toQtList (the tql is reasonable for going from a set to a vector because Qt no doubt has a QSet or something as well, which would then be reached through tq if we ever need it)
//fqj and tqj are for from/to QtJson
inline	COMMONQT_EXPORTS std::string						fq (const QString							 & from)	{ return from.toUtf8().toStdString(); }
		COMMONQT_EXPORTS std::vector<std::string>			fq (const QVector<QString>					 & vec);
		COMMONQT_EXPORTS std::vector<std::string>			fq (const std::vector<QString>				 & vec);
		COMMONQT_EXPORTS std::map<std::string, std::string>	fq (const QMap<QString, QString>			 & map);
		COMMONQT_EXPORTS QMap<QString, QString>				tq (const std::map<std::string, std::string> & map);
inline	COMMONQT_EXPORTS QString							tq (const std::string						 & from)	{ return QString::fromUtf8(from.c_str(), from.length()); }
		COMMONQT_EXPORTS QVector<QString>					tq (const std::vector<std::string>			 & vec);
inline	COMMONQT_EXPORTS QStringList						tql(const std::set<std::string>				 & from)	{ return tq(std::vector<std::string>(from.begin(), from.end())); }
inline	COMMONQT_EXPORTS QList<int>							tql(const std::set<int>						 & from)	{ return QList<int>(from.begin(), from.end()); }
		COMMONQT_EXPORTS std::set<std::string>				fql(const QStringList						 & from);

		//These need to have a different name because otherwise the default Json::Value(const std::string & str) constructor steals any fq(std::string()) call...
		COMMONQT_EXPORTS Json::Value						fqj(const QJSValue							 & jsVal);
		COMMONQT_EXPORTS QJSValue							tqj(const Json::Value						 & json,	const QQuickItem * qItem);
		COMMONQT_EXPORTS QPoint								minQModelIndex(const QItemSelection			 & list);
		COMMONQT_EXPORTS QPoint								maxQModelIndex(const QItemSelection			 & list);

template<typename T> inline		COMMONQT_EXPORTS std::vector<T>	fq(QVector<T>		in) { return in.toStdVector();				}
template<typename T> inline		COMMONQT_EXPORTS QVector<T>		tq(std::vector<T>	in) { return QVector<T>::fromStdVector(in); }

COMMONQT_EXPORTS QString stripFirstAndLastChar(QString in, const QString &strip);
COMMONQT_EXPORTS QString getShortCutKey();
COMMONQT_EXPORTS QString encrypt(const QString &input);
COMMONQT_EXPORTS QString decrypt(const QString &input);
COMMONQT_EXPORTS QString getSortableTimestamp();
COMMONQT_EXPORTS QString QJSErrorToString(QJSValue::ErrorType errorType);

COMMONQT_EXPORTS void	copyQDirRecursively(QDir copyThis, QDir toHere);

COMMONQT_EXPORTS QString shortenWinPaths(QString);

COMMONQT_EXPORTS bool pathIsSafeForR(const QString & checkThis);

COMMONQT_EXPORTS const char * QProcessErrorToString(QProcess::ProcessError error);

inline COMMONQT_EXPORTS std::ostream& operator<<(std::ostream& os, const QString & qStr) { return (os << qStr.toStdString()); }

#define GENERIC_SET_FUNCTION(WHAT_TO_SET, VARIABLE_TO_SET, EMIT_THIS, TYPE)	\
void set##WHAT_TO_SET(TYPE new##WHAT_TO_SET)								\
{																			\
	if(new##WHAT_TO_SET != VARIABLE_TO_SET)									\
	{																		\
		VARIABLE_TO_SET = new##WHAT_TO_SET;									\
		emit EMIT_THIS();													\
	}																		\
}



#endif // QUTILS_H
