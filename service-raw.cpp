#include "service-raw.h"

#include <sstream>
#include <grpcpp/grpcpp.h>
#include <grpc/support/log.h>

#include <google/protobuf/util/json_util.h>

#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>

#include "errlist.h"
#include "platform.h"
#include "utilstring.h"

// #define DEBUG 1

/**
 * @see https://habr.com/ru/post/340758/
 * @see https://github.com/Mityuha/grpc_async/blob/master/grpc_async_server.cc
 */
SMSServiceImpl::SMSServiceImpl(
    const std::string &a_grpc_listener_address,
    const std::string &a_login,
    const std::string &a_password,
    NotifyPolicy apolicy
)
  : address(a_grpc_listener_address), login(a_login), password(a_password),
    clientsListenSMS(this, apolicy), queuingMgr(NULL), running(false)
{
  start();
}

void SMSServiceImpl::stop() {
  running = false;
  // stop service
  gpr_timespec timeout {10, 0};

  server->Shutdown(timeout);
  // Always shutdown the completion queue AFTER the server.
  cq->Shutdown();
  
  if (queuingMgr) {
    delete queuingMgr;
    queuingMgr = NULL;
  }
  std::cerr << MSG_SERVICE_STOPPED << std::endl;
}

int SMSServiceImpl::start() {
  // GRPC listener
  grpc::ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(address, grpc::InsecureServerCredentials());

  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *asynchronous* service.
  builder.RegisterService(&service); 

  // Get hold of the completion queue used for the asynchronous communication
  // with the gRPC runtime.
  cq = builder.AddCompletionQueue();

  // after cq created
  queuingMgr = new QueuingMgr(this);

  // Finally assemble the server.
  server = builder.BuildAndStart();

  return 0;
}

void SMSServiceImpl::run()
{
  // Spawn a new instances to serve new clients.
  new RequestToSendData(this);
  new ListenData(this);

  void* tag;  // uniquely identifies a request.
  bool successfulEvent;
  running = true;
  while (running) {
    // Block waiting to read the next event from the completion queue. The
    // event is uniquely identified by its tag, which in this case is the
    // memory address of an instance.
    // The return value of Next should always be checked. This return value
    // tells us whether there is any kind of event or cq_ is shutting down.
    if (!cq->Next(&tag, &successfulEvent))
      break;
    static_cast<CommonCallData*>(tag)->Proceed(successfulEvent);
  }
}

SMSServiceImpl::~SMSServiceImpl()
{
  stop();
}

bool SMSServiceImpl::isAllowed(
  const pc2sms::Credentials &value
) {
  return (value.login() == this->login) && (value.password() == this->password);
}

// Take in the "service" instance (in this case representing an asynchronous
// server) and the completion queue "cq" used for asynchronous communication
// with the gRPC runtime.
CommonCallData::CommonCallData(
  pc2sms::sms::AsyncService* a_service,
  grpc::ServerCompletionQueue* a_cq) 
  : service(a_service), cq(a_cq), status(CREATE)
{
}

/**
 * =========== RequestToSendData ===========
 */
RequestToSendData::RequestToSendData(
  SMSServiceImpl *service
) : CommonCallData(&service->service, service->cq.get()), 
  responder(&ctx), service(service), new_responder_created(false)
{
  Proceed();
}

void RequestToSendData::notifyClients(const pc2sms::SMS &value)
{
  std::vector<pc2sms::SMS> values;
  values.push_back(value);
  service->queuingMgr->enqueue(values);
}

void RequestToSendData::notifyClients(const std::vector<pc2sms::SMS> &values)
{
  if (!values.empty())
    service->queuingMgr->enqueue(values);
}

void RequestToSendData::Proceed(bool successfulEvent) {
  switch (status) {
  case CREATE:
    status = PROCESS;
    service->service.RequestrequestToSend(&ctx, &request, &responder, cq, cq, this);
    break;
  case PROCESS:
    if (!new_responder_created) {
      if (service->isAllowed(request.credentials())) {
        notifyClients(request.sms());
        result.set_count(1);
      } else {
        result.set_count(-1);
      }

      new RequestToSendData(service);
      new_responder_created = true;

      responder.Finish(result, grpc::Status(), (void*) this);
    }
    status = FINISH;
    break;
  case FINISH:
    delete this;
    break;
  default:
    break;
  }
}

/**
 * =========== ListenData ===========
 */
ListenData::ListenData(
  SMSServiceImpl *sms_service
) : smsService(sms_service), CommonCallData(&sms_service->service, sms_service->cq.get()), 
    responder(&ctx), new_responder_created(false)
{
  ctx.AsyncNotifyWhenDone(this);
  Proceed();
}

