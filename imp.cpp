#define STRICT
#include <windows.h>
#include "dxw.h"
#include "gauge.h"

void Gauge::CalcI()
{
unsigned long i;
double T0,T1,P0,P1,dI;
T1=Start();
P1=0.0;
Imax=Imin=0.0;
Ipos=Ineg=Itot=0.0;
for(i=start;i<final;i++){
	T0=T1;
	P0=P1;
	T1=P2T(i); //units: s
	P1=Val(i)*100.0;    		//units: kPa
	dI=(P1+P0)/2*(T1-T0);		//units: kPa*s
	if(P1>=0 && P0>=0){
		Ipos+=dI;
	}else if(P1<=0 && P0<=0){
		Ineg+=dI;
	}else if(P0>0){
		Ipos+=P0/2*(T1-T0)*P0/(P0-P1);
		Ineg+=P1/2*(T1-T0)*P1/(P1-P0);
	}else{
		Ipos+=P1/2*(T1-T0)*P1/(P1-P0);
		Ineg+=P0/2*(T1-T0)*P0/(P0-P1);
	}
	Itot+=dI;
	Imp(i)=Itot;
	if(Itot>Imax)Imax=Itot;
	if(Itot<Imin)Imin=Itot;
}
}
