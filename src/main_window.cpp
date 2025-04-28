﻿
#include <Windows.h>
#include <CommCtrl.h>

#include "main_window.h"

#include "win_dialogue.h"
#include "win_filesystem.h"
#include "path_utility.h"

CMainWindow::CMainWindow()
{

}

CMainWindow::~CMainWindow()
{

}

bool CMainWindow::Create(HINSTANCE hInstance, const wchar_t* pwzWindowName, HICON hIcon)
{
	WNDCLASSEXW wcex{};

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WindowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hCursor = ::LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = ::GetSysColorBrush(COLOR_BTNFACE);
	wcex.lpszClassName = m_pwzClassName;
	if (hIcon != nullptr)
	{
		wcex.hIcon = hIcon;
		wcex.hIconSm = hIcon;
	}

	if (::RegisterClassExW(&wcex))
	{
		m_hInstance = hInstance;

		UINT uiDpi = ::GetDpiForSystem();
		int iWindowWidth = ::MulDiv(200, uiDpi, USER_DEFAULT_SCREEN_DPI);
		int iWindowHeight = ::MulDiv(200, uiDpi, USER_DEFAULT_SCREEN_DPI);

		if (pwzWindowName != nullptr)m_pwzDefaultWindowName = pwzWindowName;
		m_hWnd = ::CreateWindowW(m_pwzClassName, m_pwzDefaultWindowName, WS_OVERLAPPEDWINDOW & ~WS_MINIMIZEBOX & ~WS_MAXIMIZEBOX & ~WS_THICKFRAME,
			CW_USEDEFAULT, CW_USEDEFAULT, iWindowWidth, iWindowHeight, nullptr, nullptr, hInstance, this);
		if (m_hWnd != nullptr)
		{
			return true;
		}
		else
		{
			std::wstring wstrMessage = L"CreateWindowW failed; code: " + std::to_wstring(::GetLastError());
			::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
		}
	}
	else
	{
		std::wstring wstrMessage = L"RegisterClassExW failed; code: " + std::to_wstring(::GetLastError());
		::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
	}

	return false;
}

int CMainWindow::MessageLoop()
{
	MSG msg;

	for (;;)
	{
		BOOL bRet = ::GetMessageW(&msg, 0, 0, 0);
		if (bRet > 0)
		{
			::TranslateMessage(&msg);
			::DispatchMessageW(&msg);
		}
		else if (bRet == 0)
		{
			/*ループ終了*/
			return static_cast<int>(msg.wParam);
		}
		else
		{
			/*ループ異常*/
			std::wstring wstrMessage = L"GetMessageW failed; code: " + std::to_wstring(::GetLastError());
			::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
			return -1;
		}
	}
	return 0;
}
/*C CALLBACK*/
LRESULT CMainWindow::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CMainWindow* pThis = nullptr;
	if (uMsg == WM_NCCREATE)
	{
		LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
		pThis = reinterpret_cast<CMainWindow*>(pCreateStruct->lpCreateParams);
		::SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
	}

	pThis = reinterpret_cast<CMainWindow*>(::GetWindowLongPtr(hWnd, GWLP_USERDATA));
	if (pThis != nullptr)
	{
		return pThis->HandleMessage(hWnd, uMsg, wParam, lParam);
	}

	return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
