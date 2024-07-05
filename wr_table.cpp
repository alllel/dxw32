#define STRICT
#include <windows.h>
#include "dxw.h"
#include "gauge.h"
#include <cstring>
#include <cstdio>

static char*
ftoa(double v) {
  sprintf(buf, "%.6g", v);
  if (strlen(buf) > 8 && buf[0] == '0') buf[8] = 0;
  return buf;
}

void
WriteTable() {
  OFN ofn;
  char tabname[_MAX_PATH];
  ofn.lpstrFile       = tabname;
  ofn.nMaxFile        = sizeof(tabname);
  ofn.lpstrFilter     = "Text Files\0*.txt\0All Files\0*.*\0";
  ofn.lpstrInitialDir = Directory;
  ofn.Flags |= OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;
  ofn.lpstrTitle = "Write Table to";
  strcpy(tabname, ExpName);
  strcat(tabname, ".txt");
  if (!GetSaveFileName(&ofn)) return;
  FILE* tab = fopen(tabname, "w");
  if (!tab) {
    MessageBox(hFrame, "Can't create file", nullptr, MB_OK);
    return;
  }
  BeginWait();
  fprintf(tab, "Ch\tR\tline\tT1\tP1\tT2\tP2\tValue\tI+\tI-\tI\n");
  {
    for (auto G : GaugeIterator())  {
      G->LockI();
      fprintf(tab, "%s\t%s\t%d\t", G->ChNum, ftoa(G->radius), G->angle);
      if (G->pts[0] == -1) {
        fprintf(tab, "-\t-\t");
      } else {
        fprintf(tab, "%s\t%.3f\t", ftoa(G->Tp(0)), G->Pp(0));
      }
      if (G->pts[1] == -1) {
        fprintf(tab, "-\t-\t");
      } else {
        fprintf(tab, "%s\t%.3f\t", ftoa(G->Tp(1)), G->Pp(1));
      }
      if (G->pts[0] != -1 && G->pts[1] != -1) {
        fprintf(tab, "%.3f\t", G->Pp(1) - G->Pp(1));
      } else {
        fprintf(tab, "-\t");
      }
      fprintf(tab, "%s\t", ftoa(G->Ipos));
      fprintf(tab, "%s\t", ftoa(G->Ineg));
      fprintf(tab, "%s\n", ftoa(G->Itot));
    }
  }
  fclose(tab);
  sprintf(buf, "notepad.exe %s", tabname);
  EndWait();
  WinExec(buf, SW_NORMAL);
}
