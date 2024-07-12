#ifndef DXW_H
#define DXW_H

#include "dxw_rc.h"
#include <commdlg.h>
#include <cstdio>
#include <memory.h>
#include <vector>
#include <string>

#define WINPROC(name) LRESULT CALLBACK name( \
    HWND hWnd,                               \
    UINT msg,                                \
    WPARAM wParam,                           \
    LPARAM lParam)

#define DLGPROC(name) BOOL CALLBACK name( \
    HWND hDlg,                            \
    UINT msg,                             \
    WPARAM wParam,                        \
    LPARAM lParam)

extern char buf[MAX_PATH];
extern char fname[MAX_PATH + 4];
struct Recent {
  std::vector<std::string> files;
  bool changed = true;
  void AddFile(std::string const&);
  [[nodiscard]] std::string const& operator[](size_t i) const { return files[i]; }
  [[nodiscard]] size_t size() const { return files.size(); }
};
extern Recent recent;
extern HINSTANCE hInst;
extern HWND hFrame, hMDI;
extern HGLOBAL hDevMode, hDevNames;
extern char ExpName[20];

extern int Changed;

//Cut data
extern HWND hCapWin;
extern int hResize;
extern int vResize;

//Digitize window;
extern HWND hDig;
extern HWND hInfo;

//GDI objects
extern HBRUSH hbrNull, hbrGray;
extern HPEN hpPnt, hpPts, hpCut, hpImp, hpDef;

WINPROC(MainWin);
WINPROC(ChannellWin);
WINPROC(ChildWinProc);
WINPROC(RTwin);
DLGPROC(DigDlg);
DLGPROC(AboutDlg);
DLGPROC(StepDlg);
DLGPROC(RTdlg);
DLGPROC(UnitProc);

int DLG(int ID, DLGPROC(Proc), LPARAM lp = 0);

void GetDlgItemDouble(HWND hDlg, int ctrl, double& val);
void SetDlgItemDouble(HWND hDlg, int ctrl, double val);
void BeginWait();
void EndWait();
char* Uscale(int, const char*, const char*);

#define FIRST_WND 300

extern char Directory[81];
struct OFN : OPENFILENAME {
  OFN();
};

struct FileData : OFN {
  FileData();
};

//extern FileData ofn;

void GetDirs();
void SaveDirs();
int OpenExp();
int OpenExp(std::string_view);
void SetTitle();
void Digitize();
void WriteTable();
void Info();
void ReadInfo();
void WriteInfo();
void AlignWindow(HWND hwnd, int t, int l, int b, int r, int howy, int howx);
inline void
AlignWindow(HWND hwnd, RECT& rc, int howy, int howx) { AlignWindow(hwnd, rc.top, rc.left, rc.bottom, rc.right, howy, howx); }

void Text(HDC hdc, int x, int y, char const* s, UINT flags);

#ifdef NDEBUG
constexpr bool Dbg = false;
#else
constexpr bool Dbg = true;
#endif

#define DbgOut \
  if (!Dbg) {  \
  } else       \
    std::cerr << __PRETTY_FUNCTION__ << ' ' << __FILE__ << ':' << __LINE__ << "\n ----> "

#endif
