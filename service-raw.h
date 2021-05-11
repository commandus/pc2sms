#ifndef SERVICE_RAW_H_
#define SERVICE_RAW_H_	1

#include <thread>
#include <queue>

#include <grpcpp/grpcpp.h>
#include <grpcpp/alarm.h>
#include "pc2sms.grpc.pb.h"

class QueuingMgr;

/**
 * Async service
 */
class GpsTrackMgrServiceImpl final {
private:
  std::string address;
  std::unique_ptr<grpc::Server> server;
public:
  explicit GpsTrackMgrServiceImpl(
    const std::string &conninfo,
    const std::string &grpc_listener_address,
    bool allow_user_add_device
  );
  virtual ~GpsTrackMgrServiceImpl();
  // Postgresql listener
  ConnStatusType start();
  void stop();
  // GRPC main event loop
  void run();
  bool checkConnStatus();

  // Postgres
  std::string conninfo;
  PGconn *conn;
  std::unique_ptr<grpc::ServerCompletionQueue> cq;
  gpstracker::monitor::AsyncService service;
  bool allowUserAddDevice; 
  bool running;
  ListenPg clientsListenPg;
  QueuingMgr *queuingMgr;
};

/**
 * Class encompasing the state and logic needed to serve a request.
 */
class CommonCallData {
public:
  // The means of communication with the gRPC runtime for an asynchronous server.
  gpstracker::monitor::AsyncService* service;
  // The producer-consumer queue where for asynchronous server notifications.
  grpc::ServerCompletionQueue* cq;
  // Context for the rpc, allowing to tweak aspects of it such as the use
  // of compression, authentication, as well as to send metadata back to the
  // client.
  grpc::ServerContext ctx;
  enum CallStatus { CREATE, PROCESS, FINISH, LISTEN_TRACK, LISTEN_TRACK_SENT };
  CallStatus status;  // The current serving state.
  // Take in the "service" instance OrdersData(in this case representing an asynchronous
  // server) and the completion queue "cq" used for asynchronous communication
  // with the gRPC runtime.
  CommonCallData(gpstracker::monitor::AsyncService* service, grpc::ServerCompletionQueue* cq);
  virtual void Proceed(bool = true) = 0;
};

/**
 * Responds on deviceTrack()
 */
class DeviceTrackData : public CommonCallData {
  private:
    GpsTrackMgrServiceImpl *service;
    bool new_responder_created;
    gpstracker::Device result;
    int load(bool is_admin);
  public:
    // What we get from the client
    gpstracker::TrackRequest request;
    // What we send back to the client
    grpc::ServerAsyncResponseWriter<gpstracker::Device> responder;
    // Take in the "service" instance (in this case representing an asynchronous
    // server) and the completion queue "cq" used for asynchronous communication
    // with the gRPC runtime.
    DeviceTrackData(GpsTrackMgrServiceImpl *service);
    virtual void Proceed(bool = true) override;
};

/**
 * Responds on lsDevices()
 */
class LsDevicesData : public CommonCallData {
  private:
    GpsTrackMgrServiceImpl *service;
    bool new_responder_created;
    std::vector <gpstracker::Device> result;
    size_t counter;
    int load();
  public:
    // What we get from the client
    gpstracker::DeviceTrackRequest request;
    // What we send back to the client
    grpc::ServerAsyncWriter<gpstracker::Device> responder;
    // Take in the "service" instance (in this case representing an asynchronous
    // server) and the completion queue "cq" used for asynchronous communication
    // with the gRPC runtime.
    LsDevicesData(GpsTrackMgrServiceImpl *service);
    virtual void Proceed(bool = true) override;
};

/**
 * Responds on lsWatcher()
 */
class LsWatchersData : public CommonCallData {
  private:
    GpsTrackMgrServiceImpl *service;
    bool new_responder_created;
    std::vector <gpstracker::Watcher> result;
    size_t counter;
    int load();
  public:
    // What we get from the client
    gpstracker::DeviceTrackRequest request;
    // What we send back to the client
    grpc::ServerAsyncWriter<gpstracker::Watcher> responder;
    // Take in the "service" instance (in this case representing an asynchronous
    // server) and the completion queue "cq" used for asynchronous communication
    // with the gRPC runtime.
    LsWatchersData(GpsTrackMgrServiceImpl *service);
    virtual void Proceed(bool = true) override;
};

