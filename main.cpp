
#include "dot_util.h"
#include "RadioEvent.h"
#include "LoraAppLayer.h"

#ifdef CONFIG_LORA_NETWORK_ID
static uint8_t network_id[] = CONFIG_LORA_NETWORK_ID;
#endif

#ifdef CONFIG_LORA_NETWORK_KEY
static uint8_t network_key[] = CONFIG_LORA_NETWORK_KEY;
#endif


#ifndef CHANNEL_PLAN
#define CHANNEL_PLAN CP_US915
#endif


#if defined(TARGET_XDOT_L151CC)
#if CONFIG_EXT_FLASH_SPIF
#include "SPIFBlockDevice.h"
#endif

#if CONFIG_EXT_FLASH_DATAFLASH
#include "DataFlashBlockDevice.h"
#endif
#endif

#ifdef CONFIG_FOTA_VENDOR_ID
static const uint8_t VENDOR_ID[16] = CONFIG_FOTA_VENDOR_ID;
#endif

#ifdef CONFIG_FOTA_CLASS_ID
static const uint8_t CLASS_IDS[][16] = { CONFIG_FOTA_CLASS_ID };
#endif

#ifdef CONFIG_FOTA_SIG_PUB_KEY
static const uint8_t FOTA_SIG_PUB_KEY[] = CONFIG_FOTA_SIG_PUB_KEY;
#endif

mDot* dot = NULL;
lora::ChannelPlan* plan = NULL;

mbed::UnbufferedSerial debug_port(USBTX, USBRX, LOG_DEFAULT_BAUD_RATE);

FileHandle *mbed::mbed_override_console(int fd)
{
    return &debug_port;
}


#if defined(TARGET_XDOT_L151CC)
mbed::BlockDevice* ext_bd = NULL;

mbed::BlockDevice * mdot_override_external_block_device()
{
    if (ext_bd == NULL) {
#if CONFIG_EXT_FLASH_SPIF
        ext_bd = new SPIFBlockDevice();
        int ret = ext_bd->init();
#if CONFIG_EXT_FLASH_DATAFLASH
        if (ret < 0) {
            delete ext_bd;
            ext_bd = new DataFlashBlockDevice();
            ret = ext_bd->init();
            // Check for zero size because DataFlashBlockDevice doesn't
            // return an error if the chip is not present
            if ((ret < 0) || (ext_bd->size() == 0)) {
                delete ext_bd;
                ext_bd = NULL;
            }
        }
#endif
#elif CONFIG_EXT_FLASH_DATAFLASH
        ext_bd = new DataFlashBlockDevice();
        ret = ext_bd->init();
        // Check for zero size because DataFlashBlockDevice doesn't
        // return an error if the chip is not present
        if ((ret < 0) || (ext_bd->size() == 0)) {
            delete ext_bd;
            ext_bd = NULL;
        }
#endif



        if (ext_bd != NULL) {
            logInfo("External flash device detected, type: %s, size: 0x%08x",
                ext_bd->get_type(), (uint32_t)ext_bd->size());
        }
    }

    return ext_bd;
}
#endif


void lora_app_event(lora::app::EventNotification en)
{
    const char* desc = lora_app_event_code_to_str(en.code);
    logInfo("LoRa App Event : %d : %s", (uint32_t)en.at, desc);

    switch (en.code) {
        case lora::app::EVENT_CLOCK_SYNCHRONIZED:
        {
            time_t now = lora::app::syncedTime();
            logInfo("Time synced to %d", (uint32_t)now);
            break;
        }
        default:
            break;
    }
}


