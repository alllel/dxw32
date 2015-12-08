#define STRICT
#include <windows.h>
#include "dxw.h"
#include "gauge.h"
#include <stdlib.h>
#include <string.h>

//ARGSUSED
DLGPROC(InfoProc)
{
char*p;
HWND hChan;
switch(msg){
	case WM_INITDIALOG:
		hInfo=hDlg;
		hChan=(HWND)SendMessage(hMDI,WM_MDIGETACTIVE,0,0);
		if(hChan){
			Gauge*G=GaugeByWnd(hChan);
			if(G){
				G->SetInfo();
			}
		}
		return TRUE;
	case WM_DESTROY:
		hInfo=NULL;
		return FALSE;
	case WM_COMMAND:
		switch(wParam){
			case IDCANCEL:
				DestroyWindow(hDlg);
				break;
			case IDOK:{
				GetWindowText(hDlg,buf,10);
				p=strchr(buf,' ');
				if(p)*p=0;
				Gauge*G=GaugeByChNum(buf);
				if(G){
					GetDlgItemText(hDlg,IDC_INF_CHN,buf,10);
					p=strchr(buf,' ');
					if(p)*p=0;
					strncpy(G->ChNum,buf,5);
					GetDlgItemText(hDlg,IDC_INF_GAU,buf,10);
					G->number=(int)strtol(buf,&p,10);
					G->g_type=*p;
					if(!G->g_type)G->g_type=' ';
					GetDlgItemText(hDlg,IDC_INF_LIN,buf,10);
					G->angle=atoi(buf);
					GetDlgItemText(hDlg,IDC_INF_RAD,buf,10);
					G->radius=atof(buf);
					G->SetTitle();
					G->SetInfo();
					SetFocus(G->hWnd);
					G->UnlockGauge();
				}
			}break;
		}
		return TRUE;
}
return FALSE;
}

void Info()
{
	if(!hInfo){
		if(nGauges){
			CreateDialog(hInst,MAKEINTRESOURCE(ID_INFO),hFrame,InfoProc);
			AlignWindow(hInfo,0,0,GetSystemMetrics(SM_CYSCREEN),GetSystemMetrics(SM_CXSCREEN),1,1);
		}
	}else{
		DestroyWindow(hInfo);
	}
}

void ReadInfo()
{
OPENFILENAME ofn;
ofn.lStructSize=sizeof(ofn);
ofn.hwndOwner=hFrame;
ofn.hInstance=hInst;
ofn.lpstrFilter="Info (*.txt)\0*.txt\0All Files\0*.*\0";
ofn.lpstrCustomFilter=NULL;
ofn.nMaxCustFilter=0;
ofn.nFilterIndex=1;
ofn.lpstrFile=fname;
ofn.nMaxFile=sizeof(fname)-1;
ofn.lpstrFileTitle=NULL;
ofn.nMaxFileTitle=0;
ofn.lpstrInitialDir=Directory;
ofn.lpstrTitle="Read Info from file:";
ofn.Flags=OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;
ofn.lpstrDefExt="TXT";
ofn.lCustData=NULL;
ofn.lpfnHook=NULL;
ofn.lpTemplateName=NULL;
fname[0]=0;
if(!GetOpenFileName(&ofn))return;
FILE*inf=fopen(fname,"rt");
if(!inf){
	MessageBox(hFrame,"Can't open file",fname,MB_OK|MB_ICONEXCLAMATION);
	return;
}
char *p,*s,T;
int N,A;
double R;
while(!feof(inf)){
	fgets(buf,sizeof(buf),inf);
	p=buf;
	while(*p==' ' || *p=='\t')++p;
	s=p;
	while(*s!=' ' && *s!='\t' && *s!='\n')++s;
	if(*s=='\n')continue;
	*s=0;

	p=s+1;
	while(*p==' ' || *p=='\t')++p;
	s=p;
	while(*s!=' ' && *s!='\t' && *s!='\n')++s;
	if(*s=='\n')continue;
	*s=0;
	N=(int)strtol(p,&p,10);
	T=*p;
	if(!T)T=' ';

	p=s+1;
	while(*p==' ' || *p=='\t')++p;
	s=p;
	while(*s!=' ' && *s!='\t' && *s!='\n')++s;
	if(*s=='\n')continue;
	*s=0;
	R=strtod(p,&p);
	if(*p)continue;

	p=s+1;
	while(*p==' ' || *p=='\t')++p;
	s=p;
	while(*s!=' ' && *s!='\t' && *s!='\n')++s;
	*s=0;
	A=(int)strtol(p,&p,10);
	if(*p)continue;

	Gauge*G=GaugeByChNum(buf);
	if(!G)continue;
	G->number=N;
	G->g_type=T;
	G->radius=R;
	G->angle =A;

	G->UnlockGauge();
}
fclose(inf);
}

void WriteInfo()
{
OPENFILENAME ofn;
ofn.lStructSize=sizeof(ofn);
ofn.hwndOwner=hFrame;
ofn.hInstance=hInst;
ofn.lpstrFilter="Info (*.txt)\0*.txt\0All Files\0*.*\0";
ofn.lpstrCustomFilter=NULL;
ofn.nMaxCustFilter=0;
ofn.nFilterIndex=1;
ofn.lpstrFile=fname;
ofn.nMaxFile=sizeof(fname)-1;
ofn.lpstrFileTitle=NULL;
ofn.nMaxFileTitle=0;
ofn.lpstrInitialDir=Directory;
ofn.lpstrTitle="Write info to file:";
ofn.Flags=OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY;
ofn.lpstrDefExt="TXT";
ofn.lCustData=NULL;
ofn.lpfnHook=NULL;
ofn.lpTemplateName=NULL;
fname[0]=0;
if(!GetSaveFileName(&ofn))return;
FILE*inf=fopen(fname,"wt");
if(!inf){
	MessageBox(hFrame,"Can't open file",fname,MB_OK|MB_ICONEXCLAMATION);
	return;
}
GaugeIterator G;
for(;G;++G){
	fprintf(inf,"%s %d%c %lg %d\n",G->ChNum,G->number,G->g_type,G->radius,G->angle);
}
fclose(inf);
}

