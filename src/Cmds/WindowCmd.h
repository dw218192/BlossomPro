#pragma once
#include <maya/MPxCommand.h>
#include <maya/MQtUtil.h>
#include <QDialog>
#include <type_traits>
#include <maya/MGlobal.h>

template<typename T>
struct WindowCmd : public MPxCommand {
	static_assert(std::is_base_of_v<QDialog, T>, "window type must derive from QDialog");

	auto doIt(const MArgList& args)->MStatus override {
		try {
			if (!m_window) {
				m_window = new T{ nullptr }; //MQtUtil::mainWindow()
				m_window->open();
			} else {
				m_window->setVisible(true);
				m_window->raise();
			}
		} catch (...) {
			MGlobal::displayError("exception caught in WindowCMd::doIt");
		}
		return MStatus::kSuccess;
	}
	static auto creator() -> void* {
		return new WindowCmd;
	}
	static void cleanup() {
		delete m_window;
		MGlobal::displayInfo("free window");
	}
private:
	static inline T* m_window = nullptr;
};