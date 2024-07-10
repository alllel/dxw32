#ifndef DXW_H
#define DXW_H

#include "dxw_rc.h"
#include <commdlg.h>
#include <cstdio>
#include <memory.h>

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

extern char buf[256];
extern char fname[260];
extern HINSTANCE hInst;
extern HWND hFrame, hMDI;
extern HGLOBAL hDevMode, hDevNames;
//extern char HeadNam[20];
extern char ExpName[20];
//extern char CalName[20];
//extern char TimeExp[26];
//extern char Comment[66];
//extern enum CS {SF=0,LF=1,SO=2,LO=3} ChanSel;
extern double TICKS;

#define GMAX 100

extern int nGauges;
extern int Changed;
extern int TitleChanged;

//Cut data
extern HWND hCapWin;
extern int hResize;
extern int vResize;

//Digitize window;
extern HWND hDig;
extern HWND hInfo;
//Not necessary because smart callbacks used
//extern FARPROC hDigPr;

//GDI objects
extern HBRUSH hbrNull, hbrBlack, hbrGray;
extern HPEN hpPnt, hpPts, hpCut, hpImp, hpDef;

WINPROC(MainWin);
WINPROC(ChannellWin);
WINPROC(ChildWinProc);
WINPROC(RTwin);
DLGPROC(DigDlg);
//DLGPROC(OpenDlg);
//DLGPROC(DirDlg);
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

#define FIRST_CMD 20000
#define FIRST_WND 300

extern char Directory[];
extern char MainDir[];
extern char HeadDir[];
extern char CalDir[];
extern char DataDir[];
struct OFN : OPENFILENAME {
  OFN();
};

struct FileData : OFN {
  FileData();
};

//extern FileData ofn;

void GetDirs();
void SaveDirs();
int ChDir(char*);
void cbs(char*);
int OpenExp();
void SetTitle();
void Digitize();
void WriteTable();
void Info();
void ReadInfo();
void WriteInfo();
void ReadHeadTitles(FILE*);
bool FileExist(char* file);
HFILE LZOpen(char* file);
void AlignWindow(HWND hwnd, int t, int l, int b, int r, int howy, int howx);
inline void
AlignWindow(HWND hwnd, RECT& rc, int howy, int howx) { AlignWindow(hwnd, rc.top, rc.left, rc.bottom, rc.right, howy, howx); }

#define T_TOP 1
#define T_BOTTOM 3
#define T_LEFT 8
#define T_RIGHT 24
#define T_CENTER 0

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
