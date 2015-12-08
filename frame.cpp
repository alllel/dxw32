#define STRICT
#include <windows.h>
#include "dxw.h"
#include "gauge.h"
#include <commdlg.h>
#include <stdlib.h>
#include <memory.h>

static int CloseAll(void);
static int Command(WPARAM);
static void PrSetup();
void Write(BOOL);
int ReadAscii();
int OpenDL();

WINPROC(MainWin)
{
HWND hChld;
WORD cmd;
switch(msg){
	case WM_INITMENUPOPUP:
		if(LOWORD(lParam)==2){
			CheckMenuItem((HMENU)wParam,CM_DIGIT,MF_BYCOMMAND|(hDig?MF_CHECKED:MF_UNCHECKED));
			CheckMenuItem((HMENU)wParam,CM_INFO,MF_BYCOMMAND|(hInfo?MF_CHECKED:MF_UNCHECKED));
		}
		goto def;
//	case WM_SETFOCUS:
//		SetFocus(hMDI);
//		return 0;
	case WM_CLOSE:
		if(CloseAll())DestroyWindow(hFrame);
		return 0l;
	case WM_DESTROY:
		PostQuitMessage(0);
		goto def;
	case WM_COMMAND:
		cmd=LOWORD(wParam);
		if((cmd>=CM_FIRST) && (cmd<CM_LAST)){
			hChld=(HWND)SendMessage(hMDI,WM_MDIGETACTIVE,0,0);
			if(!hChld || !SendMessage(hChld,WM_COMMAND,cmd,lParam))
				Command(cmd);
			return 0l;
		}
	default:
		def:return DefFrameProc(hWnd,hMDI,msg,wParam,lParam);
}
}

int CloseAll(void)
{
HWND hChild;
if(Changed)switch(MessageBox(hFrame,"Save experiment?","DXW",MB_ICONQUESTION|MB_YESNOCANCEL)){
	case IDYES:
		Write(FALSE);
		if(Changed)return 0;
		break;
	case IDCANCEL:
		return 0;
}
Changed=0;
hChild=(HWND)SendMessage(hMDI,WM_MDIGETACTIVE,0,0);
if(hChild && IsZoomed(hChild)){
	SendMessage(hMDI,WM_MDIRESTORE,(WPARAM)hChild,0);
}//if
while(hChild){
	if(SendMessage(hMDI,WM_MDIDESTROY,(WPARAM)hChild,0))return 0;
	hChild=(HWND)SendMessage(hMDI,WM_MDIGETACTIVE,0,0);
}//while
if(hDig)DestroyWindow(hDig);
if(hInfo)DestroyWindow(hInfo);
SetTitle();
return 1;
}

static int RestAll(void)
{
HWND hChld,hCurr;
hCurr=hChld=(HWND)SendMessage(hMDI,WM_MDIGETACTIVE,0,0);
if(!hChld)return 0;
do{
	if(IsZoomed(hChld)||IsIconic(hChld))SendMessage(hMDI,WM_MDIRESTORE,(WPARAM)hChld,0);
	SendMessage(hMDI,WM_MDINEXT,(WPARAM)hChld,0);
	hChld=(HWND)SendMessage(hMDI,WM_MDIGETACTIVE,0,0);
}while(hChld!=hCurr);
return 1;
}

static int MinAll(void)
{
HWND hChld,hCurr;
hCurr=hChld=(HWND)SendMessage(hMDI,WM_MDIGETACTIVE,0,0);
if(!hChld)return 0;
if(IsZoomed(hChld))SendMessage(hMDI,WM_MDIRESTORE,(WPARAM)hChld,0);
do{
	if(!IsIconic(hChld))ShowWindow(hChld,SW_MINIMIZE);
	SendMessage(hMDI,WM_MDINEXT,(WPARAM)hChld,0);
	hChld=(HWND)SendMessage(hMDI,WM_MDIGETACTIVE,0,0);
}while(hChld!=hCurr);
return 1;
}

static void Compress()
{
sprintf(buf,"compress -r %s*.dat",Directory);
WinExec(buf,SW_NORMAL);
}

void RenumberChannels();

int Command(WPARAM cmd)
{
switch(cmd){
	case CM_EXPINFO:
		if(Experiment::nExp==1){
			GaugeIterator G;
			sprintf(buf,"notepad.exe %s",G->Exp->File("DSC"));
			WinExec(buf,SW_RESTORE);
		}
		break;
	case CM_EXIT:
		SendMessage(hFrame,WM_CLOSE,0,0);
		break;
	case CM_OPEN:
		//if(CloseAll() /* && DLG(ID_OPEN,OpenDlg)==1*/)
		OpenExp();
		if(Experiment::nExp>1)Changed=1;
		break;
	case CM_OPENDL:
		//if(CloseAll() /* && DLG(ID_OPEN,OpenDlg)==1*/)
		OpenDL();
		if(Experiment::nExp>1)Changed=1;
		break;
	case CM_SAVE:
		Write(FALSE);
		break;
	case CM_SAVEAS:
		Write(TRUE);
		break;
	case CM_IMPORT:
		if(!ReadAscii()){
			SetTitle();
			Changed=1;
		}
      break;
	case CM_RINF:
		ReadInfo();
		break;
	case CM_WINF:
		WriteInfo();
		break;
	case CM_ABOUT:
		DLG(ID_ABOUT,AboutDlg);
		break;
	case CM_PR_SETUP:
		PrSetup();
		break;
// Standard MDI commands
	case CM_TILE:
		SendMessage(hMDI,WM_MDITILE,MDITILE_HORIZONTAL,0);
		break;
	case CM_CASCADE:
		SendMessage(hMDI,WM_MDICASCADE,0,0);
		break;
	case CM_ARNG:
		SendMessage(hMDI,WM_MDIICONARRANGE,0,0);
		break;
	case CM_CL_ALL:
		CloseAll();
		break;
	case CM_REST_ALL:
		RestAll();
		break;
	case CM_MIN_ALL:
		MinAll();
		break;
	case CM_DIGIT:
		Digitize();
		break;
	case CM_HELP:
		WinHelp(hFrame,"dxw.hlp",HELP_CONTENTS,0l);
		break;
	case CM_TABLE:
		if(nGauges)WriteTable();
		break;
	case CM_R_T:
		if(nGauges)DLG(ID_RTD,RTdlg);
		break;
	case CM_COMPR:
		Compress();
		break;
	case CM_RENUMBER:
		RenumberChannels();
		break;
	default:
		return 0;
}
return 1;
}

void PrSetup()
{
PRINTDLG PD;
memset(&PD,0,sizeof(PD));
PD.lStructSize=sizeof(PD);
PD.hwndOwner=hFrame;
PD.Flags=PD_PRINTSETUP;
PD.hDevMode =hDevMode;
PD.hDevNames=hDevNames;
PrintDlg(&PD);
hDevMode =PD.hDevMode;
hDevNames=PD.hDevNames;
}

