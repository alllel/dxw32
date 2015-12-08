#define STRICT
#include <windows.h>
#include "dxw.h"
#include "gauge.h"

double Gauge::P2T (long x)
{
unsigned i;
for(i=0;i<nRates;++i){
	if(x<Rates[i].Np)return Rates[i].Tstart+x*Rates[i].rate;
	x-=Rates[i].Np;
}
i--;
return Rates[i].Tstart+Rates[i].Np*Rates[i].rate;
}

/******************************************************************************/

long Gauge::T2P(double tim)
{
unsigned i;
long ret;
long np=0;
if(tim<Rates[0].Tstart)return 0;
for(i=0;i<nRates;++i){
	ret=(tim-Rates[i].Tstart)/Rates[i].rate;
	if(ret<Rates[i].Np)return ret+np;
	else np+=Rates[i].Np;
}
return np-1;
}

/***************************************************************************/
