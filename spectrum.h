#ifndef _SPECTRUM_H_
#define _SPECTRUM_H_
#include "axis.h"

class spectrum:public CutWnd
{
	HLOCAL hG;
//	HLOCAL Abs,Phase;
	long N;
	double*abs,AbsMax;
	double*phase;
	double FrScale;
	long st,fi;
	double Amin,Amax;
	Axis Fr,Am;
	RECT rcG;
//Window
virtual ~spectrum();
virtual void AtUnlock();
virtual void Draw(HDC hdc,RECT&rc,DCtype t,RECT*rcUpd=NULL);
virtual BOOL Command(WPARAM cmd);
virtual BOOL WinProc(Msg&M);
virtual char* PicName();
//CutWnd
virtual int GetCutRC(RECT&);
virtual void SetSize(HWND,RECT&);
virtual void FinishCut(RECT&);

public:
	spectrum(Gauge*_g);
	void LockData();
	void UnlockData();
	double x2Fr(int x){return Fr.scr2d(x,rcG.left,rcG.right);}
	int Fr2x(double f){return Fr.d2scr(f,rcG.left,rcG.right);}
	double y2Am(int y){return Am.scr2d(y,rcG.bottom,rcG.top);}
	int Am2y(double a){return Am.d2scr(a,rcG.bottom,rcG.top);}
	void SetAxis(){Fr.Set(st*FrScale,fi*FrScale,AS_HORIZ|15);Am.Set(Amin,Amax,AS_VERT|15);}
};

#endif
