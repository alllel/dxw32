#define STRICT

#include <windows.h>
#include "dxw.h"
#include "gauge.h"
#include <cmath>
#include <cstdio>
#include "rt.h"

char*
RT::PicName() {
  sprintf(buf, "%.6sRT", ExpName);
  return buf;
}

bool
RT::Command(WPARAM cmd) {
  switch (cmd) {
    case CM_R_T:
      DLG(ID_RTD, RTdlg, reinterpret_cast<LPARAM>(this));
      return TRUE;
    default:
      return Window::Command(cmd);
  }
}

bool
RT::RemoveChn(const Gauge* G) {
  auto it = std::ranges::find(Chns, G->hWnd);
  if (it == Chns.end()) return false;
  Chns.erase(it);
  return true;
}

bool
RT::WinProc(Msg& m) {
  switch (m.msg) {
    case WM_RBUTTONDBLCLK:
      FindChannell(HIWORD(m.lParam));
      return TRUE;
    case WM_LBUTTONDBLCLK:
      DLG(ID_RTD, RTdlg, reinterpret_cast<LPARAM>(this));
      return TRUE;
    default:
      return Window::WinProc(m);
  }
}

void
RT::FindChannell(WORD y) {
  RECT rc;
  if (Chns.empty()) return;
  GetClientRect(hWnd, &rc);
  int height = rc.bottom - rc.top;
  int mdelt  = height;
  int tf     = (int) (height * 0.01);
  int bf     = (int) (height * 0.1);
  height -= tf + bf;
  int Ph = (int) (height * (P_height / 100.0));
  int Rh = height - Ph;
  double w;
  HWND rez = nullptr;
  for (auto hG : Chns) {
    auto G = GaugeByWnd(hG);
    if (!G) continue;
    if (G->radius < R0 || G->radius > R1) continue;
    w        = (G->radius - R0) / (R1 - R0);
    int delt = abs((rc.bottom - bf - (int) (w * Rh)) - y);
    if (delt < mdelt) {
      mdelt = delt;
      rez   = hG;
    }
  }
  if (!rez) return;
  if (IsIconic(rez)) {
    ShowWindow(rez, SW_RESTORE);
  } else {
    SendMessage(hMDI, WM_MDIACTIVATE, (WPARAM) hWnd, 0);
  }
}

static COLORREF RTColors[] = {
  RGB(0, 0, 0),
  RGB(255, 0, 0),
  RGB(0, 255, 0),
  RGB(0, 0, 255),
  RGB(255, 255, 0),
  RGB(255, 0, 255),
  RGB(0, 255, 255),
};
#define NCLR (sizeof(RTColors) / sizeof(COLORREF))

