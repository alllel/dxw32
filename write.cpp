#define STRICT
#include <windows.h>
#include "dxw.h"
#include "gauge.h"
#include <cstdio>
#include <fstream>
#include <utility>

void
Write(BOOL SaveAs) {
  std::shared_ptr<Experiment> E;
  if (!Experiment::nExp) return;
  if (!SaveAs && Experiment::nExp == 1) {
    Gauge* G = *GaugeIterator().begin();
    if (G) E = G->Exp;
  } else {
    FileData ofn;
    ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
    buf[0]    = 0;
    if (!GetSaveFileName(&ofn)) return;
    E = Experiment::SetupExp(buf);
  }
  if (!E) return;
  {
    auto ixc = std::fstream(E->File("_IX"), std::ios::out);
    auto dat = std::fstream(E->File("_DT"), std::ios::out | std::ios::binary);
    if (ixc.bad() || dat.bad()) {
      MessageBox(hFrame, "Can't create file", buf, MB_OK | MB_ICONEXCLAMATION);
      return;
    }
    BeginWait();
    for (auto G : GaugeIterator()) {
      if (G->Write(dat, ixc)) break;
    }
    EndWait();
  }
  size_t Pos = 0;
  for (auto G : GaugeIterator()) {
    G->AfterWrite(E, Pos);
  }
  auto bak = E->File(".bak");
  std::filesystem::remove(bak);
  std::filesystem::rename(E->path, bak);
  rename(E->File("._ix"), E->path);
  auto dbk = E->File(".dbk");
  std::filesystem::remove(bak);
  auto dat = E->File(".dat");
  std::filesystem::rename(dat, bak);
  std::filesystem::rename(E->File("._dt"), dat);
  Changed = 0;
  SetTitle();
}

int
Gauge::Write(std::fstream& hDAT, std::fstream& hIXC) {
  snprintf(ID, std::size(ID), "G%d%c L%d R%.3f", number, g_type, angle, radius);
  auto len = snprintf(buf, sizeof(buf), "0:%s\\%16.16s\\%g\\%g\\%3.3s\\Time %hd.%hd.%hd \\Date %hd:%hd:%hd\\\n",
                      ChNum,
                      buf,
                      dV,
                      V0 - Zero_corr,
                      unit,
          time.hour, time.min, time.sec,
          date.day, date.month, date.year);
  hIXC.write(buf, len);
  unsigned long nst = 0;
  for (auto const& P : Rates) {
    auto np     = P.Np;
    auto Tstart = P.Tstart;
    if (nst + P.Np >= start) {
      if (nst > final) break;
      if ((nst + P.Np > start) && (nst < start)) {
        np -= (start - nst);
        Tstart += (start - nst) * P.rate;
      }
      if ((nst + P.Np > final) && (nst < final)) {
        np -= nst + P.Np - final;
      }
      auto plen = snprintf(buf, std::size(buf), "\\%ld\\%lE\\%lE\\s  \n", np, P.rate, Tstart);
      hIXC.write(buf, plen);
    }
    nst += P.Np;
  }
  LockD();
  auto ldat = (final - start) * sizeof(short);
  hDAT.write(reinterpret_cast<char*>(val.data() + start), ldat);
  if (hDAT.bad()) {
    MessageBox(hFrame, "Can't write file! ", "Error writing .dat", MB_OK);
    return 1;
  }
  return 0;
}

void
Gauge::AfterWrite(std::shared_ptr<Experiment> E, size_t& Pos) {
  V0 -= Zero_corr;
  Zero_corr = 0;
  size_t nst=0, j=0;
  std::vector<Piece> newRates;
  for (auto& P : Rates) {
    auto nP=P;
    if (nst + P.Np >= start){
      if (nst > final) break;
      if ((nst + P.Np > start) && (nst < start)) {
        nP.Np -= (start - nst);
        nP.Tstart += (start - nst) * P.rate;
      }
      if ((nst + P.Np > final) && (nst < final)) {
        nP.Np -= nst + P.Np - final;
      }
      newRates.push_back(nP);
    }
    nst += P.Np;
  }

  Rates   = newRates;
  FilePos = Pos;
  Pos += (final - start) * sizeof(int);
  if (count != final - start) {
    FreeD();
    count = final - start;
    start = 0;
    final = count;
  }
  Link(std::move(E));
}
