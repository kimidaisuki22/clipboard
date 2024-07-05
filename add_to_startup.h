
#include <ShlObj.h>
#include <Windows.h>
#include <iostream>
#include <string>
#include <string_view>



bool add_to_start_up(std::string_view link_name) {
  std::string shortcutPath;
  std::string programPath;
  shortcutPath.resize(MAX_PATH);
  programPath.resize(MAX_PATH);

  // Get the path to the current program
  GetModuleFileName(NULL, programPath.data(), MAX_PATH);

  // Get the path to the Startup folder
  if (SHGetSpecialFolderPath(NULL, shortcutPath.data(), CSIDL_STARTUP, FALSE)) {
    shortcutPath = shortcutPath.data();
    // Append the program name to the Startup folder path
    shortcutPath += std::string("\\") + link_name.data();

    // Create the shortcut
    IShellLink *shellLink;
    HRESULT hr = CoInitialize(NULL);

    if (SUCCEEDED(hr)) {
      hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                            IID_IShellLink, (LPVOID *)&shellLink);

      if (SUCCEEDED(hr)) {
        shellLink->SetPath(programPath.data());

        IPersistFile *persistFile;
        hr =
            shellLink->QueryInterface(IID_IPersistFile, (LPVOID *)&persistFile);

        if (SUCCEEDED(hr)) {
          auto wshortcutPath = to_wstring(shortcutPath);
          persistFile->Save(wshortcutPath.c_str(), TRUE);
          //   persistFile->Save(L"D:/bc.lnk", TRUE);
          persistFile->Release();
        }

        shellLink->Release();
      }

      CoUninitialize();
    }

    if (SUCCEEDED(hr)) {
      std::cout << "Shortcut created successfully: " << shortcutPath
                << std::endl;
      return true;
    } else {
      std::cerr << "Failed to create shortcut." << std::endl;
    }
  } else {
    std::cerr << "Failed to retrieve Startup folder path." << std::endl;
  }
  return false;
}
