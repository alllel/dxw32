#define STRICT

#include <windows.h>
#include "dxw.h"
#include "twnd.h"
#include <commdlg.h>
#include <commctrl.h>
#include <cstring>
#include <algorithm>

std::vector<std::shared_ptr<Window>> Window::all_;

Window::~Window() {
  auto it = std::ranges::find_if(all_, [this](std::shared_ptr<Window> const& w) { return w.get() == this; });
  if (it != all_.end()) all_.erase(it);
}

void
Window::Create(MDICREATESTRUCT& cs) {
  if (hWnd) return;
  hWnd = (HWND) SendMessage(hMDI, WM_MDICREATE, 0, (LPARAM) (LPMDICREATESTRUCT) &cs);
  if (hWnd) {
    all_.emplace_back(shared_from_this());
  } else {
    MessageBox(hFrame, cs.szTitle, "Cannot Create Window", MB_OK);
  }
}

void
Window::Destroy() {
  auto it = std::ranges::find(all_, hWnd, &Window::hWnd);
  if (it != all_.end()) {
    it->get()->hWnd = nullptr;
    all_.erase(it);
  }
}

DLGPROC(WSProc);

WINPROC(ChildWinProc) {
  auto W = Window::GetWindow(hWnd);
  if (!W) return DefMDIChildProc(hWnd, msg, wParam, lParam);
  Msg M(msg, wParam, lParam);
  switch (msg) {
    case WM_COMMAND:
      M.ret = W->Command(wParam);
      break;
    case WM_DESTROY:
      W->Destroy();
      break;
    case WM_PAINT:
      if (!IsIconic(hWnd)) {
        PAINTSTRUCT ps;
        RECT rc;
        BeginPaint(hWnd, &ps);
        GetClientRect(hWnd, &rc);
        W->Draw(ps.hdc, rc, Screen, &ps.rcPaint);
        EndPaint(hWnd, &ps);
      } else {
        M.ret = DefMDIChildProc(hWnd, msg, wParam, lParam);
      }
      break;
    default:
      if (!W->WinProc(M))
        M.ret = DefMDIChildProc(hWnd, msg, wParam, lParam);
  }
  return M.ret;
}

bool
Window::WinProc(Msg&) {
  return FALSE;
}

bool
Window::Command(WPARAM cmd) {
  switch (cmd) {
    case CM_WSIZE:
      DialogBoxParam(hInst, MAKEINTRESOURCE(ID_WSIZE), hFrame, (DLGPROC) WSProc, (LPARAM) hWnd);
      return TRUE;
    case CM_REST:
      if (!IsIconic(hWnd)) break;
      ShowWindow(hWnd, SW_RESTORE);
      return TRUE;
    case CM_COPY:
      ToClp();
      return TRUE;
    case CM_TO_FILE:
      ToFile();
      return TRUE;
    case CM_TO_PRINT:
      ToPrinter();
      return TRUE;
    case CM_FONT:
      if (SelFont()) InvalidateRect(hWnd, nullptr, TRUE);
      return TRUE;
    case CM_LINEWIDTH:
      if (SelLW()) InvalidateRect(hWnd, nullptr, TRUE);
      return TRUE;
    default:
      return FALSE;
  }
  return FALSE;
}

bool
Window::SelFont() {
  LOGFONT lf;
  CHOOSEFONT cf;
  memset(&cf, 0, sizeof(cf));
  cf.lStructSize = sizeof(cf);
  cf.hwndOwner   = hFrame;
  cf.iPointSize  = FontHeight;
  cf.Flags       = CF_SCREENFONTS | CF_TTONLY | CF_NOSCRIPTSEL | CF_INITTOLOGFONTSTRUCT;
  cf.nFontType   = FontType;
  cf.lpLogFont   = &lf;
  strcpy(lf.lfFaceName, FontName);
  HDC hdc     = GetDC(hWnd);
  lf.lfHeight = -(FontHeight * GetDeviceCaps(hdc, LOGPIXELSY)) / 720;
  ReleaseDC(hWnd, hdc);
  lf.lfWidth          = 0;
  lf.lfEscapement     = 0;
  lf.lfOrientation    = 0;
  lf.lfWeight         = FW_NORMAL;
  lf.lfItalic         = FALSE;
  lf.lfUnderline      = FALSE;
  lf.lfStrikeOut      = FALSE;
  lf.lfCharSet        = DEFAULT_CHARSET;
  lf.lfOutPrecision   = OUT_DEFAULT_PRECIS;
  lf.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
  lf.lfQuality        = DEFAULT_QUALITY;
  lf.lfPitchAndFamily = DEFAULT_PITCH;
  if (ChooseFont(&cf)) {
    strcpy(FontName, lf.lfFaceName);
    FontHeight = cf.iPointSize;
    FontType   = cf.nFontType & (BOLD_FONTTYPE | ITALIC_FONTTYPE);
    return TRUE;
  }
  return FALSE;
}

