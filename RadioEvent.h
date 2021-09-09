#ifndef __RADIO_EVENT_H__
#define __RADIO_EVENT_H__

#include "dot_util.h"
#include "mDotEvent.h"
#include "LoraAppLayer.h"

class RadioEvent : public mDotEvent
{

public:
    RadioEvent() {}

    virtual ~RadioEvent() {}

    virtual void PacketRx(uint8_t port, uint8_t *payload, uint16_t size, int16_t rssi, int16_t snr, lora::DownlinkControl ctrl, uint8_t slot, uint8_t retries, uint32_t address, uint32_t fcnt, bool dupRx) {
        mDotEvent::PacketRx(port, payload, size, rssi, snr, ctrl, slot, retries, address, fcnt, dupRx);
        lora::app::ErrorCode err = lora::app::packetRx(payload, port, size, address);
        if ((err != lora::app::ERR_OK) && (err != lora::app::ERR_UNKNOWN_PORT)) {
            std::string msg;
            switch (err) {
                case lora::app::ERR_RX_OVERFLOW:
                    msg = "Overflow";
                    break;
                case lora::app::ERR_RX_REJECTED:
                    msg = "Rejected";
                    break;
                default:
                    msg = "Error";
                    break;
            }
            logWarning("LoRa App RX Error: %s", msg.c_str());
        } else if (err == lora::app::ERR_UNKNOWN_PORT) {
            // nothing
        } else {

            logInfo("LoRa App RX OK ");
        }
    }

    /*!
     * MAC layer event callback prototype.
     *
     * \param [IN] flags Bit field indicating the MAC events occurred
     * \param [IN] info  Details about MAC events occurred
     */
    virtual void MacEvent(LoRaMacEventFlags* flags, LoRaMacEventInfo* info) {

        if (mts::MTSLog::getLogLevel() == mts::MTSLog::TRACE_LEVEL) {
            std::string msg = "OK";
            switch (info->Status) {
                case LORAMAC_EVENT_INFO_STATUS_ERROR:
                    msg = "ERROR";
                    break;
                case LORAMAC_EVENT_INFO_STATUS_TX_TIMEOUT:
                    msg = "TX_TIMEOUT";
                    break;
                case LORAMAC_EVENT_INFO_STATUS_RX_TIMEOUT:
                    msg = "RX_TIMEOUT";
                    break;
                case LORAMAC_EVENT_INFO_STATUS_RX_ERROR:
                    msg = "RX_ERROR";
                    break;
                case LORAMAC_EVENT_INFO_STATUS_JOIN_FAIL:
                    msg = "JOIN_FAIL";
                    break;
                case LORAMAC_EVENT_INFO_STATUS_DOWNLINK_FAIL:
                    msg = "DOWNLINK_FAIL";
                    break;
                case LORAMAC_EVENT_INFO_STATUS_ADDRESS_FAIL:
                    msg = "ADDRESS_FAIL";
                    break;
                case LORAMAC_EVENT_INFO_STATUS_MIC_FAIL:
                    msg = "MIC_FAIL";
                    break;
                default:
                    break;
            }
            logTrace("Event: %s", msg.c_str());

            logTrace("Flags Tx: %d Rx: %d RxData: %d RxSlot: %d LinkCheck: %d JoinAccept: %d",
                     flags->Bits.Tx, flags->Bits.Rx, flags->Bits.RxData, flags->Bits.RxSlot, flags->Bits.LinkCheck, flags->Bits.JoinAccept);
            logTrace("Info: Status: %d ACK: %d Retries: %d TxDR: %d RxPort: %d RxSize: %d RSSI: %d SNR: %d Energy: %d Margin: %d Gateways: %d",
                     info->Status, info->TxAckReceived, info->TxNbRetries, info->TxDatarate, info->RxPort, info->RxBufferSize,
                     info->RxRssi, info->RxSnr, info->Energy, info->DemodMargin, info->NbGateways);
        }

        if (flags->Bits.Rx) {

            logInfo("Rx %d bytes", info->RxBufferSize);

            if (info->RxBufferSize > 0) {
#if ACTIVE_EXAMPLE != FOTA_EXAMPLE
                // print RX data as string and hexadecimal
                std::string rx((const char*)info->RxBuffer, info->RxBufferSize);
                printf("Rx data: %s [%s]\r\n", rx.c_str(), mts::Text::bin2hexString(info->RxBuffer, info->RxBufferSize).c_str());
#endif
            }
        }
    }


    virtual void ServerTime(uint32_t seconds, uint8_t sub_seconds) {
        mDotEvent::ServerTime(seconds, sub_seconds);
        lora::app::setClockOffset(seconds);
    }
};

#endif

