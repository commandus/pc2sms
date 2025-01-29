#if defined(_MSC_VER) || defined(__MINGW32__)
    #define	SYSLOG(msg)
    #define OPENSYSLOG()
    #define CLOSESYSLOG()
#else
    #include <syslog.h>
    #include <sstream>
    #define	SYSLOG(msg) { syslog (LOG_ALERT, "%s", msg); }
    #define OPENSYSLOG() { setlogmask (LOG_UPTO(LOG_NOTICE)); openlog("gpstrackmgr", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_LOCAL1); }
    #define CLOSESYSLOG() closelog();

    #include <execinfo.h>
    #include <syslog.h>
    #include <sstream>
#endif
