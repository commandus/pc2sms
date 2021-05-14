#include <sstream>
#include <algorithm>

#include <unistd.h>
#include <sys/un.h>
#include <fcntl.h>

#include <grpcpp/grpcpp.h>
#include <grpc/support/log.h>

#include "errlist.h"
#include "platform.h"

#include "service-raw.h"
#include "sms-listener.h"


ListenSMS::ListenSMS(SMSServiceImpl *owner, NotifyPolicy apolicy)
  : policy(apolicy), smsService(owner), lastResponder(NULL)
{
}

ListenSMS::~ListenSMS()
{
}

void ListenSMS::notifyClients(const std::vector<pc2sms::SMS> &values)
{
  if (!values.empty())
    smsService->queuingMgr->enqueue(values);
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
  mutexList.lock();
  listenResponders.push_back(value);
  mutexList.unlock();
}

bool ListenSMS::rm(ListenData *value)
{
  mutexList.lock();
  for (std::vector <ListenData*>::iterator it(listenResponders.begin()); it != listenResponders.end(); ++it) {
    if (*it == value) {
      listenResponders.erase(it);
      mutexList.unlock();
      return true;
    }
  }
  mutexList.unlock();
  return false;
}

void ListenSMS::enqueue(
  const std::vector<pc2sms::SMS> &values
) {
  mutexList.lock();
  if (policy == NP_ALL) {
    for (std::vector<ListenData*>::iterator it(listenResponders.begin()); it != listenResponders.end(); ++it) {
      (*it)->enqueue(values);
    }
  } else {
    // select next phone
    if (lastResponder) {
      std::vector<ListenData*>::iterator it = std::find(listenResponders.begin(), listenResponders.end(), lastResponder);
      if (it != listenResponders.end()) {
        it++;
      }
      if (it == listenResponders.end()) {
        it = listenResponders.begin();
      }
      if (it != listenResponders.end()) {
      }
      lastResponder = *it;
      if (lastResponder)
        lastResponder->enqueue(values);
    } else {
      lastResponder = *listenResponders.begin();
      lastResponder->enqueue(values);
    }
  }
  mutexList.unlock();
}
