#include <engine/Window.hpp>
#include "FileSelectWidget.hpp"
#define GLFW_EXPOSE_NATIVE_WIN32
#include <glfw/include/GLFW/glfw3.h>
#include <GLFW/include/GLFW/glfw3native.h>
#include <Windows.h>
//#include <Windows.h>
//#include <commdlg.h>
//#include <customImguiWidgets.hpp>
//#include <engine/window.hpp>
//#include <engine/frameAllocator.hpp>
//#include <math/utils.hpp>

using namespace Gui;
#include <filesystem>

static HWND getHwnd() {
	return reinterpret_cast<HWND>(glfwGetWin32Window(reinterpret_cast<GLFWwindow*>(Window::handle())));
}

auto Gui::openFileSelect() -> std::optional<std::string_view> {
	static char path[MIN_FILE_SELECT_STRING_LENGTH] = "";
	
	OPENFILENAMEA openFile{
		.lStructSize = sizeof(OPENFILENAMEA),
		.hwndOwner = getHwnd(),
		.lpstrFilter = nullptr,
		.nFilterIndex = 1,
		.lpstrFile = path,
		.nMaxFile = sizeof(path),
		.lpstrFileTitle = nullptr,
		.nMaxFileTitle = 0,
		.lpstrInitialDir = nullptr,
		.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER
	};

	if (GetOpenFileNameA(&openFile)) {
		return path;
	} else {
		ASSERT(CommDlgExtendedError() == 0);
	}

	return std::nullopt;
}

auto Gui::openFileSave() -> std::optional<std::string_view> {
	static char path[MIN_FILE_SELECT_STRING_LENGTH] = "";
	OPENFILENAMEA openFile{
		.lStructSize = sizeof(OPENFILENAMEA),
		.hwndOwner = getHwnd(),
		.lpstrFilter = nullptr,
		.nFilterIndex = 1,
		.lpstrFile = path,
		.nMaxFile = sizeof(path),
		.lpstrFileTitle = nullptr,
		.nMaxFileTitle = 0,
		.lpstrInitialDir = nullptr,
		.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER,
	};

	if (!GetSaveFileNameA(&openFile)) {
		ASSERT(CommDlgExtendedError() == 0);
		return std::nullopt;
	}

	return path;
}