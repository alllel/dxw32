#define STRICT
#include <windows.h>
#include "dxw.h"
#include "twnd.h"
#include <commdlg.h>

HGLOBAL hDM=NULL;
HGLOBAL hDN=NULL;
/*
static void tst(HDC hdc,RECT&rc)
{
MoveToEx(hdc,rc.left,rc.top,NULL);
LineTo(hdc,(rc.left+rc.right)/2,rc.bottom);
LineTo(hdc,rc.right,rc.top);
LineTo(hdc,rc.left,(rc.top+rc.bottom)/2);
LineTo(hdc,rc.right,rc.bottom);
LineTo(hdc,(rc.left+rc.right)/2,rc.top);
LineTo(hdc,rc.left,rc.bottom);
LineTo(hdc,rc.right,(rc.top+rc.bottom)/2);
LineTo(hdc,rc.left,rc.top);
}
*/
void Window::ToPrinter()
{
PRINTDLG pd;
pd.lStructSize=sizeof(pd);
pd.hwndOwner=hFrame;
pd.hDevMode=hDM;
pd.hDevNames=hDN;
pd.nCopies=1;
pd.Flags=PD_NOPAGENUMS|PD_NOSELECTION|PD_RETURNDC;
if(!PrintDlg(&pd))return;
hDM=pd.hDevMode;
hDN=pd.hDevNames;
DOCINFO di;
di.cbSize=sizeof(di);
di.lpszDocName="channell";
di.lpszOutput=NULL;
HCURSOR hc=SetCursor(LoadCursor(NULL,IDC_WAIT));
StartDoc(pd.hDC,&di);
StartPage(pd.hDC);
SetMapMode(pd.hDC,MM_TEXT);
RECT rc;
rc.top=0;
rc.left=0;
rc.right=GetDeviceCaps(pd.hDC,HORZRES);
rc.bottom=GetDeviceCaps(pd.hDC,VERTRES);
Draw(pd.hDC,rc,Printer);
//tst(pd.hDC,rc);
EndPage(pd.hDC);
EndDoc(pd.hDC);
DeleteDC(pd.hDC);
SetCursor(hc);
}

