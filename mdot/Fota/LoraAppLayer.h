/**********************************************************************
* COPYRIGHT 2021 MULTI-TECH SYSTEMS, INC.
*
* ALL RIGHTS RESERVED BY AND FOR THE EXCLUSIVE BENEFIT OF
* MULTI-TECH SYSTEMS, INC.
*
* MULTI-TECH SYSTEMS, INC. - CONFIDENTIAL AND PROPRIETARY
* INFORMATION AND/OR TRADE SECRET.
*
* NOTICE: ALL CODE, PROGRAM, INFORMATION, SCRIPT, INSTRUCTION,
* DATA, AND COMMENT HEREIN IS AND SHALL REMAIN THE CONFIDENTIAL
* INFORMATION AND PROPERTY OF MULTI-TECH SYSTEMS, INC.
* USE AND DISCLOSURE THEREOF, EXCEPT AS STRICTLY AUTHORIZED IN A
* WRITTEN AGREEMENT SIGNED BY MULTI-TECH SYSTEMS, INC. IS PROHIBITED.
*
***********************************************************************/

#ifndef LORA_APP_LAYER_H_
#define LORA_APP_LAYER_H_

#include <cstdint>
#include <ctime>
#include "LoraAppEvent.h"
#include "LoraAppPackage.h"
#include "LoraAppMessageBuffer.h"

#include "Fota.h"


// Number of bytes allocated for the application thread stack
#ifndef LORA_APP_LAYER_STACK_SIZE
#define LORA_APP_LAYER_STACK_SIZE    2048
#endif

// Number of seconds before requesting another time from the server
#ifndef LORA_APP_LAYER_CLOCK_SYNC_PERIOD
#define LORA_APP_LAYER_CLOCK_SYNC_PERIOD    (10)     // TODO: Use RTC clock drift to determine this value, defaulted to 60 days
#endif

#ifndef LORA_APP_LAYER_CLOCK_RESYNC_ATTEMPTS
#define LORA_APP_LAYER_CLOCK_RESYNC_ATTEMPTS   (3)
#endif

#ifndef LORA_APP_LAYER_CLOCK_RESYNC_DELAY
#define LORA_APP_LAYER_CLOCK_RESYNC_DELAY       (30)
#endif


#define GPS_EPOCH 315986400U

