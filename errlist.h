#define ERR_CODE_COMMAND_LINE		    -500
#define ERR_CODE_SELECT             	-501
#define ERR_CODE_MESSAGE_EMPTY			-502

#define SMS_OK	        					0

#define ERR_COMMAND_LINE        		"Wrong parameter(s)"
#define ERR_SELECT              		"select() failed"
#define ERR_MESSAGE_EMPTY				"Empty message"

#define MSG_INTERRUPTED 		    	"Interrupted "
#define MSG_DAEMON_STARTED      		"Start daemon "
#define MSG_DAEMON_STARTED_1    		". Check syslog."

// GRPC

#define ERR_CODE_GRPC_CANCELLED	        1
#define ERR_GRPC_CANCELLED	            "The operation was cancelled"
#define ERR_CODE_GRPC_UNKNOWN           2
#define ERR_GRPC_UNKNOWN                "Unknown error"
#define ERR_CODE_GRPC_INVALID_ARGUMENT  3
#define ERR_GRPC_INVALID_ARGUMENT       "The client specified an invalid argument"
#define ERR_CODE_GRPC_DEADLINE_EXCEEDED	4
#define ERR_GRPC_DEADLINE_EXCEEDED      "The deadline expired before the operation could complete"
#define ERR_CODE_GRPC_NOT_FOUND       	5
#define ERR_GRPC_NOT_FOUND              "Requested entity was not found"
#define ERR_CODE_GRPC_ALREADY_EXISTS	  6
#define ERR_GRPC_ALREADY_EXISTS         "The entity already exists"
#define ERR_CODE_GRPC_PERMISSION_DENIED	7
#define ERR_GRPC_PERMISSION_DENIED      "Does not have permission to execute the specified operation"
#define ERR_CODE_GRPC_RESOURCE_EXHAUSTED	8
#define ERR_GRPC_RESOURCE_EXHAUSTED     "Resource has been exhausted"
#define ERR_CODE_GRPC_FAILED_PRECONDITION	9
#define ERR_GRPC_FAILED_PRECONDITION    "The operation was rejected because the system is not in a state required for the operation's execution"
#define ERR_CODE_GRPC_ABORTED           10
#define ERR_GRPC_ABORTED                "The operation was aborted"
#define ERR_CODE_GRPC_OUT_OF_RANGE	    11
#define ERR_GRPC_OUT_OF_RANGE           "The operation was attempted past the valid range"
#define ERR_CODE_GRPC_UNIMPLEMENTED     12
#define ERR_GRPC_UNIMPLEMENTED          "The operation is not implemented or is not supported/enabled in this service."
#define ERR_CODE_GRPC_INTERNAL          13
#define ERR_GRPC_INTERNAL               "Internal errors"
#define ERR_CODE_GRPC_UNAVAILABLE       14
#define ERR_GRPC_UNAVAILABLE            "The service is currently unavailable"
#define ERR_CODE_GRPC_DATA_LOSS         15
#define ERR_GRPC_DATA_LOSS              "Unrecoverable data loss or corruption"
#define ERR_CODE_GRPC_UNAUTHENTICATED   16
#define ERR_GRPC_UNAUTHENTICATED        "The request does not have valid authentication credentials for the operation"

const char *strerror_sms(int errcode);
