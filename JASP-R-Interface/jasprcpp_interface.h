//
// Copyright (C) 2013-2017 University of Amsterdam
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef JASPRCPP_INTERFACE_H
#define JASPRCPP_INTERFACE_H

#include <QtCore/qglobal.h>
#include <boost/function.hpp>


#if defined(JASP_R_INTERFACE_LIBRARY)
#  define RBRIDGE_TO_JASP_INTERFACE Q_DECL_EXPORT
#else
#  define RBRIDGE_TO_JASP_INTERFACE Q_DECL_IMPORT
#endif

#ifdef __WIN32__
#define STDCALL __stdcall
#else
#define STDCALL
#endif

extern "C" {

struct RBridgeColumn {
	char* name;
	bool isScale;
	bool hasLabels;
	bool isOrdinal;
	double* doubles;
	int* ints;
	char** labels;
	int nbRows;
	int nbLabels;
} ;

struct RBridgeColumnDescription {
	int type;
	char* name;
	bool isScale;
	bool hasLabels;
	bool isOrdinal;
	char** labels;
	int nbLabels;
} ;

struct RBridgeColumnType {
	char* name;
	int type;
};

// Callbacks from jaspRCPP to rbridge
typedef RBridgeColumn* (STDCALL *ReadDataSetCB)(RBridgeColumnType* columns, int colMax);
typedef char** (STDCALL *ReadDataColumnNamesCB)(int *maxCol);
typedef RBridgeColumnDescription* (STDCALL *ReadDataSetDescriptionCB)(RBridgeColumnType* columns, int colMax);
typedef bool (STDCALL *RequestStateFileSourceCB)(const char **root, const char **relativePath);
typedef bool (STDCALL *RequestTempFileNameCB)(const char* extensionAsString, const char **root, const char **relativePath);
typedef bool (STDCALL *RunCallbackCB)(const char* in, int progress, const char** out);

struct RBridgeCallBacks {
	ReadDataSetCB readDataSetCB;
	ReadDataColumnNamesCB readDataColumnNamesCB;
	ReadDataSetDescriptionCB readDataSetDescriptionCB;
	RequestStateFileSourceCB requestStateFileSourceCB;
	RequestTempFileNameCB requestTempFileNameCB;
	RunCallbackCB runCallbackCB;
};


// Calls from rbridge to jaspRCPP
RBRIDGE_TO_JASP_INTERFACE void STDCALL jaspRCPP_init(const char* buildYear, const char* version, RBridgeCallBacks *calbacks);
RBRIDGE_TO_JASP_INTERFACE const char* STDCALL jaspRCPP_run(const char* name, const char* options, const char* perform, int ppi);
RBRIDGE_TO_JASP_INTERFACE const char* STDCALL jaspRCPP_check();
RBRIDGE_TO_JASP_INTERFACE const char* STDCALL jaspRCPP_saveImage(const char *name, const char *type, const int height, const int width, const int ppi);


} // extern "C"

#endif // JASPRCPP_INTERFACE_H
