#ifndef ANALYSISINTERFACE_H
#define ANALYSISINTERFACE_H

#include <string>
#include <QString>

class AnalysisInterface
{
public:
	AnalysisInterface();

	virtual bool isOwnComputedColumn(const std::string &col) = 0;

	virtual void sendRScript(QString script, QString controlName, bool whiteListedVersion) = 0;
	virtual void refresh() = 0;
	virtual void exportResults() = 0;
	virtual bool isDuplicate() = 0;
};

#endif // ANALYSISINTERFACE_H
