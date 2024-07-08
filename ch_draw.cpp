#define STRICT
#include <windows.h>
#include "dxw.h"
#include "gauge.h"
#include <commdlg.h>
#include <cmath>
#include <cstdlib>

static BOOL PolyCache = 0;

inline double
clip(double x, double min, double max) {
  if (x > max) return max;
  if (x < min) return min;
  return x;
}

void
Text(HDC hdc, int x, int y, char const* s, UINT flags) {
  int l = lstrlen(s);
  SetBkMode(hdc, TRANSPARENT);
  SetTextAlign(hdc, flags);
  TextOut(hdc, x, y, s, l);
}

struct UPrefix {
  int scale;
  char const* prefix;
};
static UPrefix Scales[] = {
  { -9, "n" },
  { -6, "mk" },
  { -3, "m" },
  { +3, "k" },
  { +6, "M" },
  { +9, "G" },
  { +12, "T" },
  { 0, "" }
};

char*
Uscale(int i, const char* unit, const char* suff) {
  int l;
  UPrefix* p;
  for (p = Scales; (l = strlen(p->prefix)) != 0; ++p) {
    if (!strncmp(unit, p->prefix, l)) break;
  }
  i += p->scale;
  for (p = Scales; p->scale; ++p) {
    if (i == p->scale) break;
  }
  if (i == p->scale) {
    sprintf(buf, "%s%s%s", p->prefix, unit + l, suff);
  } else {
    sprintf(buf, "10^%d%s%s", i, unit + l, suff);
  }
  return buf;
}

char*
DrOpt::Tscale(int i) {
  return Uscale(i, "s", "");
}

char*
DrOpt::Pscale(int i, char* unit) {
  return Uscale(i, unit, "");
}

char*
DrOpt::Iscale(int i, char* unit) {
  return Uscale(i, unit, "*s");
}

void
DrOpt::SetRect(RECT& rc) {
  int DigT      = (T.style & AM_DIGITS) == AS_DIGIT;
  int DigP      = (P.style & AM_DIGITS) == AS_DIGIT;
  int DigI      = (I.style & AM_DIGITS) == AS_DIGIT;
  double fields = (DigT || DigP || DigI) ? 0.06 : 0.0;
  rcG.left      = rc.left + (DigP ? 0.1 : fields) * (rc.right - rc.left);
  rcG.right     = rc.right - (DigI ? 0.1 : fields) * (rc.right - rc.left);
  rcG.top       = rc.top + fields * (rc.bottom - rc.top);
  rcG.bottom    = rc.bottom - (DigT ? 0.1 : fields) * (rc.bottom - rc.top);
  tickP         = (rc.right - rc.left) * 0.015;
  if (!tickP) tickP = 1;
  tickI = tickP;
  tickT = (rc.bottom - rc.top) * 0.015;
  if (!tickT) tickT = 1;
}

void
DrOpt::SetCutRect(Gauge& G) {
  rcCut.left   = T2scr(G.Start());
  rcCut.right  = T2scr(G.Final());
  rcCut.top    = P2scr(G.Upper());
  rcCut.bottom = P2scr(G.Lower());
}

void
Gauge::Draw(HDC hdc, RECT& rc, DCtype t, RECT* rcU) {

  int Res, LW;
  HPEN hpSave = hpDef;
  if ((int) t < 0) {
    Res = GetDeviceCaps(hdc, LOGPIXELSX);
  } else {
    Res = (int) t;
  }
  LW    = LineWidth * Res / 720;
  hpDef = CreatePen(PS_SOLID, LW, RGB(0, 0, 0));

  DrOpt dr;
  //LockI();
  dr.DrCut = TRUE;
  dr.T.Set(Start(), Final(),
           10 | AS_DRAXIS | AS_TICKOUT | AS_DIGIT | AS_SNAP | AS_HORIZ | AS_LOW);
  dr.P.Set(Lower(), Lower() == Upper() ? Lower() + 1.0 : Upper(),
           10 | AS_DRAXIS | AS_TICKOUT | AS_DIGIT | AS_SNAP | AS_VERT | AS_LOW);
  if (drawI) {
    LockI();
    dr.I.Set(Imin, Imax,
             10 | AS_DRAXIS | AS_TICKOUT | AS_DIGIT | AS_SNAP | AS_VERT | AS_HIGH);
  } else {
    dr.I.Set(0, 1,
             10 | AS_NOAXIS | AS_NOTICK | AS_NODIGIT | AS_NOSNAP);
  }
  dr.SetRect(rc);
  if (rcValid && frcValid && rcU) {
    dr.upd.Set(
        scr2x(rcU->left, fr, rcGrf),
        scr2x(rcU->right, fr, rcGrf));
  } else {
    dr.upd.Set(Start(), Final());
  }
  if (t == Screen) PolyCache = 1;
  Plot(hdc, dr);
  PolyCache = 0;
  if (t == Screen) {
    //	HRGN hrgnWin=CreateRectRgnIndirect(&rc);
    //	SelectClipRgn(hdc,hrgnWin);
    //	DeleteObject(hrgnWin);
    memcpy(&rcGrf, &dr.rcG, sizeof(RECT));
    fr.x    = dr.T;
    fr.y    = dr.P;
    rcValid = frcValid = TRUE;
    DrawPointer(hdc);
    DrawCutRect(hdc);
    /*	SelectObject(hdc,hbrGray);
	Handle(hdc,dr.rcCut.left ,dr.rcCut.top);
	Handle(hdc,dr.rcCut.right,dr.rcCut.top);
	Handle(hdc,dr.rcCut.left ,dr.rcCut.bottom);
	Handle(hdc,dr.rcCut.right,dr.rcCut.bottom);
	Handle(hdc,(dr.rcCut.left+dr.rcCut.right)/2,dr.rcCut.top);
	Handle(hdc,(dr.rcCut.left+dr.rcCut.right)/2,dr.rcCut.bottom);
	Handle(hdc,dr.rcCut.right,(dr.rcCut.bottom+dr.rcCut.top)/2);
	Handle(hdc,dr.rcCut.left ,(dr.rcCut.bottom+dr.rcCut.top)/2);
*/
  }
  hpDef = hpSave;
  SelectObject(hdc, GetStockObject(BLACK_PEN));
}