static int DlgLW;

DLGPROC(LWDlgProc) {
  switch (msg) {
    case WM_INITDIALOG:
      SetDlgItemText(hDlg, IDC_LW, gcvt(DlgLW * 0.1, 3, buf));
      SendDlgItemMessage(hDlg, IDC_SPIN, UDM_SETRANGE, 0, (LPARAM) MAKELONG(1000, 0));
      SendDlgItemMessage(hDlg, IDC_SPIN, UDM_SETPOS, 0, (LPARAM) MAKELONG(DlgLW, 0));
      return TRUE;
    case WM_VSCROLL:
      DlgLW = HIWORD(wParam);
      SetDlgItemText(hDlg, IDC_LW, gcvt(DlgLW * 0.1, 3, buf));
      return TRUE;
    case WM_COMMAND:
      switch (LOWORD(wParam)) {
        case IDOK:
          EndDialog(hDlg, 1);
          return TRUE;
        case IDCANCEL:
          EndDialog(hDlg, 0);
          return TRUE;
        case IDC_LW:
          if (HIWORD(wParam) == EN_KILLFOCUS) {
            GetDlgItemText(hDlg, IDC_LW, buf, sizeof(buf));
            DlgLW = (int) (atof(buf) * 10);
            SendDlgItemMessage(hDlg, IDC_SPIN, UDM_SETPOS, 0, (LPARAM) MAKELONG(DlgLW, 0));
          }
      }
      return FALSE;
    default:
      return FALSE;
  }
}

bool
Window::SelLW() {
  DlgLW = LineWidth;
  if (DLG(IDD_LW, LWDlgProc)) {
    LineWidth = DlgLW;
    return TRUE;
  }
  return FALSE;
}

struct MetaPict {
  HDC hdc;
  RECT rc;
  int width, height; //MM_HIMETRIC
  short int res;

  MetaPict(HWND hWnd, short int r);

  MetaPict(double w, double h, short int r);

  void Create();

  void ToClp();

  void ToFile(char* file);
};

static void
WindowSize(HWND hWnd, int& width, int& height) {
  RECT rc;
  HDC hdc = GetDC(hWnd);
  GetClientRect(hWnd, &rc);
  SetMapMode(hdc, MM_HIMETRIC);
  DPtoLP(hdc, (POINT*) (&rc), 2);
  ReleaseDC(hWnd, hdc);
  height = -rc.bottom;
  width  = rc.right;
}

inline MetaPict::MetaPict(HWND hWnd, short int r) {
  WindowSize(hWnd, width, height);
  res = r;
  Create();
}

inline MetaPict::MetaPict(double w, double h, short int r) : width(w), height(h), res(r) { Create(); }

void
MetaPict::Create() {
  rc.top = rc.left = 0;
  rc.right         = width / 2540.0 * res;
  rc.bottom        = height / 2540.0 * res;
  hdc              = CreateMetaFile(nullptr);
  SetWindowOrgEx(hdc, 0, 0, nullptr);
  SetWindowExtEx(hdc, rc.right, rc.bottom, nullptr);
}

void
MetaPict::ToClp() {
  HGLOBAL hMP      = GlobalAlloc(GHND, sizeof(METAFILEPICT));
  METAFILEPICT* MP = (METAFILEPICT*) GlobalLock(hMP);
  MP->xExt         = width;
  MP->yExt         = height;
  MP->mm           = MM_ANISOTROPIC;
  MP->hMF          = CloseMetaFile(hdc);
  GlobalUnlock(hMP);
  if (OpenClipboard(hFrame) && EmptyClipboard()) {
    SetClipboardData(CF_METAFILEPICT, hMP);
    CloseClipboard();
  } else {
    GlobalFree(hMP);
  }
}

