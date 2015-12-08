#define STRICT
#include <windows.h>
#include "dxw.h"
#include "gauge.h"

void Gauge::HomePointer(BOOL home)
{
PointerSet(home?start:final-1);
}

void Gauge::MovePointer(int Delt)
{
unsigned long NP;
if(Curr==-1)NP=start;
else NP=Curr;
NP+=Delt;
while(NP<start)NP+=final-start;
while(NP>=final)NP-=final-start;
PointerSet(NP);
}

void Gauge::SetPointer(WORD x)
{
double time=scr2x(x,fr,rcGrf);
if(time>=Start() && time<=Final())
	PointerSet(T2P(time));
}

void Gauge::PointerSet(long NewPos)
{
LockD();
HDC hdc=GetDC(hWnd);
DrawPointer(hdc);
Curr=NewPos;
if(Curr!=-1){
	if(Curr<start)Curr=start;
	if(Curr>=final)Curr=final-1;
}
DrawPointer(hdc);
ReleaseDC(hWnd,hdc);
SetDigitize();
}

void Gauge::PointSet(int n) //n=0 or 1
{
LockD();
HDC hdc=GetDC(hWnd);
DrawPointer(hdc);
pts[n]=Curr;
DrawPointer(hdc);
ReleaseDC(hWnd,hdc);
}

static void cross(HDC hdc,int x,int y)
{
MoveToEx(hdc,x-8,y  ,NULL);
LineTo  (hdc,x+8,y  );
MoveToEx(hdc,x  ,y-8,NULL);
LineTo  (hdc,x  ,y+8);
}

void Gauge::DrawPointer(HDC hdc)
{
if(!frcValid || !rcValid)return;
HPEN hOldPen;
hOldPen=(HPEN)SelectObject(hdc,hpPnt);
int R2_old=SetROP2(hdc,R2_XORPEN);

LockD();
if(Curr!=-1)cross(hdc,x2scr(P2T(Curr),fr,rcGrf),y2scr(Val(Curr),fr,rcGrf));
SelectObject(hdc,hpPts);
if(pts[0]!=-1)cross(hdc,x2scr(Tp(0),fr,rcGrf),y2scr(Pp(0),fr,rcGrf));
if(pts[1]!=-1)cross(hdc,x2scr(Tp(1),fr,rcGrf),y2scr(Pp(1),fr,rcGrf));

SetROP2(hdc,R2_old);
SelectObject(hdc,hOldPen);
}
