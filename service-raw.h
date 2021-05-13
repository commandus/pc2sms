#ifndef SERVICE_RAW_H_
#define SERVICE_RAW_H_	1

#include <thread>
#include <queue>

#include <grpcpp/grpcpp.h>
#include <grpcpp/alarm.h>
#include "pc2sms.grpc.pb.h"
#include "sms-listener.h"

class QueuingMgr;

/**
 * Async service
 */
class SMSServiceImpl final {
private:
  std::string address;
  std::unique_ptr<grpc::Server> server;
  std::string login;
  std::string password;
public:
  explicit SMSServiceImpl(
    const std::string &grpc_listener_address,
    const std::string &login,
    const std::string &password
  );
  virtual ~SMSServiceImpl();
  int start();
  void stop();
  // GRPC main event loop
  void run();

  std::unique_ptr<grpc::ServerCompletionQueue> cq;
  pc2sms::sms::AsyncService service;
  bool running;
  ListenSMS clientsListenSMS;
  QueuingMgr *queuingMgr;

  bool isAllowed(
    const pc2sms::Credentials &value
  );
};

/**
 * Class encompasing the state and logic needed to serve a request.
 */
class CommonCallData {
public:
  // The means of communication with the gRPC runtime for an asynchronous server.
  pc2sms::sms::AsyncService *service;
  // The producer-consumer queue where for asynchronous server notifications.
  grpc::ServerCompletionQueue* cq;
  // Context for the rpc, allowing to tweak aspects of it such as the use
  // of compression, authentication, as well as to send metadata back to the
  // client.
  grpc::ServerContext ctx;
  enum CallStatus { CREATE, PROCESS, FINISH, LISTEN_COMMAND, LISTEN_COMMAND_SENT };
  CallStatus status;  // The current serving state.
  // Take in the "service" instance OrdersData(in this case representing an asynchronous
  // server) and the completion queue "cq" used for asynchronous communication
  // with the gRPC runtime.
  CommonCallData(pc2sms::sms::AsyncService* service, grpc::ServerCompletionQueue* cq);
  virtual void Proceed(bool = true) = 0;
};

/**
 * Responds on requestToSend
 */
class RequestToSendData : public CommonCallData {
  private:
    SMSServiceImpl *service;
    bool new_responder_created;
    pc2sms::ResponseCommand result;
    void notifyClients(const std::vector<pc2sms::SMS> &values);
    void notifyClients(const pc2sms::SMS &value);
  public:
    // What we get from the client
    pc2sms::RequestCommand request;
    // What we send back to the client
    grpc::ServerAsyncResponseWriter<pc2sms::ResponseCommand> responder;
    // Take in the "service" instance (in this case representing an asynchronous
    // server) and the completion queue "cq" used for asynchronous communication
    // with the gRPC runtime.
    RequestToSendData(SMSServiceImpl *service);
    virtual void Proceed(bool = true) override;
};

/**
 * Responds on listen
 */
class ListenData : public CommonCallData {
  private:
    SMSServiceImpl *smsService;
    bool new_responder_created;
    std::queue<pc2sms::SMS> result;
    // listen for a notifications of new records
    int listenNotifications();
    bool writeNext();
  public:
    // What we get from the client in login.
    pc2sms::Credentials credentials;
    // The means to get back to the client.
    grpc::ServerAsyncWriter<pc2sms::SMS> responder;
    // Take in the "service" instance (in this case representing an asynchronous
    // server) and the completion queue "cq" used for asynchronous communication
    // with the gRPC runtime.
    ListenData(SMSServiceImpl *sms_service);
    virtual void Proceed(bool = true) override;
    void enqueue(const std::vector<pc2sms::SMS> &values);
};

/**
 * Enqueue ListenData after signal a new SMS to send request arrived
 */
class QueuingMgr : public CommonCallData {
  private:
    SMSServiceImpl *smsService;
    std::vector<pc2sms::SMS> result;
  public:
    // Take in the "service" instance (in this case representing an asynchronous
    // server) and the completion queue "cq" used for asynchronous communication
    // with the gRPC runtime.
    QueuingMgr(SMSServiceImpl *sms_service);
    virtual void Proceed(bool = true) override;
    void enqueue(const std::vector<pc2sms::SMS> &values);
};

#endif
