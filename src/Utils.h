#pragma once
#define HANDLE_EXCEPTION(func_call)\
do{\
try{\
func_call;\
}catch (MStatus status) {\
status.pAPIerror(__FILE__, __LINE__);\
}\
catch (std::runtime_error rerr) {\
	MGlobal::displayError(rerr.what());\
}}while(false)