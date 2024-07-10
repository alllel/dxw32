#define STRICT
#include <windows.h>
#include <cstdlib>
#include "dxw.h"
#include "gauge.h"
#include "rt.h"
#include <cstring>
#include <utility>
#include <ctype.h>

void trim(char* str);

int Experiment::nExp = 0;

void
Gauge::Create() {
  MDICREATESTRUCT cs;
  cs.szClass = "DXWchannel";
  cs.hOwner  = hInst;
  cs.x       = CW_USEDEFAULT;
  cs.y       = CW_USEDEFAULT;
  cs.cx      = CW_USEDEFAULT;
  cs.cy      = CW_USEDEFAULT;
  cs.style   = WS_MINIMIZE;
  cs.lParam  = 0;
  Window::Create(cs);
}


Gauge::Gauge(std::shared_ptr<Experiment> e) : Exp(std::move(e)) {
  nGauges++;
}

std::shared_ptr<Gauge>
GaugeByChNum(char* cn) {
  for (auto G : GaugeIterator()) {
    if (!stricmp(cn, G->ChNum)) return G;
  }
  return nullptr;
}

char*
Gauge::WinTitle() {
  sprintf(buf, "%s A%d R=%g %s",
          ChNum, angle, radius, unit);
  return buf;
}

char*
Gauge::PicName() //virtual
{
  snprintf(buf, std::size(buf), "%ls_%s", Exp->path.c_str(), ChNum);
  return buf;
}

Gauge::~Gauge() = default;

void
Gauge::LockD() {
  if (val.empty()){
    val.resize(count, 0);
    ReadD();
  }
}

void
Gauge::LockI() {
  if (imp.empty()) {
    imp.resize(count, 0.0);
    CalcI();
  }
}

void
Gauge::SetInfo() {
  if (hInfo) {
    sprintf(buf, "%s - Info", ChNum);
    SetWindowText(hInfo, buf);
    SetDlgItemText(hInfo, IDC_INF_CHN, ChNum);
    sprintf(buf, "%d%c", number, g_type);
    SetDlgItemText(hInfo, IDC_INF_GAU, buf);
    SetDlgItemText(hInfo, IDC_INF_LIN, itoa(angle, buf, 10));
    SetDlgItemText(hInfo, IDC_INF_RAD, gcvt(radius, 5, buf));
    SetDlgItemText(hInfo, IDC_INF_ST, gcvt(Start(), 5, buf));
    SetDlgItemText(hInfo, IDC_INF_FI, gcvt(Final(), 5, buf));
    SetDlgItemText(hInfo, IDC_INF_LO, gcvt(Lower(), 5, buf));
    SetDlgItemText(hInfo, IDC_INF_UP, gcvt(Upper(), 5, buf));
    if (drawI) {
      LockI();
      SetDlgItemText(hInfo, IDC_INF_IP, gcvt(Ipos, 5, buf));
      SetDlgItemText(hInfo, IDC_INF_IM, gcvt(Ineg, 5, buf));
      SetDlgItemText(hInfo, IDC_INF_IT, gcvt(Itot, 5, buf));
    } else {
      SetDlgItemText(hInfo, IDC_INF_IP, "");
      SetDlgItemText(hInfo, IDC_INF_IM, "");
      SetDlgItemText(hInfo, IDC_INF_IT, "");
    }
  }
}

void
Gauge::FreeD() {
  val.clear();
}

void
Gauge::FreeI() {
  imp.clear();
}

void
Gauge::Setup() {
  start     = 0;
  final     = count;
  Zero_corr = 0;
  _Upper = _Lower = 0;
  char *p, *t;
  p = strchr(ID, 'G');
  if (p) {
    number = (int) strtol(p + 1, &t, 10);
    g_type = *t;
    if (!g_type) g_type = ' ';
    else
      *t = ' ';
  } else {
    number = 0;
    g_type = ' ';
  }
  p = strchr(ID, 'L');
  if (p) angle = atoi(p + 1);
  else
    angle = 0;
  p = strchr(ID, 'R');
  if (p) radius = atof(p + 1);
  else
    radius = 0;
  trim(unit);
}

void
Gauge::ULSetup() {
  BeginWait();
  LockD();
  unsigned long i;
  double Min, Max, V;
  Min = Max = Val(start);
  for (i = start + 1; i < final; ++i) {
    V = Val(i);
    if (V < Min) {
      Min = V;
    }
    if (V > Max) {
      Max = V;
    }
  }
  _Upper = Max;
  _Lower = Min;
  EndWait();
}

void
Gauge::Redraw() {
  InvalidateRect(hWnd, nullptr, TRUE);
  PolyP.reset();
  PolyI.reset();
  for (auto rt : RTbyGaugeIterator(this)) {
    InvalidateRect(rt->hWnd, nullptr, TRUE);
  }
} //Redraw
