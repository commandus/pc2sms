#include <string.h>
#include "errlist.h"
#define ERR_COUNT 3
static const char* errlist[ERR_COUNT] = {
  ERR_COMMAND_LINE,
  ERR_SELECT,
  ERR_MESSAGE_EMPTY
};

#define ERR_GRPC_COUNT 16
static const char* errGRPClist[ERR_GRPC_COUNT] = {
  ERR_GRPC_CANCELLED,
  ERR_GRPC_UNKNOWN,
  ERR_GRPC_INVALID_ARGUMENT,
  ERR_GRPC_DEADLINE_EXCEEDED,
  ERR_GRPC_NOT_FOUND,
  ERR_GRPC_ALREADY_EXISTS,
  ERR_GRPC_PERMISSION_DENIED,
  ERR_GRPC_RESOURCE_EXHAUSTED,
  ERR_GRPC_FAILED_PRECONDITION,
  ERR_GRPC_ABORTED,
  ERR_GRPC_OUT_OF_RANGE,
  ERR_GRPC_UNIMPLEMENTED,
  ERR_GRPC_INTERNAL,
  ERR_GRPC_UNAVAILABLE,
  ERR_GRPC_DATA_LOSS,
  ERR_GRPC_UNAUTHENTICATED
};

const char *strerror_sms(
  int errcode
)
{
  if ((errcode <= -500) && (errcode >= -500 - ERR_COUNT)) {
    return errlist[-(errcode + 500)];
  }
  if ((errcode <= ERR_GRPC_COUNT) && (errcode >= 0)) {
        return errGRPClist[errcode];
  }
    return strerror(errcode);
}
