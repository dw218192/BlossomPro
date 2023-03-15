#pragma once
#include <maya/MStatus.h>

#define HANDLE_EXCEPTION(func_call)\
do{\
try{\
func_call;\
}catch (MayaErrorException ex) {\
	MGlobal::displayError(ex.what());\
}\
catch (std::exception stdex) {\
	MGlobal::displayError(stdex.what());\
}}while(false)

#define MAYA_EXCEPTION(status) MayaErrorException { status, __FILE__, __LINE__ }
#define CHECK(status) do { if(!(status)) { MGlobal::displayError((status).errorString()); } } while(false) 
struct MayaErrorException {
	MayaErrorException(MStatus const& status, char const* file, int line);
	auto what() const -> char const* { return m_msg.c_str(); }
private:
	std::string m_msg;
};