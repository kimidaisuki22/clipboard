#include <Windows.h>
#include <iostream>
#include <optional>

HWND nextClipboardViewer = NULL;

std::optional<std::string> read_clipboard();
LRESULT CALLBACK ClipboardViewerCallback(HWND hWnd, UINT msg, WPARAM wParam,
                                         LPARAM lParam);
