#define STRICT
#include <windows.h>
#include "dxw.h"
#include "gauge.h"
#include <lzexpand.h>
#include <cstring>
#include <fstream>

bool
Gauge::ReadD() {
  auto dat = std::fstream { FileName(), std::ios::in | std::ios::binary };
  if (dat.bad()) return false;
  dat.seekg(FilePos);
  auto len = count * sizeof(short);
  dat.read(reinterpret_cast<char*>(val.data()), count * sizeof(short int));
  return true;
}
