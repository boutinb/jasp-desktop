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

#include "jasprcpp.h"

using namespace std;

RInside *rinside;
ReadDataSetCB readDataSetCB;
ReadDataColumnNamesCB readDataColumnNamesCB;
ReadDataSetDescriptionCB readDataSetDescriptionCB;
RequestStateFileSourceCB requestStateFileSourceCB;
RequestTempFileNameCB requestTempFileNameCB;
RunCallbackCB runCallbackCB;

static const string NullString = "null";


extern "C" {
void STDCALL jaspRCPP_init(const char* buildYear, const char* version, RBridgeCallBacks* callbacks)
{
	rinside = new RInside();

	RInside &rInside = rinside->instance();

	rInside[".readDatasetToEndNative"] = Rcpp::InternalFunction(&jaspRCPP_readDataSetSEXP);
	rInside[".readDataSetHeaderNative"] = Rcpp::InternalFunction(&jaspRCPP_readDataSetHeaderSEXP);
	rInside[".callbackNative"] = Rcpp::InternalFunction(&jaspRCPP_callbackSEXP);
	rInside[".requestTempFileNameNative"] = Rcpp::InternalFunction(&jaspRCPP_requestTempFileNameSEXP);
	rInside[".requestStateFileNameNative"] = Rcpp::InternalFunction(&jaspRCPP_requestStateFileNameSEXP);
	static const char *baseCitationFormat = "JASP Team (%s). JASP (Version %s) [Computer software].";
	char baseCitation[200];
	sprintf(baseCitation, baseCitationFormat, buildYear, version);
	rInside[".baseCitation"] = baseCitation;

	rInside["jasp.analyses"] = Rcpp::List();
	rInside.parseEvalQNT("suppressPackageStartupMessages(library(\"JASP\"))");
	rInside.parseEvalQNT("suppressPackageStartupMessages(library(\"methods\"))");

	readDataSetCB = callbacks->readDataSetCB;
	readDataColumnNamesCB = callbacks->readDataColumnNamesCB;
	readDataSetDescriptionCB = callbacks->readDataSetDescriptionCB;
	requestStateFileSourceCB = callbacks->requestStateFileSourceCB;
	requestTempFileNameCB = callbacks->requestTempFileNameCB;
	runCallbackCB = callbacks->runCallbackCB;
}


const char* STDCALL jaspRCPP_run(const char *name, const char *options, const char *perform, int ppi)
{
	SEXP results;

	RInside &rInside = rinside->instance();

	rInside["name"] = name;
	rInside["options.as.json.string"] = options;
	rInside["perform"] = perform;
	rInside[".ppi"] = ppi;

	rInside.parseEval("run(name=name, options.as.json.string=options.as.json.string, perform)", results);

	static string str;
	str = Rcpp::as<string>(results);
	return str.c_str();
}

const char* STDCALL jaspRCPP_check()
{
	SEXP result = rinside->parseEvalNT("checkPackages()");
	static string staticResult;
	staticResult = Rf_isString(result) ? Rcpp::as<string>(result) : NullString;

	return staticResult.c_str();
}

const char* STDCALL jaspRCPP_saveImage(const char *name, const char *type, const int height, const int width, const int ppi)
{
	RInside &rInside = rinside->instance();

	rInside["plotName"] = name;
	rInside["format"] = type;

	rInside["height"] = height;
	rInside["width"] = width;
	rInside[".ppi"] = ppi;

	SEXP result = rinside->parseEvalNT("saveImage(plotName,format,height,width)");
	static string staticResult;
	staticResult = Rf_isString(result) ? Rcpp::as<string>(result) : NullString;

	return staticResult.c_str();
}

} // extern "C"

SEXP jaspRCPP_requestTempFileNameSEXP(SEXP extension)
{
	const char *root, *relativePath;
	string extensionAsString = Rcpp::as<string>(extension);

	if (!requestTempFileNameCB(extensionAsString.c_str(), &root, &relativePath))
		return R_NilValue;

	Rcpp::List paths;
	paths["root"] = root;
	paths["relativePath"] = relativePath;

	return paths;
}

SEXP jaspRCPP_requestStateFileNameSEXP()
{
	const char* root;
	const char* relativePath;

	if (!requestStateFileSourceCB(&root, &relativePath))
		return R_NilValue;

	Rcpp::List paths;
	paths["root"] = root;
	paths["relativePath"] = relativePath;

	return paths;
}


SEXP jaspRCPP_callbackSEXP(SEXP in, SEXP progress)
{
	string inStr = Rf_isNull(in) ? "null" : Rcpp::as<string>(in);
	int progressInt = Rf_isNull(progress) ? -1 : Rcpp::as<int>(progress);
	const char *out;
	bool ok = runCallbackCB(inStr.c_str(), progressInt, &out);
	if (ok)
	{
		return Rcpp::CharacterVector(out);
	}
	else
	{
		return 0;
	}
}

