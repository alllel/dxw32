#define STRICT
#include <windows.h>
#include "dxw.h"
#include "gauge.h"
#include <cstring>
//#include <dir.h>
#include <direct.h>
#include <ctype.h>
#include <lzexpand.h>

typedef enum { Pre   = 0,
               A_B   = 1,
               Delay = 2 } ChType;
static enum CS { SF = 0,
                 LF = 1,
                 SO = 2,
                 LO = 3 } ChanSel;
static void SetInfo(HWND, char*);
static void ClearInfo(HWND);
//static void ReadHeadTitles(FILE*);
static BOOL is_info;
static char EXPName[20];
static char CalName[20];
static char TimeExp[26];
static char Comment[66];
static char HeadNam[20];

static void reverse(int count, short int* dp);

DLGPROC(DirProc);

DLGPROC(OpenDlg) {
  WORD wNotCode;
  char buf[80];
  int i;
  switch (msg) {
    case WM_INITDIALOG: {
      strcpy(buf, HeadDir);
      strcat(buf, "*.");
      CheckDlgButton(hDlg, IDC_OPN_S_F, 1);
      ClearInfo(hDlg);
      SendDlgItemMessage(hDlg, IDC_OPN_LIST, LB_DIR, DDL_READWRITE | DDL_ARCHIVE, (LPARAM) (LPSTR) buf);
      if (!SendDlgItemMessage(hDlg, IDC_OPN_LIST, LB_GETCOUNT, 0, 0)) {
        MessageBox(hFrame, "No experiments found", nullptr, MB_ICONEXCLAMATION | MB_OK);
        if (DialogBox(hInst, MAKEINTRESOURCE(ID_DIR), hFrame, DirProc)) {
          EndDialog(hDlg, 2);
        } else {
          EndDialog(hDlg, 0);
        }
      } else {
        SendDlgItemMessage(hDlg, IDC_OPN_LIST, LB_SETCURSEL, 0, 0);
        SetFocus(GetDlgItem(hDlg, IDC_OPN_LIST));
      }
    } break;
    case WM_COMMAND: {
      wNotCode = HIWORD(lParam);
      switch (wParam) {
        case IDC_OPN_LIST: {
          if (is_info && wNotCode == LBN_SELCHANGE) {
            ClearInfo(hDlg);
          }
          if (wNotCode != LBN_DBLCLK) return 1;
        }
        case IDC_OPN_INFO: {
          i = (int) SendDlgItemMessage(hDlg, IDC_OPN_LIST, LB_GETCURSEL, 0, 0);
          SendDlgItemMessage(hDlg, IDC_OPN_LIST, LB_GETTEXT, i, (LPARAM) (LPSTR) buf);
          SetInfo(hDlg, buf);
          return 1;
        }
        case IDOK: {
          i = (int) SendDlgItemMessage(hDlg, IDC_OPN_LIST, LB_GETCURSEL, 0, 0);
          SendDlgItemMessage(hDlg, IDC_OPN_LIST, LB_GETTEXT, i, (LPARAM) (LPSTR) HeadNam);
          for (i = 0;
               IsDlgButtonChecked(hDlg, IDC_OPN_S_F + i) != 1;
               i++);
          ChanSel = (enum CS) i;
          EndDialog(hDlg, 1);
          return 1;
        }
        case IDCANCEL: {
          EndDialog(hDlg, 0);
          return 1;
        }
        case IDC_OPN_DIRS: {
          if (DialogBox(hInst, MAKEINTRESOURCE(ID_DIR), hFrame, DirProc)) {
            EndDialog(hDlg, 2);
          }
          return 1;
        }
      }
    } break;
  }
  return 0;
}

void
ClearInfo(HWND hDlg) {
  SetDlgItemText(hDlg, IDC_OPN_I_EXP, "");
  SetDlgItemText(hDlg, IDC_OPN_I_CAL, "");
  SetDlgItemText(hDlg, IDC_OPN_I_TIM, "");
  SetDlgItemText(hDlg, IDC_OPN_I_COMM, "");
  SetDlgItemText(hDlg, IDC_OPN_N_CH, "");
  is_info = FALSE;
}

