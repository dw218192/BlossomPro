#pragma once
#include <maya/MStatus.h>
#include <maya/MString.h>
#include <maya/MPlug.h>
#include <maya/MObject.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MDGModifier.h>
#include <maya/MGlobal.h>

#include <exception>
#include <variant>


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

template <typename T, typename E = MStatus>
struct Result {
    explicit constexpr Result() noexcept = default;

    constexpr Result(T&& t) noexcept : m_data{ std::move(t) } {}
    constexpr Result(E&& e) noexcept : m_data{ std::move(e) } {}
    constexpr Result(T const& t) noexcept : m_data{ t } {}
    constexpr Result(E const& e) noexcept : m_data{ e } {}

    [[nodiscard]] constexpr auto valid() const noexcept { return std::holds_alternative<T>(m_data); }
    [[nodiscard]] constexpr auto value() const& -> const T& { return std::get<0>(m_data); }
    [[nodiscard]] constexpr auto value() & -> T& { return std::get<0>(m_data); }
    [[nodiscard]] constexpr auto value() && -> T&& { return std::move(std::get<0>(m_data)); }
    [[nodiscard]] constexpr auto error() const& -> const E& { return std::get<1>(m_data); }
    [[nodiscard]] constexpr auto error() && -> E&& { return std::move(std::get<1>(m_data)); }

private:
    std::variant<T, E> m_data;
};

std::string loadResource(char const* path);
void loadAndExecuteMelScript(char const* scriptFileName);

template<typename T>
MStatus updateAttr(MObject const& node, char const* attrName, T&& value) {
    MStatus status = MStatus::kSuccess;
    MFnDependencyNode const fnNode{ node };

    MPlug plug = fnNode.findPlug(attrName, false, &status);
    CHECK(status, status);
    {
        std::decay_t<T> test;
        status = plug.getValue(test);
        CHECK(status, status);

        if (test != value) {
            status = plug.setValue(std::forward<T>(value));
            CHECK(status, status);
        }
    }
    return status;
}

inline MStatus connectAttr(MObject const& from, char const* fromAttr, MObject const& to, char const* toAttr) {
    MStatus status = MStatus::kSuccess;
    MDGModifier dgModifier;
    MFnDependencyNode const fnFrom{ from }, fnTo{ to };
    MPlug const fromPlug = fnFrom.findPlug(fromAttr, false, &status);
    CHECK(status, status);

    MPlug const toPlug = fnTo.findPlug(toAttr, false, &status);
    CHECK(status, status);

    if(fromPlug.isArray() && toPlug.isArray()) {
        unsigned int const numElements = fromPlug.numElements();
        for (unsigned int i = 0; i < numElements; ++i) {
            MPlug fromElement = fromPlug.elementByLogicalIndex(i, &status);
            MPlug toElement = toPlug.elementByLogicalIndex(i, &status);
            dgModifier.connect(fromElement, toElement);
        }
    } else {
        dgModifier.connect(fromPlug, toPlug);
    }

    dgModifier.doIt();

    return status;
}

inline Result<MString> getName(MObject const& obj) {
    MStatus status = MStatus::kSuccess;
    MFnDependencyNode const fnBranchNode{ obj, &status };
    CHECK(status, status);
    MString ret = fnBranchNode.name(&status);
    CHECK(status, status);
    return ret;
}