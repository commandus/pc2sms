// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: pc2sms.proto
#ifndef GRPC_pc2sms_2eproto__INCLUDED
#define GRPC_pc2sms_2eproto__INCLUDED

#include "pc2sms.pb.h"

#include <functional>
#include <grpc/impl/codegen/port_platform.h>
#include <grpcpp/impl/codegen/async_generic_service.h>
#include <grpcpp/impl/codegen/async_stream.h>
#include <grpcpp/impl/codegen/async_unary_call.h>
#include <grpcpp/impl/codegen/client_callback.h>
#include <grpcpp/impl/codegen/client_context.h>
#include <grpcpp/impl/codegen/completion_queue.h>
#include <grpcpp/impl/codegen/message_allocator.h>
#include <grpcpp/impl/codegen/method_handler.h>
#include <grpcpp/impl/codegen/proto_utils.h>
#include <grpcpp/impl/codegen/rpc_method.h>
#include <grpcpp/impl/codegen/server_callback.h>
#include <grpcpp/impl/codegen/server_callback_handlers.h>
#include <grpcpp/impl/codegen/server_context.h>
#include <grpcpp/impl/codegen/service_type.h>
#include <grpcpp/impl/codegen/status.h>
#include <grpcpp/impl/codegen/stub_options.h>
#include <grpcpp/impl/codegen/sync_stream.h>

