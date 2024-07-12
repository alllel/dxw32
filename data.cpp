#define STRICT
#include <windows.h>
#include "dxw.h"

char buf[MAX_PATH];
char fname[MAX_PATH + 4];
Recent recent;
char Directory[81];
char ExpName[20];
int Changed      = 0;

//Digitize window
HWND hDig  = nullptr;
HWND hInfo = nullptr;

//GDI objects
HBRUSH hbrNull = nullptr, hbrGray = nullptr;
HPEN hpPnt = nullptr, hpPts = nullptr, hpImp = nullptr, hpDef = nullptr;

void
GetDirs() {
  GetPrivateProfileString("Krenz", "Directory", ".", Directory, std::size(Directory), "dxw.ini");
  int N   = GetPrivateProfileInt("Recent", "N", 0, "dxw.ini");
  char cidx[4] = { "C" };
  for (int i = N-1; i >=0; --i) {
    itoa(i, cidx + 1, 10);
    GetPrivateProfileString("Recent", cidx, "", buf, std::size(buf), "dxw.ini");
    if (buf[0]) {
      recent.AddFile(buf);
    }
  }
}

void
SaveDirs() {
  WritePrivateProfileString("Krenz", "Directory", Directory, "dxw.ini");
  int N = static_cast<int>(recent.size());
  WritePrivateProfileString("Recent", "N", itoa(N, buf, 10), "dxw.ini");
  char cidx[4] = { "C" };
  for (int i = 0; i < N; ++i) {
    itoa(i, cidx + 1, 10);
    WritePrivateProfileString("Recent", cidx, recent[i].c_str(), "dxw.ini");
  }
}

void
Recent::AddFile(const std::string& path) {
  if (!files.empty() && files[0] == path) return;
  changed = true;
  files.insert(files.begin(), path);
  auto it = std::find(std::next(files.begin(), 1), files.end(), path);
  if (it != files.end()) {
    files.erase(it);
  }
  if (files.size() > 9) files.resize(9);
}