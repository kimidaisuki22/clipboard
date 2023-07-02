#include "watch_clipboard.h"
#include <chrono>
#include <concurrentqueue/blockingconcurrentqueue.h>
#include <exception>
#include <filesystem>
#include <memory>
#include <optional>
#include <stdint.h>
#include <string>
#include <thread>

enum class Clipboard_type { text };
class Clip_data {
public:
  Clip_data(std::string str) : text_(str) {}
  Clipboard_type get_type() { return Clipboard_type::text; }
  std::string get_text() { return text_; }

private:
  std::string text_;
};
std::shared_ptr<moodycamel::BlockingConcurrentQueue<std::unique_ptr<Clip_data>>>
    g_queue_;
class ClipQueue {
public:
  ClipQueue(std::shared_ptr<
                moodycamel::BlockingConcurrentQueue<std::unique_ptr<Clip_data>>>
                queue = std::make_shared<moodycamel::BlockingConcurrentQueue<
                    std::unique_ptr<Clip_data>>>()) {
    queue_ = queue;
    g_queue_ = queue;
  }
  auto get_queue() { return queue_; }
  void run() {
    hwnd_ = create_wnd();
    nextClipboardViewer = SetClipboardViewer(hwnd_);

    MSG msg;
    while (GetMessage(&msg, hwnd_, 0, 0)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    ChangeClipboardChain(hwnd_, nextClipboardViewer);
  }

private:
  HWND create_wnd() {
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = 0;
    wcex.lpfnWndProc = ClipboardViewerCallback;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = GetModuleHandle(NULL);
    wcex.hIcon = NULL;
    wcex.hCursor = NULL;
    wcex.hbrBackground = NULL;
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = ("MyMessageOnlyWindow");
    wcex.hIconSm = NULL;

    if (!RegisterClassEx(&wcex)) {
      std::cerr << "Failed to register window class." << std::endl;
      return nullptr;
    }

    HWND hWnd = CreateWindowEx(0, ("MyMessageOnlyWindow"), (""), 0, 0, 0, 0, 0,
                               HWND_MESSAGE, NULL, GetModuleHandle(NULL), NULL);

    if (hWnd == NULL) {
      std::cerr << "Failed to create message-only window." << std::endl;
    }
    return hWnd;
  }

  HWND hwnd_;
  std::shared_ptr<
      moodycamel::BlockingConcurrentQueue<std::unique_ptr<Clip_data>>>
      queue_;
};

LRESULT CALLBACK ClipboardViewerCallback(HWND hWnd, UINT msg, WPARAM wParam,
                                         LPARAM lParam) {
  switch (msg) {
  case WM_DRAWCLIPBOARD: {

    std::cout << "Clipboard content has changed." << std::endl;
    auto text = read_clipboard();
    if (text) {
      auto data = std::make_unique<Clip_data>(text.value());
      g_queue_->enqueue(std::move(data));
    }

  } break;
  case WM_CHANGECBCHAIN:
    if (reinterpret_cast<HWND>(wParam) == nextClipboardViewer)
      nextClipboardViewer = reinterpret_cast<HWND>(lParam);
    else if (nextClipboardViewer != NULL)
      SendMessage(nextClipboardViewer, msg, wParam, lParam);
    break;
  }

  return DefWindowProc(hWnd, msg, wParam, lParam);
}

#include "add_to_startup.h"
#include <dirsystem/app_dirs.h>
#include <leveldb/db.h>

int main() {
  ClipQueue queue;

  add_to_start_up("clipboard.lnk");

  auto queue_t = [&queue] { queue.run(); };
  auto run_t = [&] {
    try {
      dirsystem::App_dirs dirs{"clipboards"};
      if (!std::filesystem::exists(dirs.data())) {

        std::cout << "create config dir: " << dirs.data() << "\n";
        std::filesystem::create_directories(dirs.data());
      }
      std::cout << "running on " << dirs.data() << "\n";
      leveldb::Options opt;
      opt.create_if_missing = true;
      leveldb::DB *db{};
      leveldb::DB::Open(opt, (dirs.data() / "clipboard_db").string(), &db);

      auto data_queue = queue.get_queue();
      std::unique_ptr<Clip_data> data;
      while (true) {
        data_queue->wait_dequeue(data);
        std::cout << data->get_text() << "\n";
        uint64_t t{};
        if (db) {
          t = std::chrono::high_resolution_clock::now()
                  .time_since_epoch()
                  .count();
          db->Put({}, std::to_string(t), data->get_text());
        }
      }
    } catch (std::exception &e) {
      std::cerr << e.what() << "\n";
    }
  };
  std::thread{queue_t}.detach();
  run_t();
  return 0;
}

std::optional<std::string> read_clipboard() {
  std::optional<std::string> content;
  if (!OpenClipboard(NULL)) {
    std::cerr << "Failed to open clipboard." << std::endl;
    return {};
  }

  HANDLE clipboardDataHandle = GetClipboardData(CF_TEXT);
  if (clipboardDataHandle == NULL) {
    std::cerr << "Failed to retrieve clipboard data." << std::endl;
    CloseClipboard();
    return {};
  }

  char *clipboardText = static_cast<char *>(GlobalLock(clipboardDataHandle));
  if (clipboardText == NULL) {
    std::cerr << "Failed to lock clipboard data." << std::endl;
    CloseClipboard();
    return {};
  }
  content = clipboardText;

  GlobalUnlock(clipboardDataHandle);
  CloseClipboard();
  return content;
}
