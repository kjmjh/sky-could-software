#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ID_INPUT 1
#define ID_BUTTON_SET 2
#define ID_BUTTON_CANCEL 3
#define MAX_INPUT_LENGTH 20
#define MAX_COMMAND_LENGTH 100

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hLabel;
    static HWND hInput;
    static HWND hButtonSet;
    static HWND hButtonCancel;

    switch (uMsg) {
        case WM_CREATE: {
            // 创建标签
            hLabel = CreateWindowEx(0, "STATIC", "请输入关机时间（秒）：", WS_CHILD | WS_VISIBLE, 10, 10, 200, 20, hwnd, NULL, NULL, NULL);
            if (!hLabel) {
                MessageBox(hwnd, "创建标签失败。", "错误", MB_OK | MB_ICONERROR);
                return -1;
            }

            // 创建输入框
            hInput = CreateWindowEx(0, "EDIT", "", WS_CHILD | WS_VISIBLE | WS_BORDER, 10, 40, 100, 25, hwnd, (HMENU)ID_INPUT, NULL, NULL);
            if (!hInput) {
                MessageBox(hwnd, "创建输入框失败。", "错误", MB_OK | MB_ICONERROR);
                return -1;
            }

            // 创建“设置关机”按钮
            hButtonSet = CreateWindowEx(0, "BUTTON", "设置关机", WS_CHILD | WS_VISIBLE, 120, 40, 100, 25, hwnd, (HMENU)ID_BUTTON_SET, NULL, NULL);
            if (!hButtonSet) {
                MessageBox(hwnd, "创建“设置关机”按钮失败。", "错误", MB_OK | MB_ICONERROR);
                return -1;
            }

            // 创建“取消关机”按钮
            hButtonCancel = CreateWindowEx(0, "BUTTON", "取消关机", WS_CHILD | WS_VISIBLE, 230, 40, 100, 25, hwnd, (HMENU)ID_BUTTON_CANCEL, NULL, NULL);
            if (!hButtonCancel) {
                MessageBox(hwnd, "创建“取消关机”按钮失败。", "错误", MB_OK | MB_ICONERROR);
                return -1;
            }
            break;
        }

        case WM_COMMAND:
            if (LOWORD(wParam) == ID_BUTTON_SET) {
                char buffer[MAX_INPUT_LENGTH];
                GetWindowText(hInput, buffer, sizeof(buffer));

                // 验证输入
                if (strlen(buffer) == 0) {
                    MessageBox(hwnd, "请输入一个时间。", "错误", MB_OK | MB_ICONERROR);
                    break;
                }

                int seconds = atoi(buffer);
                if (seconds <= 0) {
                    MessageBox(hwnd, "请输入一个正整数。", "错误", MB_OK | MB_ICONERROR);
                    break;
                }

                // 设置最大限制为24小时
                if (seconds > 86400) {
                    MessageBox(hwnd, "请输入一个不超过86400秒（24小时）的时间。", "错误", MB_OK | MB_ICONERROR);
                    break;
                }

                char command[MAX_COMMAND_LENGTH];
                sprintf_s(command, sizeof(command), "shutdown -s -t %d", seconds);
                if (system(command) != 0) {
                    MessageBox(hwnd, "执行关机命令失败。", "错误", MB_OK | MB_ICONERROR);
                } else {
                    MessageBox(hwnd, "关机已安排。", "信息", MB_OK);
                }
            } else if (LOWORD(wParam) == ID_BUTTON_CANCEL) {
                if (system("shutdown -a") != 0) {
                    MessageBox(hwnd, "取消关机失败。", "错误", MB_OK | MB_ICONERROR);
                } else {
                    MessageBox(hwnd, "关机已取消。", "信息", MB_OK);
                }
            }
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    const char CLASS_NAME[] = "ShutdownTimerClass";

    WNDCLASSA wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (!RegisterClassA(&wc)) {
        MessageBox(NULL, "窗口注册失败！", "错误", MB_OK | MB_ICONERROR);
        return 1;
    }

    HWND hwnd = CreateWindowExA(0, CLASS_NAME, "关机定时器", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 400, 150, NULL, NULL, hInstance, NULL);
    if (!hwnd) {
        MessageBox(NULL, "窗口创建失败！", "错误", MB_OK | MB_ICONERROR);
        return 1;
    }

    ShowWindow(hwnd, nShowCmd);

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}