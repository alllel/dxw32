#define STRICT
#include <windows.h>
#include "dxw.h"
#include <algorithm>
#include "RegKey.hpp"

char buf[MAX_PATH];
char fname[MAX_PATH + 4];
Recent recent;
char Directory[81];
char ExpName[20];
int Changed = 0;

//Digitize window
HWND hDig  = nullptr;
HWND hInfo = nullptr;

//GDI objects
HBRUSH hbrNull = nullptr, hbrGray = nullptr;
HPEN hpPnt = nullptr, hpPts = nullptr, hpImp = nullptr, hpDef = nullptr;

void
GetDirs() {
  RegKey settings { R"(Software\KIT\Dxw32)" };
  if (!settings) return;
  auto dir = settings.GetString(nullptr);
  if (!dir.empty()) std::strncpy(Directory, dir.data(), std::size(Directory));
  auto N       = settings.GetDword("Nlast");
  char cidx[4] = { "C" };
  for (int i = N - 1; i >= 0; --i) {
    itoa(i, cidx + 1, 10);
    auto recent_name = settings.GetString(cidx);
    if (!recent_name.empty()) {
      recent.AddFile(recent_name);
    }
  }
}

void
SaveDirs() {
  RegKey settings { R"(Software\KIT\Dxw32)" };
  if (!settings) return;
  settings.SetDefStr(Directory);
  auto N = static_cast<DWORD>(recent.size());
  settings.SetDword("Nlast", N);
  char cidx[4] = { "C" };
  for (int i = 0; i < N; ++i) {
    itoa(i, cidx + 1, 10);
    settings.SetStr(cidx, recent[i].c_str());
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