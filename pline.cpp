#define STRICT
#include <windows.h>
#include <memory.h>
#include "pline.h"

void Pline::_drop()
{
	if(p_x!=-1){
		if(p_ymax==p_ymin){
			OutPoint(p_x,p_ymin);
		}else{
			OutPoint(p_x,p_ymin);
			OutPoint(p_x,p_ymax);
		}
	}
	p_x=-1;
}

void Pline::_ensure(int newlen)
{
	alloc=newlen;
	POINT*tmp=pts;
	pts=new POINT[alloc];
	if(len)memcpy(pts,tmp,len*sizeof(POINT));
	if(tmp)delete tmp;
}

void Pline::OutPoint(int _x,int _y)
{
	if(len==alloc)_ensure(alloc+MAXPOLY);
	pts[len].x=_x;
	pts[len].y=_y;
	++len;
}

void Pline::FilterPoint(int x,int y)
{
if(p_x!=x){
	_drop();
	p_x=x;
	p_ymin=p_ymax=y;
}else{
	if(y>p_ymax)p_ymax=y;
	if(y<p_ymin)p_ymin=y;
}
}

void Pline::Polyline(HDC hdc)
{
	int i;
	int l;
	
	i=0;
	do{
		l=len-i;
		if(l>MAXPOLY)l=MAXPOLY;
		::Polyline(hdc,pts+i,l);
		i+=l-1;
	}while(i<(len-1));
}
