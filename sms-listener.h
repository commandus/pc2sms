#ifndef SMS_LISTENER_H_
#define SMS_LISTENER_H_	1

#include <vector>
#include <mutex>
#include <grpcpp/grpcpp.h>
#include <grpcpp/alarm.h>

#include "pc2sms.grpc.pb.h"

class SMSServiceImpl;
class ListenData;

enum NotifyPolicy {
  NP_ROUND_ROBIN,
  NP_ALL
};

class ListenSMS {
private:
  NotifyPolicy policy;
  // Postgres
  std::string conninfo;
  std::vector <ListenData*> listenResponders;
  SMSServiceImpl *smsService;
  ListenData* lastResponder;
  std::mutex mutexList;

  void notifyClients(const std::vector<pc2sms::SMS> &values);
  int listenNotifications();
public:
  ListenSMS(SMSServiceImpl *owner, NotifyPolicy policy);
  virtual ~ListenSMS();
  bool checkConnStatus();
  void printListenResponders();
  void put(ListenData *value);
  bool rm(ListenData *value);
  /**
   *
   * @param values SMS to be send
   * @return count of recipients
   */
  int enqueue(const std::vector<pc2sms::SMS> &values);
};

#endif
