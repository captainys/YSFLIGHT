#include "fsairproperty.h"



double FsGetAirDensity(const double &alt)
{
	int a;
	static double rhoTab[]=
	{
		1.224991,  /*     0m */
		0.819122,  /*  4000m */
		0.529999,  /*  8000m */
		0.299988,  /* 12000m */
		0.153000,  /* 16000m */
		0.084991   /* 20000m */
	};

	a=(int)(alt/4000.0F);
	if(8<a)
	{
		return 0.0F;
	}
	else if(a<0)
	{
		return rhoTab[0];
	}
	else if(4<a)
	{
		return 0.084991F;
	}
	else
	{
		double base,diff,t;
		base=rhoTab[a];
		diff=rhoTab[a+1]-rhoTab[a];
		t=(alt-4000.0F*a)/4000.0F;
		return base+diff*t;
	}
}

const double &FsGetZeroAirDensity(void)
{
	static const double rhoZero=1.224991;
	return rhoZero;
}

////////////////////////////////////////////////////////////

double FsGetMachOne(const double &alt)
{
	int a;
	static double machTab[]=
	{
		340.294,  /*     0m */
		324.579,  /*  4000m */
		308.063,  /*  8000m */
		295.069,  /* 12000m */
		295.069,  /* 16000m */
		295.069   /* 20000m */
	};

	a=(int)(alt/4000.0F);
	if(a<0)
	{
		return machTab[0];
	}
	else if(4<a)
	{
		return machTab[4];
	}
	else
	{
		double base,diff,t;
		base=machTab[a];
		diff=machTab[a+1]-machTab[a];
		t=(alt-4000.0F*a)/4000.0F;
		return base+diff*t;
	}
}

