#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#include <commctrl.h>
#include <string>
#include <atomic>
#include "resource.h"

static bool IsRunningAsAdmin() {
    BOOL elevated = FALSE;
    HANDLE token = NULL;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &token)) {
        TOKEN_ELEVATION te{};
        DWORD size = sizeof(te);
        if (GetTokenInformation(token, TokenElevation, &te, sizeof(te), &size))
            elevated = te.TokenIsElevated;
        CloseHandle(token);
    }
    return elevated != FALSE;
}

#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "comctl32.lib")

#define IDC_CLEAN_BTN    101
#define IDC_LOG_EDIT     102
#define IDC_STATUS_LBL   103
#define IDC_TITLE_LBL    104
#define IDC_SUBTITLE_LBL 105

#define IDM_FILE_CLEAN   201
#define IDM_FILE_EXIT    202
#define IDM_EDIT_CLEAR   203
#define IDM_HELP_ABOUT   204

#define WM_LOG_MESSAGE  (WM_USER + 1)
#define WM_CLEAN_DONE   (WM_USER + 2)

#define BG_COLOR  RGB(245, 246, 250)
#define ACCENT    RGB(50, 120, 220)

static HWND g_hWnd    = NULL;
static HWND g_hLog    = NULL;
static HWND g_hBtn    = NULL;
static HWND g_hStatus = NULL;
static std::atomic<bool> g_cleaning(false);
static LONG g_filesDeleted = 0;
static LONG g_errors = 0;

static HBRUSH g_hBgBrush = NULL;
static HFONT  g_hFontTitle = NULL;
static HFONT  g_hFontNormal = NULL;
static HFONT  g_hFontBtn = NULL;

void AppendLog(const std::wstring& text) {
    PostMessage(g_hWnd, WM_LOG_MESSAGE, 0, (LPARAM) new std::wstring(text));
}

void DeleteDirectory(const std::wstring& path) {
    WIN32_FIND_DATA fd;
    HANDLE hFind = FindFirstFile((path + L"\\*").c_str(), &fd);
    if (hFind == INVALID_HANDLE_VALUE) return;

    do {
        std::wstring name = fd.cFileName;
        if (name == L"." || name == L"..") continue;

        std::wstring full = path + L"\\" + name;
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            DeleteDirectory(full);
        } else {
            if (DeleteFile(full.c_str())) {
                InterlockedIncrement(&g_filesDeleted);
                AppendLog(L"  Deleted: " + full);
            } else {
                InterlockedIncrement(&g_errors);
                AppendLog(L"  Skipped: " + full);
            }
        }
    } while (FindNextFile(hFind, &fd));

    FindClose(hFind);
    RemoveDirectory(path.c_str());
}

