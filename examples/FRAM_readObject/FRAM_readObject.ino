//
//    FILE: FRAM_readObject.ino
//  AUTHOR: Rob Tillaart
// PURPOSE: demo reading objects
//     URL: https://github.com/RobTillaart/FRAM_I2C
//
// experimental


#include "FRAM.h"

FRAM fram;

uint32_t start;
uint32_t stop;

uint32_t sizeInBytes = 0;


void setup()
{
  Serial.begin(115200);
  Serial.println(__FILE__);
  Serial.print("FRAM_LIB_VERSION: ");
  Serial.println(FRAM_LIB_VERSION);

  Serial.println("\n run writeObject Demo first!\n");

  Wire.begin();

  int rv = fram.begin(0x50);
  if (rv != 0)
  {
    Serial.print("INIT ERROR: ");
    Serial.println(rv);
  }

  //  get size in bytes
  sizeInBytes = fram.getSize() * 1024;
  //  clear FRAM
  for (uint32_t addr = 0; addr < sizeInBytes; addr++)
  {
    fram.write8(addr, 0x00);
  }

  test_float();
  test_struct();

}


void loop()
{
}


void test_float()
{
  float magic = 0;
  Serial.println(magic, 6);

  fram.readObject(100, magic);
  Serial.println(magic, 6);
}

struct point
{
  float x;
  float y;
  float z;
} Q;

void test_struct()
{
  fram.readObject(50, Q);
  Serial.println(Q.x);
  Serial.println(Q.y);
  Serial.println(Q.z);
}


// -- END OF FILE --
