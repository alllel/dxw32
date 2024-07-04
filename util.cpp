#define STRICT
#include <windows.h>
#include <cstdlib>
#include "dxw.h"
#include <direct.h>
#include <ctype.h>
#include <cstring>

void
trim(char* str) {
  int i;
  for (i = 0; str[i]; ++i);
  for (--i; (i > 0) && (str[i] == ' '); --i) str[i] = 0;
}

void
SetDlgItemDouble(HWND hDlg, int ctrl, double val) { SetDlgItemText(hDlg, ctrl, gcvt(val, 5, buf)); }

void
GetDlgItemDouble(HWND hDlg, int ctrl, double& val) {
  double v;
  char* eptr;
  if (GetDlgItemText(hDlg, ctrl, buf, 20)) {
    v = strtod(buf, &eptr);
    if (*eptr != 0) {
      sprintf(buf + 21, "Wrong real value: %s,\nuse default", buf);
      MessageBox(hFrame, buf + 21, nullptr, MB_OK);
    } else {
      val = v;
    }
  }
}

static HCURSOR hCur = nullptr;

void
BeginWait() {
  hCur = SetCursor(LoadCursor(nullptr, IDC_WAIT));
}

void
EndWait() {
  SetCursor(hCur);
  hCur = nullptr;
}

int
ChDir(char* Dir) {
  int i;
  i = chdir(Dir);
  if (Dir[1] == ':') i = i || _chdrive(toupper(Dir[0]) - 'A' + 1);
  return i;
}

void
cbs(char* dir) { // enshures backslash at the end of dirname
  int l = strlen(dir);
  if (!l) {
    lstrcpy(dir, ".\\");
    return;
  }
  if (dir[l - 1] == '\\') return;
  if (dir[l - 1] == ':') {
    strcat(dir, ".\\");
    return;
  }
  strcat(dir, "\\");
  return;
}

OFN::OFN() {
  lStructSize       = sizeof(*this);
  hwndOwner         = hFrame;
  hInstance         = nullptr;
  lpstrFilter       = nullptr;
  lpstrCustomFilter = nullptr;
  nMaxCustFilter    = 0;
  nFilterIndex      = 1;
  lpstrFile         = nullptr;
  nMaxFile          = NULL;
  lpstrFileTitle    = nullptr;
  nMaxFileTitle     = 0;
  lpstrInitialDir   = nullptr;
  lpstrTitle        = nullptr;
  Flags             = OFN_HIDEREADONLY;
  lpstrDefExt       = nullptr;
  lCustData         = NULL;
  lpfnHook          = nullptr;
  lpTemplateName    = nullptr;
}

FileData::FileData() {
  lpstrFilter     = "Krenz experiments\0*.ixc\0";
  lpstrFile       = buf;
  nMaxFile        = sizeof(buf) - 1;
  lpstrInitialDir = Directory;
  Flags           = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
  lpstrDefExt     = "IXC";
}

FileData ofn;

int
DLG(int ID, DLGPROC Proc, LPARAM lp) {
  int ret;
  //Not necessary because smart callbacks used
  //FARPROC proc=MakeProcInstance((FARPROC)Proc,hInst);
  ret = DialogBoxParam(hInst, MAKEINTRESOURCE(ID), hFrame, Proc /*proc*/, lp);
  //Not necessary because smart callbacks used
  //FreeProcInstance(proc);
  return ret;
}

void
AlignWindow(HWND hwnd, int t, int l, int b, int r, int howy, int howx) //how: -1 - left 0 center 1 right
{
  RECT rc;
  GetWindowRect(hwnd, &rc);
  rc.right -= rc.left;
  rc.bottom -= rc.top;
  switch (howx) {
    case -1:
      rc.left = l;
      break;
    case 0:
      rc.left = (r + l - rc.right) / 2;
      break;
    case 1:
      rc.left = r - rc.right;
      break;
  }
  switch (howy) {
    case -1:
      rc.top = t;
      break;
    case 0:
      rc.top = (t + b - rc.bottom) / 2;
      break;
    case 1:
      rc.top = b - rc.bottom;
      break;
  }
  MoveWindow(hwnd, rc.left, rc.top, rc.right, rc.bottom, TRUE);
  ShowWindow(hwnd, SW_SHOW);
}