DWORD WINAPI CleanThread(LPVOID) {
    g_filesDeleted = 0;
    g_errors       = 0;

    wchar_t tempPath[MAX_PATH];
    if (GetTempPath(MAX_PATH, tempPath)) {
        AppendLog(L"Cleaning Temp folder: " + std::wstring(tempPath));
        AppendLog(L"");
        DeleteDirectory(tempPath);
    }

    AppendLog(L"");
    AppendLog(L"Cleaning Windows Temp folder: C:\\Windows\\Temp");
    AppendLog(L"");
    DeleteDirectory(L"C:\\Windows\\Temp");

    AppendLog(L"");
    AppendLog(L"Cleaning Prefetch folder: C:\\Windows\\Prefetch");
    AppendLog(L"");
    DeleteDirectory(L"C:\\Windows\\Prefetch");

    AppendLog(L"");
    AppendLog(L"Emptying Recycle Bin...");
    HRESULT hr = SHEmptyRecycleBin(NULL, NULL,
        SHERB_NOCONFIRMATION | SHERB_NOPROGRESSUI | SHERB_NOSOUND);
    if (SUCCEEDED(hr) || hr == S_FALSE)
        AppendLog(L"  Recycle Bin emptied.");
    else
        AppendLog(L"  Recycle Bin: already empty or access denied.");

    PostMessage(g_hWnd, WM_CLEAN_DONE, 0, 0);
    return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        HMENU hMenuBar  = CreateMenu();
        HMENU hFile     = CreatePopupMenu();
        HMENU hEdit     = CreatePopupMenu();
        HMENU hHelp     = CreatePopupMenu();

        AppendMenu(hFile, MF_STRING, IDM_FILE_CLEAN, L"&Clean Now");
        AppendMenu(hFile, MF_SEPARATOR, 0, NULL);
        AppendMenu(hFile, MF_STRING, IDM_FILE_EXIT,  L"E&xit");

        AppendMenu(hEdit, MF_STRING, IDM_EDIT_CLEAR, L"&Clear Log");

        AppendMenu(hHelp, MF_STRING, IDM_HELP_ABOUT, L"&About");

        AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hFile, L"&File");
        AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hEdit, L"&Edit");
        AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hHelp, L"&Help");

        SetMenu(hWnd, hMenuBar);

        g_hBgBrush   = CreateSolidBrush(BG_COLOR);
        g_hFontTitle  = CreateFont(26, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
        g_hFontNormal = CreateFont(14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
        g_hFontBtn    = CreateFont(14, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE,
            DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
            CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");

        CreateWindow(L"STATIC", L"Temp & Bin Cleaner",
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            0, 18, 500, 34, hWnd, (HMENU)IDC_TITLE_LBL, NULL, NULL);

        CreateWindow(L"STATIC", L"Simple 1-click cleaner by Ladro",
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            0, 54, 500, 20, hWnd, (HMENU)IDC_SUBTITLE_LBL, NULL, NULL);

        g_hLog = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL |
            ES_MULTILINE | ES_READONLY | ES_AUTOVSCROLL,
            20, 84, 460, 240, hWnd, (HMENU)IDC_LOG_EDIT, NULL, NULL);

        g_hStatus = CreateWindow(L"STATIC", L"Ready. Press \"Clean Now\" to start.",
            WS_CHILD | WS_VISIBLE | SS_CENTER,
            20, 334, 460, 20, hWnd, (HMENU)IDC_STATUS_LBL, NULL, NULL);

        g_hBtn = CreateWindow(L"BUTTON", L"Clean Now",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON | WS_TABSTOP,
            175, 362, 150, 36, hWnd, (HMENU)IDC_CLEAN_BTN, NULL, NULL);

        SendDlgItemMessage(hWnd, IDC_TITLE_LBL,    WM_SETFONT, (WPARAM)g_hFontTitle,  TRUE);
        SendDlgItemMessage(hWnd, IDC_SUBTITLE_LBL,  WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
        SendMessage(g_hLog,    WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
        SendMessage(g_hStatus, WM_SETFONT, (WPARAM)g_hFontNormal, TRUE);
        SendMessage(g_hBtn,    WM_SETFONT, (WPARAM)g_hFontBtn,    TRUE);
        break;
    }

    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDM_FILE_CLEAN:
            if (!g_cleaning)
                SendMessage(hWnd, WM_COMMAND, IDC_CLEAN_BTN, 0);
            break;
        case IDM_FILE_EXIT:
            DestroyWindow(hWnd);
            break;
        case IDM_EDIT_CLEAR:
            SetWindowText(g_hLog, L"");
            break;
        case IDM_HELP_ABOUT:
            MessageBox(hWnd,
                L"Temp & Bin Cleaner V2\n"
                L"by Ladro\n\n"
                L"Cleans user Temp, Windows Temp,\n"
                L"Prefetch, and the Recycle Bin.",
                L"About", MB_OK | MB_ICONINFORMATION);
            break;
        }
        if (LOWORD(wParam) == IDC_CLEAN_BTN && !g_cleaning) {
            if (!IsRunningAsAdmin())
                MessageBox(hWnd,
                    L"For a full clean (including C:\\Windows\\Temp and C:\\Windows\\Prefetch), "
                    L"please run this app as Administrator.\n\n"
                    L"Cleaning will now proceed.",
                    L"Run as Administrator for Full Clean",
                    MB_OK | MB_ICONINFORMATION);
            g_cleaning = true;
            EnableWindow(g_hBtn, FALSE);
            SetWindowText(g_hLog, L"");
            SetWindowText(g_hStatus, L"Cleaning... please wait.");
            HANDLE hThread = CreateThread(NULL, 0, CleanThread, NULL, 0, NULL);
            if (hThread) CloseHandle(hThread);
        }
        break;

    case WM_LOG_MESSAGE: {
        std::wstring* pMsg = reinterpret_cast<std::wstring*>(lParam);
        int len = GetWindowTextLength(g_hLog);
        SendMessage(g_hLog, EM_SETSEL, len, len);
        SendMessage(g_hLog, EM_REPLACESEL, FALSE, (LPARAM)(pMsg->c_str()));
        SendMessage(g_hLog, EM_REPLACESEL, FALSE, (LPARAM)L"\r\n");
        delete pMsg;
        break;
    }

    case WM_CLEAN_DONE: {
        g_cleaning = false;
        EnableWindow(g_hBtn, TRUE);

        wchar_t status[128];
        swprintf_s(status, L"Done!  %ld file(s) deleted,  %ld skipped.", g_filesDeleted, g_errors);
        SetWindowText(g_hStatus, status);

        AppendLog(L"");
        AppendLog(L"============================");
        AppendLog(L"  Cleaning complete!");
        wchar_t summary[128];
        swprintf_s(summary, L"  Files deleted : %ld", g_filesDeleted);
        AppendLog(summary);
        swprintf_s(summary, L"  Skipped       : %ld", g_errors);
        AppendLog(summary);
        AppendLog(L"============================");
        break;
    }

    case WM_CTLCOLORSTATIC: {
        HDC hdc = (HDC)wParam;
        SetBkColor(hdc, BG_COLOR);
        // Give the title a distinct color
        if ((HWND)lParam == GetDlgItem(hWnd, IDC_TITLE_LBL))
            SetTextColor(hdc, ACCENT);
        else
            SetTextColor(hdc, RGB(50, 50, 60));
        return (LRESULT)g_hBgBrush;
    }

    case WM_ERASEBKGND: {
        RECT rc;
        GetClientRect(hWnd, &rc);
        FillRect((HDC)wParam, &rc, g_hBgBrush);
        return 1;
    }

    case WM_DESTROY:
        DeleteObject(g_hBgBrush);
        DeleteObject(g_hFontTitle);
        DeleteObject(g_hFontNormal);
        DeleteObject(g_hFontBtn);
        PostQuitMessage(0);
        break;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int nCmdShow) {
    INITCOMMONCONTROLSEX icc = { sizeof(icc), ICC_STANDARD_CLASSES };
    InitCommonControlsEx(&icc);

    WNDCLASSEX wc       = {};
    wc.cbSize           = sizeof(WNDCLASSEX);
    wc.lpfnWndProc      = WndProc;
    wc.hInstance        = hInstance;
    wc.hCursor          = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground    = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName    = L"TempBinCleanerV2";
    wc.hIcon            = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wc.hIconSm          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
    RegisterClassEx(&wc);

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    const int winW = 520, winH = 430;

    g_hWnd = CreateWindowEx(0, L"TempBinCleanerV2",
        L"Temp & Bin Cleaner",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX,
        (screenW - winW) / 2, (screenH - winH) / 2,
        winW, winH, NULL, NULL, hInstance, NULL);

    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}
