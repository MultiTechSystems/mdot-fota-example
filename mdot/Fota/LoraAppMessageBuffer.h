
#ifndef LORA_APP_MESSAGE_BUFFER_H_
#define LORA_APP_MESSAGE_BUFFER_H_

#include <vector>
#include <cstdint>

#include "mbed.h"

namespace lora {
namespace app {

class MessageBuffer : private mbed::NonCopyable<MessageBuffer>{
public:
   MessageBuffer();
   ~MessageBuffer();

   uint32_t address;
   uint8_t group;
   bool pending;
   bool is_request;
   uint8_t port;
   int16_t attempts;
   uint32_t delay;
   std::vector<uint8_t> payload;
   size_t rxi;   // payload read index
   size_t next_rxi;

   void reset();

   size_t bytesToRead() const;

   bool take(size_t size);

   bool next();

   uint8_t peek();

   bool readBytes(uint8_t* bytes, size_t size);

   bool readByte(uint8_t* byte);

   bool readUint(uint32_t* n, uint8_t size = sizeof(uint32_t));

   bool readInt(int32_t* n, uint8_t size = sizeof(int32_t));

   bool writeByte(uint8_t byte);

   bool writeBytes(const uint8_t* bytes, size_t size);

   bool writeUint(uint32_t n, uint8_t size = sizeof(uint32_t));

   bool writeUint(uint16_t n);

   bool writeInt(int32_t n, uint8_t size = sizeof(uint32_t));

   bool writeInt(int16_t n);
};


} } // namespace lora::app

#endif // LORA_APP_MESSAGE_BUFFER_H_