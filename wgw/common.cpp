

#include "stdafx.h"


unsigned chrMask()
{
	unsigned mask = (unsigned)(-1);
	switch(sizeof(TCHAR)){
		case 1:
			mask = 0xff;
			break;
		case 2:
			mask = 0xffff;
			break;
		case 4:
			mask = 0xffffffff;
			break;
	}

	return mask;
}

unsigned chrVal(TCHAR c)
{
	return c&chrMask();
}