/*メッセージ処理*/
LRESULT CMainWindow::HandleMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CREATE:
		return OnCreate(hWnd);
	case WM_DESTROY:
		return OnDestroy();
	case WM_CLOSE:
		return OnClose();
	case WM_SIZE:
		return OnSize(wParam, lParam);
	case WM_PAINT:
		return OnPaint();
	case WM_ERASEBKGND:
		return 1;
	case WM_KEYDOWN:
		return OnKeyDown(wParam, lParam);
	case WM_KEYUP:
		return OnKeyUp(wParam, lParam);
	case WM_COMMAND:
		return OnCommand(wParam, lParam);
	case WM_MOUSEMOVE:
		return OnMouseMove(wParam, lParam);
	case WM_MOUSEWHEEL:
		return OnMouseWheel(wParam, lParam);
	case WM_LBUTTONDOWN:
		return OnLButtonDown(wParam, lParam);
	case WM_LBUTTONUP:
		return OnLButtonUp(wParam, lParam);
	case WM_MBUTTONUP:
		return OnMButtonUp(wParam, lParam);
	default:
		break;
	}

	return ::DefWindowProcW(hWnd, uMsg, wParam, lParam);
}
/*WM_CREATE*/
LRESULT CMainWindow::OnCreate(HWND hWnd)
{
	m_hWnd = hWnd;

	InitialiseMenuBar();

	m_pDxLibInit = std::make_unique<SDxLibInit>(m_hWnd);

	m_pKarinchanScenePlayer = std::make_unique<CKarinchanScenePlayer>();

	return 0;
}
/*WM_DESTROY*/
LRESULT CMainWindow::OnDestroy()
{
	::PostQuitMessage(0);

	return 0;
}
/*WM_CLOSE*/
LRESULT CMainWindow::OnClose()
{
	::DestroyWindow(m_hWnd);
	::UnregisterClassW(m_pwzClassName, m_hInstance);

	m_pKarinchanScenePlayer.reset();
	m_pDxLibInit.reset();

	return 0;
}
/*WM_PAINT*/
LRESULT CMainWindow::OnPaint()
{
	PAINTSTRUCT ps;
	HDC hdc = ::BeginPaint(m_hWnd, &ps);

	if (m_bPlayReady && m_pKarinchanScenePlayer.get() != nullptr)
	{
		m_pKarinchanScenePlayer->Update();

		DxLib::ClearDrawScreen();

		m_pKarinchanScenePlayer->Redraw();

		DxLib::ScreenFlip();

		::InvalidateRect(m_hWnd, nullptr, FALSE);
	}

	::EndPaint(m_hWnd, &ps);

	return 0;
}
/*WM_SIZE*/
LRESULT CMainWindow::OnSize(WPARAM wParam, LPARAM lParam)
{
	int iClientWidth = LOWORD(lParam);
	int iClientHeight = HIWORD(lParam);

	int iDesktopWidth = ::GetSystemMetrics(SM_CXSCREEN);
	int iDesktopHeight = ::GetSystemMetrics(SM_CYSCREEN);

	int iGraphWidth = iClientWidth < iDesktopWidth ? iClientWidth : iDesktopWidth;
	int iGraphHeight = iClientHeight < iDesktopHeight ? iClientHeight : iDesktopHeight;

	DxLib::SetGraphMode(iGraphWidth, iGraphHeight, 32);
	return 0;
}
/*WM_KEYDOWN*/
LRESULT CMainWindow::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case VK_RIGHT:
		if (m_pKarinchanScenePlayer.get() != nullptr)
		{
			if (!m_pKarinchanScenePlayer->HasReachedLastScene())
			{
				m_pKarinchanScenePlayer->ShiftScene(true);
			}
		}
		break;
	case VK_LEFT:
		if (m_pKarinchanScenePlayer.get() != nullptr)
		{
			m_pKarinchanScenePlayer->ShiftScene(false);
		}
		break;
	default:
		break;
	}

	return 0;
}
/*WM_KEYUP*/
LRESULT CMainWindow::OnKeyUp(WPARAM wParam, LPARAM lParam)
{
	switch (wParam)
	{
	case VK_ESCAPE:
		::PostMessage(m_hWnd, WM_CLOSE, 0, 0);
		break;
	case VK_UP:
		KeyOnForeFile();
		break;
	case VK_DOWN:
		KeyOnNextFile();
		break;
	case 'C':
		if (m_pKarinchanScenePlayer.get() != nullptr)
		{
			m_pKarinchanScenePlayer->ToggleTextColour();
		}
		break;
	case 'T':
		if (m_pKarinchanScenePlayer.get() != nullptr)
		{
			m_pKarinchanScenePlayer->SetTextVisibility(!m_pKarinchanScenePlayer->GetTextVisibility());
		}
		break;
	}
	return 0;
}
/*WM_COMMAND*/
LRESULT CMainWindow::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int wmId = LOWORD(wParam);
	int wmKind = LOWORD(lParam);
	if (wmKind == 0)
	{
		/*Menus*/
		switch (wmId)
		{
		case Menu::kOpenFolder:
			MenuOnOpenFolder();
			break;
		case Menu::kAudioSetting:

			break;
		case Menu::kFontSetting:

			break;
		}
	}
	else
	{
		/*Controls*/

	}

	return 0;
}
/*WM_MOUSEMOVE */
LRESULT CMainWindow::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
	WORD usKey = LOWORD(wParam);
	if (usKey == MK_LBUTTON)
	{
		if (m_bLeftCombinated)return 0;

		POINT pt{};
		::GetCursorPos(&pt);
		int iX = m_cursorPos.x - pt.x;
		int iY = m_cursorPos.y - pt.y;

		if (m_pKarinchanScenePlayer.get() != nullptr)
		{
			m_pKarinchanScenePlayer->MoveViewPoint(iX, iY);
		}

		m_cursorPos = pt;
		m_bLeftDragged = true;
	}

	return 0;
}
/*WM_MOUSEWHEEL*/
LRESULT CMainWindow::OnMouseWheel(WPARAM wParam, LPARAM lParam)
{
	int iScroll = -static_cast<short>(HIWORD(wParam)) / WHEEL_DELTA;
	WORD usKey = LOWORD(wParam);

	if (usKey == MK_LBUTTON)
	{
		if (m_pKarinchanScenePlayer.get() != nullptr)
		{
			m_pKarinchanScenePlayer->RescaleAnimationTime(iScroll > 0);
		}
	}
	else if (usKey == MK_RBUTTON)
	{
		if (m_pKarinchanScenePlayer.get() != nullptr)
		{
			m_pKarinchanScenePlayer->ShiftScene(iScroll > 0);
		}
	}
	else
	{
		if (m_pKarinchanScenePlayer.get() != nullptr)
		{
			m_pKarinchanScenePlayer->RescaleImage(iScroll > 0);
			if (!(usKey & MK_CONTROL))
			{
				ResizeWindow();
			}
		}
	}


	return 0;
}
/*WM_LBUTTONDOWN*/
LRESULT CMainWindow::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
	::GetCursorPos(&m_cursorPos);

	m_bLeftDowned = true;

	return 0;
}
/*WM_LBUTTONUP*/
LRESULT CMainWindow::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
	if (m_bLeftCombinated || m_bLeftDragged)
	{
		m_bLeftDragged = false;
		m_bLeftCombinated = false;
		m_bLeftDowned = false;

		return 0;
	}
	WORD usKey = LOWORD(wParam);
	if (usKey == MK_RBUTTON)
	{
		if (m_bBarHidden)
		{
			::PostMessage(m_hWnd, WM_SYSCOMMAND, SC_MOVE, 0);
			INPUT input{};
			input.type = INPUT_KEYBOARD;
			input.ki.wVk = VK_DOWN;
			::SendInput(1, &input, sizeof(input));

			m_bRightCombinated = true;
		}
	}

	m_bLeftDowned = false;

	return 0;
}
/*WM_RBUTTONUP*/
LRESULT CMainWindow::OnRButtonUp(WPARAM wParam, LPARAM lParam)
{
	if (m_bRightCombinated)
	{
		m_bRightCombinated = false;
		return 0;
	}

	return 0;
}
/*WM_MBUTTONUP*/
LRESULT CMainWindow::OnMButtonUp(WPARAM wParam, LPARAM lParam)
{
	WORD usKey = LOWORD(wParam);

	if (usKey == 0)
	{
		if (m_bPlayReady)
		{
			if (m_pKarinchanScenePlayer.get() != nullptr)
			{
				m_pKarinchanScenePlayer->ResetScale();
				ResizeWindow();
			}
		}
	}
	else if (usKey == MK_RBUTTON)
	{
		ToggleWindowBorderStyle();

		m_bRightCombinated = true;
	}

	return 0;
}
/*操作欄作成*/
void CMainWindow::InitialiseMenuBar()
{
	HMENU hMenuFile = nullptr;
	HMENU hMenuSetting = nullptr;
	HMENU hMenuBar = nullptr;
	BOOL iRet = FALSE;

	if (m_hMenuBar != nullptr)return;

	hMenuFile = ::CreateMenu();
	if (hMenuFile == nullptr)goto failed;

	iRet = ::AppendMenuA(hMenuFile, MF_STRING, Menu::kOpenFolder, "Open folder");
	if (iRet == 0)goto failed;

	//hMenuSetting = ::CreateMenu();
	//if (hMenuSetting == nullptr)goto failed;

	//iRet = ::AppendMenuA(hMenuSetting, MF_STRING, Menu::kAudioSetting, "Audio");
	//if (iRet == 0)goto failed;
	//iRet = ::AppendMenuA(hMenuSetting, MF_STRING, Menu::kFontSetting, "Font");
	//if (iRet == 0)goto failed;

	hMenuBar = ::CreateMenu();
	if (hMenuBar == nullptr) goto failed;

	iRet = ::AppendMenuA(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuFile), "File");
	if (iRet == 0)goto failed;
	//iRet = ::AppendMenuA(hMenuBar, MF_POPUP, reinterpret_cast<UINT_PTR>(hMenuSetting), "Setting");
	//if (iRet == 0)goto failed;

	iRet = ::SetMenu(m_hWnd, hMenuBar);
	if (iRet == 0)goto failed;

	m_hMenuBar = hMenuBar;

	return;

