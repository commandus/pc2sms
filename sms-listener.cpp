#include <sstream>

#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>

#include <grpcpp/grpcpp.h>
#include <grpc/support/log.h>

#include "errlist.h"
#include "platform.h"

#include "service-raw.h"
#include "sms-listener.h"


ListenSMS::ListenSMS(SMSServiceImpl *owner)
  : stopped(true), listenerThread(NULL), smsService(owner)
{
}

ListenSMS::~ListenSMS()
{
  stop();
}

void ListenSMS::notifyClients(const std::vector<pc2sms::SMS> &values)
{
  // std::cerr << "notify clients: track " << t.id() << std::endl;
  if (!values.empty())
    smsService->queuingMgr->enqueue(values);
}

void ListenSMS::start(
)
{
  stopped = false;
  // listenerThread = new std::thread(&ListenSMS::listenNotifications, this);
  listenerThread = NULL;
}

void ListenSMS::stop()
{
  if (stopped)
    return;
  stopped = true;
  if (!listenerThread)
    return;
  listenerThread->join();
  listenerThread = NULL;
}

void ListenSMS::done()
{
  listenerThread = NULL;
  stopped = true;
}

int ListenSMS::listenNotifications()
{
  int connected = 0;
  int connPollReady = 0;

  while (!stopped) {
    struct timeval timeout;
    timeout.tv_sec = 30;
    timeout.tv_usec = 0;
    // hang out until there is stuff to read or write
    int retval = select(0, 0, 0, NULL, &timeout);
    switch (retval) {
    case -1:
      if (!stopped) {
        done();
        return ERR_CODE_SELECT;
      }
      stopped = true;
      break;
    case 0:
      break;
    };
  };
  done();
  return SMS_OK;
}

void ListenSMS::printListenResponders()
{
  std::cerr << "Responders:" << std::endl;
  for (auto it(listenResponders.begin()); it != listenResponders.end(); ++it) {
    std::cerr << *it << std::endl;
  }
}

void ListenSMS::put(ListenData *value)
{
  std::cerr << "ListenSMS::put" << std::endl;
  listenResponders.push_back(value);
}

bool ListenSMS::rm(ListenData *value)
{
  std::cerr << "ListenSMS::rm" << std::endl;
  for (std::vector <ListenData*>::iterator it(listenResponders.begin()); it != listenResponders.end(); ++it) {
    if (*it == value) {
      listenResponders.erase(it);
      return true;
    }
  }
  return false;
}

void ListenSMS::enqueue(
  const std::vector<pc2sms::SMS> &values
) {
  std::cerr << "ListenSMS::enqueue, size: " << listenResponders.size() << std::endl;
  
  for (std::vector<ListenData*>::iterator it(listenResponders.begin()); it != listenResponders.end(); ++it) {
    std::cerr << "ListenSMS::enqueue: " << std::endl;
    (*it)->enqueue(values);
  }
}
