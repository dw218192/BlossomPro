#include "Utils.h"

#include <QFile>
#include <QTextStream>
#include <maya/MString.h>
#include <maya/MGlobal.h>
#include <maya/MQtUtil.h>

MayaErrorException::MayaErrorException(MStatus const& status, char const* file, int line) : m_msg(file) {
	auto const str = status.errorString();
	auto const cstr = str.asChar();
	m_msg.push_back(':');
	m_msg.append(std::to_string(line));
	m_msg.push_back('\n');
	for (unsigned i = 0; i < str.length(); ++i) {
		m_msg.push_back(cstr[i]);
	}
}

void loadAndExecuteMelScript(char const* scriptFileName) {
	std::string script = loadResource(scriptFileName);
	if (!script.empty()) {
		MGlobal::executeCommand(script.c_str(), true, false);
	}
}

std::string loadResource(char const* path) {
	QString const filePath = QString{ ":/" } + path;
	QFile file{ filePath };
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		MGlobal::displayError(MQtUtil::toMString(QString{ "Failed to load " } + filePath));
		return "";
	}
	QString const script = QTextStream{ &file }.readAll();
	return script.toStdString();
}