void
SetInfo(HWND hDlg, char* exp) {
  int i, Ca, Cl, Cs, s, l;
  char buf[80];
  strcpy(buf, HeadDir);
  strcat(buf, exp);
  FILE* fh = fopen(buf, "rt");
  if (!fh) {
    SetDlgItemText(hDlg, IDC_OPN_I_COMM, "Can't open file");
    return;
  } else {
    ReadHeadTitles(fh);
    fclose(fh);
    SetDlgItemText(hDlg, IDC_OPN_I_EXP, EXPName);
    SetDlgItemText(hDlg, IDC_OPN_I_CAL, CalName);
    SetDlgItemText(hDlg, IDC_OPN_I_TIM, TimeExp);
    SetDlgItemText(hDlg, IDC_OPN_I_COMM, Comment);
    Ca = Cs = Cl = 0;
    char s_or_l;
    for (i = 0; i < 30; i++) {
      s_or_l = 's';
      sprintf(buf, "%s%s%c.%02d", DataDir, EXPName, s_or_l, i + 1);
      s      = FileExist(buf);
      s_or_l = 'l';
      sprintf(buf, "%s%s%c.%02d", DataDir, EXPName, s_or_l, i + 1);
      l = FileExist(buf);
      Cs += s;
      Cl += l;
      Ca += s || l;
    }
    sprintf(buf, "%2d channels; %2d short; %2d long", Ca, Cs, Cl);
    SetDlgItemText(hDlg, IDC_OPN_N_CH, buf);
  }
  is_info = TRUE;
}

struct CalEntry {
  int gnum;
  double fcal;
};

struct CalTab {
  int len;
  CalEntry Tab[30];
  void read(FILE*);
  double calibr(int gauge);
};

static HLOCAL ReadCalTab();

void
ReadHeadTitles(FILE* fp) {
  fgets(buf, 81, fp);
  fgets(buf, 81, fp);
  sscanf(buf, " *    NAMES: %s %s", EXPName, CalName);
  strncpy(TimeExp, &buf[52], 25);
  TimeExp[25] = 0;
  fgets(buf, 81, fp);
  strncpy(Comment, &buf[13], 65);
  Comment[65] = 0;
}