RBridgeColumnType* jaspRCPP_marshallSEXPs(SEXP columns, SEXP columnsAsNumeric, SEXP columnsAsOrdinal, SEXP columnsAsNominal, SEXP allColumns, int *colMax)
{
	map<string, ColumnType> columnsRequested;

	if (Rf_isLogical(allColumns) && Rcpp::as<bool>(allColumns))
	{
		char** columns = readDataColumnNamesCB(colMax);
		if (columns)
		{
			for (int i = 0; i < *colMax; i++)
				columnsRequested[columns[i]] = ColumnTypeUnknown;
		}
	}

	if (Rf_isString(columns))
	{
		vector<string> temp = Rcpp::as<vector<string> >(columns);
		for (size_t i = 0; i < temp.size(); i++)
			columnsRequested[temp.at(i)] = ColumnTypeUnknown;
	}

	if (Rf_isString(columnsAsNumeric))
	{
		vector<string> temp = Rcpp::as<vector<string> >(columnsAsNumeric);
		for (size_t i = 0; i < temp.size(); i++)
			columnsRequested[temp.at(i)] = ColumnTypeScale;
	}

	if (Rf_isString(columnsAsOrdinal))
	{
		vector<string> temp = Rcpp::as<vector<string> >(columnsAsOrdinal);
		for (size_t i = 0; i < temp.size(); i++)
			columnsRequested[temp.at(i)] = ColumnTypeOrdinal;
	}

	if (Rf_isString(columnsAsNominal))
	{
		vector<string> temp = Rcpp::as<vector<string> >(columnsAsNominal);
		for (size_t i = 0; i < temp.size(); i++)
			columnsRequested[temp.at(i)] = ColumnTypeNominal;
	}

	RBridgeColumnType* result = (RBridgeColumnType*)calloc(columnsRequested.size(), sizeof(RBridgeColumnType));
	int colNo = 0;
	for (auto const &columnRequested : columnsRequested)
	{
		result[colNo].name = strdup(columnRequested.first.c_str());
		result[colNo].type = columnRequested.second;
		colNo++;
	}
	*colMax = colNo;

	return result;
}

void freeRBridgeColumnType(RBridgeColumnType *columns, int colMax)
{
	for (int i = 0; i < colMax; i++)
		free(columns[i].name);

	free(columns);
}

Rcpp::DataFrame jaspRCPP_readDataSetSEXP(SEXP columns, SEXP columnsAsNumeric, SEXP columnsAsOrdinal, SEXP columnsAsNominal, SEXP allColumns)
{
	int colMax = 0;
	RBridgeColumnType* columnsRequested = jaspRCPP_marshallSEXPs(columns, columnsAsNumeric, columnsAsOrdinal, columnsAsNominal, allColumns, &colMax);
	RBridgeColumn* colResults = readDataSetCB(columnsRequested, colMax);
	freeRBridgeColumnType(columnsRequested, colMax);

	Rcpp::DataFrame dataFrame = Rcpp::DataFrame();

	if (colResults)
	{
		Rcpp::List list(colMax);
		Rcpp::CharacterVector columnNames;
		for (int i = 0; i < colMax; i++)
		{
			RBridgeColumn& colResult = colResults[i];
			columnNames.push_back(colResult.name);
			int maxRow = colResult.nbRows;
			if (colResult.isScale)
			{
				Rcpp::NumericVector v(maxRow);
				for (int j = 0; j < maxRow; j++)
					v[j] = colResult.doubles[j];

				list[i] = v;
			}
			else if (!colResult.hasLabels)
			{
				Rcpp::IntegerVector v(maxRow);
				for (int j = 0; j < maxRow; j++)
					v[j] = colResult.ints[j];
				list[i] = v;
			}
			else
			{
				Rcpp::IntegerVector v(maxRow);
				for (int j = 0; j < maxRow; j++)
					v[j] = colResult.ints[j];
				jaspRCPP_makeFactor(v, colResult.labels, colResult.nbLabels, colResult.isOrdinal);
				list[i] = v;
			}
		}
		list.attr("names") = columnNames;
		dataFrame = Rcpp::DataFrame(list);
	}

	return dataFrame;
}

Rcpp::DataFrame jaspRCPP_readDataSetHeaderSEXP(SEXP columns, SEXP columnsAsNumeric, SEXP columnsAsOrdinal, SEXP columnsAsNominal, SEXP allColumns)
{
	int colMax = 0;
	RBridgeColumnType* columnsRequested = jaspRCPP_marshallSEXPs(columns, columnsAsNumeric, columnsAsOrdinal, columnsAsNominal, allColumns, &colMax);
	RBridgeColumnDescription* columnsDescription = readDataSetDescriptionCB(columnsRequested, colMax);
	freeRBridgeColumnType(columnsRequested, colMax);

	Rcpp::DataFrame dataFrame = Rcpp::DataFrame();

	if (columnsDescription)
	{
		Rcpp::List list(colMax);
		Rcpp::CharacterVector columnNames;

		for (int i = 0; i < colMax; i++)
		{
			RBridgeColumnDescription& colDescription = columnsDescription[i];
			columnNames.push_back(colDescription.name);
			if (colDescription.isScale)
			{
				list(i) = Rcpp::NumericVector(0);
			}
			else if (!colDescription.hasLabels)
			{
				list(i) = Rcpp::IntegerVector(0);
			}
			else
			{
				Rcpp::IntegerVector v(0);
				jaspRCPP_makeFactor(v, colDescription.labels, colDescription.nbLabels, colDescription.isOrdinal);
				list(i) = v;
			}
		}

		list.attr("names") = columnNames;
		dataFrame = Rcpp::DataFrame(list);
	}

	return dataFrame;

}

void jaspRCPP_makeFactor(Rcpp::IntegerVector &v, char** levels, int nbLevels, bool ordinal)
{
	Rcpp::CharacterVector labels;
	for (int i = 0; i < nbLevels; i++)
		labels.push_back(levels[i]);

	v.attr("levels") = labels;
	vector<string> cla55;
	if (ordinal)
		cla55.push_back("ordered");
	cla55.push_back("factor");

	v.attr("class") = cla55;
}
