#include "Utils.h"
#include <maya/MString.h>

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