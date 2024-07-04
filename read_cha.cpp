#define STRICT
#include <windows.h>
#include "dxw.h"
#include "gauge.h"
#include <lzexpand.h>
#include <cstring>

BOOL
FileExist(char* file) {
  HFILE h = LZOpen(file);
  if (h == HFILE_ERROR) {
    return 0;
  }
  LZClose(h);
  return 1;
}

HFILE
LZOpen(char* file) {
  HFILE hFile;
  OFSTRUCT of;
  of.cBytes = sizeof(of);
  int l;
  char c;
  hFile = LZOpenFile(file, &of, OF_READ);
  if (hFile == HFILE_ERROR) {
    l           = strlen(file);
    file[l]     = '_';
    file[l + 1] = 0;
    hFile       = LZOpenFile(file, &of, OF_READ);
    file[l]     = 0;
    if (hFile == HFILE_ERROR) {
      c           = file[l - 1];
      file[l - 1] = '_';
      hFile       = LZOpenFile(file, &of, OF_READ);
      file[l]     = c;
    }
  }
  return hFile;
}

int
ReadLZ(HFILE h, void* ptr, long len) { return (LZRead(h, (LPSTR) ptr, len) == len) ? 0 : -1; }

BOOL
Gauge::ReadD() {
  HFILE hFile;
  long len;
  hFile = LZOpen(FileName());
  if (hFile == HFILE_ERROR) {
    return FALSE;
  }
  if (LZSeek(hFile, FilePos, 0) != FilePos) {
    LZClose(hFile);
    return FALSE;
  }
  len = count * sizeof(short);
  if (ReadLZ(hFile, val, len) < 0) {
    LZClose(hFile);
    return FALSE;
  }
  LZClose(hFile);
  return TRUE;
}
