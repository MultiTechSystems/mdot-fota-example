

#ifndef LORA_APP_PACKAGE_H_
#define LORA_APP_PACKAGE_H_

#include <vector>

#include "mDot.h"
#include "LoraAppMessageBuffer.h"
#include "LoraAppEvent.h"

#define DEFAULT_RETRY_DELAY   (5000)


// Defined by TS008 LoRa Alliance Assigned Value Registries
// LoRa Alliance assigned package identifiers
#define LA_PKID_MPACKACC      (0)
#define LA_PKID_CLKSYNC       (1)
#define LA_PKID_MCAST         (2)
#define LA_PKID_FRAG          (3)
#define LA_PKID_FWMNGT        (4)
#define LA_PKID_CERTIF        (5)

// LoRa Alliance assigned FPorts
#define LA_FPORT_MAC          (0)
#define LA_FPORT_CERTIF       (224)
#define LA_FPORT_MPACKACC     (225)
#define LA_FPORT_WMBUS        (227)
//

// LoRa App Package FPorts
#define LAP_FPORT_CLKSYNC       (202)
#define LAP_FPORT_MCAST         (200)
#define LAP_FPORT_FRAG          (201)
#define LAP_FPORT_FWMNGT        (203)


namespace lora {
namespace app {

struct PackageInfo {
   uint8_t port;
   uint8_t id;
   uint8_t version;
};

class Package
{
public:
   Package(const PackageInfo& info) : _info(info) { }
   virtual ~Package() { }

   virtual const PackageInfo& getPackageInfo() const { return _info; }

   virtual void handleMessage(MessageBuffer& recv, MessageBuffer& resp) = 0;

protected:
   PackageInfo _info;
};

} } // namespace lora::app

#endif // LORA_APP_PACKAGE_H_