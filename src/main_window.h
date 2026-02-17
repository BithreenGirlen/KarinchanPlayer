#ifndef MAIN_WINDOW_H_
#define MAIN_WINDOW_H_

#include <Windows.h>

#include <string>
#include <vector>
#include <memory>

#include "dxlib_init.h"
#include "karinchan_scene_player.h"

class CMainWindow
{
public:
	CMainWindow();
	~CMainWindow();

	bool Create(HINSTANCE hInstance, const wchar_t* windowName = nullptr);
	int MessageLoop();

	HWND GetHwnd()const { return m_hWnd; }
private:
	const wchar_t* m_pClassName = L"Karin-chan player window";
	const wchar_t* m_pDefaultWindowName = L"Karin-chan player";

	HINSTANCE m_hInstance = nullptr;
	HWND m_hWnd = nullptr;

	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT OnCreate(HWND hWnd);
	LRESULT OnDestroy();
	LRESULT OnClose();
	LRESULT OnPaint();
	LRESULT OnSize(WPARAM wParam, LPARAM lParam);
	LRESULT OnKeyDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnKeyUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnCommand(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseMove(WPARAM wParam, LPARAM lParam);
	LRESULT OnMouseWheel(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonDown(WPARAM wParam, LPARAM lParam);
	LRESULT OnLButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnRButtonUp(WPARAM wParam, LPARAM lParam);
	LRESULT OnMButtonUp(WPARAM wParam, LPARAM lParam);

	struct Menu
	{
		enum
		{
			kOpenFolder = 1,
			kAudioSetting, kFontSetting,
		};
	};
	struct MenuBar { enum { kFile, kSetting }; };

	HMENU m_hMenuBar = nullptr;
	bool m_isFramelessWindow = false;

	POINT m_cursorPos{};
	bool m_wasLeftCombinated = false;
	bool m_wasLeftPressed = false;
	bool m_hasLeftBeenDragged = false;
	bool m_wasRightCombinated = false;

	void InitialiseMenuBar();

	void MenuOnOpenFolder();

	void KeyOnNextFile();
	void KeyOnForeFile();

	std::unique_ptr<SDxLibInit> m_pDxLibInit;
	std::unique_ptr<CKarinchanScenePlayer> m_pKarinchanScenePlayer;

	std::vector<std::wstring> m_folderPaths;
	size_t m_nFolderPathIndex = 0;

	bool SetupScenario(const std::wstring& wstrFolderPath);

	void ToggleWindowBorderStyle();
	void ResizeWindow();
};

#endif //MAIN_WINDOW_H_