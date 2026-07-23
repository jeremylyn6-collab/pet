/*
 * Desktop Cockroach — a harmless, self-contained Windows prank.
 * Draws only into its own temporary transparent window.  It never reads or
 * writes user files, changes settings, starts child processes, or uses a
 * network API.
 */
#define UNICODE
#define _UNICODE
#include <windows.h>
#include <math.h>

#define APP_CLASS L"DesktopCockroachWindow"
#define TIMER_ID  1
#define FRAME_MS  16
#define LIFETIME_MS 30000
#define KEY_COLOR RGB(255, 0, 255)

static ULONGLONG started_at;
static int screen_left, screen_top, screen_width, screen_height;
static HWND window_handle;

static void line(HDC dc, int x1, int y1, int x2, int y2, int width) {
    HPEN pen = CreatePen(PS_SOLID, width, RGB(39, 22, 12));
    HGDIOBJ old = SelectObject(dc, pen);
    MoveToEx(dc, x1, y1, NULL); LineTo(dc, x2, y2);
    SelectObject(dc, old); DeleteObject(pen);
}

static void ellipse_fill(HDC dc, int l, int t, int r, int b, COLORREF color) {
    HBRUSH brush = CreateSolidBrush(color);
    HGDIOBJ old_brush = SelectObject(dc, brush);
    HGDIOBJ old_pen = SelectObject(dc, GetStockObject(NULL_PEN));
    Ellipse(dc, l, t, r, b);
    SelectObject(dc, old_pen); SelectObject(dc, old_brush); DeleteObject(brush);
}

static void draw_cockroach(HDC dc, double phase) {
    const int cx = 102, cy = 58;
    const int wiggle = (int)(sin(phase * 8.0) * 4.0);
    const int leg_a = (int)(sin(phase * 16.0) * 8.0);
    const int leg_b = (int)(sin(phase * 16.0 + 3.14159) * 8.0);

    /* Antennae and six alternating walking legs. */
    line(dc, cx - 34, cy - 9, cx - 58, cy - 29 + wiggle, 2);
    line(dc, cx - 34, cy - 6, cx - 55, cy + 13 - wiggle, 2);
    line(dc, cx - 8, cy - 15, cx - 28, cy - 36 + wiggle, 3);
    line(dc, cx - 6, cy - 4, cx - 32, cy - 10 + leg_a, 3);
    line(dc, cx - 2, cy + 10, cx - 29, cy + 30 + leg_b, 3);
    line(dc, cx + 17, cy - 15, cx + 34, cy - 33 + leg_b, 3);
    line(dc, cx + 19, cy - 3, cx + 45, cy - 9 + leg_a, 3);
    line(dc, cx + 16, cy + 11, cx + 43, cy + 29 + leg_b, 3);

    /* Dark abdomen, warm-brown wing covers, and small head. */
    ellipse_fill(dc, cx - 18, cy - 23, cx + 42, cy + 24, RGB(61, 31, 15));
    ellipse_fill(dc, cx - 18, cy - 20, cx + 15, cy + 20, RGB(116, 57, 22));
    ellipse_fill(dc, cx + 5, cy - 20, cx + 39, cy + 20, RGB(104, 47, 18));
    line(dc, cx + 4, cy - 17, cx + 4, cy + 18, 1);
    ellipse_fill(dc, cx - 39, cy - 15, cx - 14, cy + 15, RGB(48, 25, 13));
    ellipse_fill(dc, cx - 33, cy - 8, cx - 29, cy - 4, RGB(15, 10, 6));
}

static void update_position(void) {
    ULONGLONG elapsed = GetTickCount64() - started_at;
    if (elapsed >= LIFETIME_MS) { DestroyWindow(window_handle); return; }
    double progress = (double)elapsed / 28000.0;
    if (progress > 1.0) progress = 1.0;
    int x = screen_left - 220 + (int)((screen_width + 440) * progress);
    int y = screen_top + (int)(screen_height * 0.58 + sin(progress * 6.28318) * 36.0);
    SetWindowPos(window_handle, HWND_TOPMOST, x, y, 220, 120,
                 SWP_NOACTIVATE | SWP_SHOWWINDOW);
    InvalidateRect(window_handle, NULL, FALSE);
}

static LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    switch (message) {
    case WM_TIMER: update_position(); return 0;
    case WM_KEYDOWN: if (wparam == VK_ESCAPE) { DestroyWindow(hwnd); return 0; } break;
    case WM_ERASEBKGND: return 1;
    case WM_PAINT: {
        PAINTSTRUCT paint;
        HDC dc = BeginPaint(hwnd, &paint);
        RECT r; GetClientRect(hwnd, &r);
        HBRUSH background = CreateSolidBrush(KEY_COLOR);
        FillRect(dc, &r, background); DeleteObject(background);
        draw_cockroach(dc, (double)(GetTickCount64() - started_at) / 1000.0);
        EndPaint(hwnd, &paint);
        return 0;
    }
    case WM_DESTROY: KillTimer(hwnd, TIMER_ID); PostQuitMessage(0); return 0;
    }
    return DefWindowProc(hwnd, message, wparam, lparam);
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE previous, PWSTR command, int show) {
    (void)previous; (void)command; (void)show;
    screen_left = GetSystemMetrics(SM_XVIRTUALSCREEN);
    screen_top = GetSystemMetrics(SM_YVIRTUALSCREEN);
    screen_width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    screen_height = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    WNDCLASS wc = {0};
    wc.hInstance = instance; wc.lpszClassName = APP_CLASS; wc.lpfnWndProc = window_proc;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    if (!RegisterClass(&wc)) return 1;

    window_handle = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        APP_CLASS, L"", WS_POPUP, screen_left - 220, screen_top, 220, 120,
        NULL, NULL, instance, NULL);
    if (!window_handle) return 1;
    SetLayeredWindowAttributes(window_handle, KEY_COLOR, 0, LWA_COLORKEY);
    /* The process launched by the user owns focus, so Esc works immediately. */
    ShowWindow(window_handle, SW_SHOW);
    started_at = GetTickCount64();
    SetTimer(window_handle, TIMER_ID, FRAME_MS, NULL);
    SetFocus(window_handle);

    MSG message;
    while (GetMessage(&message, NULL, 0, 0)) { TranslateMessage(&message); DispatchMessage(&message); }
    return 0;
}
