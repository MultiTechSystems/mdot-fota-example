
#ifndef LORA_APP_EVENT_H_
#define LORA_APP_EVENT_H_

#include <cstdint>
#include <ctime>


namespace lora {
namespace app {

/** Type used for event codes. */
typedef uint32_t EventCode;


enum {
    EVENT_NONE = 0,
    EVENT_SYSTEM_RESET,
    EVENT_CLOCK_SYNCHRONIZED,
    EVENT_MULTICAST,
    EVENT_MULTICAST_GROUP_JOINED,
    EVENT_MULTICAST_GROUP_LEFT,
    EVENT_MULTICAST_SESSION_SETUP,
    EVENT_MULTICAST_SESSION_STARTED,
    EVENT_MULTICAST_SESSION_CLOSED,
    EVENT_FOTA,
    EVENT_FOTA_SESSION_CREATED,
    EVENT_FOTA_SESSION_RESTORED,
    EVENT_FOTA_SESSION_STARTED,
    EVENT_FOTA_FILE_RECEIVED,
    EVENT_FOTA_FILE_VALID,
    EVENT_FOTA_FILE_INVALID,
    EVENT_FOTA_SESSION_ABORTED,
    EVENT_FOTA_SESSION_COMPLETE
};

struct EventNotification {
    time_t at;
    EventCode code;
};

} } // namespace lora::app

#endif /* LORA_APP_EVENT_H_ */