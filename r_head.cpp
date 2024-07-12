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
  buf[0] = 0;
  FileData ofn;
  ofn.hwndOwner = hFrame;
  ofn.Flags     = OFN_HIDEREADONLY | OFN_FILEMUSTEXIST;
  if (!GetOpenFileName(&ofn)) return 0;
  else
    return OpenExp(buf);
}

int
OpenExp(std::string_view exp_fname) {
  auto E = Experiment::SetupExp(exp_fname);
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
  } else {
    auto dir = E->IXC().parent_path().string();
    std::strncpy(Directory, dir.c_str(), std::size(Directory));
    auto name = E->IXC().filename().string();
    std::strncpy(ExpName, name.c_str(), std::size(ExpName));
    recent.AddFile(E->IXC().string());
  }

  std::shared_ptr<Gauge> NG;
  long datapos = 0;
  do {
    unsigned short h, min, s, d, m, y;
    fp.getline(buf, std::size(buf), '\n');
    if (fp.gcount() == 0 || buf[0] == '#') continue;
    NG    = std::make_shared<Gauge>(E);
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
    //cs.szTitle = NG->WinTitle();
    NG->Create();
  } while (!fp.eof());
  if (!Gauge::nGauges) MessageBox(hFrame, "No channels available", ExpName, MB_OK | MB_ICONINFORMATION);
  else {
    PostMessage(hFrame, WM_COMMAND, CM_REST_ALL, 0);
    PostMessage(hFrame, WM_COMMAND, CM_TILE, 0);
  }
  SetTitle();
  return 0;
}

void
SetTitle() {
  static int nE = 0, nG = 0, ch;
  if (nE == Experiment::nExp && nG == Gauge::nGauges && ch == Changed) return;
  else {
    nE = Experiment::nExp;
    nG = Gauge::nGauges;
    ch = Changed;
  }
  switch (Experiment::nExp) {
    case 1: {
      auto name = (*GaugeIterator().begin())->Exp->IXC().filename().string();
      std::strncpy(ExpName, name.c_str(), std::size(ExpName));
      sprintf(buf, "DXW:%s (%d channels)%s", ExpName, Gauge::nGauges, (Changed ? " modified" : ""));
      SetWindowText(hFrame, buf);
    } break;
    case 0: {
      Changed = 0;
      SetWindowText(hFrame, "DX for Windows");
    } break;
    default: {
      sprintf(buf, "DXW:%d exp (%d channels)", Experiment::nExp, Gauge::nGauges);
      SetWindowText(hFrame, buf);
    } break;
  }
  TitleChanged = 0;
}
