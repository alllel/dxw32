#define STRICT
#include <windows.h>
#include <stdlib.h>
#include "dxw.h"
#include "gauge.h"
#include "rt.h"
#include <string.h>
#include <ctype.h>

void trim(char*str);

void RTbyGaugeIterator::release()
{
if(rt)rt->UnlockWindow();
rt=NULL;
}

void RTbyGaugeIterator::step()
{
int i;
Window*W;
release();
if(hWin)hWin=::GetWindow(hWin,GW_HWNDNEXT);
else hWin=::GetWindow(G->hWnd,GW_HWNDFIRST);
while(hWin){
	W=Window::GetWindow(hWin);
	if(W){
		if(W->WinType=='R'){
			rt=(RT*)W;
			for(i=0;i<rt->n;i++){
				if(rt->Chns[i]==G->hThis)return;
			}
			rt=NULL;
		}
		W->UnlockWindow();
	}
	hWin=::GetWindow(hWin,GW_HWNDNEXT);
}
}


HLOCAL Gauge::hFirst=NULL;
HLOCAL Gauge::hLast =NULL;

int Experiment::nExp=0;

Experiment* SetupExp()
{
Experiment*E=NULL;
if(ofn.nFileExtension){
	buf[ofn.nFileExtension-1]=0;
	lstrcpy(ExpName,buf+ofn.nFileOffset);
}
buf[ofn.nFileOffset]=0;
lstrcpy(Directory,buf);
cbs(Directory);
E=new Experiment(Directory,ExpName);
return E;
}

Experiment::Experiment(char*D,char*N)
{
Name=strdup(N);
Dir=strdup(D);
count=0;
nExp++;
}

Experiment::~Experiment()
{
if(Name)delete Name;
if(Dir)delete Dir;
nExp--;
}

char*Experiment::File(const char*Ext)
{
sprintf(fname,"%s%s.%.3s",Dir,Name,Ext);
return fname;
}

int Experiment::operator==(Experiment&E)
{
if(stricmp(Name,E.Name))return 0;
if(stricmp(Dir,E.Dir))return 0;
return 1;
}


Gauge*GaugeIterator::operator++()
{
if(!curr)return curr;
HLOCAL h=curr->hNext;
curr->UnlockGauge();
curr=Gauge::LockGauge(h);
return curr;
}

GaugeIterator::~GaugeIterator()
{
if(curr)curr->UnlockGauge();
}

Gauge::Gauge(Experiment*e)
{
nGauges++;
if(hLast){
	Gauge*last=LockGauge(hLast);
	last->hNext=hThis;
	hPrev=last->hThis;
	last->UnlockGauge();
}else{
	hFirst=hThis;
	hPrev=NULL;
}
hNext=NULL;
hLast=hThis;

Exp=e;
if(Exp)Exp->Inc();
WinType='G';
ChNum[0]=0;
Z0=0;
ID[0]=0;
Z1=0;
unit[0]=0;
Z2=0;
Rates=NULL;
nRates=0;
Zero_corr=0;
hVal=hImp=NULL;
PolyP=PolyI=NULL;
val=NULL;
imp=NULL;
count=0;
Curr=-1;
pts[0]=pts[1]=-1;
rcValid=FALSE;
frcValid=FALSE;
drawI=FALSE;
_Upper=_Lower=0;
}

Gauge* GaugeByChNum(char*cn)
{
for(GaugeIterator G;G;++G){
	if(!stricmp(cn,G->ChNum))return G;
}
return NULL;
}

char*Gauge::WinTitle()
{
sprintf(buf,"%s A%d R=%g %s",
				ChNum,angle,radius,unit);
return buf;
}

char*Gauge::PicName() //virtual
{
#ifdef _WIN32
sprintf(buf,"%s%.4s%s",Exp->Dir,Exp->Name,ChNum);
#else
int i,l1,l2,l3;
char chnum[6];
strcpy(buf,Exp->Dir);
l1=strlen(buf);
l2=strlen(Exp->Name);
l3=0;
for(i=0;ChNum[i];++i){
	if(isalnum(ChNum[i])){
		chnum[l3]=ChNum[i];
		++l3;
	}
}
chnum[l3]=0;
if(l2+l3>8)l2=8-l3;
strncpy(buf+l1,Exp->Name,l2);
strcpy(buf+l1+l2,chnum);
#endif
return buf;
}

Gauge::~Gauge() //virtual
{
nGauges--;
if(hPrev){
	Gauge*prev=LockGauge(hPrev);
	prev->hNext=hNext;
	prev->UnlockGauge();
}else{
	hFirst=hNext;
}
if(hNext){
	Gauge*next=LockGauge(hNext);
	next->hPrev=hPrev;
	next->UnlockGauge();
}else{
	hLast=hPrev;
}
if(Exp)Exp->Dec();
FreeD();
if(Rates)delete Rates;
if(PolyI)delete PolyI;
if(PolyP)delete PolyP;

TitleChanged=1;
SetTitle();
}

