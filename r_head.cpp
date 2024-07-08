#define STRICT

#include <windows.h>
#include "dxw.h"
#include "gauge.h"
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <lzexpand.h>
#include <commdlg.h>
#include <sstream>
#include <fstream>

int
OpenExp() {
  buf[0]        = 0;
  FileData ofn;
  ofn.hwndOwner = hFrame;
  ofn.Flags     = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
  if (!GetOpenFileName(&ofn)) return 0;
  auto E = Experiment::SetupExp(buf);
  {
    for (auto G : GaugeIterator()) 
      if (*(G->Exp) == *E) {
        MessageBox(hFrame, "Experiment is already open!", nullptr, MB_OK | MB_ICONEXCLAMATION);
        return 0;
      }
  }
  auto fp = std::fstream { E->IXC(), std::ios::in };
  if (!fp) {
    MessageBox(hFrame, "Can't open file", buf, MB_OK | MB_ICONSTOP);
    return 0;
  }

  Gauge* NG;
  MDICREATESTRUCT cs;
  cs.szClass = "DXWchannel";
  cs.hOwner  = hInst;
  cs.x       = CW_USEDEFAULT;
  cs.y       = CW_USEDEFAULT;
  cs.cx      = CW_USEDEFAULT;
  cs.cy      = CW_USEDEFAULT;
  cs.style   = WS_MINIMIZE;
  cs.lParam  = 0;
  long datapos = 0;
  do {
    unsigned short h, min, s, d, m, y;
    fp.getline(buf, std::size(buf), '\n');
    if (fp.gcount() == 0 || buf[0] == '#') continue;
    NG = new Gauge(E);
    int c = sscanf(buf, R"(%*1d:%[0-9A-Z#]\%16c\%lg\%lg\%3c\Time %hu.%hu.%hu \Date %hu:%hu:%hu)",
                   NG->ChNum,
                   NG->ID,
                   &NG->dV,
                   &NG->V0,
                   NG->unit,
                   &h, &min, &s,
                &d, &m, &y);
    if (c != 11) {
      std::stringstream msg;
      msg << "Error reading channel from line: \n|" << buf << "|\n"
          << " Only " << c << " values decoded (of 11)";
      MessageBox(hFrame, msg.str().c_str(), "Can't read channel", MB_OK | MB_ICONSTOP);
      delete NG;
      break;
    }
    NG->time.hour  = h;
    NG->time.min   = min;
    NG->time.sec   = s;
    NG->date.day   = d;
    NG->date.month = m;
    NG->date.year  = y;
    NG->FilePos    = datapos;
    while (fp.peek() == '\\') {
      fp.getline(buf, 200, '\n');
      Gauge::Piece P { 0, 0, 0 };
      if (sscanf(buf, R"(\%zu\%lE\%lE)", &P.Np, &P.rate, &P.Tstart) != 3) {
        MessageBox(hFrame, buf, "Cant read rates line", MB_OK);
        break;
      }
      NG->count += P.Np;
      NG->Rates.push_back(P);
    }
    datapos += NG->count * sizeof(short int);
    NG->Setup();
    cs.szTitle = NG->WinTitle();
    NG->Create(cs);
    if (!NG->hWnd) {
      delete NG;
    }
  } while (!fp.eof());
  if (!nGauges) MessageBox(hFrame, "No channels available", ExpName, MB_OK | MB_ICONINFORMATION);
  SetTitle();
  return 0;
}

void
SetTitle() {
  switch (Experiment::nExp) {
    case 1: {
      sprintf(buf, "DXW:%s (%d channels)", ExpName, nGauges);
      SetWindowText(hFrame, buf);
    } break;
    case 0: {
      Changed = 0;
      SetWindowText(hFrame, "DX for Windows");
    } break;
    default: {
      sprintf(buf, "DXW:%d exp (%d channels)", Experiment::nExp, nGauges);
      SetWindowText(hFrame, buf);
    } break;
  }
  TitleChanged = 0;
}
