#pragma once
//
//    FILE: FRAM_RINGBUFFER.h
//  AUTHOR: Rob Tillaart
// VERSION: 0.1.0
//    DATE: 2022-10-03
// PURPOSE: Arduino library for I2C FRAM based ring buffer
//     URL: https://github.com/RobTillaart/FRAM_I2C
//


#include "FRAM.h"


class FRAM_RINGBUFFER
{
public:
  FRAM_RINGBUFFER()
  {
  }

  void begin(FRAM *fram, uint32_t size = 100)
  {
    _fram = fram;
    _size = size;
  }

  bool full()
  {
    return ((_front + 1) % _size) == _tail;
  }


  bool empty()
  {
    return _front == _tail;
  }


  int write(uint8_t value)
  {
    if (full()) return 0;
    _fram->write8(_front, value);
    _front++;
    if (_front >= _size) _front = 0;
    return 1;
  }


  int read()
  {
    if (empty()) return -1;
    int value = _fram->read8(_tail);
    _tail++;
    if (_tail >= _size) _tail = 0;
    return value;
  }


  int peek()
  {
    if (empty()) return -1;
    return _fram->read8(_tail);
  }


  uint32_t size()
  {
    return _size;
  }


  uint32_t count()
  {
    if (_front >= _tail) return _front - _tail;
    return _size + _front - _tail;
  }


private:
  uint32_t _size  = 0;
  uint32_t _start = 0;
  uint32_t _front = _start;
  uint32_t _tail  = _start;
  FRAM *   _fram;
};


// -- END OF FILE --