void Gauge::LockD()
{
if(!count)return;
long l=sizeof(int)*count;
int rd;
if(!val){
	if(!hVal){
		hVal=GlobalAlloc(GMEM_MOVEABLE|GMEM_DISCARDABLE,l);
		rd=1;
	}else if(GlobalFlags(hVal)&GMEM_DISCARDED){
		hVal=GlobalReAlloc(hVal,l,0);
		rd=1;
	}else{
		rd=0;
	}
	val=(short int _huge *)GlobalLock(hVal);
	if(rd){
		if(!ReadD())MessageBox(hFrame,"Error Reading Channell",NULL,MB_OK);
	}
}
}

void Gauge::LockI()
{
if(!count)return;
LockD();
long l=count*sizeof(float);
int rd;
if(!imp){
	if(!hImp){
		hImp=GlobalAlloc(GMEM_MOVEABLE|GMEM_DISCARDABLE,l);
		rd=TRUE;
	}else if(GlobalFlags(hImp)&GMEM_DISCARDED){
		hImp=GlobalReAlloc(hImp,l,0);
		rd=TRUE;
	}else{
		rd=FALSE;
	}
	imp=(float _huge *)GlobalLock(hImp);
	if(rd)CalcI();
}
return;
}

void Gauge::AtUnlock() //virtual
{
UnlockD();
}

void Gauge::SetInfo()
{
if(hInfo){
	sprintf(buf,"%s - Info",ChNum);
	SetWindowText(hInfo,buf);
	SetDlgItemText(hInfo,IDC_INF_CHN,ChNum);
	sprintf(buf,"%d%c",number,g_type);
	SetDlgItemText(hInfo,IDC_INF_GAU,buf);
	SetDlgItemText(hInfo,IDC_INF_LIN,itoa(angle,buf,10));
	SetDlgItemText(hInfo,IDC_INF_RAD,gcvt(radius,5,buf));
	SetDlgItemText(hInfo,IDC_INF_ST,gcvt(Start(),5,buf));
	SetDlgItemText(hInfo,IDC_INF_FI,gcvt(Final(),5,buf));
	SetDlgItemText(hInfo,IDC_INF_LO,gcvt(Lower(),5,buf));
	SetDlgItemText(hInfo,IDC_INF_UP,gcvt(Upper(),5,buf));
	if(drawI){
		LockI();
		SetDlgItemText(hInfo,IDC_INF_IP,gcvt(Ipos,5,buf));
		SetDlgItemText(hInfo,IDC_INF_IM,gcvt(Ineg,5,buf));
		SetDlgItemText(hInfo,IDC_INF_IT,gcvt(Itot,5,buf));
	}else{
		SetDlgItemText(hInfo,IDC_INF_IP,"");
		SetDlgItemText(hInfo,IDC_INF_IM,"");
		SetDlgItemText(hInfo,IDC_INF_IT,"");
	}
}
}

void Gauge::UnlockD()
{
if(val){
	GlobalUnlock(hVal);
	val=NULL;
}
UnlockI();
}

void Gauge::UnlockI()
{
if(imp){
	GlobalUnlock(hImp);
	imp=NULL;
}
}

void Gauge::FreeD()
{
UnlockD();
FreeI();
hVal=GlobalFree(hVal);
}

void Gauge::FreeI()
{
UnlockI();
hImp=GlobalFree(hImp);
}

void Gauge::Setup()
{
start=0;
final=count;
Zero_corr=0;
_Upper=_Lower=0;
char*p,*t;
p=strchr(ID,'G');
if(p){
	number=(int)strtol(p+1,&t,10);
	g_type=*t;
	if(!g_type)g_type=' ';
	else *t=' ';
}else{
	number=0;
	g_type=' ';
}
p=strchr(ID,'L');
if(p)angle=atoi(p+1);
else angle=0;
p=strchr(ID,'R');
if(p)radius=atof(p+1);
else radius=0;
trim(unit);
}

void Gauge::ULSetup()
{
BeginWait();
LockD();
unsigned long i;
double Min,Max,V;
Min=Max=Val(start);
for(i=start+1;i<final;++i){
	V=Val(i);
	if(V<Min){
		Min=V;
	}
	if(V>Max){
		Max=V;
	}
}
UnlockD();
_Upper=Max;
_Lower=Min;
EndWait();
}

void Gauge::Redraw()
{
InvalidateRect(hWnd,NULL,TRUE);
if(PolyP){
	delete PolyP;
	PolyP=NULL;
}
if(PolyI){
	delete PolyI;
	PolyI=NULL;
}
//HWND hWin;
//Window*W;
//RT*R;
//int i;
{for(RTbyGaugeIterator RT(this);RT;++RT){
	InvalidateRect(RT->hWnd,NULL,TRUE);
}}
/*
for(hWin=::GetWindow(hWnd,GW_HWNDFIRST);hWin;hWin=::GetWindow(hWin,GW_HWNDNEXT)){
	W=Window::GetWindow(hWin);
	if(W){
		if(W->WinType=='R'){
			R=(RT*)W;
			for(i=0;i<R->n;i++){
				if(R->Chns[i]==hThis){
					InvalidateRect(hWin,NULL,TRUE);
				}//if
			}//for
		}//if
		W->UnlockWindow();
	}//if
}//for
*/
}//Redraw