/**
 * Responds on chDevice()
 */
class ChDeviceData : public CommonCallData {
  private:
    GpsTrackMgrServiceImpl *service;
    bool new_responder_created;
    gpstracker::Device result;
    int load();
  public:
    // What we get from the client
    gpstracker::ChDeviceRequest request;
    // What we send back to the client
    grpc::ServerAsyncResponseWriter<gpstracker::Device> responder;
    // Take in the "service" instance (in this case representing an asynchronous
    // server) and the completion queue "cq" used for asynchronous communication
    // with the gRPC runtime.
    ChDeviceData(GpsTrackMgrServiceImpl *service);
    virtual void Proceed(bool = true) override;
};

/**
 * Responds on chDevice()
 */
class ChWatcherData : public CommonCallData {
  private:
    GpsTrackMgrServiceImpl *service;
    bool new_responder_created;
    gpstracker::Watcher result;
    int load();
  public:
    // What we get from the client
    gpstracker::ChWatcherRequest request;
    // What we send back to the client
    grpc::ServerAsyncResponseWriter<gpstracker::Watcher> responder;
    // Take in the "service" instance (in this case representing an asynchronous
    // server) and the completion queue "cq" used for asynchronous communication
    // with the gRPC runtime.
    ChWatcherData(GpsTrackMgrServiceImpl *service);
    virtual void Proceed(bool = true) override;
};

/**
 * Responds on addDevices()
 */
class AddDevicesData : public CommonCallData {
  private:
    GpsTrackMgrServiceImpl *service;
    bool new_responder_created;
    gpstracker::BatchResponse result;
    int put();
  public:
    // What we get from the client
    grpc::ServerAsyncReader< ::gpstracker::BatchResponse, ::gpstracker::Device> responder;
    gpstracker::Device request;
    AddDevicesData(GpsTrackMgrServiceImpl *service);
    virtual void Proceed(bool = true) override;
};

/**
 * Responds on addTracks()
 */
class AddTracksData : public CommonCallData {
  private:
    GpsTrackMgrServiceImpl *service;
    bool new_responder_created;
    gpstracker::BatchResponse result;
    int put();
  public:
    // What we get from the client
    grpc::ServerAsyncReader< ::gpstracker::BatchResponse, ::gpstracker::Track> responder;
    gpstracker::Track request;
    AddTracksData(GpsTrackMgrServiceImpl *service);
    virtual void Proceed(bool = true) override;
};

/**
 * Listen tracks
 */
class ListenTrackData : public CommonCallData {
  private:
    GpsTrackMgrServiceImpl *trackService;
    bool new_responder_created;
    std::queue<gpstracker::Track> result;
    // listen for a notifications of new records
    int listenNotifications();
    bool isWatcherCanSeeTrack(
      const gpstracker::Watcher &watcher,
      const gpstracker::Track &track,
      gpstracker::RawData *rawdata
    );
    bool writeNext();
  public:
    // What we get from the client in login.
    gpstracker::Watcher rq;
    // The means to get back to the client.
    grpc::ServerAsyncWriter<gpstracker::TrackNRawData> responder;
    // Take in the "service" instance (in this case representing an asynchronous
    // server) and the completion queue "cq" used for asynchronous communication
    // with the gRPC runtime.
    ListenTrackData(GpsTrackMgrServiceImpl *track_service);
    virtual void Proceed(bool = true) override;
    void enqueue(const std::vector<gpstracker::Track> &values);
};

/**
 * Enqueue ListenOrderData after database signal a new record arrived
 */
class QueuingMgr : public CommonCallData {
  private:
    GpsTrackMgrServiceImpl *trackService;
    std::vector<gpstracker::Track> result;
  public:
    // Take in the "service" instance (in this case representing an asynchronous
    // server) and the completion queue "cq" used for asynchronous communication
    // with the gRPC runtime.
    QueuingMgr(GpsTrackMgrServiceImpl *track_service);
    virtual void Proceed(bool = true) override;
    void enqueue(const std::vector<gpstracker::Track> &values);
};

#endif
