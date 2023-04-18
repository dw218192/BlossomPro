#pragma once
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MPlug.h>
#include <maya/MObject.h>
#include <maya/MFnDependencyNode.h>

#include <exception>


#define HANDLE_EXCEPTION(func_call)\
do{\
try{\
func_call;\
}catch (MayaErrorException const& ex) {\
	MGlobal::displayError(ex.what());\
}\
catch (std::exception const& stdex) {\
	MGlobal::displayError(stdex.what());\
}}while(false)

#define MAYA_EXCEPTION(status) MayaErrorException { status, __FILE__, __LINE__ }
#define ERROR_MESSAGE(str) do { MGlobal::displayError(str); MGlobal::displayError(MString{"at line "} + __LINE__ + MString{" of "} + __FILE__); } while(false)
#define CHECK(status, ret) do { if(MFAIL(status)) { ERROR_MESSAGE((status).errorString()); return (ret); } } while(false)
#define STR(val) #val

struct MayaErrorException : public std::exception {
	MayaErrorException(MStatus const& status, char const* file, int line);
	char const* what() const override { return m_msg.c_str(); }
private:
	std::string m_msg;
};

inline bool operator==(std::string const& str, MString const& mstr) {
	return std::strcmp(str.c_str(), mstr.asChar()) == 0;
}
inline bool operator!=(std::string const& str, MString const& mstr) {
	return !(str == mstr);
}
inline bool operator==(MString const& mstr, std::string const& str) {
	return str == mstr;
}
inline bool operator!=(MString const& mstr, std::string const& str) {
	return str != mstr;
}

std::string loadResource(char const* path);
void loadAndExecuteMelScript(char const* scriptFileName);

template<typename T>
MStatus updateAttr(MObject const& node, char const* attrName, T&& value) {
    MStatus status = MStatus::kSuccess;
    MFnDependencyNode const fnNode{ node };

    MPlug plug = fnNode.findPlug(attrName, false, &status);
    CHECK(status, status);
    {
        T test;
        status = plug.getValue(test);
        CHECK(status, status);

        if (test != value) {
            status = plug.setValue(std::forward<T>(value));
            CHECK(status, status);
        }
    }
    return status;
}