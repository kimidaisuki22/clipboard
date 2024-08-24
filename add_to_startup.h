
#include <ShlObj.h>
#include <Windows.h>
#include <filesystem>
#include <iostream>
#include <string>
#include <string_view>

bool add_to_start_up(std::string_view link_name) {
  std::wstring shortcutPath;
  std::wstring programPath;
  shortcutPath.resize(MAX_PATH);
  programPath.resize(MAX_PATH);

  // Get the path to the current program
  GetModuleFileNameW(NULL, programPath.data(), MAX_PATH);

  // Get the path to the Startup folder
  if (SHGetSpecialFolderPathW(NULL, shortcutPath.data(), CSIDL_STARTUP,
                              FALSE)) {
    shortcutPath = shortcutPath.data();
    // Append the program name to the Startup folder path
    std::filesystem::path path{shortcutPath};
    path /= link_name;

    // Create the shortcut
    IShellLinkW *shellLink;
    HRESULT hr = CoInitialize(NULL);

    if (SUCCEEDED(hr)) {
      hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                            IID_IShellLinkW, (LPVOID *)&shellLink);

      if (SUCCEEDED(hr)) {
        shellLink->SetPath(programPath.data());

        IPersistFile *persistFile;
        hr =
            shellLink->QueryInterface(IID_IPersistFile, (LPVOID *)&persistFile);

        if (SUCCEEDED(hr)) {
          persistFile->Save(path.native().c_str(), TRUE);
          //   persistFile->Save(L"D:/bc.lnk", TRUE);
          persistFile->Release();
        }

        shellLink->Release();
      }

      CoUninitialize();
    }

    if (SUCCEEDED(hr)) {
      std::cout << "Shortcut created successfully: " << path << std::endl;
      return true;
    } else {
      std::cerr << "Failed to create shortcut." << std::endl;
    }
  } else {
    std::cerr << "Failed to retrieve Startup folder path." << std::endl;
  }
  return false;
}
