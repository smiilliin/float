#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <mutex>
#include <windows.h>

using namespace std;

struct HwndTitle {
    HWND hwnd;
    wstring title;
};
#define PI 3.1415926535897932384626433832795028f
#define toRadian(x) (x * (PI / 180.f))

mutex angleSpeedLock;
float speed;
float angle;

void floatWindow(HWND hwnd, int taskbarSize) {
    RECT rect;
    GetWindowRect(hwnd, &rect);

    const int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    const int screenHeight = GetSystemMetrics(SM_CYSCREEN) - taskbarSize;
    const int windowWidth = rect.right - rect.left;
    const int windowHeight = rect.bottom - rect.top;
    float windowX = (float)rect.left;
    float windowY = (float)rect.top;

    const auto floatTarget = [&]() {
        SetWindowPos(hwnd, nullptr, (int)roundf(windowX), (int)roundf(windowY), rect.right - rect.left, rect.bottom - rect.top, SWP_NOZORDER | SWP_SHOWWINDOW);
    };
    const auto isCollideLeft = [&]() -> bool {
        return windowX <= 0;
    };
    const auto isCollideTop = [&]() -> bool {
        return windowY <= 0;
    };
    const auto isCollideRight = [&]() -> bool {
        return windowX + windowWidth >= screenWidth;
    };
    const auto isCollideBottom = [&]() -> bool {
        return windowY + windowHeight >= screenHeight;
    };

    const auto goToAngle = [&]() {
        windowX += cosf(angle) * speed;
        windowY -= sinf(angle) * speed;
    };

    while (true) {
        goToAngle();

        if (isCollideTop()) {
            angleSpeedLock.lock();
            angle = -angle;

            while (isCollideTop()) { 
                goToAngle();
            }
            angleSpeedLock.unlock();
        }
        if (isCollideBottom()) {
            angleSpeedLock.lock();
            angle = -angle;

            while (isCollideBottom()) {
                goToAngle();
            }
            angleSpeedLock.unlock();
        }
        if (isCollideLeft()) {
            angleSpeedLock.lock();
            angle = toRadian(180) - angle;

            while (isCollideLeft()) {
                goToAngle();
            }
            angleSpeedLock.unlock();
        }
        if (isCollideRight()) {
            angleSpeedLock.lock();
            angle = toRadian(180) - angle;

            while (isCollideRight()) {
                goToAngle();
            }
            angleSpeedLock.unlock();
        }

        floatTarget();
    }
}


int main() {
    wcout.imbue(locale(""));
    vector<HwndTitle> windows;

    int windowIndex = 0;
    for (HWND hwnd = GetTopWindow(nullptr); hwnd != nullptr; hwnd = GetNextWindow(hwnd, GW_HWNDNEXT))
    {
        if (!IsWindowVisible(hwnd)) {
            continue;
        }

        int length = GetWindowTextLength(hwnd);
        if (length == 0) {
            continue;
        }

        const wstring title(length, 0);
        GetWindowTextW(hwnd, (wchar_t*)&title[0], length + 1);

        wcout << windowIndex++ << ": " << title << endl;
        const HwndTitle hwndTitle = { hwnd, title };
        windows.push_back(hwndTitle);
    }

    int windowNumber;
    while (true) {
        cout << "What window you want to float?" << "(0~" << windows.size() - 1 << "): ";
        wcin >> windowNumber;

        if (windowNumber < 0 || windowNumber > windows.size()) {
            cout << "Wrong window number" << endl;
        }
        else {
            break;
        }
    }
    const HWND targetHwnd = windows[windowNumber].hwnd;
    angle = toRadian(45.f);
    speed = 0.1f;
    
    int taskbarSize = 0;

    HWND taskbarHwnd = FindWindow(L"Shell_TrayWnd", nullptr);
    if (taskbarHwnd) {
        RECT taskbarRect;
        GetWindowRect(taskbarHwnd, &taskbarRect);

        taskbarSize = taskbarRect.bottom - taskbarRect.top;
    }
    cout << "Direction (LEFT/RIGHT) key to change speed" << endl;
    cout << "Direction (DOWN/UP) key to change angle" << endl;

    thread t(floatWindow, targetHwnd, taskbarSize);
    t.detach();

    while (true) {
        if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
            angleSpeedLock.lock();
            speed -= 0.0005f;

            if (speed < 0) speed = 0;
            angleSpeedLock.unlock();
        }
        if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
            angleSpeedLock.lock();
            speed += 0.0005f;
            angleSpeedLock.unlock();
        }
        if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
            angleSpeedLock.lock();
            angle -= toRadian(0.01f);
            angleSpeedLock.unlock();
        }
        if (GetAsyncKeyState(VK_UP) & 0x8000) {
            angleSpeedLock.lock();
            angle += toRadian(0.01f);
            angleSpeedLock.unlock();
        }
    }


    return 0;
}
