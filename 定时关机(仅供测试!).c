#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define ID_INPUT           1
#define ID_BUTTON_SET      2
#define ID_BUTTON_CANCEL   3
#define MAX_INPUT_LENGTH   20
#define MAX_COMMAND_LENGTH 100
#define TIMER_ID           1
#define TIMER_INTERVAL     1000 // 1 second

// Function prototypes
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void ScheduleShutdown(int seconds);
void CancelShutdown();
void UpdateButtonStates(HWND hwnd, HWND hButtonSet, HWND hButtonCancel);

// Global variable to track shutdown status
BOOL g_shutdownScheduled = FALSE;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
    const char CLASS_NAME[] = "ShutdownTimerClass";

    WNDCLASSA wc = {0};
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    if (!RegisterClassA(&wc)) {
        MessageBox(NULL, "窗口注册失败！", "错误", MB_OK | MB_ICONERROR);
        return 1;
    }

    HWND hwnd = CreateWindowExA(
        0,
        CLASS_NAME,
        "关机定时器",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 400, 180,
        NULL,
        NULL,
        hInstance,
        NULL
    );

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

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hLabel;
    static HWND hInput;
    static HWND hButtonSet;
    static HWND hButtonCancel;
    static HWND hStatus;

    switch (uMsg) {
        case WM_CREATE: {
            // 创建标签
            hLabel = CreateWindowExA(
                0,
                "STATIC",
                "请输入关机时间（秒）：",
                WS_CHILD | WS_VISIBLE,
                10, 10,
                200, 20,
                hwnd,
                NULL,
                NULL,
                NULL
            );
            if (!hLabel) {
                MessageBox(hwnd, "创建标签失败。", "错误", MB_OK | MB_ICONERROR);
                return -1;
            }

            // 创建输入框
            hInput = CreateWindowExA(
                0,
                "EDIT",
                "",
                WS_CHILD | WS_VISIBLE | WS_BORDER,
                10, 40,
                100, 25,
                hwnd,
                (HMENU)ID_INPUT,
                NULL,
                NULL
            );
            if (!hInput) {
                MessageBox(hwnd, "创建输入框失败。", "错误", MB_OK | MB_ICONERROR);
                return -1;
            }

            // 创建“设置关机”按钮
            hButtonSet = CreateWindowExA(
                0,
                "BUTTON",
                "设置关机",
                WS_CHILD | WS_VISIBLE,
                120, 40,
                100, 25,
                hwnd,
                (HMENU)ID_BUTTON_SET,
                NULL,
                NULL
            );
            if (!hButtonSet) {
                MessageBox(hwnd, "创建“设置关机”按钮失败。", "错误", MB_OK | MB_ICONERROR);
                return -1;
            }

            // 创建“取消关机”按钮
            hButtonCancel = CreateWindowExA(
                0,
                "BUTTON",
                "取消关机",
                WS_CHILD | WS_VISIBLE,
                230, 40,
                100, 25,
                hwnd,
                (HMENU)ID_BUTTON_CANCEL,
                NULL,
                NULL
            );
            if (!hButtonCancel) {
                MessageBox(hwnd, "创建“取消关机”按钮失败。", "错误", MB_OK | MB_ICONERROR);
                return -1;
            }

            // 创建状态标签
            hStatus = CreateWindowExA(
                0,
                "STATIC",
                "",
                WS_CHILD | WS_VISIBLE,
                10, 80,
                380, 20,
                hwnd,
                NULL,
                NULL,
                NULL
            );
            if (!hStatus) {
                MessageBox(hwnd, "创建状态标签失败。", "错误", MB_OK | MB_ICONERROR);
                return -1;
            }

            // 初始化按钮状态
            UpdateButtonStates(hwnd, hButtonSet, hButtonCancel);
            break;
        }

        case WM_COMMAND: {
            if (LOWORD(wParam) == ID_BUTTON_SET) {
                char buffer[MAX_INPUT_LENGTH];
                GetWindowTextA(hInput, buffer, sizeof(buffer));

                // 输入验证
                if (strlen(buffer) == 0) {
                    MessageBox(hwnd, "请输入一个时间。", "错误", MB_OK | MB_ICONERROR);
                    break;
                }

                // 检查是否为纯数字
                for (int i = 0; i < strlen(buffer); i++) {
                    if (!isdigit(buffer[i])) {
                        MessageBox(hwnd, "请输入一个有效的正整数。", "错误", MB_OK | MB_ICONERROR);
                        break;
                    }
                }

                int seconds = atoi(buffer);
                if (seconds <= 0) {
                    MessageBox(hwnd, "请输入一个正整数。", "错误", MB_OK | MB_ICONERROR);
                    break;
                }

                if (seconds > 86400) {
                    MessageBox(hwnd, "请输入一个不超过86400秒（24小时）的时间。", "错误", MB_OK | MB_ICONERROR);
                    break;
                }

                // 设置关机
                ScheduleShutdown(seconds);
                SetWindowTextA(hStatus, "关机已安排。");
                UpdateButtonStates(hwnd, hButtonSet, hButtonCancel);
            } else if (LOWORD(wParam) == ID_BUTTON_CANCEL) {
                CancelShutdown();
                SetWindowTextA(hStatus, "关机已取消。");
                UpdateButtonStates(hwnd, hButtonSet, hButtonCancel);
            }
            break;
        }

        case WM_TIMER: {
            // 检查是否有挂起的关机命令
            // 如果需要，可以在这里添加额外的逻辑
            break;
        }

        case WM_DESTROY: {
            // 取消任何挂起的关机
            CancelShutdown();
            PostQuitMessage(0);
            break;
        }

        default:
            return DefWindowProcA(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

void ScheduleShutdown(int seconds) {
    char command[MAX_COMMAND_LENGTH];
    sprintf_s(command, sizeof(command), "shutdown -s -t %d", seconds);
    if (system(command) != 0) {
        MessageBox(NULL, "执行关机命令失败。", "错误", MB_OK | MB_ICONERROR);
    } else {
        g_shutdownScheduled = TRUE;
    }
}

void CancelShutdown() {
    if (g_shutdownScheduled) {
        if (system("shutdown -a") != 0) {
            MessageBox(NULL, "取消关机失败。", "错误", MB_OK | MB_ICONERROR);
        } else {
            g_shutdownScheduled = FALSE;
        }
    }
}

void UpdateButtonStates(HWND hwnd, HWND hButtonSet, HWND hButtonCancel) {
    if (g_shutdownScheduled) {
        EnableWindow(hButtonSet, FALSE);
        EnableWindow(hButtonCancel, TRUE);
    } else {
        EnableWindow(hButtonSet, TRUE);
        EnableWindow(hButtonCancel, FALSE);
    }
}