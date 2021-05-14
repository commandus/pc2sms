#include <string>
#include <iostream>
#include <signal.h>
#include <unistd.h>
#include <limits.h>

#include "argtable3/argtable3.h"

#include "platform.h"
#include "config-filename.h"
#include "pc2sms-config.h"
#include "utilstring.h"
#include "daemonize.h"
#include "service-raw.h"
#include "errlist.h"

#define DEF_CONFIG_FILE_NAME    ".pc2sms"
#define progname                "pc2sms"

SMSServiceImpl *server = NULL;

static void done() {
  if (server != NULL) {
    delete server;
    server = NULL;
  }
}

static void stop() {
  done();
  exit(SMS_OK);
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
    void *t[256];
    size_t size = backtrace(t, 256);
    backtrace_symbols_fd(t, size, STDERR_FILENO);
    exit(ERR_CODE_SEGFAULT);
    }
	default:
		break;
	}
}

#ifdef _MSC_VER
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
	sigaction(SIGINT, &action, NULL);
	// sigaction(SIGHUP, &action, NULL);
  	sigaction(SIGSEGV, &action, NULL);
}
#endif

void run(
) {
	std::string configFileName = getDefaultConfigFileName(DEF_CONFIG_FILE_NAME);
    std::string config = file2string(configFileName.c_str());
    std::string listenAddress;
    std::string login;
    std::string password;
	NotifyPolicy policy;

    parseServiceConfig(
        listenAddress,
        login,
        password,
		policy,
		config
    );

    server = new SMSServiceImpl(listenAddress, login, password, policy);
    server->run();
}

int main(int argc, char **argv) {
	struct arg_lit *a_daemonize = arg_lit0("d", "daemonize", "run daemon");
	struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 3, "Set verbosity level");
	struct arg_lit *a_help = arg_lit0("h", "help", "Show this help");
	struct arg_end *a_end = arg_end(20);

	void* argtable[] = { 
		a_daemonize, a_verbosity,
		a_help, a_end 
	};

	// verify the argtable[] entries were allocated successfully
	if (arg_nullcheck(argtable) != 0)
	{
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return ERR_CODE_COMMAND_LINE;
	}
	// Parse the command line as defined by argtable[]
	int nerrors = arg_parse(argc, argv, argtable);

	uint64_t id = 0;
	bool daemonize = a_daemonize->count > 0;
	int verbosity = a_verbosity->count;

	// special case: '--help' takes precedence over error reporting
	if ((a_help->count) || nerrors) {
		if (nerrors)
			arg_print_errors(stderr, a_end, progname);
		std::cerr << "Usage: " << progname << std::endl;
		arg_print_syntax(stderr, argtable, "\n");
		std::cerr << "Serve gRPC clients" << std::endl
			<< "Config file ~/" DEF_CONFIG_FILE_NAME << std::endl;
		arg_print_glossary(stderr, argtable, "  %-27s %s\n");
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return ERR_CODE_COMMAND_LINE;
	}
	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));

	if (daemonize) {
		char wd[PATH_MAX];
		std::string progpath = getcwd(wd, PATH_MAX);	
		if (verbosity > 1)
			std::cerr << MSG_DAEMON_STARTED << progpath << "/" << progname << MSG_DAEMON_STARTED_1 << std::endl;
		Daemonize daemonize(progname, progpath, run, stop, done);
	} else {
		setSignalHandler();
		run();
		done();
	}
}
