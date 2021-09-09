/**********************************************************************
* COPYRIGHT 2020 MULTI-TECH SYSTEMS, INC.
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

#ifndef _FRAGMENTATION_CONTEXT_H
#define _FRAGMENTATION_CONTEXT_H

#include "mbed.h"
#include "SuitManifest.h"


namespace lora {
namespace app {

static const uint8_t VERSION_SIZE = 4;
static const size_t DESCRIPTOR_SIZE = 4;

static const uint8_t VENDOR_ID_SIZE = 16;
static const uint8_t CLASS_ID_SIZE = 16;


class FragmentedFile;
class FragmentationMath;
class FragmentationSession;


enum FragmentationState {
    FRAG_STATE_NONE,
    FRAG_STATE_IDLE,
    FRAG_STATE_CREATING_SESSION,
    FRAG_STATE_AWAITING_SESSION,
    FRAG_STATE_RECEIVING,
    FRAG_STATE_VALIDATING,
    FRAG_STATE_SENDING_CONFIRMATION,
    FRAG_STATE_UPDATE_PENDING,
    FRAG_STATE_EXCEPTION
};

struct FragmentedFileInfo {
    uint8_t version[VERSION_SIZE];
    size_t size;
    bool upgrade : 1;
    bool received : 1;
    bool validated : 1;
    bool authenticated : 1;
    bool compatible : 1;
};

struct FragmentationContext {
    uint8_t index;
    uint16_t nFrags;
    uint8_t fragSize;
    uint8_t padding;
    uint8_t algo;
    uint8_t ackDelay;
    uint8_t groupMask;
    uint32_t mic;
    uint8_t descriptor[DESCRIPTOR_SIZE];
    uint16_t sessionCnt;
    int16_t missing;
    int16_t filled;
    struct {
        bool ackReception : 1;
        bool ackSent : 1;
        bool complete : 1;
        bool fragmentsReceived : 1;
        bool abort : 1;
        bool micError : 1;
        bool matrixMemoryError : 1;
        bool voltageLowError : 1;
    } flags;

    // Begin unsaved data
    FragmentationState state;
    FragmentedFile* file;
    FragmentationMath* math;
    FragmentationSession* session;
    FragmentedFileInfo fileInfo;

    const uint8_t (*classIds)[CLASS_ID_SIZE];
    uint8_t classIdsCnt;
    const uint8_t* vendorId;
    const SuitManifest::Validator* validator;
    const SuitManifest::Authenticator* authenticator;

    int getLostFrameCount() {
        return missing;
    }

    int getTotalMissingFrameCount() {
        return (filled < missing) ? (missing - filled) : 0;
    }

    int getTotalRcvdFrameCount() {
        return nFrags - missing + filled;
    }
};

} } // namespace lora::app

#endif // _FRAGMENTATION_CONTEXT_H