void
Gauge::Plot(HDC hdc, DrOpt& dr) {
  int i;
  unsigned long st, fi, k;
  //LockI();

  // Cut rectangle
  SelectObject(hdc, hpDef);
  dr.SetCutRect(*this);
  if (dr.DrCut) {
    SelectObject(hdc, hbrNull);
    Rectangle(hdc, dr.rcCut.left, dr.rcCut.top, dr.rcCut.right, dr.rcCut.bottom);
  }
  HFONT fntLab = nullptr;
  if ((dr.T.style & AM_DIGITS) == AS_DIGIT || (dr.P.style & AM_DIGITS) == AS_DIGIT || (dr.I.style & AM_DIGITS) == AS_DIGIT) {
    fntLab = CreateFont(
        -abs((int) dr.tickT * 5), 0,
        //	0,abs((int)dr.tickP*2),
        0, 0,
        FW_NORMAL, 0, 0, 0, ANSI_CHARSET,
        OUT_TT_PRECIS, CLIP_TT_ALWAYS, DRAFT_QUALITY, 0,
        "Arial");
    SelectObject(hdc, fntLab);
    SetROP2(hdc, R2_MASKPEN);
  }

  // T axis

  if ((dr.T.style & AM_AXIS) == AS_DRAXIS) {
    if (dr.T.style & AS_MID) {
      dr.T.Draw(hdc, dr.rcG, dr.P2scr(0));
    } else {
      i = dr.TAxis(hdc);
      if ((dr.T.style & AM_DIGITS) == AS_DIGIT) {
        dr.Tscale(i);
        Text(hdc, dr.rcG.right, dr.rcG.bottom - dr.tickT, buf, TA_RIGHT | TA_BOTTOM);
      }
    }
  }
  // P axis
  if ((dr.P.style & AM_AXIS) == AS_DRAXIS) {
    i = dr.PAxis(hdc);
    if ((dr.P.style & AM_DIGITS) == AS_DIGIT) {
      dr.Pscale(i, unit);
      Text(hdc, dr.rcG.left + dr.tickP, dr.rcG.top, buf, TA_LEFT | TA_TOP);
    }
  }
  // I axis
  if ((dr.I.style & AM_AXIS) == AS_DRAXIS) {
    SelectObject(hdc, hpImp);
    i = dr.IAxis(hdc);
    if ((dr.I.style & AM_DIGITS) == AS_DIGIT) {
      dr.Iscale(i, unit);
      Text(hdc, dr.rcG.right - dr.tickI, dr.rcG.top, buf, TA_RIGHT | TA_TOP);
    }
    SelectObject(hdc, hpDef);
  }

  if (fntLab) {
    SelectObject(hdc, GetStockObject(SYSTEM_FONT));
    DeleteObject(fntLab);
  }

  st = T2P(dr.upd.Min());
  fi = T2P(dr.upd.Max()) + 1;
  if (st < start) st = start;
  if (fi > final) fi = final;
  if (fi == st) return;
  if (PolyCache) {
    st = start;
    fi = final;
  }

  std::unique_ptr<Pline> psave;

  if ((dr.I.style & AM_AXIS) == AS_DRAXIS) {
    SelectObject(hdc, hpImp);
    LockI();
    i = dr.I2scr(0.0);
    if (i > dr.rcG.top && i < dr.rcG.bottom) {
      MoveToEx(hdc, dr.rcG.left, i, nullptr);
      LineTo(hdc, dr.rcG.right, i);
    }
    if (!PolyCache) {
      std::swap(psave, PolyI);
    }
    if (!PolyI) {
      PolyI = std::make_unique<Pline>();
      BeginWait();
      for (k = st; k < fi; k++) {
        PolyI->FilterPoint(dr.T2scr(P2T(k)), dr.I2scr(Imp(k)));
      }
      EndWait();
      PolyI->Finish();
    }
    PolyI->Polyline(hdc);

    if (!PolyCache) {
      std::swap(PolyI, psave);
    }
    SelectObject(hdc, hpDef);
  }

  if ((dr.P.style & AM_AXIS) == AS_DRAXIS) {
    LockD();
    i = dr.P2scr(0.0);
    if (i > dr.rcG.top && i < dr.rcG.bottom) {
      MoveToEx(hdc, dr.rcG.left, i, nullptr);
      LineTo(hdc, dr.rcG.right, i);
    }
    //	IntersectClipRect(hdc,dr.rcCut.left,dr.rcCut.top,
    //						  dr.rcCut.right,dr.rcCut.bottom);
    if (!PolyCache) {
      std::swap(psave, PolyP);
    }
    if (!PolyP) {
      PolyP = std::make_unique<Pline>();
      BeginWait();
      for (k = st; k < fi; k++) {
        PolyP->FilterPoint(dr.T2scr(P2T(k)), dr.P2scr(clip(Val(k), _Lower, _Upper)));
      }
      EndWait();
      PolyP->Finish();
    }
    PolyP->Polyline(hdc);

    if (!PolyCache) {
      std::swap(PolyP, psave);
    }
  }
}
