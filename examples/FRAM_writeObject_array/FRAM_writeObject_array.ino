//
//    FILE: FRAM_writeObject.ino
//  AUTHOR: Rob Tillaart
// PURPOSE: demo writing reading objects
//     URL: https://github.com/RobTillaart/FRAM_I2C
//
// experimental


#include "FRAM.h"

FRAM fram;

uint32_t start;
uint32_t stop;

uint32_t sizeInBytes = 0;

float magic[10];

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

  //  FILL AND WRITE 10 FLOATS
  uint16_t address = 100;
  for (int i = 0; i < 10; i++)
  {
    magic[i] = random(1000) / 3.14159265;
    Serial.println(magic[i], 5);
    address = fram.writeObject(address, magic[i]);
  }
  Serial.println();

  //  CLEAR THE ARRAY
  for (int i = 0; i < 10; i++)
  {
    magic[i] = 0;
    Serial.println(magic[i], 5);
  }
  Serial.println();

  //  READ BACK 10 FLOATS
  address = 100;
  for (int i = 0; i < 10; i++)
  {
    address = fram.readObject(address, magic[i]);
    Serial.println(magic[i], 5);
  }
  Serial.println();
  Serial.println("done...");
}


void loop()
{
}

// -- END OF FILE --
