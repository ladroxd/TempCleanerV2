# Temp and Bin Cleaner V2

A simple one-click Windows temp and recycle bin cleaner that works on **any Windows PC**.

## Features

- ✅ **Universal compatibility** - Works on any Windows PC without hardcoded paths
- 🧹 Cleans user temp folder (%TEMP%)
- 🧹 Cleans Windows system temp folder (C:\Windows\Temp)
- 🧹 Cleans Windows Prefetch folder (C:\Windows\Prefetch)
- 🗑️ Empties Recycle Bin
- 🛡️ Safe - Only deletes temp file contents, never system directories
- 🔒 Handles access denied errors gracefully
- 📊 Shows detailed progress during cleaning

## How It Works

The cleaner uses Windows APIs to automatically detect the correct paths on any PC:
- `GetTempPath()` - Gets the current user's temp folder
- `GetWindowsDirectory()` - Gets the Windows installation directory
- `SHEmptyRecycleBin()` - Empties the recycle bin

No hardcoded paths means it works on any Windows installation, regardless of:
- Different user names
- Different drive letters
- Different Windows versions
- Different system configurations

## Usage

1. Download and run the executable
2. Click once and let it clean
3. Press Enter when done

**Note:** Some folders (like Windows\Temp and Prefetch) may require administrator rights to clean fully.

## Building from Source

This is a C++ Windows application. You can build it using:
- Visual Studio 2019 or later
- Windows SDK

Simply open the solution file (`.sln`) in Visual Studio and build.

## Contact

Made by Ladro
Email: znahairy@gmail.com
