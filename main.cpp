#define STRICT
#include <windows.h>
#include "dxw.h"

HINSTANCE hInst = 0;
HWND hFrame = 0, hMDI = 0;
HGLOBAL hDevNames = nullptr, hDevMode = nullptr;
HACCEL hAccel;

static void ShutDown();
static int MessageLoop(void);
static int FirstInit(HINSTANCE);
static int Init(int);

//ARGSUSED
int pascal
WinMain(HINSTANCE hInstance, HINSTANCE hPrev, LPSTR cmdline, int cmdshow) {
  hInst = hInstance;
  if (!hPrev)
    if (!FirstInit(hInstance))
      return -1;
  if (!Init(cmdshow))
    return -2;
  int ret = MessageLoop();

  ShutDown();
  return ret;
}

int
MessageLoop(void) {
  MSG msg;
  while (GetMessage(&msg, (HWND) nullptr, 0, 0)) {
    if (hInfo && IsDialogMessage(hInfo, &msg)) continue;
    if (hDig && IsDialogMessage(hDig, &msg)) continue;
    if (TranslateMDISysAccel(hMDI, &msg)) continue;
    if (TranslateAccelerator(hFrame, hAccel, &msg)) continue;
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }
  return msg.wParam;
}

int
FirstInit(HINSTANCE hInstance) {
  hInst = hInstance;
  WNDCLASS wc;
  wc.style         = 0;
  wc.lpfnWndProc   = MainWin;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = 0;
  wc.hInstance     = hInst;
  wc.hIcon         = LoadIcon(hInst, MAKEINTRESOURCE(ID_ICON));
  wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
  wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
  wc.lpszMenuName  = MAKEINTRESOURCE(ID_MENU);
  wc.lpszClassName = "DXWframe";
  if (!RegisterClass(&wc)) return FALSE;
  wc.style         = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc   = ChildWinProc;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = sizeof(HLOCAL);
  wc.hInstance     = hInst;
  wc.hIcon         = LoadIcon(hInst, MAKEINTRESOURCE(ID_ICON));
  wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
  wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
  wc.lpszMenuName  = nullptr;
  wc.lpszClassName = "DXWchannel";
  if (!RegisterClass(&wc)) return FALSE;
  wc.style         = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc   = ChildWinProc;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = sizeof(HLOCAL);
  wc.hInstance     = hInst;
  wc.hIcon         = LoadIcon(hInst, MAKEINTRESOURCE(ID_ICON));
  wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
  wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
  wc.lpszMenuName  = nullptr;
  wc.lpszClassName = "DXWspectr";
  if (!RegisterClass(&wc)) return FALSE;
  wc.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
  wc.lpfnWndProc   = ChildWinProc;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = sizeof(HLOCAL);
  wc.hInstance     = hInst;
  wc.hIcon         = LoadIcon(hInst, MAKEINTRESOURCE(ID_RT));
  wc.hCursor       = LoadCursor(nullptr, IDC_ARROW);
  wc.hbrBackground = (HBRUSH) GetStockObject(WHITE_BRUSH);
  wc.lpszMenuName  = nullptr;
  wc.lpszClassName = "DXWRT";
  if (!RegisterClass(&wc)) return FALSE;
  return TRUE;
}

int
Init(int cmdshow) {
  CLIENTCREATESTRUCT cs;
  HMENU hMenu;
  RECT rc;
  hFrame = CreateWindow("DXWframe", "DX for Windows",
                        WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_VISIBLE,
                        CW_USEDEFAULT, cmdshow,
                        CW_USEDEFAULT, CW_USEDEFAULT,
                        nullptr, nullptr, hInst, 0);
  if (!hFrame) return FALSE;
  hMenu           = GetMenu(hFrame);
  cs.hWindowMenu  = GetSubMenu(hMenu, 4);
  cs.idFirstChild = FIRST_WND;
  GetClientRect(hFrame, &rc);
  hMDI = CreateWindow("MDICLIENT", nullptr,
                      WS_CHILD | WS_HSCROLL | WS_VSCROLL | WS_VISIBLE,
                      0, 0, rc.right, rc.bottom,
                      hFrame, 0, hInst, &cs);
  if (!hMDI) return FALSE;
  GetDirs();
  //ChDir(MainDir);
  hAccel = LoadAccelerators(hInst, MAKEINTRESOURCE(ID_MENU));
  hpPnt  = CreatePen(PS_SOLID, 0, RGB(0, 255, 255));
  hpPts  = CreatePen(PS_SOLID, 0, RGB(255, 255, 0));
  hpCut  = CreatePen(PS_DOT, 0, RGB(0, 0, 0));
  hpImp  = CreatePen(PS_SOLID, 0, RGB(0, 255, 0));
  hpDef  = (HPEN) GetStockObject(BLACK_PEN);
  //Not necessary because smart callbacks used
  //hDigPr=MakeProcInstance((FARPROC)DigDlg,hInst);
  hbrNull  = (HBRUSH) GetStockObject(NULL_BRUSH);
  hbrBlack = (HBRUSH) GetStockObject(BLACK_BRUSH);
  hbrGray  = (HBRUSH) GetStockObject(GRAY_BRUSH);
  return TRUE;
}

void
ShutDown() {
  //Not necessary because smart callbacks used
  //FreeProcInstance(hDigPr);
  SaveDirs();
  DeleteObject(hpPnt);
  DeleteObject(hpPts);
  DeleteObject(hpCut);
  DeleteObject(hpImp);
}
