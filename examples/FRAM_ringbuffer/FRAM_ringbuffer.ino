//
//    FILE: FRAM_ringbuffer.ino
//  AUTHOR: Rob Tillaart
// PURPOSE: demo FRAM_RINGBUFFER class.
//     URL: https://github.com/RobTillaart/FRAM_I2C

// experimental code


#include "FRAM.h"
#include "FRAM_RINGBUFFER.h"


FRAM fram;
uint32_t sizeInBytes = 0;


FRAM_RINGBUFFER fb;


///////////////////////////////////////////////////////////

void setup()
{
  Serial.begin(115200);
  Serial.println(__FILE__);
  Serial.print("FRAM_LIB_VERSION: ");
  Serial.println(FRAM_LIB_VERSION);

  Wire.begin();

  int rv = fram.begin(0x50);
  if (rv != 0)
  {
    Serial.print("INIT ERROR: ");
    Serial.println(rv);
  }

  // get size in bytes
  sizeInBytes = fram.getSize() * 1024;
  // clear FRAM
  for (uint32_t addr = 0; addr < sizeInBytes; addr++)
  {
    fram.write8(addr, 0x00);
  }

  //  initialize the ring buffer.
  fb.begin(&fram, 20);
  //  clear the ring buffer.
  fb.flush();

  //  dump initial state.
  Serial.print("SIZE:\t");
  Serial.println(fb.size());
  Serial.print("COUNT:\t");
  Serial.println(fb.count());
  Serial.print("FULL:\t");
  Serial.println(fb.full());
  Serial.print("EMPTY:\t");
  Serial.println(fb.empty());

  //  add some bytes.
  for (int i = 0; i < 10; i++)
  {
    fb.write('A' + i);   // write ABCDEFGHIJ
  }
  Serial.print("COUNT:\t");
  Serial.println(fb.count());
  Serial.print("PEEK:\t");
  Serial.println((char)fb.peek());

  //  read some bytes.
  for (int i = 0; i < 3; i++)
  {
    fb.read();
  }
  Serial.print("COUNT:\t");
  Serial.println(fb.count());
  Serial.print("PEEK:\t");
  Serial.println((char)fb.peek());

}


void loop()
{
}


//  -- END OF FILE --