int
OpenDL() {
  int i;
  char fname[80];
  ChDir(MainDir);
  do {
    i = DialogBox(hInst, MAKEINTRESOURCE(ID_OPEN), hFrame, OpenDlg);
  } while (i == 2);
  if (!i) return 0;
  strcpy(fname, HeadDir);
  strcat(fname, HeadNam);
  FILE* fp = fopen(fname, "rt");
  if (!fp) {
    MessageBox(hFrame, "Can't open file", fname, MB_OK | MB_ICONSTOP);
    return -1;
  }
  ReadHeadTitles(fp);
  HLOCAL ct = ReadCalTab();
  if (!ct) {
    fclose(fp);
    return -1;
  }
  fgets(buf, 81, fp);
  fgets(buf, 81, fp);
  fgets(buf, 81, fp);
  fgets(buf, 81, fp);

  FILE* ixc;
  HFILE dat;
  ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
  sprintf(buf, "%s%s.ixc", Directory, EXPName);
  if (!GetSaveFileName(&ofn)) {
    LocalFree(ct);
    fclose(fp);
    return 0;
  }
  Experiment* E = SetupExp();
  E->Inc();

  ixc = fopen(E->IXC(), "wt");
  dat = _lcreat(E->DAT(), 0);

  for (i = 0; i < 30; i++) {
    double radius, g_factor, volts, offset, calibr;
    int angle, number;
    char s_or_l, g_type[2];
    if (fscanf(fp, "%*s%*d%lf%d%d%1s%lf%lf%lf%*s", &radius, &angle, &number, g_type, &g_factor, &volts, &offset) != 7) {
      MessageBox(hFrame, "Error in head file", fname, MB_OK | MB_ICONSTOP);
      fclose(fp);
      LocalFree(ct);
      return -1;
    }

    if (number == 0) continue;
    calibr = ((CalTab*) LocalLock(ct))->calibr(number);
    LocalUnlock(ct);
    Gauge* G = new Gauge(E);
    sprintf(G->ChNum, "DL%02d", i + 1);
    G->number = number;
    G->g_type = g_type[0];
    G->radius = radius;
    G->angle  = angle;
    G->dV     = volts * calibr / 5.0F / g_factor / 1024.0F;
    G->V0     = 0;
    strcpy(G->unit, "Atm");
    s_or_l = ChanSel & LF ? 'l' : 's';
    sprintf(buf, "%s%s%c.%02d", DataDir, EXPName, s_or_l, i + 1);
    if (!FileExist(buf)) {
      s_or_l = ChanSel & LF ? 's' : 'l';
      sprintf(buf, "%s%s%c.%02d", DataDir, EXPName, s_or_l, i + 1);
      if (ChanSel & SO || !FileExist(buf)) {
        delete G;
        continue;
      }
    }
    HFILE hFile = LZOpen(buf);
    if (hFile == HFILE_ERROR) {
      delete G;
      continue;
    }
    double rate_A, rate_B;
    short int lower, upper, zero_level;
    long start, final;
    long delay;
    ChType flag;
    if (LZRead(hFile, buf, 80) != 80 || sscanf(buf, "%lf %lf %d %ld %hd %hd %hd %ld %ld", &rate_A, &rate_B, (int*) &flag, &delay, &lower, &upper, &zero_level, &start, &final) != 9) {
      delete G;
      continue;
    }
    rate_A /= 1000; //ms -> s
    rate_B /= 1000;
    G->V0    = -G->I2D(zero_level);
    G->count = final - start;
    G->start = 0;
    G->final = G->count;
    switch (flag) {
      case Pre:
        G->nRates        = 1;
        G->Rates         = new Gauge::Piece[1];
        G->Rates[0].Np   = final - start;
        G->Rates[0].rate = rate_A;
        if (delay) {
          G->Rates[0].Tstart = (start - (0x4000 - delay)) * rate_A;
        } else {
          G->Rates[0].Tstart = (start + delay) * rate_A;
        }
        break;
      case A_B:
        if (start > delay) {
          G->nRates          = 1;
          G->Rates           = new Gauge::Piece[1];
          G->Rates[0].Np     = final - start;
          G->Rates[0].rate   = rate_B;
          G->Rates[0].Tstart = delay * rate_A + (start - delay) * rate_B;
        } else if (final < delay) {
          G->nRates          = 1;
          G->Rates           = new Gauge::Piece[1];
          G->Rates[0].Np     = final - start;
          G->Rates[0].rate   = rate_A;
          G->Rates[0].Tstart = start * rate_A;
        } else {
          G->nRates          = 2;
          G->Rates           = new Gauge::Piece[2];
          G->Rates[0].Np     = delay - start;
          G->Rates[0].rate   = rate_A;
          G->Rates[0].Tstart = start * rate_A;
          G->Rates[1].Np     = final - delay;
          G->Rates[1].rate   = rate_B;
          G->Rates[1].Tstart = delay * rate_A;
        }
        break;
      case Delay:
        G->nRates          = 1;
        G->Rates           = new Gauge::Piece[1];
        G->Rates[0].Np     = final - start;
        G->Rates[0].rate   = rate_A;
        G->Rates[0].Tstart = (start + delay) * rate_A;
        break;
    }
    G->hVal = GlobalAlloc(GMEM_MOVEABLE | GMEM_DISCARDABLE, (final - start) * sizeof(int));
    G->val  = (short int*) GlobalLock(G->hVal);
    LZRead(hFile, (LPSTR) G->val, (final - start) * sizeof(short));
    reverse(final - start, G->val);
    LZClose(hFile);
    MDICREATESTRUCT cs;
    cs.szClass = "DXWchannel";
    cs.szTitle = G->WinTitle();
    cs.hOwner  = hInst;
    cs.x       = CW_USEDEFAULT;
    cs.y       = CW_USEDEFAULT;
    cs.cx      = CW_USEDEFAULT;
    cs.cy      = CW_USEDEFAULT;
    cs.style   = WS_MINIMIZE;
    cs.lParam  = 0;
    G->Create(cs);
    if (G->hWnd) G->Write(dat, ixc);
    G->UnlockGauge();
  }
  LocalFree(ct);
  fclose(fp);
  fclose(ixc);
  _lclose(dat);
  E->Dec();
  if (!nGauges) MessageBox(hFrame, "No channels available", ExpName, MB_OK | MB_ICONINFORMATION);
  SetTitle();
  return 0;
}

