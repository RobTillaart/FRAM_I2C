#pragma once
//
//    FILE: FRAM_RINGBUFFER.h
//  AUTHOR: Rob Tillaart
//    DATE: 2022-10-03
// PURPOSE: Arduino library for I2C FRAM based ring buffer
//     URL: https://github.com/RobTillaart/FRAM_I2C
//


#include "FRAM.h"


//  TODO ERROR CODES


class FRAM_RINGBUFFER
{
public:


  //////////////////////////////////////////////////////////////////
  //
  //  CONSTRUCTOR + BEGIN
  //
  FRAM_RINGBUFFER()
  {
  }

  //  fram = pointer to FRAM object
  //  size in bytes
  //  start in bytes
  void begin(FRAM *fram, uint32_t size, uint32_t start)
  {
    _fram  = fram;
    _size  = size;
    _start = start;
    flush();
  }


  //////////////////////////////////////////////////////////////////
  //
  //  ADMINISTRATIVE
  //
  void flush()
  {
    _front = _tail = _start;
    _count = 0;
  }


  uint32_t size()
  {
    return _size;
  }


  uint32_t count()
  {
    return _count;
  }
  

  bool full()
  {
    return _count == _size;
  }


  bool empty()
  {
    return _count == 0;
  }


  uint32_t free()
  {
    return _size - _count;
  }


  float freePercent()
  {
    return (100.0 * _count) / _size;
  }


  //////////////////////////////////////////////////////////////////
  //
  //  BYTE INTERFACE
  //
  //  returns bytes written.
  //  returns -1 indicates full buffer.
  int write(uint8_t value)
  {
    if (full()) return -1;
    _fram->write8(_front, value);
    _front++;
    _count++;
    if (_front >= _size) _front = _start;
    return 1;
  }


  //  returns value read
  //  returns -2 indicates empty buffer.
  int read()
  {
    if (empty()) return -2;
    int value = _fram->read8(_tail);
    _tail++;
    _count--;
    if (_tail >= _size) _tail = _start;
    return value;
  }


  //  returns value read
  //  returns -2 indicates empty buffer.
  int peek()
  {
    if (empty()) return -2;
    return _fram->read8(_tail);
  }


  //////////////////////////////////////////////////////////////////
  //
  //  OBJECT INTERFACE
  //
  //  returns bytes written.
  //  returns -21 indicates (almost) full buffer == object does not fit.
  int write(uint8_t * data, uint8_t objectSize)
  {
    if ((_size - _count) <  objectSize) return -21;
    uint8_t * p = data;
    for (uint8_t i = 0; i < objectSize; i++)
    {
      write(*p++);
    }
    return objectSize;
  }


  //  returns bytes read.
  //  returns -22 indicates (almost) empty buffer == Too few bytes to read object.
  int read(uint8_t * data, uint8_t objectSize)
  {
    if (_count <  objectSize) return -22;
    uint8_t * p = data;
    for (uint8_t i = 0; i < objectSize; i++)
    {
      *p++ = read();
    }
    return objectSize;
  }

  //  returns bytes read.
  //  returns -22 indicates (almost) empty buffer == Too few bytes to read object.
  int peek(uint8_t * data, uint8_t objectSize)
  {
    if (_count <  objectSize) return -22;
    uint32_t tmp = _tail;    //  remember _tail 'pointer'
    int n = read(data, objectSize);
    _tail = tmp;             //  restore _tail 'pointer'
    _count += n;
    return n;
  }


///////////////////////////////////////////////////
//
//  MAKE PERSISTENT OVER REBOOTS
//  - later
//
  void save()
  {
    //  write front + tail + start + size + count (where to)
  }


  void load()
  {
    //  read front + tail  (from where)
  }


private:
  uint32_t _count = 0;
  uint32_t _size  = 0;
  uint32_t _start = 0;
  uint32_t _front = _start;
  uint32_t _tail  = _start;
  FRAM *   _fram;
};


// -- END OF FILE --

