#pragma once
#include <maya/MStatus.h>
#include <exception>

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
#define ERROR_MESSAGE(str) do { MGlobal::displayError(str); MGlobal::displayError(MString{"at line "} + __LINE__ + MString{" of "} + __FILE__); } while(false)
#define CHECK(status, ret) do { if(MFAIL(status)) { ERROR_MESSAGE((status).errorString()); return (ret); } } while(false)

struct MayaErrorException : public std::exception {
	MayaErrorException(MStatus const& status, char const* file, int line);
	char const* what() const override { return m_msg.c_str(); }
private:
	std::string m_msg;
};