namespace lora {
namespace app {

/** Type used for error codes. */
typedef int32_t ErrorCode;

/** Application layer errors */
enum {
    ERR_OK = 0,                 //! No error
    ERR_RX_OVERFLOW = -1,       //! Receive buffer was full  
    ERR_UNKNOWN_PORT = -2,      //! No package registered for the given port
    ERR_TX_OVERFLOW = -3,       //! Transmit buffer was full
    ERR_TX_BUSY = -4,           //! Transmission in progress
    ERR_RX_REJECTED = -5,       //! Received packet was rejected
    ERR_INVALID_INDEX = -6,
    ERR_OUT_OF_MEMORY = -7,
    ERR_NULL_PARAMETER = -8,
    ERR_PORT_REUSE = -9         //! Port was already assigned a package
};

/**
 * Initialize the LoRa Application Layer.  Creates the application thread.
 *
 * Call lora::app::begin to start the application thread.
 * 
 * Subsequent calls do nothing.
 *
 * @param stack_size            Bytes allocated for the application thread stack (optional)
 * @param clock_sync_period     Seconds between requesting time from server (optional)
 */
void init(size_t stack_size = LORA_APP_LAYER_STACK_SIZE, 
          uint8_t clock_sync_period = LORA_APP_LAYER_CLOCK_SYNC_PERIOD,
          uint8_t notification_queue_size = LORA_APP_LAYER_NOTIFY_QUEUE_SIZE);

/**
 * Start the application thread.
 * 
 * Call lora::app::::init first.
 */
void begin();


/**
 * Indicates there are no pending actions in the LoRa Application Layer.
 * 
 * Sleeping when not idle may cause events to be missed.
 * 
 * @return True when nothing is pending.
 */
bool idle();

/**
 * Add an application layer package.
 *
 * @param port      Port the package receives messages on.
 * @param package   Pointer to a package object.
 * @return ERR_PORT_REUSE if package uses a port that was already assigned
 */
ErrorCode addPackage(Package* package);

bool getPackageByPort(uint8_t fport, Package** package);

bool getPackageById(uint8_t pid, Package** package);

void getAllPackageInfo(std::vector<PackageInfo>& info);

/**
 * Receive a packet and queue it for processing on application thread.
 * 
 * @param payload   Received packet
 * @param port      Port the packet was received on
 * @param size      Size of payload in bytes
 * @param address   Network address the packet was received on
 * @return ERR_OK if 
 */
ErrorCode packetRx(uint8_t* payload, uint8_t port, uint8_t size, uint32_t address);

/**
 * Indicates if a packet is pending transmission.
 */
bool packetTxPending();

/**
 * Submit a packet for transmission by the application thread.
 *
 * Only one packet can be pending for transmission at a time.
 *
 * @param payload   Packet payload to transmit
 * @param port      Port the packet will be sent from
 * @param size      Size of payload in bytes
 * @param attempts  Number of attempts that will be made to send the packet
 * @param delay     Milliseconds to delay before transmitting
 * @return
 */
int32_t packetTx(uint8_t* payload, uint8_t port, uint8_t size, uint8_t attempts = 1, uint32_t delay = 0);

/**
 * Indicates if the clock has been synchronized with the server.
 */
bool getClockSynced();

/**
 * Sets the offset between RTC and server time.
 *
 * @param gps_time  Time received from the server.
 * @return
 */
int32_t setClockOffset(uint32_t gps_time);

/**
 * Adjust the RTC offset.
 *
 * @param diff      Number of seconds to adjust the clock.
 * @return
 */
int32_t adjustClockOffset(int32_t diff);

/**
 * Request a clock sync from the server.  Delay value will be used when replying to resync requests from a server.
 *
 * @note Set attempts to 0 to change the delay without sending an uplink.
 *
 * @param attempts  Number of times request will be attempted, default is LORA_APP_LAYER_CLOCK_RESYNC_ATTEMPTS
 * @param delay     Minimum number of seconds between attempts, default is LORA_APP_LAYER_CLOCK_RESYNC_DELAY
 */
void resyncClock(uint8_t attempts = LORA_APP_LAYER_CLOCK_RESYNC_ATTEMPTS, uint16_t delay = LORA_APP_LAYER_CLOCK_RESYNC_DELAY);

/**
 * Set the perodicity of clock sync requests.  Valid values are 1-15.
 * 
 * Periodicity = 128 * 2^period +/- rand(30)
 */
void setClockResyncPeriod(uint8_t period);

/**
 * Get time synchronized with the server, only valid when 
 * lora::app::::getClockSynced is true.
 *
 * @return
 */
time_t syncedTime();

/**
 * Attach a function to call whenever an event occurs.
 */
void attach(Callback<void(EventNotification)> func);

void notify(EventCode ec);

/** Update the event queue.  Call after any operation that could affect timers
 * such as sleeping. */
void fixEventQueue();

/**
 * Get a reference to the FOTA instance.
 */
Fota& fota();

/**
 * Close the active multicast session.  Device class will swtich back to original.
 */
void closeActiveMulticastSession();

/**
 * Reboot the device in a specified number of seconds.
 *
 * @params seconds  Number of seconds before reboot.
 */ 
void rebootSched(uint32_t seconds);

/**
 * Cancel a pending reboot.
 */ 
void rebootCancel();

/**
 * Indicates a reboot has been scheduled.
 */
bool rebootPending();

/**
 * Number of seconds until a scheduled reboot occurs.
 */
int32_t rebootSeconds();

/**
 * Set the firmware version.  This version information is used for FOTA firmware management protocol.
 */
void setFwVersion(const int8_t* ver);

/**
 * Get the firmware version.
 */
void getFwVersion(int8_t* ver);

/**
 * Set the hardware version.  This version information is used for FOTA firmware management protocol.
 */
void setHwVersion(const int8_t* ver);

/**
 * Get the firmware version.
 */
void getHwVersion(int8_t* ver);

} } // namespace lora::app

#endif // LORA_APP_LAYER_H_