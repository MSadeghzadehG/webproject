
#include "stdafx.h"


#include <stdarg.h>

void ErrorLog(const char* _fmt, ...)
{
	char Msg[512];
	va_list ap;

	va_start(ap, _fmt);
	vsprintf(Msg, _fmt, ap);
	va_end(ap);

	printf("Error");

}