void
RT::Draw(HDC hdc, RECT& rc, DCtype dct, RECT* rcUpd) {
  //HRGN hrgnWin=CreateRectRgnIndirect(&rc);
  HPEN hpSave = hpDef;
  HPEN hpClr[NCLR];
  //int i;
  if (Chns.empty()) return;
  int Res, LW;
  if ((int) dct < 0) {
    Res = GetDeviceCaps(hdc, LOGPIXELSX);
  } else {
    Res = (int) dct;
  }
  LW    = LineWidth * Res / 720;
  hpDef = hpClr[0] = CreatePen(PS_SOLID, LW, RTColors[0]);
  for (unsigned i = 1; i < NCLR; ++i) {
    hpClr[i] = CreatePen(PS_SOLID, LW, RTColors[i]);
  }

  int height = rc.bottom - rc.top;
  int lenght = rc.right - rc.left;
  int lf     = (int) (lenght * 0.01);
  int rf     = (int) (lenght * 0.01);
  int tf     = (int) (height * 0.01);
  int bf     = (int) (height * 0.1);
  if (R_left || (P_st == Left)) lf = (int) (lenght * 0.1);
  if (!R_left || (P_st == Right)) rf = (int) (lenght * 0.1);
  height -= tf + bf;
  lenght -= lf + rf;

  int Ph = (int) (height * (P_height / 100.0));
  int Rh = height - Ph;

  double ang = tan(R_angle / 180 * 3.1415926535);
  int Rang   = (int) (Rh * ang);
  int Tlen   = lenght - abs(Rang);
  int Torg   = (R_angle < 0 ? -Rang : 0) + lf;

  int h;
  double w;
  DrOpt dr;
  dr.T.Set(T0, T1, 10 | AS_DRAXIS | AS_NOTICK | AS_NODIGIT | AS_LOW | AS_HORIZ | AS_SNAP | AS_MID);
  dr.I.style = AS_NOAXIS | AS_NODIGIT;
  if (P_st == None) dr.P.style = 7 | AS_DRAXIS | AS_NOTICK | AS_NODIGIT | AS_LOW;
  else
    dr.P.style = 7 | AS_DRAXIS | AS_NOTICK | AS_NODIGIT | AS_NOSNAP | ((P_st == Left) ? AS_LOW : AS_HIGH) | AS_VERT;
  dr.tickP = (int) (lenght * 0.01);
  dr.tickT = (int) (height * 0.01);
  dr.DrCut = FALSE;

  HFONT fntLab = CreateFont(
      -(FontHeight * Res / 720), 0, //height,width
      0, 0,                         //escapement,orientation
      ((FontType & BOLD_FONTTYPE) ? FW_BOLD : FW_NORMAL),
      ((FontType & ITALIC_FONTTYPE) ? TRUE : FALSE),
      0, 0, ANSI_CHARSET,
      OUT_TT_PRECIS, CLIP_TT_ALWAYS, DRAFT_QUALITY, 0,
      FontName);
  SelectObject(hdc, fntLab);
  SelectObject(hdc, hpDef);

  for (auto hG : Chns) {
    auto G = GaugeByWnd(hG);
    if (!G) continue;
    if (G->radius < R0 || G->radius > R1) continue;
    w            = (G->radius - R0) / (R1 - R0);
    dr.rcG.left  = Torg + (int) (Rang * w);
    dr.rcG.right = dr.rcG.left + Tlen;

    dr.rcG.bottom = Ph;
    dr.rcG.top    = 0;
    dr.P.Set(G->Lower(), G->Upper());
    h = rc.bottom - (int) (w * Rh) - bf;
    sprintf(buf, "%g", G->radius);
    if (R_left) {
      Text(hdc, dr.rcG.left - dr.tickP, h, buf, TA_BASELINE | TA_RIGHT);
    } else {
      Text(hdc, dr.rcG.right + dr.tickP, h, buf, TA_BASELINE | TA_LEFT);
    }
    h -= dr.P2scr(0);
    dr.rcG.bottom += h;
    dr.rcG.top += h;
    dr.upd.Set(T0, T1);
    if (rcUpd) {
      if (rcUpd->top > dr.rcG.bottom) continue;
      if (rcUpd->bottom < dr.rcG.top) continue;
      //		dr.upd.Set(dr.scr2T(rcUpd->left),dr.scr2T(rcUpd->right));
    }
    if (clr) hpDef = hpClr[G->angle % NCLR];
    G->Plot(hdc, dr);
    hpDef = hpClr[0];
    //	SelectClipRgn(hdc,hrgnWin);
    if (P_st != None) {
      sprintf(buf, "%g%s", dr.P.tick.val, G->unit);
      h = dr.P2scr(0);
      int xt, x0;
      int h0 = dr.P2scr(0), h1 = dr.P2scr(dr.P.tick.val);
      if (P_st == Left) {
        x0 = dr.rcG.left;
        xt = x0 - dr.tickP;
      } else {
        x0 = dr.rcG.right;
        xt = x0 + dr.tickP;
      }
      SelectObject(hdc, hpDef);
      MoveToEx(hdc, xt, h0, nullptr);
      LineTo(hdc, x0, h0);
      LineTo(hdc, x0, h1);
      LineTo(hdc, xt, h1);
      Text(hdc, xt, h0, buf, TA_BASELINE | (P_st == Left ? TA_RIGHT : TA_LEFT));
    }
  }
  SelectObject(hdc, hpDef);
  Text(hdc, Torg + Rang + (R_left ? -dr.tickP : Tlen + dr.tickP), rc.bottom - Rh - (int) (0.5 * Ph) - bf, "X,m",
       TA_TOP | (R_left ? TA_RIGHT : TA_LEFT));

  //T axis
  dr.rcG.left   = Torg;
  dr.rcG.right  = Torg + Tlen;
  dr.rcG.bottom = rc.bottom - bf;
  dr.rcG.top    = dr.rcG.bottom - Rh - Ph;
  dr.T.style &= ~(AM_DIGITS | AM_TICKS | AM_MID);
  dr.T.style |= AS_DIGIT | AS_GRID;
  Rang = (int) (ang * (dr.rcG.bottom - dr.rcG.top));
  dr.Tscale(dr.T.Draw(hdc, dr.rcG, Rang));
  memcpy(buf + 2, buf, lstrlen(buf) + 1);
  buf[0] = 't';
  buf[1] = ',';
  Text(hdc,
       dr.rcG.right + (int) ((dr.rcG.right - dr.rcG.left) * 0.03),
       dr.rcG.bottom + (int) ((dr.rcG.bottom - dr.rcG.top) * 0.02),
       buf, TA_LEFT | TA_TOP);

  MoveToEx(hdc, Torg + Rang, dr.rcG.top, nullptr);
  LineTo(hdc, Torg + Rang + Tlen, dr.rcG.top);

  SelectObject(hdc, GetStockObject(SYSTEM_FONT));
  SelectObject(hdc, GetStockObject(BLACK_PEN));
  hpDef = hpSave;
  DeleteObject(fntLab);
  if (clr)
    for (unsigned i = 0; i < NCLR; ++i) DeleteObject(hpClr[i]);
  //DeleteObject(hrgnWin);
}
