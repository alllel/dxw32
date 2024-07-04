#define STRICT
#include <windows.h>
#include "dxw.h"
#include "gauge.h"
#include <cstdio>

void
Write(BOOL SaveAs) {
  Experiment* E = nullptr;
  //if(!Experiment::nExp)return;
  GaugeIterator G;
  if (!SaveAs && Experiment::nExp == 1) {
    if (G) E = G->Exp;
  } else {
    ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
    buf[0]    = 0;
    if (!GetSaveFileName(&ofn)) return;
    E = SetupExp();
  }
  if (!E) return;
  E->Inc();
  FILE* ixc = fopen(E->File("_IX"), "wt");
  HFILE dat = _lcreat(E->File("_DT"), 0);
  if (!ixc || (dat == HFILE_ERROR)) {
    MessageBox(hFrame, "Can't create file", buf, MB_OK | MB_ICONEXCLAMATION);
    E->Dec();
    return;
  }
  BeginWait();
  for (G.reset(); G; ++G) {
    if (G->Write(dat, ixc)) break;
  }
  EndWait();
  int succ = !G;
  char bak[81];
  if (_lclose(dat)) succ = 0;
  if (fclose(ixc)) succ = 0;
  if (succ) {
    long Pos = 0;
    for (G.reset(); G; ++G) {
      G->AfterWrite(E, Pos);
    }
    sprintf(bak, "%s%s.bak", E->Dir, E->Name);
    unlink(bak);
    sprintf(buf, "%s%s.ixc", E->Dir, E->Name);
    rename(buf, bak);
    sprintf(bak, "%s%s._ix", E->Dir, E->Name);
    rename(bak, buf);

    sprintf(bak, "%s%s.dbk", E->Dir, E->Name);
    unlink(bak);
    sprintf(buf, "%s%s.dat", E->Dir, E->Name);
    rename(buf, bak);
    sprintf(bak, "%s%s._dt", E->Dir, E->Name);
    rename(bak, buf);
    Changed = 0;
  } else {
    sprintf(bak, "%s%s._ix", E->Dir, E->Name);
    unlink(bak);
    sprintf(bak, "%s%s._dt", E->Dir, E->Name);
    unlink(bak);
    MessageBox(hFrame, "Error writing experiment!", nullptr, MB_OK);
  }
  E->Dec();
  SetTitle();
}

int
Gauge::Write(HFILE hDAT, FILE* hIXC) {
  sprintf(buf, "G%d%c L%d R%.3f", number, g_type, angle, radius);
  fprintf(hIXC, "0:%s\\%16.16s\\%g\\%g\\%3.3s\\Time %hd.%hd.%hd \\Date %hd:%hd:%hd\\\n",
          ChNum,
          buf,
          dV,
          V0 - Zero_corr,
          unit,
          time.hour, time.min, time.sec,
          date.day, date.month, date.year);
  unsigned long nst;
  unsigned long np, l;
  float Tstart;
  unsigned i;
  for (i = 0, nst = 0; i < nRates; nst += Rates[i].Np, ++i) {
    np     = Rates[i].Np;
    Tstart = Rates[i].Tstart;
    if (nst + Rates[i].Np < start) continue;
    if (nst > final) break;
    if ((nst + Rates[i].Np > start) && (nst < start)) {
      np -= (start - nst);
      Tstart += (start - nst) * Rates[i].rate;
    }
    if ((nst + Rates[i].Np > final) && (nst < final)) {
      np -= nst + Rates[i].Np - final;
    }
    fprintf(hIXC, "\\%ld\\%lE\\%lE\\s  \n", np, Rates[i].rate, Tstart);
  }
  LockD();
  l = (final - start) * sizeof(short);
  if (_hwrite(hDAT, (LPSTR) (val + start), l) != l) {
    //	MessageBox(hFrame,"Can't write file! Disk full?",nullptr,MB_OK);
    return 1;
  }
  return 0;
}

void
Gauge::AfterWrite(Experiment* E, long& Pos) {
  int i, j;
  V0 -= Zero_corr;
  Zero_corr = 0;
  unsigned long nst;
  Piece* newRates = new Piece[nRates];
  for (i = 0, j = 0, nst = 0; i < nRates; nst += Rates[i].Np, ++i) {
    newRates[j].Np     = Rates[i].Np;
    newRates[j].Tstart = Rates[i].Tstart;
    newRates[j].rate   = Rates[i].rate;
    if (nst + Rates[i].Np < start) continue;
    if (nst > final) break;
    if ((nst + Rates[i].Np > start) && (nst < start)) {
      newRates[j].Np -= (start - nst);
      newRates[j].Tstart += (start - nst) * Rates[i].rate;
    }
    if ((nst + Rates[i].Np > final) && (nst < final)) {
      newRates[j].Np -= nst + Rates[i].Np - final;
    }
    j++;
  }
  delete Rates;
  Rates   = newRates;
  nRates  = j;
  FilePos = Pos;
  Pos += (final - start) * sizeof(int);
  if (count != final - start) {
    FreeD();
    count = final - start;
    start = 0;
    final = count;
  }
  Link(E);
}