namespace pc2sms {

class sms final {
 public:
  static constexpr char const* service_full_name() {
    return "pc2sms.sms";
  }
  class StubInterface {
   public:
    virtual ~StubInterface() {}
    virtual ::grpc::Status requestToSend(::grpc::ClientContext* context, const ::pc2sms::RequestCommand& request, ::pc2sms::ResponseCommand* response) = 0;
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::pc2sms::ResponseCommand>> AsyncrequestToSend(::grpc::ClientContext* context, const ::pc2sms::RequestCommand& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::pc2sms::ResponseCommand>>(AsyncrequestToSendRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::pc2sms::ResponseCommand>> PrepareAsyncrequestToSend(::grpc::ClientContext* context, const ::pc2sms::RequestCommand& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::pc2sms::ResponseCommand>>(PrepareAsyncrequestToSendRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientReaderInterface< ::pc2sms::SMS>> listenSMSToSend(::grpc::ClientContext* context, const ::pc2sms::Credentials& request) {
      return std::unique_ptr< ::grpc::ClientReaderInterface< ::pc2sms::SMS>>(listenSMSToSendRaw(context, request));
    }
    std::unique_ptr< ::grpc::ClientAsyncReaderInterface< ::pc2sms::SMS>> AsynclistenSMSToSend(::grpc::ClientContext* context, const ::pc2sms::Credentials& request, ::grpc::CompletionQueue* cq, void* tag) {
      return std::unique_ptr< ::grpc::ClientAsyncReaderInterface< ::pc2sms::SMS>>(AsynclistenSMSToSendRaw(context, request, cq, tag));
    }
    std::unique_ptr< ::grpc::ClientAsyncReaderInterface< ::pc2sms::SMS>> PrepareAsynclistenSMSToSend(::grpc::ClientContext* context, const ::pc2sms::Credentials& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncReaderInterface< ::pc2sms::SMS>>(PrepareAsynclistenSMSToSendRaw(context, request, cq));
    }
    class experimental_async_interface {
     public:
      virtual ~experimental_async_interface() {}
      virtual void requestToSend(::grpc::ClientContext* context, const ::pc2sms::RequestCommand* request, ::pc2sms::ResponseCommand* response, std::function<void(::grpc::Status)>) = 0;
      virtual void requestToSend(::grpc::ClientContext* context, const ::grpc::ByteBuffer* request, ::pc2sms::ResponseCommand* response, std::function<void(::grpc::Status)>) = 0;
      #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
      virtual void requestToSend(::grpc::ClientContext* context, const ::pc2sms::RequestCommand* request, ::pc2sms::ResponseCommand* response, ::grpc::ClientUnaryReactor* reactor) = 0;
      #else
      virtual void requestToSend(::grpc::ClientContext* context, const ::pc2sms::RequestCommand* request, ::pc2sms::ResponseCommand* response, ::grpc::experimental::ClientUnaryReactor* reactor) = 0;
      #endif
      #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
      virtual void requestToSend(::grpc::ClientContext* context, const ::grpc::ByteBuffer* request, ::pc2sms::ResponseCommand* response, ::grpc::ClientUnaryReactor* reactor) = 0;
      #else
      virtual void requestToSend(::grpc::ClientContext* context, const ::grpc::ByteBuffer* request, ::pc2sms::ResponseCommand* response, ::grpc::experimental::ClientUnaryReactor* reactor) = 0;
      #endif
      #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
      virtual void listenSMSToSend(::grpc::ClientContext* context, ::pc2sms::Credentials* request, ::grpc::ClientReadReactor< ::pc2sms::SMS>* reactor) = 0;
      #else
      virtual void listenSMSToSend(::grpc::ClientContext* context, ::pc2sms::Credentials* request, ::grpc::experimental::ClientReadReactor< ::pc2sms::SMS>* reactor) = 0;
      #endif
    };
    #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
    typedef class experimental_async_interface async_interface;
    #endif
    #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
    async_interface* async() { return experimental_async(); }
    #endif
    virtual class experimental_async_interface* experimental_async() { return nullptr; }
  private:
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::pc2sms::ResponseCommand>* AsyncrequestToSendRaw(::grpc::ClientContext* context, const ::pc2sms::RequestCommand& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::pc2sms::ResponseCommand>* PrepareAsyncrequestToSendRaw(::grpc::ClientContext* context, const ::pc2sms::RequestCommand& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientReaderInterface< ::pc2sms::SMS>* listenSMSToSendRaw(::grpc::ClientContext* context, const ::pc2sms::Credentials& request) = 0;
    virtual ::grpc::ClientAsyncReaderInterface< ::pc2sms::SMS>* AsynclistenSMSToSendRaw(::grpc::ClientContext* context, const ::pc2sms::Credentials& request, ::grpc::CompletionQueue* cq, void* tag) = 0;
    virtual ::grpc::ClientAsyncReaderInterface< ::pc2sms::SMS>* PrepareAsynclistenSMSToSendRaw(::grpc::ClientContext* context, const ::pc2sms::Credentials& request, ::grpc::CompletionQueue* cq) = 0;
  };
  class Stub final : public StubInterface {
   public:
    Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel);
    ::grpc::Status requestToSend(::grpc::ClientContext* context, const ::pc2sms::RequestCommand& request, ::pc2sms::ResponseCommand* response) override;
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::pc2sms::ResponseCommand>> AsyncrequestToSend(::grpc::ClientContext* context, const ::pc2sms::RequestCommand& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::pc2sms::ResponseCommand>>(AsyncrequestToSendRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::pc2sms::ResponseCommand>> PrepareAsyncrequestToSend(::grpc::ClientContext* context, const ::pc2sms::RequestCommand& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::pc2sms::ResponseCommand>>(PrepareAsyncrequestToSendRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientReader< ::pc2sms::SMS>> listenSMSToSend(::grpc::ClientContext* context, const ::pc2sms::Credentials& request) {
      return std::unique_ptr< ::grpc::ClientReader< ::pc2sms::SMS>>(listenSMSToSendRaw(context, request));
    }
    std::unique_ptr< ::grpc::ClientAsyncReader< ::pc2sms::SMS>> AsynclistenSMSToSend(::grpc::ClientContext* context, const ::pc2sms::Credentials& request, ::grpc::CompletionQueue* cq, void* tag) {
      return std::unique_ptr< ::grpc::ClientAsyncReader< ::pc2sms::SMS>>(AsynclistenSMSToSendRaw(context, request, cq, tag));
    }
    std::unique_ptr< ::grpc::ClientAsyncReader< ::pc2sms::SMS>> PrepareAsynclistenSMSToSend(::grpc::ClientContext* context, const ::pc2sms::Credentials& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncReader< ::pc2sms::SMS>>(PrepareAsynclistenSMSToSendRaw(context, request, cq));
    }
    class experimental_async final :
      public StubInterface::experimental_async_interface {
     public:
      void requestToSend(::grpc::ClientContext* context, const ::pc2sms::RequestCommand* request, ::pc2sms::ResponseCommand* response, std::function<void(::grpc::Status)>) override;
      void requestToSend(::grpc::ClientContext* context, const ::grpc::ByteBuffer* request, ::pc2sms::ResponseCommand* response, std::function<void(::grpc::Status)>) override;
      #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
      void requestToSend(::grpc::ClientContext* context, const ::pc2sms::RequestCommand* request, ::pc2sms::ResponseCommand* response, ::grpc::ClientUnaryReactor* reactor) override;
      #else
      void requestToSend(::grpc::ClientContext* context, const ::pc2sms::RequestCommand* request, ::pc2sms::ResponseCommand* response, ::grpc::experimental::ClientUnaryReactor* reactor) override;
      #endif
      #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
      void requestToSend(::grpc::ClientContext* context, const ::grpc::ByteBuffer* request, ::pc2sms::ResponseCommand* response, ::grpc::ClientUnaryReactor* reactor) override;
      #else
      void requestToSend(::grpc::ClientContext* context, const ::grpc::ByteBuffer* request, ::pc2sms::ResponseCommand* response, ::grpc::experimental::ClientUnaryReactor* reactor) override;
      #endif
      #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
      void listenSMSToSend(::grpc::ClientContext* context, ::pc2sms::Credentials* request, ::grpc::ClientReadReactor< ::pc2sms::SMS>* reactor) override;
      #else
      void listenSMSToSend(::grpc::ClientContext* context, ::pc2sms::Credentials* request, ::grpc::experimental::ClientReadReactor< ::pc2sms::SMS>* reactor) override;
      #endif
     private:
      friend class Stub;
      explicit experimental_async(Stub* stub): stub_(stub) { }
      Stub* stub() { return stub_; }
      Stub* stub_;
    };
    class experimental_async_interface* experimental_async() override { return &async_stub_; }

   private:
    std::shared_ptr< ::grpc::ChannelInterface> channel_;
    class experimental_async async_stub_{this};
    ::grpc::ClientAsyncResponseReader< ::pc2sms::ResponseCommand>* AsyncrequestToSendRaw(::grpc::ClientContext* context, const ::pc2sms::RequestCommand& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::pc2sms::ResponseCommand>* PrepareAsyncrequestToSendRaw(::grpc::ClientContext* context, const ::pc2sms::RequestCommand& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientReader< ::pc2sms::SMS>* listenSMSToSendRaw(::grpc::ClientContext* context, const ::pc2sms::Credentials& request) override;
    ::grpc::ClientAsyncReader< ::pc2sms::SMS>* AsynclistenSMSToSendRaw(::grpc::ClientContext* context, const ::pc2sms::Credentials& request, ::grpc::CompletionQueue* cq, void* tag) override;
    ::grpc::ClientAsyncReader< ::pc2sms::SMS>* PrepareAsynclistenSMSToSendRaw(::grpc::ClientContext* context, const ::pc2sms::Credentials& request, ::grpc::CompletionQueue* cq) override;
    const ::grpc::internal::RpcMethod rpcmethod_requestToSend_;
    const ::grpc::internal::RpcMethod rpcmethod_listenSMSToSend_;
  };
  static std::unique_ptr<Stub> NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options = ::grpc::StubOptions());

  class Service : public ::grpc::Service {
   public:
    Service();
    virtual ~Service();
    virtual ::grpc::Status requestToSend(::grpc::ServerContext* context, const ::pc2sms::RequestCommand* request, ::pc2sms::ResponseCommand* response);
    virtual ::grpc::Status listenSMSToSend(::grpc::ServerContext* context, const ::pc2sms::Credentials* request, ::grpc::ServerWriter< ::pc2sms::SMS>* writer);
  };
  template <class BaseClass>
  class WithAsyncMethod_requestToSend : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithAsyncMethod_requestToSend() {
      ::grpc::Service::MarkMethodAsync(0);
    }
    ~WithAsyncMethod_requestToSend() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status requestToSend(::grpc::ServerContext* /*context*/, const ::pc2sms::RequestCommand* /*request*/, ::pc2sms::ResponseCommand* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestrequestToSend(::grpc::ServerContext* context, ::pc2sms::RequestCommand* request, ::grpc::ServerAsyncResponseWriter< ::pc2sms::ResponseCommand>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(0, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithAsyncMethod_listenSMSToSend : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithAsyncMethod_listenSMSToSend() {
      ::grpc::Service::MarkMethodAsync(1);
    }
    ~WithAsyncMethod_listenSMSToSend() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status listenSMSToSend(::grpc::ServerContext* /*context*/, const ::pc2sms::Credentials* /*request*/, ::grpc::ServerWriter< ::pc2sms::SMS>* /*writer*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestlistenSMSToSend(::grpc::ServerContext* context, ::pc2sms::Credentials* request, ::grpc::ServerAsyncWriter< ::pc2sms::SMS>* writer, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncServerStreaming(1, context, request, writer, new_call_cq, notification_cq, tag);
    }
  };
  typedef WithAsyncMethod_requestToSend<WithAsyncMethod_listenSMSToSend<Service > > AsyncService;
  template <class BaseClass>
  class ExperimentalWithCallbackMethod_requestToSend : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    ExperimentalWithCallbackMethod_requestToSend() {
    #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
      ::grpc::Service::
    #else
      ::grpc::Service::experimental().
    #endif
        MarkMethodCallback(0,
          new ::grpc_impl::internal::CallbackUnaryHandler< ::pc2sms::RequestCommand, ::pc2sms::ResponseCommand>(
            [this](
    #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
                   ::grpc::CallbackServerContext*
    #else
                   ::grpc::experimental::CallbackServerContext*
    #endif
                     context, const ::pc2sms::RequestCommand* request, ::pc2sms::ResponseCommand* response) { return this->requestToSend(context, request, response); }));}
    void SetMessageAllocatorFor_requestToSend(
        ::grpc::experimental::MessageAllocator< ::pc2sms::RequestCommand, ::pc2sms::ResponseCommand>* allocator) {
    #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
      ::grpc::internal::MethodHandler* const handler = ::grpc::Service::GetHandler(0);
    #else
      ::grpc::internal::MethodHandler* const handler = ::grpc::Service::experimental().GetHandler(0);
    #endif
      static_cast<::grpc_impl::internal::CallbackUnaryHandler< ::pc2sms::RequestCommand, ::pc2sms::ResponseCommand>*>(handler)
              ->SetMessageAllocator(allocator);
    }
    ~ExperimentalWithCallbackMethod_requestToSend() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status requestToSend(::grpc::ServerContext* /*context*/, const ::pc2sms::RequestCommand* /*request*/, ::pc2sms::ResponseCommand* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
    virtual ::grpc::ServerUnaryReactor* requestToSend(
      ::grpc::CallbackServerContext* /*context*/, const ::pc2sms::RequestCommand* /*request*/, ::pc2sms::ResponseCommand* /*response*/)
    #else
    virtual ::grpc::experimental::ServerUnaryReactor* requestToSend(
      ::grpc::experimental::CallbackServerContext* /*context*/, const ::pc2sms::RequestCommand* /*request*/, ::pc2sms::ResponseCommand* /*response*/)
    #endif
      { return nullptr; }
  };
  template <class BaseClass>
  class ExperimentalWithCallbackMethod_listenSMSToSend : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    ExperimentalWithCallbackMethod_listenSMSToSend() {
    #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
      ::grpc::Service::
    #else
      ::grpc::Service::experimental().
    #endif
        MarkMethodCallback(1,
          new ::grpc_impl::internal::CallbackServerStreamingHandler< ::pc2sms::Credentials, ::pc2sms::SMS>(
            [this](
    #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
                   ::grpc::CallbackServerContext*
    #else
                   ::grpc::experimental::CallbackServerContext*
    #endif
                     context, const ::pc2sms::Credentials* request) { return this->listenSMSToSend(context, request); }));
    }
    ~ExperimentalWithCallbackMethod_listenSMSToSend() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status listenSMSToSend(::grpc::ServerContext* /*context*/, const ::pc2sms::Credentials* /*request*/, ::grpc::ServerWriter< ::pc2sms::SMS>* /*writer*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
    virtual ::grpc::ServerWriteReactor< ::pc2sms::SMS>* listenSMSToSend(
      ::grpc::CallbackServerContext* /*context*/, const ::pc2sms::Credentials* /*request*/)
    #else
    virtual ::grpc::experimental::ServerWriteReactor< ::pc2sms::SMS>* listenSMSToSend(
      ::grpc::experimental::CallbackServerContext* /*context*/, const ::pc2sms::Credentials* /*request*/)
    #endif
      { return nullptr; }
  };
  #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
  typedef ExperimentalWithCallbackMethod_requestToSend<ExperimentalWithCallbackMethod_listenSMSToSend<Service > > CallbackService;
  #endif

  typedef ExperimentalWithCallbackMethod_requestToSend<ExperimentalWithCallbackMethod_listenSMSToSend<Service > > ExperimentalCallbackService;
  template <class BaseClass>
  class WithGenericMethod_requestToSend : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithGenericMethod_requestToSend() {
      ::grpc::Service::MarkMethodGeneric(0);
    }
    ~WithGenericMethod_requestToSend() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status requestToSend(::grpc::ServerContext* /*context*/, const ::pc2sms::RequestCommand* /*request*/, ::pc2sms::ResponseCommand* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithGenericMethod_listenSMSToSend : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithGenericMethod_listenSMSToSend() {
      ::grpc::Service::MarkMethodGeneric(1);
    }
    ~WithGenericMethod_listenSMSToSend() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status listenSMSToSend(::grpc::ServerContext* /*context*/, const ::pc2sms::Credentials* /*request*/, ::grpc::ServerWriter< ::pc2sms::SMS>* /*writer*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithRawMethod_requestToSend : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawMethod_requestToSend() {
      ::grpc::Service::MarkMethodRaw(0);
    }
    ~WithRawMethod_requestToSend() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status requestToSend(::grpc::ServerContext* /*context*/, const ::pc2sms::RequestCommand* /*request*/, ::pc2sms::ResponseCommand* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestrequestToSend(::grpc::ServerContext* context, ::grpc::ByteBuffer* request, ::grpc::ServerAsyncResponseWriter< ::grpc::ByteBuffer>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(0, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithRawMethod_listenSMSToSend : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawMethod_listenSMSToSend() {
      ::grpc::Service::MarkMethodRaw(1);
    }
    ~WithRawMethod_listenSMSToSend() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status listenSMSToSend(::grpc::ServerContext* /*context*/, const ::pc2sms::Credentials* /*request*/, ::grpc::ServerWriter< ::pc2sms::SMS>* /*writer*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestlistenSMSToSend(::grpc::ServerContext* context, ::grpc::ByteBuffer* request, ::grpc::ServerAsyncWriter< ::grpc::ByteBuffer>* writer, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncServerStreaming(1, context, request, writer, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class ExperimentalWithRawCallbackMethod_requestToSend : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    ExperimentalWithRawCallbackMethod_requestToSend() {
    #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
      ::grpc::Service::
    #else
      ::grpc::Service::experimental().
    #endif
        MarkMethodRawCallback(0,
          new ::grpc_impl::internal::CallbackUnaryHandler< ::grpc::ByteBuffer, ::grpc::ByteBuffer>(
            [this](
    #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
                   ::grpc::CallbackServerContext*
    #else
                   ::grpc::experimental::CallbackServerContext*
    #endif
                     context, const ::grpc::ByteBuffer* request, ::grpc::ByteBuffer* response) { return this->requestToSend(context, request, response); }));
    }
    ~ExperimentalWithRawCallbackMethod_requestToSend() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status requestToSend(::grpc::ServerContext* /*context*/, const ::pc2sms::RequestCommand* /*request*/, ::pc2sms::ResponseCommand* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
    virtual ::grpc::ServerUnaryReactor* requestToSend(
      ::grpc::CallbackServerContext* /*context*/, const ::grpc::ByteBuffer* /*request*/, ::grpc::ByteBuffer* /*response*/)
    #else
    virtual ::grpc::experimental::ServerUnaryReactor* requestToSend(
      ::grpc::experimental::CallbackServerContext* /*context*/, const ::grpc::ByteBuffer* /*request*/, ::grpc::ByteBuffer* /*response*/)
    #endif
      { return nullptr; }
  };
  template <class BaseClass>
  class ExperimentalWithRawCallbackMethod_listenSMSToSend : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    ExperimentalWithRawCallbackMethod_listenSMSToSend() {
    #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
      ::grpc::Service::
    #else
      ::grpc::Service::experimental().
    #endif
        MarkMethodRawCallback(1,
          new ::grpc_impl::internal::CallbackServerStreamingHandler< ::grpc::ByteBuffer, ::grpc::ByteBuffer>(
            [this](
    #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
                   ::grpc::CallbackServerContext*
    #else
                   ::grpc::experimental::CallbackServerContext*
    #endif
                     context, const::grpc::ByteBuffer* request) { return this->listenSMSToSend(context, request); }));
    }
    ~ExperimentalWithRawCallbackMethod_listenSMSToSend() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status listenSMSToSend(::grpc::ServerContext* /*context*/, const ::pc2sms::Credentials* /*request*/, ::grpc::ServerWriter< ::pc2sms::SMS>* /*writer*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    #ifdef GRPC_CALLBACK_API_NONEXPERIMENTAL
    virtual ::grpc::ServerWriteReactor< ::grpc::ByteBuffer>* listenSMSToSend(
      ::grpc::CallbackServerContext* /*context*/, const ::grpc::ByteBuffer* /*request*/)
    #else
    virtual ::grpc::experimental::ServerWriteReactor< ::grpc::ByteBuffer>* listenSMSToSend(
      ::grpc::experimental::CallbackServerContext* /*context*/, const ::grpc::ByteBuffer* /*request*/)
    #endif
      { return nullptr; }
  };
  template <class BaseClass>
  class WithStreamedUnaryMethod_requestToSend : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithStreamedUnaryMethod_requestToSend() {
      ::grpc::Service::MarkMethodStreamed(0,
        new ::grpc::internal::StreamedUnaryHandler<
          ::pc2sms::RequestCommand, ::pc2sms::ResponseCommand>(
            [this](::grpc_impl::ServerContext* context,
                   ::grpc_impl::ServerUnaryStreamer<
                     ::pc2sms::RequestCommand, ::pc2sms::ResponseCommand>* streamer) {
                       return this->StreamedrequestToSend(context,
                         streamer);
                  }));
    }
    ~WithStreamedUnaryMethod_requestToSend() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable regular version of this method
    ::grpc::Status requestToSend(::grpc::ServerContext* /*context*/, const ::pc2sms::RequestCommand* /*request*/, ::pc2sms::ResponseCommand* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    // replace default version of method with streamed unary
    virtual ::grpc::Status StreamedrequestToSend(::grpc::ServerContext* context, ::grpc::ServerUnaryStreamer< ::pc2sms::RequestCommand,::pc2sms::ResponseCommand>* server_unary_streamer) = 0;
  };
  typedef WithStreamedUnaryMethod_requestToSend<Service > StreamedUnaryService;
  template <class BaseClass>
  class WithSplitStreamingMethod_listenSMSToSend : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithSplitStreamingMethod_listenSMSToSend() {
      ::grpc::Service::MarkMethodStreamed(1,
        new ::grpc::internal::SplitServerStreamingHandler<
          ::pc2sms::Credentials, ::pc2sms::SMS>(
            [this](::grpc_impl::ServerContext* context,
                   ::grpc_impl::ServerSplitStreamer<
                     ::pc2sms::Credentials, ::pc2sms::SMS>* streamer) {
                       return this->StreamedlistenSMSToSend(context,
                         streamer);
                  }));
    }
    ~WithSplitStreamingMethod_listenSMSToSend() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable regular version of this method
    ::grpc::Status listenSMSToSend(::grpc::ServerContext* /*context*/, const ::pc2sms::Credentials* /*request*/, ::grpc::ServerWriter< ::pc2sms::SMS>* /*writer*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    // replace default version of method with split streamed
    virtual ::grpc::Status StreamedlistenSMSToSend(::grpc::ServerContext* context, ::grpc::ServerSplitStreamer< ::pc2sms::Credentials,::pc2sms::SMS>* server_split_streamer) = 0;
  };
  typedef WithSplitStreamingMethod_listenSMSToSend<Service > SplitStreamedService;
  typedef WithStreamedUnaryMethod_requestToSend<WithSplitStreamingMethod_listenSMSToSend<Service > > StreamedService;
};

}  // namespace pc2sms


#endif  // GRPC_pc2sms_2eproto__INCLUDED
