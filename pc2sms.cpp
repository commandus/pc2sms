#include <string>
#include <iostream>
#include <csignal>
#include <climits>

#if defined(_MSC_VER) || defined(__MINGW32__)
#else
#include <unistd.h>
#define TRACE_BUFFER_SIZE   256
#endif

#include "argtable3/argtable3.h"

#include "platform.h"
#include "config-filename.h"
#include "pc2sms-config.h"
#include "utilstring.h"
#include "daemonize.h"
#include "service-raw.h"
#include "errlist.h"
#include "file-helper.h"

#define DEF_CONFIG_FILE_NAME    ".pc2sms"
#define progname                "pc2sms"

#ifdef ENABLE_LIBUV
#define DAEMONIZE_CLOSE_FILE_DESCRIPTORS_AFTER_FORK false
#else
#define DAEMONIZE_CLOSE_FILE_DESCRIPTORS_AFTER_FORK true
#endif

SMSServiceImpl *server = nullptr;

static void done() {
  if (server) {
    delete server;
    server = nullptr;
  }
}

static void stop() {
  done();
  exit(SMS_OK);
}

static void printTrace() {
#if defined(_MSC_VER) || defined(__MINGW32__)
#else
    void *t[TRACE_BUFFER_SIZE];
    auto size = backtrace(t, TRACE_BUFFER_SIZE);
    backtrace_symbols_fd(t, size, STDERR_FILENO);
#endif
}

void signalHandler(int signal)
{
	switch(signal)
	{
	case SIGINT:
		std::cerr << MSG_INTERRUPTED << std::endl;
		stop();
		break;
	case SIGSEGV:
    {
        std::cerr << "Segmentation fault\n";
        printTrace();
        exit(ERR_CODE_SEGFAULT);
    }
	default:
		break;
	}
}

#if defined(_MSC_VER) || defined(__MINGW32__)
// TODO
void setSignalHandler()
{
}
#else
void setSignalHandler()
{
	struct sigaction action;
	memset(&action, 0, sizeof(struct sigaction));
	action.sa_handler = &signalHandler;
	sigaction(SIGINT, &action, nullptr);
	// sigaction(SIGHUP, &action, nullptr);
  	sigaction(SIGSEGV, &action, nullptr);
}
#endif

static Pc2SmsConfig cfg;

void run(
) {
    if (cfg.verbose)
        std::cout << "Serve\n";
    server = new SMSServiceImpl(cfg.listenAddress, cfg.login, cfg.password, cfg.policy);
    if (cfg.verbose)
        std::cout << "Run\n";
    server->run();
}

int main(int argc, char **argv) {
	struct arg_lit *a_daemonize = arg_lit0("d", "daemonize", "run daemon");
    struct arg_str *a_pidfile = arg_str0("p", "pidfile", "<file>", "Check whether a process has created the file pidfile");
	struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 3, "Set verbosity level");
	struct arg_lit *a_help = arg_lit0("h", "help", "Show this help");
	struct arg_end *a_end = arg_end(20);

	void* argtable[] = { 
		a_daemonize, a_pidfile, a_verbosity,
		a_help, a_end 
	};

	// verify the argtable[] entries were allocated successfully
	if (arg_nullcheck(argtable) != 0) {
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return ERR_CODE_COMMAND_LINE;
	}
	// Parse the command line as defined by argtable[]
	int nerrors = arg_parse(argc, argv, argtable);

	uint64_t id = 0;

    std::string configFileName = getDefaultConfigFileName(getProgramDir().c_str(), DEF_CONFIG_FILE_NAME);
    std::string configStr = file2string(configFileName.c_str());
    parseServiceConfig(cfg, configStr);
    if (a_daemonize->count > 0)
        cfg.daemonize = true;
    if (a_pidfile->count)
        cfg.pidfile = *a_pidfile->sval;
    else
        cfg.pidfile = "";

	cfg.verbose = a_verbosity->count;

	// special case: '--help' takes precedence over error reporting
	if ((a_help->count) || nerrors) {
		if (nerrors)
			arg_print_errors(stderr, a_end, progname);
		std::cerr << "Usage: " << progname << std::endl;
		arg_print_syntax(stderr, argtable, "\n");
		std::cerr << "Serving gRPC clients for sending SMS messages through connected mobile phones\n"
			<< "config file " << getDefaultConfigFileName(getProgramDir().c_str(), DEF_CONFIG_FILE_NAME) << '\n'
            << "or " << getProgramDir() << '\\' << DEF_CONFIG_FILE_NAME << '\n'
            << "- listen address. Default 0.0.0.0:50053 - all interfaces\n"
            << "- allowed login\n"
            << "- password\n"
            << "- all. Add line \"all\" to send over all connected phones\n"
            << "- daemonize. Add line \"daemonize\" to force run as daemon\n";
		arg_print_glossary(stderr, argtable, "  %-27s %s\n");
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return ERR_CODE_COMMAND_LINE;
	}
	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));

    if (cfg.verbose)
        std::cout << cfg.toString() << std::endl;

    if (cfg.daemonize) {
		if (cfg.verbose > 1)
			std::cerr << MSG_DAEMON_STARTED << getCurrentDir() << "/" << progname << MSG_DAEMON_STARTED_1 << std::endl;
		Daemonize daemonize(progname, getCurrentDir(), run, stop, done,
            0, cfg.pidfile, DAEMONIZE_CLOSE_FILE_DESCRIPTORS_AFTER_FORK);
	} else {
		setSignalHandler();
		run();
		done();
	}
}
