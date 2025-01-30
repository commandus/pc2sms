#include <sstream>
#include <algorithm>

#include <fcntl.h>

#if defined(_MSC_VER) || defined(__MINGW32__)
#else
#include <sys/un.h>
#include <unistd.h>
#endif

#include <grpcpp/grpcpp.h>
#include <grpc/support/log.h>

#include "errlist.h"
#include "platform.h"

#include "service-raw.h"
#include "sms-listener.h"


ListenSMS::ListenSMS(SMSServiceImpl *owner, NotifyPolicy apolicy)
  : policy(apolicy), smsService(owner), lastResponder(nullptr)
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

int ListenSMS::enqueue(
    const std::vector<pc2sms::SMS> &values
) {
    int r = 0;
    mutexList.lock();
    if (policy == NP_ALL) {
        for (std::vector<ListenData*>::iterator it(listenResponders.begin()); it != listenResponders.end(); ++it) {
            (*it)->enqueue(values);
            r++;
        }
    } else {
        // select next phone
        if (lastResponder) {
            std::vector<ListenData*>::iterator it = std::find(listenResponders.begin(), listenResponders.end(), lastResponder);
            if (it != listenResponders.end())
                it++;
            if (it == listenResponders.end())
                it = listenResponders.begin();
            if (it != listenResponders.end()) {
                lastResponder = *it;
                if (lastResponder) {
                    lastResponder->enqueue(values);
                    r++;
                }
            }
        } else {
            if (!listenResponders.empty()) {
                lastResponder = *listenResponders.begin();
                lastResponder->enqueue(values);
                r++;
            }
        }
      }
      mutexList.unlock();
    return r;
}
