#ifndef SMS_LISTENER_H_
#define SMS_LISTENER_H_	1

#include <thread>
#include <vector>
#include <grpcpp/grpcpp.h>
#include <grpcpp/alarm.h>

#include "pc2sms.grpc.pb.h"

class SMSServiceImpl;
class ListenData;

class ListenSMS {
private:
  bool stopped;
  // Postgres
  std::string conninfo;
  std::vector <ListenData*> listenResponders;
  std::thread *listenerThread;
  SMSServiceImpl *smsService;

  void notifyClients(const std::vector<pc2sms::SMS> &values);
  int listenNotifications();
public:
  ListenSMS(SMSServiceImpl *owner);
  virtual ~ListenSMS();
  void start();
  void stop();
  void done();
  bool checkConnStatus();
  void printListenResponders();
  void put(ListenData *value);
  bool rm(ListenData *value);
  void enqueue(const std::vector<pc2sms::SMS> &values);
};

#endif
