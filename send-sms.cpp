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
  gpstracker::Device &device,
  gpstracker::Watcher &user,
  gpstracker::Watcher &watcher,
  std::string &newdevicepassword,
  std::string &newwatcherpassword,
  char &cmd,
  char &object,
  int &verbosity,
	int argc,
	char* argv[]
)
{
  struct arg_str *a_cmd = arg_str1(NULL, NULL, "<command>", "get|set|add|rm|ls|devices|tracks|notify");
	
  // device
  struct arg_str *a_id = arg_str0("i", "id", "<number>", "collar's identifier");
  struct arg_str *a_imei = arg_str0("m", "imei", "<number>", "collar's IMEI");
  struct arg_str *a_name = arg_str0("n", "name", "<string>", "collar's name");
  struct arg_str *a_color = arg_str0("c", "color", "<string>", "collar's color e.g. #ff0000");
  struct arg_str *a_password = arg_str0("p", "pwd", "<string>", "collar's password");
  struct arg_str *a_newdevicepassword = arg_str0("w", "newdevicepwd", "<string>", "set new collar's password");
  struct arg_str *a_newwatcherpassword = arg_str0("W", "newwatcherpwd", "<string>", "set new watcher's password");
  struct arg_int *a_status = arg_int0("s", "status", "<number>", "watcher status");
  struct arg_int *a_rights = arg_int0("r", "rights", "<number>", "wtacher rights");
  struct arg_int *a_tag = arg_int0("t", "tag", "<number>", "tag");
  struct arg_str *a_object = arg_str0("o", "object", "device|user", "default device");

  // service 
  struct arg_str *a_service = arg_str0("s", "service", "host:port", "e.g. 167.172.99.203:5002");
  // watcher
  struct arg_str *a_login = arg_str0("l", "login", "<string>", "override watcher's login");
  struct arg_str *a_wpassword = arg_str0("P", "password", "<string>", "override  watcher's password");

  struct arg_lit *a_verbosity = arg_litn("v", "verbose", 0, 3, "Set verbosity level");
	struct arg_lit *a_help = arg_lit0("h", "help", "Show this help");
	struct arg_end *a_end = arg_end(20);

	void* argtable[] = { 
		a_cmd, a_id, a_imei, a_name, a_color, a_password, a_newdevicepassword, a_newwatcherpassword, 
    a_status, a_rights, a_tag, a_object,
    a_service, a_login, a_wpassword,
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

  cmd = '\0';
  object = 'd';
  if (a_cmd->count) {
    std::string c(*a_cmd->sval);
    if (c.size()) {
      cmd = c[0]; // get|set|add|rm|ls|devices|tracks|notify -> gsarldtn
    }
  }
  if (a_object->count) {
    std::string o(*a_object->sval);
    if (o.size()) {
      object = o[0]; // device|user|track -> dut
    }
  }

  switch (object)
  {
  case 'u':
    if (a_id->count) {
      user.set_id(strtoull(*a_id->sval, NULL, 10));
    }

    if (a_status->count) {
      user.set_status(*a_status->ival);
    }
    
    if (a_name->count) {
      user.set_name(std::string(*a_name->sval));
    }

    if (a_login->count) {
      user.set_login(std::string(*a_login->sval));
    }

    if (a_password->count) {
      user.set_password(std::string(*a_password->sval));
    }

    if (a_tag->count) {
      user.set_tag(*a_tag->ival);
    }

    break;
  case 'd':
    if (a_id->count) {
      device.set_id(strtoull(*a_id->sval, NULL, 10));
    }

    if (a_imei->count) {
      device.set_imei(std::string(*a_imei->sval));
    }
    
    if (a_name->count) {
      device.set_name(std::string(*a_name->sval));
    }

    if (a_color->count) {
      device.set_color(std::string(*a_color->sval));
    }

    if (a_password->count) {
      device.set_password(std::string(*a_password->sval));
    }

    if (a_tag->count) {
      device.set_tag(*a_tag->ival);
    }
    break;
  default:
    break;
  }
  
  verbosity = a_verbosity->count;

  if (a_service->count) {
    service = *a_service->sval;
  }

  if (a_login->count) {
    watcher.set_login(std::string(*a_login->sval));
  }

  if (a_wpassword->count) {
    watcher.set_password(std::string(*a_wpassword->sval));
  }

  if (a_newdevicepassword->count) {
    newdevicepassword = std::string(*a_newdevicepassword->sval);
  } else {
    newdevicepassword = "";
  }

  if (a_newwatcherpassword->count) {
    newwatcherpassword = std::string(*a_newwatcherpassword->sval);
  } else {
    newwatcherpassword = "";
  }

	// special case: '--help' takes precedence over error reporting
	if ((a_help->count) || nerrors)
	{
		if (nerrors)
			arg_print_errors(stderr, a_end, progname.c_str());
		std::cerr << "Usage: " << progname << std::endl;
		arg_print_syntax(stderr, argtable, "\n");
		std::cerr << "Command line interface monitor" << std::endl;
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
    return "localhost:50051";
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

int sLsDevice(
  gpstracker::monitor::Stub *client,
  gpstracker::Watcher &watcher,
  size_t cnt,
  size_t ofs,
  int verbosity
) {
  grpc::ClientContext context;
  gpstracker::DeviceTrackRequest req;
  req.mutable_timewindow()->set_finish(cnt);
  req.mutable_timewindow()->set_start(ofs);
  *req.mutable_watcher() = watcher;
  std::unique_ptr<grpc::ClientReader<gpstracker::Device>> readerDevice(client->lsDevices(&context, req));

  gpstracker::Device d;
  while (readerDevice->Read(&d)) {
    std::string s;
    google::protobuf::util::MessageToJsonString(d, &s);
    std::cout << s << std::endl;
  }
  grpc::Status status = readerDevice->Finish();
  if (!status.ok())
    return (int) status.error_code();
  return TRACKER_OK;
}

int sLsUser(
  gpstracker::monitor::Stub *client,
  gpstracker::Watcher &watcher,
  size_t cnt,
  size_t ofs,
  int verbosity
) {
  grpc::ClientContext context;
  gpstracker::DeviceTrackRequest req;
  req.mutable_timewindow()->set_finish(cnt);
  req.mutable_timewindow()->set_start(ofs);
  *req.mutable_watcher() = watcher;
  std::unique_ptr<grpc::ClientReader<gpstracker::Watcher>> readerWatcher(client->lsWatchers(&context, req));

  gpstracker::Watcher w;
  while (readerWatcher->Read(&w)) {
    std::string s;
    google::protobuf::util::MessageToJsonString(w, &s);
    std::cout << s << std::endl;
  }
  grpc::Status status = readerWatcher->Finish();
  if (!status.ok())
    return (int) status.error_code();
  return TRACKER_OK;
}

/**
 * Device manipulation
 * 0- get, 1- change, 2- add(clear), 3- rm
 * @param client GRPC client
 * @return 0- success
 */
int sChDevice(
  int op,
  gpstracker::monitor::Stub *client,
  gpstracker::Device &device,
  gpstracker::Watcher &watcher,
  std::string &newdevicepassword,
  std::string &newwatcherpassword,
  int verbosity
) {
  grpc::ClientContext context;
  gpstracker::ChDeviceRequest req;
  req.set_op(op);
  req.set_newdevicepassword(newdevicepassword);
  req.set_newwatcherpassword(newwatcherpassword);
  *req.mutable_device() = device;
  *req.mutable_watcher() = watcher;
  gpstracker::Device retDevice;
  grpc::Status r = client->chDevice(&context, req, &retDevice);
  if (!r.ok()) {
    std::cerr << "Error " << r.error_code() << ": " << r.error_message() << std::endl;
    std::cerr << std::endl << "Config file: " << configString() << std::endl;
  }
  switch (op) {
    default:
      {
        std::string s;
        google::protobuf::util::MessageToJsonString(retDevice, &s);
        std::cout << s << std::endl;
      }
      break;
  }
  return r.ok() ? TRACKER_OK : ERR_CODE_GRPC_UNKNOWN;
}

/**
 * Device manipulation
 * 0- get, 1- change, 2- add(clear), 3- rm
 * @param client GRPC client
 * @return 0- success
 */
int sChWatcher(
  int op,
  gpstracker::monitor::Stub *client,
  gpstracker::Watcher &user,
  gpstracker::Watcher &watcher,
  int verbosity
) {
  grpc::ClientContext context;
  gpstracker::ChWatcherRequest req;
  req.set_op(op);
  *req.mutable_user() = user;
  *req.mutable_watcher() = watcher;
  gpstracker::Watcher retWatcher;
  grpc::Status r = client->chWatcher(&context, req, &retWatcher);
  if (!r.ok()) {
    std::cerr << "Error " << r.error_code() << ": " << r.error_message() << std::endl;
    std::cerr << std::endl << "Config file: " << configString() << std::endl;
  }
  switch (op) {
    default:
      {
        std::string s;
        google::protobuf::util::MessageToJsonString(retWatcher, &s);
        std::cout << s << std::endl;
      }
      break;
  }
  return r.ok() ? TRACKER_OK : ERR_CODE_GRPC_UNKNOWN;
}

int sAddDevices(
  std::istream &strm,
  gpstracker::monitor::Stub *client,
  gpstracker::Watcher &watcher,
  int verbosity
) {
  grpc::ClientContext context;
  gpstracker::BatchResponse retval;
  std::unique_ptr <grpc::ClientWriter <gpstracker::Device>> writer = client->addDevices(&context, &retval);

  gpstracker::Device device;
  char c;
  int state = 0;
  std::stringstream ss;
  grpc::WriteOptions writeOptions;
  while (strm.get(c)) {
    switch (state) {
      case 0: // wait '['
        if (c == '[') {
          state = 1;
        }
      break;
    case 1: // wait '{', ']'
        if (c == ']') {
          state = 0;
          break;
        }
       if (c == '{') {
          ss.str("");
          ss << c;
          state = 2;
          break;
        }
      break;
    case 2: // wait '}'
      ss << c;
      if (c == '}') {
        google::protobuf::util::Status status = google::protobuf::util::JsonStringToMessage(ss.str(), &device, google::protobuf::util::JsonParseOptions());
        if (!status.ok()) {
          std::cerr << "Error " << status.error_code() << ": " << status.error_message() << std::endl;
        } else {
          if (!writer->Write(device, writeOptions)) {
            std::cerr << "Error write " << std::endl;
          }
        }
        state = 1;
        break;
      }
      break;
    default:
      break;
    }
  }
  return TRACKER_OK;
}

int sAddTracks(
  std::istream &strm,
  gpstracker::monitor::Stub *client,
  gpstracker::Watcher &watcher,
  int verbosity
) {
  grpc::ClientContext context;
  gpstracker::BatchResponse retval;
  std::unique_ptr <grpc::ClientWriter <gpstracker::Track>> writer = client->addTracks(&context, &retval);

  gpstracker::Track track;
  char c;
  int state = 0;
  std::stringstream ss;
  grpc::WriteOptions writeOptions;
  while (strm.get(c)) {
    switch (state) {
      case 0: // wait '['
        if (c == '[') {
          state = 1;
        }
      break;
    case 1: // wait '{', ']'
        if (c == ']') {
          state = 0;
          break;
        }
       if (c == '{') {
          ss.str("");
          ss << c;
          state = 2;
          break;
        }
      break;
    case 2: // wait '}'
        ss << c;
        if (c == '}') {
          google::protobuf::util::Status status = google::protobuf::util::JsonStringToMessage(ss.str(), &track, google::protobuf::util::JsonParseOptions());
          if (!status.ok()) {
            std::cerr << "Error " << status.error_code() << ": " << status.error_message() << std::endl;
          } else {
            if (!writer->Write(track, writeOptions)) {
              std::cerr << "Error write " << std::endl;
            }
          }
          state = 1;
          break;
        }
      break;
    default:
      break;
    }
  }
  return TRACKER_OK;
}

int sListenTracks(
  std::ostream &strm,
  gpstracker::monitor::Stub *client,
  gpstracker::Watcher &watcher,
  int verbosity
) {
  grpc::ClientContext context;
  gpstracker::BatchResponse retval;
  std::unique_ptr <grpc::ClientReader <gpstracker::TrackNRawData>> reader = client->listenTrack(&context, watcher);
  gpstracker::TrackNRawData track;
  while (!stopped && reader->Read(&track)) {
      std::string s;
      google::protobuf::util::MessageToJsonString(track, &s);

      std::string t = stringFromHex(track.rawdata().hex());
      // parse rawdata
      std::string p = parsePacket(t);
      if (p.empty()) {
        p = "{}";
      }

      std::cout << "{\"data\":" << p << ", \"msg\": " << s << "}" << std::endl;
  }
  grpc::Status status = reader->Finish();
  if (!status.ok())
    return (int) status.error_code();
  return TRACKER_OK;
}

/**
 * 
 */
int run(
  const std::string &svc,
  gpstracker::Device &device,
  gpstracker::Watcher &user,
  gpstracker::Watcher &watcher,
  std::string &newdevicepassword,
  std::string &newwatcherpassword,
  char op,  // get|set|add|rm|ls|devices|tracks|notify
  char object,  // device|user|track
  int verbosity
) {
  // Create a default SSL ChannelCredentials object.
  auto channel_creds = grpc::InsecureChannelCredentials();
  // Create a channel using the credentials created in the previous step.
  std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(mkServiceUrl(svc), channel_creds);
  // Create a stub on the channel.
  std::unique_ptr<gpstracker::monitor::Stub> client(gpstracker::monitor::NewStub(channel));

  if (object != 'd' && object != 'u' && object != 't') {
    object = 'd';
  }

  switch (op) {
    case '\0': // nothing
      break;
    case 'l': // ls
    {
      size_t sz = 1024;
      size_t ofs = 0;
      switch (object) { 
        case 'd':
          return sLsDevice(client.get(), watcher, sz, ofs, verbosity);
        case 'u':
          return sLsUser(client.get(), watcher, sz, ofs, verbosity);
        default:
          break;
      }
    }
    case 'd':
      // add devices from stdin JSON
      return sAddDevices(std::cin, client.get(), watcher, verbosity);
      break;
    case 't':
      // add tracks from stdin JSON
      return sAddTracks(std::cin, client.get(), watcher, verbosity);
      break;
    case 'n':
      // notification
      return sListenTracks(std::cout, client.get(), watcher, verbosity);
      break;
    default:
      switch (object) { 
        case 'd':
          return sChDevice(op, client.get(), device, watcher, newdevicepassword, newwatcherpassword, verbosity);
        case 'u':
          return sChWatcher(op, client.get(), user, watcher, verbosity);
        default:
          break;
      }
  }
}

int main(
	int argc,
	char* argv[]
) {
  std::string config = file2string(getDefaultConfigFileName(DEF_CONFIG_FILE_NAME).c_str());
  std::string conninfo;
  std::string listenAddress;
  // not used, used in gps-track-cli
  std::string login;
  std::string password;
  std::string newdevicepassword;
  std::string newwatcherpassword;
  bool allow_user_add_device;
  
  parseConfig(
    conninfo,
    listenAddress,
    login,
    password,       
    allow_user_add_device,  // not used
    config
  );

  gpstracker::Device device;
  gpstracker::Watcher watcher;
  gpstracker::Watcher user;

  watcher.set_login(login);
  watcher.set_password(password);

  char cmd, object;
  int verbosity;
  
  if (parseCmd(listenAddress, device, user, watcher, newdevicepassword, newwatcherpassword,
    cmd, object, verbosity, argc, argv) != 0) {
    exit(ERR_CODE_COMMAND_LINE);  
  };

  // Signal handler
  setSignalHandler();

  int r = run(listenAddress, device, user, watcher, newdevicepassword, newwatcherpassword, cmd, object, verbosity);

  if (r) {
    std::cerr << "Error " << r << ": " << strerror_gps(r) << std::endl;
  }

  done();
  exit(r);
}
