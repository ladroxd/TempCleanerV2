#include <iostream>
#include <string>
#include <Windows.h>
#include <shellapi.h>
#include <shlobj.h>

// Forward declarations
void DeleteDirectoryRecursive(const std::wstring& directoryPath);
void DeleteDirectoryContents(const std::wstring& directoryPath);
void CleanDirectory(const std::wstring& path, const std::wstring& name);

void DeleteDirectoryContents(const std::wstring& directoryPath)
{
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile((directoryPath + L"\\*").c_str(), &findFileData);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            std::wstring fileName = findFileData.cFileName;
            if (fileName != L"." && fileName != L"..")
            {
                std::wstring filePath = directoryPath + L"\\" + fileName;
                if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    DeleteDirectoryRecursive(filePath);
                }
                else
                {
                    if (!DeleteFile(filePath.c_str()))
                    {
                        DWORD error = GetLastError();
                        if (error != ERROR_ACCESS_DENIED && error != ERROR_SHARING_VIOLATION)
                        {
                            std::wcout << L"Error deleting file: " << filePath << L" (error code " << error << L")" << std::endl;
                        }
                    }
                    else
                    {
                        std::wcout << L"Deleted file: " << filePath << std::endl;
                    }
                }
            }
        } while (FindNextFile(hFind, &findFileData));

        FindClose(hFind);
    }
}

void DeleteDirectoryRecursive(const std::wstring& directoryPath)
{
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile((directoryPath + L"\\*").c_str(), &findFileData);

    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            std::wstring fileName = findFileData.cFileName;
            if (fileName != L"." && fileName != L"..")
            {
                std::wstring filePath = directoryPath + L"\\" + fileName;
                if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    DeleteDirectoryRecursive(filePath);
                }
                else
                {
                    if (!DeleteFile(filePath.c_str()))
                    {
                        DWORD error = GetLastError();
                        if (error != ERROR_ACCESS_DENIED && error != ERROR_SHARING_VIOLATION)
                        {
                            std::wcout << L"Error deleting file: " << filePath << L" (error code " << error << L")" << std::endl;
                        }
                    }
                    else
                    {
                        std::wcout << L"Deleted file: " << filePath << std::endl;
                    }
                }
            }
        } while (FindNextFile(hFind, &findFileData));

        FindClose(hFind);
    }

    if (!RemoveDirectory(directoryPath.c_str()))
    {
        DWORD error = GetLastError();
        if (error != ERROR_ACCESS_DENIED && error != ERROR_DIR_NOT_EMPTY)
        {
            std::wcout << L"Error deleting directory: " << directoryPath << L" (error code " << error << L")" << std::endl;
        }
    }
    else
    {
        std::wcout << L"Deleted directory: " << directoryPath << std::endl;
    }
}

void CleanDirectory(const std::wstring& path, const std::wstring& name)
{
    if (GetFileAttributes(path.c_str()) != INVALID_FILE_ATTRIBUTES)
    {
        std::wcout << L"\nCleaning " << name << L": " << path << std::endl;
        DeleteDirectoryContents(path);
    }
    else
    {
        std::wcout << L"\nSkipping " << name << L" (not accessible or doesn't exist): " << path << std::endl;
    }
}

int main()
{
    std::wcout << L"==================================================" << std::endl;
    std::wcout << L"  Temp and Bin Cleaner V2 - Universal Edition" << std::endl;
    std::wcout << L"==================================================" << std::endl;
    std::wcout << L"\nStarting cleanup process...\n" << std::endl;

    // Get the path to the current user's temp folder
    wchar_t tempPath[MAX_PATH];
    if (GetTempPath(MAX_PATH, tempPath))
    {
        // Remove trailing backslash if present
        size_t len = wcslen(tempPath);
        if (len > 0 && tempPath[len - 1] == L'\\')
        {
            tempPath[len - 1] = L'\0';
        }
        CleanDirectory(tempPath, L"User Temp Folder");
    }
    else
    {
        std::wcerr << L"Failed to get user temp path!" << std::endl;
    }

    // Get Windows directory and clean Windows\Temp
    wchar_t windowsPath[MAX_PATH];
    if (GetWindowsDirectory(windowsPath, MAX_PATH))
    {
        std::wstring windowsTempPath = std::wstring(windowsPath) + L"\\Temp";
        CleanDirectory(windowsTempPath, L"Windows Temp Folder");

        // Clean Prefetch folder (requires admin rights)
        std::wstring prefetchPath = std::wstring(windowsPath) + L"\\Prefetch";
        CleanDirectory(prefetchPath, L"Windows Prefetch Folder");
    }
    else
    {
        std::wcerr << L"Failed to get Windows directory path!" << std::endl;
    }

    // Empty the recycle bin
    std::wcout << L"\nEmptying Recycle Bin..." << std::endl;
    HRESULT result = SHEmptyRecycleBin(NULL, NULL, SHERB_NOCONFIRMATION | SHERB_NOPROGRESSUI | SHERB_NOSOUND);
    if (SUCCEEDED(result))
    {
        std::wcout << L"Recycle Bin emptied successfully!" << std::endl;
    }
    else
    {
        std::wcout << L"Failed to empty Recycle Bin (may already be empty or require admin rights)" << std::endl;
    }

    std::wcout << L"\n\n==================================================" << std::endl;
    std::wcout << L"             Cleaning Completed!" << std::endl;
    std::wcout << L"==================================================" << std::endl;
    std::wcout << L"\n Simple 1 Click Temp & Bin cleaner made by Ladro" << std::endl;
    std::wcout << L" If you have any request consider contacting me at: ladrozje@gmail.com" << std::endl;
    std::wcout << L"\n NOTE: This cleaner works on any Windows PC." << std::endl;
    std::wcout << L" Some folders may require administrator rights to clean." << std::endl;
    std::wcout << L"\n\nPress Enter to exit." << std::endl;
    getchar();

    return 0;
}
