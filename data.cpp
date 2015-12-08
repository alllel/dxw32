#define STRICT
#include <windows.h>
#include "dxw.h"

char buf[256];
char fname[260];
char Directory[81];
char ExpName[20];
double TICKS=25;
int nGauges=0;
int Changed=0;
int TitleChanged=0;
char MainDir[81];
char HeadDir[81];
char CalDir[81];
char DataDir[81];

//Digitize window
HWND hDig=NULL;
HWND hInfo=NULL;
//Not necessary because smart callbacks used
//FARPROC hDigPr=NULL;

//GDI objects
HBRUSH hbrNull=NULL,hbrBlack=NULL,hbrGray=NULL;
HPEN hpPnt=NULL,hpPts=NULL,hpImp=NULL,hpDef=NULL;

void GetDirs(void)
{
GetPrivateProfileString("Krenz","Directory",".",Directory,sizeof(Directory),"dxw.ini");
GetPrivateProfileString("directories","main",".",     MainDir,sizeof(MainDir),"dxw.ini");
GetPrivateProfileString("directories","head","HEAD\\",HeadDir,sizeof(HeadDir),"dxw.ini");
GetPrivateProfileString("directories","data","DATA\\",DataDir,sizeof(DataDir),"dxw.ini");
GetPrivateProfileString("directories","cal" ,"CAL\\", CalDir ,sizeof(CalDir) ,"dxw.ini");
}

void SaveDirs(void)
{
WritePrivateProfileString("Krenz","Directory",Directory,"dxw.ini");
WritePrivateProfileString("directories","main",MainDir,"dxw.ini");
WritePrivateProfileString("directories","head",HeadDir,"dxw.ini");
WritePrivateProfileString("directories","data",DataDir,"dxw.ini");
WritePrivateProfileString("directories","cal" ,CalDir ,"dxw.ini");
}