/**
 * @see https://www.gresearch.co.uk/2019/03/20/lessons-learnt-from-writing-asynchronous-streaming-grpc-services-in-c/
 */
void ListenData::enqueue(
  const std::vector<pc2sms::SMS> &values
) {
  // completion queue alarm
  for (std::vector<pc2sms::SMS>::const_iterator it = values.begin(); it != values.end(); it++) {
    result.push(*it);
  }
  if (result.empty())
    return;
  if (status == LISTEN_COMMAND) {
    grpc::Alarm alarm;
    alarm.Set(cq, gpr_now(gpr_clock_type::GPR_CLOCK_REALTIME), this);
  }
}

bool ListenData::writeNext()
{
  if (result.empty())
    return false;
  pc2sms::SMS sms = result.front();
  result.pop();
  responder.Write(sms, (void*) this);
  return true;
}

/**
 *
 * Before call ctx.IsCancelled() you must call ctx.AsyncNotifyWhenDone(this), in constructor
 * After ctx.AsyncNotifyWhenDone(this), GRPC generate event when client has been occasionally disconnected and "send event" to the  Proceed() 
 * You must check is user disconnected using ctx.IsCancelled() in the Proceed() 
 * 
 * Another approach is do not call ctx.AsyncNotifyWhenDone(this), in constructor.
 * In cis case ctx.IsCancelled() cause AVE.
 * You dont need check is user disconnected, and there no notice except 
 * check paramater successfulEvent = false 
 *  
 */
void ListenData::Proceed(bool successfulEvent) 
{
  switch (status) {
  case CREATE:
    status = LISTEN_COMMAND;
    service->RequestlistenSMSToSend(&ctx, &credentials, &responder, cq, cq, this);
    break;
  case LISTEN_COMMAND:
    if (!new_responder_created) {
      new ListenData(smsService);
      new_responder_created = true;
      smsService->clientsListenSMS.put(this);
      break;
		}
    if (result.empty()) { // if (ctx.IsCancelled()) { == ctx.AsyncNotifyWhenDone(this);
      status = FINISH;
      smsService->clientsListenSMS.rm(this);
      responder.Finish(grpc::Status(), (void*) this);
      break;
    }
    writeNext();
    status = LISTEN_COMMAND_SENT;
    break;
  case LISTEN_COMMAND_SENT:
    if (!writeNext())
      status = LISTEN_COMMAND;
    break;
  case FINISH:
    delete this;
    break;
  default:
    break;
  }
}

/**
 * =========== QueuingMgr ===========
 * 
  GRPC distribution contains an example of an asynchronous service: the service sends messages to the client one by one using 
  the responder.Write() method from predefined array.
  When all elements of the array are sent, the service calls the responder's Finish().
  After that, the responder???s object is destroyed.
  In the example, the responder is a member of the object that lives until the RPC works out to the end.

  As events occur on the database server, namely, inserting new preorder records into the table,
  service in the ListenOrderData() RPC method must send messages to connected clients.
  These events can happen at any time.
  Example does not show how to do it.

  ListenPg::listenNotifications() is waiting for the database event to occur in a separate thread.
  When this happens, it calls ListenPg::handlePgRead() to read PostgreSQL NOTIFY payload and then
  calls ListenPg::notifyClients().

  ListenPg::notifyClients() puts QueuingMgr object into queue using QueuingMgr::enqueue() method.

  When the queue reaches the QueuingMgr object, QueuingMgr queues the objects from the
  listenResponders vector using enqueue().
  You cannot directly queue these objects from another thread due to a race.
 */
QueuingMgr::QueuingMgr(
  SMSServiceImpl *sms_service
)  : smsService(sms_service), CommonCallData(&sms_service->service, sms_service->cq.get())
{
}
    
/**
 * Push into queue itself.
 * when the queue reaches this QueuingMgr object, the Proceed() method will insert
 * objects from the vector of ListenTrackData into the queue.
 * It is important to insert ListenTrackData objects from the queue handler
 * to avoid race conditions
 * 
 * @see https://www.gresearch.co.uk/2019/03/20/lessons-learnt-from-writing-asynchronous-streaming-grpc-services-in-c/
 */
void QueuingMgr::enqueue(
  const std::vector<pc2sms::SMS> &values
) {
  // completion queue alarm
  result = values;
  grpc::Alarm alarm;
  alarm.Set(cq, gpr_now(gpr_clock_type::GPR_CLOCK_REALTIME), this);
}

/**
 * Push into queue ListenTrackData objects from vector of active clients to send notification about tracks
 */
void QueuingMgr::Proceed(bool successfulEvent) {
  if (result.empty())
    return;
  smsService->clientsListenSMS.enqueue(result);
}
