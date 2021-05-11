#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <inttypes.h>
#include <unistd.h>
#include <execinfo.h>

#include "argtable3/argtable3.h"

#include <grpcpp/grpcpp.h>
#include <google/protobuf/util/json_util.h>

#include "pc2sms.grpc.pb.h"

#include "errlist.h"
#include "utilstring.h"
#include "config-filename.h"
#include "send-sms-config.h"

static bool stopped = false;

const std::string progname = "send-sms";
#define  DEF_CONFIG_FILE_NAME ".send-sms"

static void done()
{
  exit(0);
}

static void stop()
{
  stopped = true;
}

void signalHandler(int signal)
{
	switch(signal)
	{
	case SIGINT:
		std::cerr << MSG_INTERRUPTED << std::endl;
		stop();
    done();
		break;
	case SIGSEGV:
    {
    void *t[256];
    size_t size = backtrace(t, 256);
    backtrace_symbols_fd(t, size, STDERR_FILENO);
    exit(11);
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
	sigaction(SIGHUP, &action, NULL);
  sigaction(SIGSEGV, &action, NULL);
}
#endif

/**
 * Parse command line
 * Return 0- success
 *        1- show help and exit, or command syntax error
 *        2- output file does not exists or can not open to write
 **/
int parseCmd
(
  std::string &service,
  std::string &login,
  std::string &password,
  std::string &phoneNumber,
  int &verbosity,
  bool listen,
	int argc,
	char* argv[]
)
{
  struct arg_str *a_phone_number = arg_str1(NULL, NULL, "<phone-number>", "");
  // service 
  struct arg_str *a_service = arg_str0("s", "service", "host:port", "e.g. 167.172.99.203:5002");
  // 
  struct arg_str *a_login = arg_str0("l", "login", "<string>", "service login");
  struct arg_str *a_password = arg_str0("p", "password", "<string>", "service password");
  struct arg_lit *a_listen = arg_lit0("l", "listen", "Listen mode (debug)");

  struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 3, "Set verbosity level");
	struct arg_lit *a_help = arg_lit0("h", "help", "Show this help");
	struct arg_end *a_end = arg_end(20);

	void* argtable[] = { 
		a_phone_number, a_service, a_login, a_password,
    a_verbosity, a_help, a_end 
	};

	int nerrors;

	// verify the argtable[] entries were allocated successfully
	if (arg_nullcheck(argtable) != 0)
	{
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return 1;
	}
	// Parse the command line as defined by argtable[]
	nerrors = arg_parse(argc, argv, argtable);

  if (a_phone_number->count) {
    phoneNumber = *a_phone_number->sval;
  }
  if (a_service->count) {
    service = *a_service->sval;
  }
  if (a_login->count) {
    login = *a_login->sval;
  }
  if (a_password->count) {
    password = *a_password->sval;
  }
  listen = a_listen->count > 0;

  verbosity = a_verbosity->count;

	// special case: '--help' takes precedence over error reporting
	if ((a_help->count) || nerrors)
	{
		if (nerrors)
			arg_print_errors(stderr, a_end, progname.c_str());
		std::cerr << "Usage: " << progname << std::endl;
		arg_print_syntax(stderr, argtable, "\n");
		std::cerr << "Command line SMS send utility" << std::endl;
		arg_print_glossary(stderr, argtable, "  %-25s %s\n");
		arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
		return 1;
	}

	arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
	return 0;
}

static std::string mkServiceUrl(
  const std::string &svc
) {
  if (svc.empty())
    return "localhost:50053";
  else
    return svc;    
}

static std::string configString() {
  std::string fn = getDefaultConfigFileName(DEF_CONFIG_FILE_NAME).c_str();
  std::string config = file2string(fn.c_str());
  std::stringstream ss;
  ss << fn << std::endl << config << std::endl;
  return ss.str();
}

int sendSMS(
  pc2sms::sms::Stub *client,
  const std::string  &phoneNumber,
  const std::string  &login,
  const std::string  &password,
  const std::string  &message
) {
  grpc::ClientContext context;
  pc2sms::RequestCommand req;
  pc2sms::ResponseCommand response;
  
  req.mutable_credentials()->set_login(login);
  req.mutable_credentials()->set_password(password);
  req.mutable_sms()->set_phone(phoneNumber);
  req.mutable_sms()->set_message(message);
  grpc::Status status = client->requestToSend(&context, req, &response);
  if (!status.ok()) {
    std::cerr << "Error " << status.error_code() << ": " << status.error_message() << std::endl;
    return (int) status.error_code();
  }
  return SMS_OK;
}

int listenSMS(
  std::ostream &strm,
  pc2sms::sms::Stub *client,
  const std::string  &login,
  const std::string  &password
) {
  grpc::ClientContext context;
  pc2sms::Credentials credentials;
  credentials.set_login(login);
  credentials.set_password(password);
  
  std::unique_ptr <grpc::ClientReader <pc2sms::SMS>> reader = client->listen(&context, credentials);
  pc2sms::SMS sms;
  while (!stopped && reader->Read(&sms)) {
      std::string s;
      google::protobuf::util::MessageToJsonString(sms, &s);
      std::cout << s << std::endl;
  }
  grpc::Status status = reader->Finish();
  if (!status.ok())
    return (int) status.error_code();
  return SMS_OK;
}

/**
 * 
 */
int run(
  const std::string &service,
  const std::string &login,
  const std::string &password,
  const std::string &phoneNumber,
  const std::string &message,
  bool listen
) {
  // Create a default SSL ChannelCredentials object.
  auto channel_creds = grpc::InsecureChannelCredentials();
  // Create a channel using the credentials created in the previous step.
  std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(mkServiceUrl(service), channel_creds);
  // Create a stub on the channel.
  std::unique_ptr<pc2sms::sms::Stub> client(pc2sms::sms::NewStub(channel));
  
  int r;
  if (listen) {
    r = listenSMS(std::cout, client.get(), login, password);
  } else {
    r = sendSMS(client.get(), phoneNumber, login, password, message);
  }
  return r;
}

int main(
	int argc,
	char* argv[]
) {
  std::string config = file2string(getDefaultConfigFileName(DEF_CONFIG_FILE_NAME).c_str());
  std::string listenAddress;
  std::string login;
  std::string password;
  std::string phoneNumber;
  bool listen;
  std::string message("");
  
  parseClientConfig(
    listenAddress,
    login,
    password,       
    config
  );

  int verbosity;
  
  if (parseCmd(listenAddress, login, password, phoneNumber, verbosity, listen, argc, argv) != 0) {
    exit(ERR_CODE_COMMAND_LINE);  
  };

  // Signal handler
  setSignalHandler();

  std::string line;
  while (std::getline(std::cin, line)) {
    message += line;
  }

  if (message.empty()) {
    std::cerr << "Error " << ERR_CODE_MESSAGE_EMPTY << ": " << strerror_sms(ERR_CODE_MESSAGE_EMPTY) << std::endl;
  }

  int r = run(listenAddress, login, password, phoneNumber, message, listen);

  if (r) {
    std::cerr << "Error " << r << ": " << strerror_sms(r) << std::endl;
  }

  done();
  exit(r);
}