inline short int
swap2(short int v) {
  return ((v >> 8) & 0xFF) | ((v & 0xFF) << 8);
}

void
reverse(int count, short int* dp) {
  for (int i = 0; i < count; ++i) dp[i] = swap2(dp[i]);
}

static HLOCAL
ReadCalTab() {
  FILE* fp;
  HLOCAL ret;
  int n;
  strcpy(buf, CalDir);
  strcat(buf, CalName);
  fp = fopen(buf, "rt");
  if (!fp) {
    MessageBox(hFrame, "Can't open", buf, MB_OK);
    return nullptr;
  }
  fgets(buf, 81, fp);
  fgets(buf, 81, fp);
  fgets(buf, 81, fp);
  fgets(buf, 81, fp);
  fgets(buf, 81, fp);
  fgets(buf, 81, fp);
  fgets(buf, 81, fp);
  ret        = LocalAlloc(LMEM_MOVEABLE, sizeof(CalTab));
  CalTab* tb = (CalTab*) LocalLock(ret);
  tb->read(fp);
  n = tb->len;
  fclose(fp);
  LocalUnlock(ret);
  if (!n) {
    LocalFree(ret);
    return nullptr;
  } else {
    return LocalReAlloc(ret, n * sizeof(CalEntry) + sizeof(int), 0);
  }
}

void
CalTab::read(FILE* fp) {
  int j;
  int dummy_d;
  char dummy_s[2];
  for (j = 0, len = 0; j < 30; j++) {
    if (fscanf(fp, " | %d%d%lf%1s |", &Tab[len].gnum, &dummy_d, &Tab[len].fcal, dummy_s) != 4) {
      MessageBox(hFrame, "Error in calibration file", nullptr, MB_OK);
      len = 0;
      return;
    }
    if (Tab[len].gnum != 0) len++;
  }
}

double
CalTab::calibr(int gauge) {
  int i;
  for (i = 0; i < len; i++) {
    if (gauge == Tab[i].gnum) return Tab[i].fcal;
  }
  sprintf(buf, "Can't find gauge number %2d in calibration file", gauge);
  MessageBox(hFrame, buf, nullptr, MB_OK);
  return 0;
}

//ARGSUSED

DLGPROC(DirProc) {
  int i;
  switch (msg) {
    case WM_INITDIALOG: {
      SetDlgItemText(hDlg, IDC_DIR_MAIN, MainDir);
      SetDlgItemText(hDlg, IDC_DIR_HEAD, HeadDir);
      SetDlgItemText(hDlg, IDC_DIR_DATA, DataDir);
      SetDlgItemText(hDlg, IDC_DIR_CAL, CalDir);
      SetFocus(GetDlgItem(hDlg, IDC_DIR_MAIN));
    } break;
    case WM_COMMAND: {
      switch (wParam) {
        case IDOK: {
          GetDlgItemText(hDlg, IDC_DIR_MAIN, MainDir, 80);
          GetDlgItemText(hDlg, IDC_DIR_HEAD, HeadDir, 80);
          GetDlgItemText(hDlg, IDC_DIR_DATA, DataDir, 80);
          GetDlgItemText(hDlg, IDC_DIR_CAL, CalDir, 80);
          cbs(MainDir);
          i = strlen(MainDir);
          if (MainDir[i - 2] != ':') MainDir[i - 1] = 0;
          cbs(HeadDir);
          cbs(DataDir);
          cbs(CalDir);
          i = ChDir(MainDir);
          sprintf(buf, "Can't change directory to %s", MainDir);
          if (i) MessageBox(hDlg, buf, "Main Directory", MB_ICONEXCLAMATION | MB_OK);
          else {
            EndDialog(hDlg, 1);
            SaveDirs();
          }
          return 1;
        }
        case IDCANCEL: {
          EndDialog(hDlg, 0);
          return 1;
        }
      }
    } break;
  }
  return 0;
}