int main() {
    // Custom event handler for automatically displaying RX data
    RadioEvent events;

    debug_port.baud(115200);

    mts::MTSLog::setLogLevel(mts::MTSLog::TRACE_LEVEL);

    // Create channel plan
    plan = create_channel_plan();
    assert(plan);

    dot = mDot::getInstance(plan);
    assert(dot);

    logInfo("mbed-os library version: %d.%d.%d", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);

    // For test only.  Frag session count should not be reset normally.
    dot->setFragSessionCnt(0, 0);
    dot->setFragSessionCnt(1, 0);
    dot->setFragSessionCnt(2, 0);
    dot->setFragSessionCnt(3, 0);

    // Initialize LoRa Application layer and FOTA
    lora::app::init();
    lora::app::setFwVersion(APPLICATION_VERSION_CODE);
    {
        int8_t hwver[4];
        hwver[0] = dot->getHwVersion();
        lora::app::setHwVersion(hwver);
    }

    #ifdef CONFIG_FOTA_CLASS_ID
    lora::app::fota().setClassIds(CLASS_IDS, 1);
    #endif

    #ifdef CONFIG_FOTA_VENDOR_ID
    lora::app::fota().setVendorId(VENDOR_ID);
    #endif

#if defined(TARGET_MTS_MDOT_F411RE)
    lora::app::fota().setAuthenticator(new lora::app::SuitManifestAuthenticatorMbedTlsEcdsa(FOTA_SIG_PUB_KEY, sizeof(FOTA_SIG_PUB_KEY)));
    lora::app::fota().setValidator(new lora::app::SuitManifestValidatorMbedTlsSha256());
#endif

    lora::app::attach(&lora_app_event);

    lora::app::begin();

    // start from a well-known state
    logInfo("defaulting Dot configuration");
    dot->resetConfig();
    dot->resetNetworkSession();

    // make sure library logging is turned on
    dot->setLogLevel(mts::MTSLog::DEBUG_LEVEL);

    // attach the custom events handler
    dot->setEvents(&events);

    // update configuration if necessary
    if (dot->getJoinMode() != mDot::OTA) {
        logInfo("changing network join mode to OTA");
        if (dot->setJoinMode(mDot::OTA) != mDot::MDOT_OK) {
            logError("failed to set network join mode to OTA");
        }
    }

    {
        const uint8_t gak[] = CONFIG_LORA_GEN_APP_KEY;
        std::vector<uint8_t> gakv(gak, gak + KEY_LENGTH);
        dot->setGenAppKey(gakv);
    }

    // in OTA and AUTO_OTA join modes, the credentials can be passed to the library as a name and passphrase or an ID and KEY
    // only one method or the other should be used!
#if defined(CONFIG_LORA_NETWORK_NAME) && defined(CONFIG_LORA_NETWORK_PASSPHRASE)
    update_ota_config_name_phrase(CONFIG_LORA_NETWORK_NAME, CONFIG_LORA_NETWORK_PASSPHRASE, CONFIG_LORA_FSB, CONFIG_LORA_NETWORK_TYPE, CONFIG_LORA_ACK);
#elif defined(CONFIG_LORA_NETWORK_ID) && defined(CONFIG_LORA_NETWORK_KEY)
    update_ota_config_id_key(network_id, network_key, CONFIG_LORA_FSB, CONFIG_LORA_NETWORK_TYPE, CONFIG_LORA_ACK);
#else
#error Invalid configuration
#endif

    if (dot->setClass("A") != mDot::MDOT_OK) {
        logError("failed to set network mode to class A");
    }

    // enable or disable Adaptive Data Rate
    dot->setAdr(CONFIG_LORA_ADR);

    // Configure the join delay
    dot->setJoinDelay(CONFIG_LORA_JOIN_DELAY);

    // save changes to configuration
    logInfo("saving configuration");
    if (!dot->saveConfig()) {
        logError("failed to save configuration");
    }

    // display configuration
    display_config();

    uint16_t n = 1;
    std::vector<uint8_t> tx_data;
    LowPowerTimer send_timer;
    send_timer.start();

    if (!dot->getNetworkJoinStatus()) {
        join_network();
    }

    std::chrono::milliseconds send_interval = 30s;

    while (true) {
        if (send_timer.elapsed_time() > send_interval) {
            send_timer.reset();
            tx_data.push_back((n >> 8) & 0xFF);
            tx_data.push_back(n & 0xFF);
            n++;
            send_data(tx_data);
            tx_data.clear();
        }

        ThisThread::sleep_for(1s);

        if (lora::app::idle()) {
            send_interval = 30s;
            dot->sleep(10, mDot::RTC_ALARM, false);
        } else if (lora::app::fota().ready() && (lora::app::fota().timeToStart() > 0)) {
            logInfo("FOTA starting in %d seconds", lora::app::fota().timeToStart());
            while (!dot->getIsIdle()) {
                ThisThread::sleep_for(100ms);
            }
            dot->sleep(lora::app::fota().timeToStart(), mDot::RTC_ALARM, false);
        } else if (lora::app::fota().active()) {
            // Reduce uplinks during FOTA, dot cannot receive while transmitting
            // Too many lost packets will cause FOTA to fail
            send_interval = 300s;
        } else {
            send_interval = 30s;
        }
    }

    return 0;
}
