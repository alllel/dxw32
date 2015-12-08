#define STRICT
#include <windows.h>
#include "dxw.h"
#include "gauge.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <lzexpand.h>
#include <commdlg.h>

int OpenExp()
{
Experiment*E;
buf[0]=0;
ofn.hwndOwner=hFrame;
ofn.Flags=OFN_HIDEREADONLY|OFN_FILEMUSTEXIST;
if(!GetOpenFileName(&ofn))return 0;
E=SetupExp();
E->Inc();
{for(GaugeIterator G;G;++G)if(*(G->Exp)==*E){
	MessageBox(hFrame,"Experiment is already open!",NULL,MB_OK|MB_ICONEXCLAMATION);
	E->Dec();
	return 0;
}}
FILE*fp=fopen(E->IXC(),"rt");
if(!fp){
	MessageBox(hFrame,"Can't open file",buf,MB_OK|MB_ICONSTOP);
	E->Dec();
	return 0;
}

Gauge* NG;
MDICREATESTRUCT cs;
cs.szClass="DXWchannel";
cs.hOwner=hInst;
cs.x=CW_USEDEFAULT;
cs.y=CW_USEDEFAULT;
cs.cx=CW_USEDEFAULT;
cs.cy=CW_USEDEFAULT;
cs.style=WS_MINIMIZE;
cs.lParam=0;
int c,i;
long datapos=0;
do{
	unsigned short h,min,s,d,m,y;
	if(!fgets(buf,200,fp))break;
	NG=new Gauge(E);
	c=sscanf(buf,"%*1d:%[0-9A-Z#]\\%16c\\%lg\\%lg\\%3c\\Time %hu.%hu.%hu \\Date %hu:%hu:%hu",
		NG->ChNum,
		NG->ID,
		&NG->dV,
		&NG->V0,
		NG->unit,
		&h,&min,&s,
		&d,&m,&y
	);
	if(c!=11){
		delete NG;
		break;
	}
	NG->time.hour=h;NG->time.min=min;NG->time.sec=s;
	NG->date.day=d;NG->date.month=m;NG->date.year=y;
	NG->FilePos=datapos;
	NG->nRates=0;
	long pos=ftell(fp);
	do{
		++(NG->nRates);
		if(!fgets(buf,200,fp))break;
	}while(buf[0]=='\\');
	fseek(fp,pos,0);
	--(NG->nRates);
	if(!NG->nRates){
		delete NG;
		break;
	}
	NG->Rates=new Gauge::Piece[NG->nRates];
	NG->count=0;
	for(i=0;i<NG->nRates;++i){
		fgets(buf,200,fp);
		if(sscanf(buf,"\\%lu\\%lE\\%lE",&NG->Rates[i].Np,&NG->Rates[i].rate,&NG->Rates[i].Tstart)!=3)break;
		NG->count+=NG->Rates[i].Np;
	}
	datapos+=NG->count*sizeof(short int);
	NG->Setup();
	cs.szTitle=NG->WinTitle();
	NG->Create(cs);
	if(NG->hWnd){
		NG->UnlockGauge();
	}else{
		delete NG;
	}
}while(1);
E->Dec();
fclose(fp);
if(!nGauges)MessageBox(hFrame,"No channels available",ExpName,MB_OK|MB_ICONINFORMATION);
SetTitle();
return 0;
}

void SetTitle(void)
{
switch(Experiment::nExp){
	case 1:{
		sprintf(buf,"DXW:%s (%d channels)",ExpName,nGauges);
		SetWindowText(hFrame,buf);
	}break;
	case 0:{
		Changed=0;
		SetWindowText(hFrame,"DX for Windows");
	}break;
	default:{
		sprintf(buf,"DXW:%d exp (%d channels)",Experiment::nExp,nGauges);
		SetWindowText(hFrame,buf);
	}break;
}
TitleChanged=0;
}

