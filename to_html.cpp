#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>

bool GetClipboardHtml(const std::wstring& outputFile) {
    if (!OpenClipboard(NULL)) {
        std::cerr << "Failed to open clipboard" << std::endl;
        return false;
    }
    auto CF_HTML = RegisterClipboardFormatA("HTML format");
    HANDLE hData = GetClipboardData(CF_HTML);
    if (hData == NULL) {
        std::cerr << "Failed to get clipboard data" << std::endl;
        CloseClipboard();
        return false;
    }

    char* pszHtml = static_cast<char*>(GlobalLock(hData));
    if (pszHtml == NULL) {
        std::cerr << "Failed to lock clipboard data" << std::endl;
        CloseClipboard();
        return false;
    }

    std::ofstream outFile(outputFile);
    if (!outFile.is_open()) {
        std::cerr << "Failed to open output file" << std::endl;
        GlobalUnlock(hData);
        CloseClipboard();
        return false;
    }

    outFile.write(pszHtml, GlobalSize(hData));
    outFile.close();

    GlobalUnlock(hData);
    CloseClipboard();

    return true;
}

int main() {
    std::wstring outputFile = L"clipboard_html.html";

    if (GetClipboardHtml(outputFile)) {
        std::wcout << L"HTML content saved to " << outputFile << std::endl;
    } else {
        std::wcout << L"Failed to save HTML content" << std::endl;
    }

    return 0;
}