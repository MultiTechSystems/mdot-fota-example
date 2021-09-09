/**********************************************************************
* COPYRIGHT 2018 MULTI-TECH SYSTEMS, INC.
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

#ifndef FOTA_H
#define FOTA_H
#include "mDot.h"
#include "mbed.h"

#include "FragmentationContext.h"
#include "LoraAppPackage.h"

namespace lora {
namespace app {

namespace packages {
class FragmentedDataBlockTransport;
}

static const uint8_t FOTA_MAX_FRAG_SESSIONS = 4; // Defined by LoRaWAN Fragmented Data Block Transport Specification

class Fota : private mbed::NonCopyable<Fota> {

public:
    Fota();
    ~Fota();

    /**
     * Set the class IDs used to validate received upgrades.
     */
    void setClassIds(const uint8_t (*ids)[CLASS_ID_SIZE], uint8_t cnt);

    /**
     * Set the vendor ID used to validate received upgrades.
     */
    void setVendorId(const uint8_t id[VENDOR_ID_SIZE]);

    void setAuthenticator(SuitManifest::Authenticator* authenticator);

    void setValidator(SuitManifest::Validator* validator);

    /**
     * Enable FOTA
     */
    void enable();
    
    /**
     * Indicates if FOTA is enabled.
     */
    bool isEnabled() const;
    
    /**
     * Disable FOTA, active session is stopped and received data is discarded.
     */
    void disable();

    /**
     * Reset FOTA, stop the active session and discard received data.
     */
    void reset();

    /**
     * True when there is no active session or the sesion has completed setup and is waiting for multicast to begin.
     */
    bool idle() const;

    /**
     * True when sesion has completed setup.
     */
    bool ready() const;

    /**
     * True when sesion is actively receiving fragments.
     */
    bool active() const;

    /**
     * Number of seconds until the multicast session begins.
     */
    int32_t timeToStart() const;

    /**
     * Get bitmask for fragmentation session indicies that are firmware upgrades.
     */
    uint8_t getUpgradeIndexMask() const;

    /**
     * Set a bitmask for fragmentation session indices that are firmware upgrades.
     * 
     * @param   mask  Bits 0-3 indicate which sessions are handled as upgrades, bits 3-7 are discarded.
     * @return        ERR_OK
     */
    int32_t setUpgradeIndexMask(uint8_t mask);

    /**
     * Get the session context for the given index.
     *
     * @param   index   Session index
     * @return          Pointer to current session context or NULL if index does not match the current session
     */
    FragmentationContext* getSessionContext(uint8_t index);

    /**
     * Determine if a session index is suppoprted.  Supported indices are within the valid range (0-3).  If a
     * session is active the index must also match the active session.
     *
     * @param   index    Session index to validate
     * @return           True if the index is supported.
     */
    bool isSessionIndexSupported(uint8_t index) const;

    /**
     * Create a session from provided context.
     *
     * Copies properties of the provided context to a local instance.
     *
     * @param   context   FragmentationContext to create a session from
     * @return            ERR_OK on success, ERR_INVALID_INDEX if the session index is unsupported,
     *                    ERR_OUT_OF_MEMORY if file creation fails or cannot allocate buffers
     */
    int32_t createSession(FragmentationContext& context);

    /**
     * Delete a session with the given index.
     *
     * @param   index   Fragmentation session index to delete
     *
     */
    int32_t deleteSession(uint8_t index);

    /**
     * Save current session to non-volatile memory. FOTA sessions cannot be
     * restored once a file transfer has begun.
     *
     * @return True if a session was saved.
     */
    bool saveSession();

    /**
     * Restore a saved session.
     *
     * @return True if a session was found and restored.
     */
    bool restoreSession();

    /**
     * Get information about the received file.
     */
    const FragmentedFileInfo& getFileInfo();


    /**
     * Server confirmed ACK reception.
     */
    void recvAckConfirmed();

protected:
    friend packages::FragmentedDataBlockTransport;
    FragmentationContext* fragmentReceived(uint8_t index, uint16_t n, uint8_t* fragment);

private:

    void resetFileInfo();

    FragmentationContext _context;

    bool _enabled;
    uint8_t _upgradeBitmask;
};

} } // namespace lora::app

#endif
