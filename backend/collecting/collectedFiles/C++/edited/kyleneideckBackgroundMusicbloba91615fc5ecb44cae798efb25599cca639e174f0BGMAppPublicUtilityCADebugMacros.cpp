
#include "CADebugMacros.h"
#include <stdio.h>
#include <stdarg.h>
#if TARGET_API_MAC_OSX
	#include <syslog.h>
#endif

#if DEBUG
#include <stdio.h>

void	DebugPrint(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}
#endif 
void	LogError(const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
        #if (DEBUG || !TARGET_API_MAC_OSX) && !CoreAudio_UseSysLog
    printf("[ERROR] ");
    vprintf(fmt, args);
    printf("\n");
#else
    vsyslog(LOG_ERR, fmt, args);
#endif
#if DEBUG
    CADebuggerStop();
#endif
    	va_end(args);
}

void	LogWarning(const char *fmt, ...)
{
	va_list args;
    va_start(args, fmt);
        #if (DEBUG || !TARGET_API_MAC_OSX) && !CoreAudio_UseSysLog
    printf("[WARNING] ");
    vprintf(fmt, args);
    printf("\n");
#else
    vsyslog(LOG_WARNING, fmt, args);
#endif
#if DEBUG
    #endif
    	va_end(args);
}
