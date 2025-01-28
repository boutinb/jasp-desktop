#ifndef QMLUTILS_H
#define QMLUTILS_H

#include <QObject>
#include <QJSValue>
#include <QQuickItem>
#include <QDir>
#include "commonqtglobals.h"

struct qmlLoadError  : public std::runtime_error
{
	qmlLoadError(std::string msg) : std::runtime_error(msg) {}
	const char* what() const noexcept override;
};

/// Simply links through utilities for use in and around QML
class COMMONQT_EXPORTS QmlUtils : public QObject
{
	Q_OBJECT
public:
	explicit QmlUtils(QObject *parent = nullptr);

	static void setGlobalPropertiesInQMLContext(QQmlContext * ctxt);

#ifdef linux
// Functions for qml cache bug workaround on linux
public:
	static void configureQMLCacheDir();
private:
	static QDir generateQMLCacheDir();
#endif


public slots:
	QString		encodeAllColumnNames(	const QString	& str);
	QString		decodeAllColumnNames(	const QString	& str);

	QJSValue	encodeJson(				const QJSValue	& val, QQuickItem * caller);
	QJSValue	decodeJson(				const QJSValue	& val, QQuickItem * caller);

};

COMMONQT_EXPORTS QObject * instantiateQml(							const QUrl 	& filePath, const std::string & moduleName,																		QQmlContext * ctxt = nullptr);
COMMONQT_EXPORTS QObject * instantiateQml(const QString 	& qmlTxt, 	const QUrl & url, 		const std::string & moduleName, const std::string & whatAmILoading, const std::string & filename, 	QQmlContext * ctxt = nullptr);

#endif // QMLUTILS_H