static WORD
XOR(WORD* p, int l) {
  WORD s = 0;
  while (l) {
    s ^= *p;
    ++p;
    --l;
  }
  return s;
}

struct METAFILEHEADER {
  DWORD key;
  WORD hmf;
  SMALL_RECT bbox;
  WORD inch;
  DWORD reserved;
  WORD checksum;

  METAFILEHEADER() {
    key      = 0x9AC6CDD7L;
    hmf      = 0;
    reserved = 0;
  }

  void Check() { checksum = XOR((WORD*) this, 10); }
};

void
MetaPict::ToFile(char* file) {
  HMETAFILE hmf = CloseMetaFile(hdc);
  METAFILEHEADER MH;
  HFILE f        = _lcreat(file, 0);
  MH.bbox.Left   = rc.left;
  MH.bbox.Top    = rc.top;
  MH.bbox.Right  = rc.right;
  MH.bbox.Bottom = rc.bottom;
  MH.inch        = res;
  MH.Check();
  _lwrite(f, (LPCSTR) &MH, 22);
  UINT size  = GetMetaFileBitsEx(hmf, 0, nullptr);
  HGLOBAL hb = GlobalAlloc(GMEM_FIXED, size);
  size       = GetMetaFileBitsEx(hmf, size, (LPVOID) hb);
  _hwrite(f, (LPCSTR) hb, size);
  _lclose(f);
  GlobalFree(hb);
}

void
Window::ToClp() {
  MetaPict mp(hWnd, 600);
  Draw(mp.hdc, mp.rc, (DCtype) 600);
  mp.ToClp();
}

void
Window::ToFile() {
  char file[80];
  char dir[80];
  char* p;
  lstrcpy(dir, PicName());
  p = strrchr(dir, '\\');
  if (p) {
    *p = 0;
    strcpy(file, p + 1);
    p = dir;
  } else {
    strcpy(file, dir);
    p = nullptr;
  }
  OPENFILENAME opfn;
  opfn.lStructSize       = sizeof(opfn);
  opfn.hwndOwner         = hFrame;
  opfn.lpstrFilter       = "Picture\0*.wmf\0";
  opfn.lpstrCustomFilter = nullptr;
  opfn.nFilterIndex      = 1;
  opfn.lpstrFile         = file;
  opfn.nMaxFile          = 128;
  opfn.lpstrFileTitle    = nullptr;
  opfn.lpstrInitialDir   = p;
  opfn.lpstrTitle        = "Save Picture as";
  opfn.Flags             = OFN_NOCHANGEDIR | OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY;
  opfn.lpstrDefExt       = "wmf";

  MetaPict mp(hWnd, 600);

  if (GetSaveFileName(&opfn)) {
    Draw(mp.hdc, mp.rc, WMF);
    mp.ToFile(file);
  }
}

DLGPROC(WSProc) {
  unsigned w, h;
  BOOL err;
  RECT rc;
  static HWND hWnd = nullptr;
  switch (msg) {
    case WM_INITDIALOG:
      hWnd = (HWND) lParam;
      GetWindowRect(hWnd, &rc);
      SetDlgItemInt(hDlg, IDC_WS_W, rc.right - rc.left, FALSE);
      SetDlgItemInt(hDlg, IDC_WS_H, rc.bottom - rc.top, FALSE);
      return TRUE;
    case WM_COMMAND:
      switch (wParam) {
        case IDOK:
          w = GetDlgItemInt(hDlg, IDC_WS_W, &err, FALSE);
          if (!err) {
            MessageBox(hDlg, "Wrong width", nullptr, MB_OK);
            return TRUE;
          }
          h = GetDlgItemInt(hDlg, IDC_WS_H, &err, FALSE);
          if (!err) {
            MessageBox(hDlg, "Wrong height", nullptr, MB_OK);
            return TRUE;
          }
          SetWindowPos(hWnd, nullptr, 0, 0, w, h, SWP_NOMOVE | SWP_NOZORDER);
          EndDialog(hDlg, TRUE);
        case IDCANCEL:
          EndDialog(hDlg, FALSE);
        default:
          break;
      }
      return TRUE;
    default:
      break;
  }
  return FALSE;
}