failed:
	std::wstring wstrMessage = L"Failed to create menu; code: " + std::to_wstring(::GetLastError());
	::MessageBoxW(nullptr, wstrMessage.c_str(), L"Error", MB_ICONERROR);
	if (hMenuFile != nullptr)
	{
		::DestroyMenu(hMenuFile);
	}
	if (hMenuSetting != nullptr)
	{
		::DestroyMenu(hMenuSetting);
	}
	if (hMenuBar != nullptr)
	{
		::DestroyMenu(hMenuBar);
	}
}

void CMainWindow::MenuOnOpenFolder()
{
	std::wstring wstrSelectedFolderPath = win_dialogue::SelectWorkFolder(m_hWnd);
	if (wstrSelectedFolderPath.empty())return;

	if (SetupScenario(wstrSelectedFolderPath))
	{
		m_folderPaths.clear();
		m_nFolderPathIndex = 0;
		win_filesystem::GetFilePathListAndIndex(wstrSelectedFolderPath, nullptr, m_folderPaths, &m_nFolderPathIndex);
	}

}

void CMainWindow::KeyOnNextFile()
{
	if (m_folderPaths.empty())return;

	++m_nFolderPathIndex;
	if (m_nFolderPathIndex >= m_folderPaths.size())m_nFolderPathIndex = 0;

	SetupScenario(m_folderPaths[m_nFolderPathIndex]);
}

