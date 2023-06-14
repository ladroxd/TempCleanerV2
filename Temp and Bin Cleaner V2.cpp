#include <iostream>
#include <string>
#include <Windows.h>
#include <shellapi.h>

void DeleteDirectory(const std::wstring& directoryPath)
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
                    DeleteDirectory(filePath);
                }
                else
                {
                    if (!DeleteFile(filePath.c_str()))
                    {
                        DWORD error = GetLastError();
                        std::wcout << L"Error deleting file: " << filePath << L" (error code " << error << L")" << std::endl;
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
        std::wcout << L"Error deleting directory: " << directoryPath << L" (error code " << error << L")" << std::endl;
    }
    else
    {
        std::wcout << L"Deleted directory: " << directoryPath << std::endl;
    }
}

int main()
{
    std::wstring tempPath = L"C:\\Users\\znaha\\AppData\\Local\\Temp";

    DeleteDirectory(tempPath);

    SHEmptyRecycleBin(NULL, NULL, SHERB_NOCONFIRMATION | SHERB_NOPROGRESSUI | SHERB_NOSOUND);
    std::cout << "\n\n\n\n\n Cleaning Completed! \n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
    std::cout << " Simple 1 Click Temp & Bin cleaner made by Ladro \n\n" << " If you have any request consider contacting me at: ladrozje@gmail.com";
    std::cout << "\n\n\n\n\n\n";
    std::cout << "Press any key to close." << std::endl;
    getchar();

    return 0;



}