void CMainWindow::KeyOnForeFile()
{
	if (m_folderPaths.empty())return;

	--m_nFolderPathIndex;
	if (m_nFolderPathIndex >= m_folderPaths.size())m_nFolderPathIndex = m_folderPaths.size() - 1;

	SetupScenario(m_folderPaths[m_nFolderPathIndex]);
}

bool CMainWindow::SetupScenario(const std::wstring& wstrFolderPath)
{
	bool bRet = false;
	if (m_pKarinchanScenePlayer.get() != nullptr)
	{
		bRet = m_pKarinchanScenePlayer->LoadScenario(wstrFolderPath);
		if (bRet)
		{
			ResizeWindow();
		}
	}

	if (bRet)
	{
		std::wstring wstrWindowTitle = path_utility::TruncateFilePath(wstrFolderPath);
		::SetWindowTextW(m_hWnd, wstrWindowTitle.c_str());
	}
	else
	{
		::SetWindowTextW(m_hWnd, m_pwzDefaultWindowName);
	}

	m_bPlayReady = bRet;

	return bRet;
}
/*表示形式変更*/
void CMainWindow::ToggleWindowBorderStyle()
{
	if (!m_bPlayReady)return;

	RECT rect;
	::GetWindowRect(m_hWnd, &rect);
	LONG lStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);

	m_bBarHidden ^= true;

	if (m_bBarHidden)
	{
		::SetWindowLong(m_hWnd, GWL_STYLE, lStyle & ~WS_CAPTION & ~WS_SYSMENU);
		::SetWindowPos(m_hWnd, nullptr, 0, 0, rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER);
		::SetMenu(m_hWnd, nullptr);
	}
	else
	{
		::SetWindowLong(m_hWnd, GWL_STYLE, lStyle | WS_CAPTION | WS_SYSMENU);
		::SetMenu(m_hWnd, m_hMenuBar);
	}

	ResizeWindow();
}
/*窓寸法変更*/
void CMainWindow::ResizeWindow()
{
	unsigned int uiWidth = 0;
	unsigned int uiHeight = 0;
	m_pKarinchanScenePlayer->GetStillImageSize(&uiWidth, &uiHeight);

	RECT rect;
	::GetWindowRect(m_hWnd, &rect);
	int iX = static_cast<int>(uiWidth);
	int iY = static_cast<int>(uiHeight);

	rect.right = iX + rect.left;
	rect.bottom = iY + rect.top;

	LONG lStyle = ::GetWindowLong(m_hWnd, GWL_STYLE);
	const auto IsWidowBarHidden = [&lStyle]()
		-> bool
		{
			return !((lStyle & WS_CAPTION) && (lStyle & WS_SYSMENU));
		};

	::AdjustWindowRect(&rect, lStyle, IsWidowBarHidden() ? FALSE : TRUE);
	::SetWindowPos(m_hWnd, HWND_TOP, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOMOVE | SWP_NOZORDER);
}
