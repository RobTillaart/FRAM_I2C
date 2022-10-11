#pragma once
//
//    FILE: FRAM_RINGBUFFER.h
//  AUTHOR: Rob Tillaart
//    DATE: 2022-10-03
// PURPOSE: Arduino library for I2C FRAM based ring buffer
//     URL: https://github.com/RobTillaart/FRAM_I2C
//


#include "FRAM.h"   //   https://github.com/RobTillaart/FRAM_I2C


//  TODO ERROR CODES
#define FRAM_RB_OK                     0
#define FRAM_RB_ERR_BUF_FULL          -1
#define FRAM_RB_ERR_BUF_EMPTY         -2
#define FRAM_RB_ERR_BUF_NO_ROOM       -21    //  (almost) full
#define FRAM_RB_ERR_BUF_NO_DATA       -22    //  (almost) empty


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
  //  returns uint32_t == first free FRAM location.
  uint32_t begin(FRAM *fram, uint32_t size, uint32_t start)
  {
    _fram  = fram;
    _size  = size;
    _start = start + 24;    //  allocate 6 uint32_t for storage.
    flush();
    _saved = false;
    return _start + _size;  //  first free FRAM location.
  }


  //////////////////////////////////////////////////////////////////
  //
  //  ADMINISTRATIVE
  //
  void flush()
  {
    _front = _tail = _start;
    _count = 0;
    _saved = false;
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
  //  - FRAM_RB_ERR_BUF_FULL indicates full buffer.
  int write(uint8_t value)
  {
    if (full()) return FRAM_RB_ERR_BUF_FULL;
    _fram->write8(_front, value);
    _saved = false;
    _front++;
    _count++;
    if (_front >= _size) _front = _start;
    return 1;
  }


  //  returns value read
  //  - FRAM_RB_ERR_BUF_EMPTY indicates empty buffer.
  int read()
  {
    if (empty()) return FRAM_RB_ERR_BUF_EMPTY;
    int value = _fram->read8(_tail);
    _saved = false;
    _tail++;
    _count--;
    if (_tail >= _size) _tail = _start;
    return value;
  }


  //  returns value read
  //  - FRAM_RB_ERR_BUF_EMPTY indicates empty buffer.
  int peek()
  {
    if (empty()) return FRAM_RB_ERR_BUF_EMPTY;
    int value = _fram->read8(_tail);
    return value;
  }


  //////////////////////////////////////////////////////////////////
  //
  //  OBJECT INTERFACE
  //
  //  returns bytes written.
  //  - FRAM_RB_ERR_BUF_NO_ROOM indicates (almost) full buffer 
  //    ==>  object does not fit.
  template <class T> int write(T &obj)
  {
    uint8_t objectSize = sizeof(obj);
    if ((_size - _count) <  objectSize) return FRAM_RB_ERR_BUF_NO_ROOM;
    uint8_t * p = (uint8_t *)&obj;
    for (uint8_t i = 0; i < objectSize; i++)
    {
      write(*p++);
    }
    _saved = false;
    return objectSize;
  }


  //  returns bytes read.
  //  - FRAM_RB_ERR_BUF_NO_DATA indicates (almost) empty buffer 
  //    ==>  Too few bytes to read object.
  template <class T> int read(T &obj)
  {
    uint8_t objectSize = sizeof(obj);
    if (_count <  objectSize) return FRAM_RB_ERR_BUF_NO_DATA;
    uint8_t * p = (uint8_t *)&obj;
    for (uint8_t i = 0; i < objectSize; i++)
    {
      *p++ = read();
    }
    _saved = false;
    return objectSize;
  }

  //  returns bytes read.
  //  - FRAM_RB_ERR_BUF_NO_DATA indicates (almost) empty buffer 
  //    ==>  Too few bytes to read object.
  template <class T> int peek(T &obj)
  {
    uint8_t objectSize = sizeof(obj);
    if (_count <  objectSize) return FRAM_RB_ERR_BUF_NO_DATA;
    bool prevSaved = _saved;          //  remember saved state
    uint32_t previousTail = _tail;    //  remember _tail 'pointer'
    int n = read(obj);
    _tail = previousTail;             //  restore _tail 'pointer'
    _saved = prevSaved;               //  restore _saved
    _count += n;
    return n;
  }


///////////////////////////////////////////////////
//
//  MAKE RINGBUFFER PERSISTENT OVER REBOOTS
//

  bool isSaved()
  {
    return _saved;
  }


  //  store the internal variables + checksum.
  //  if you need constant persistency, 
  //  call save() after every read() write() flush()
  void save() 
  {
    uint32_t pos = _start - 16;
    if (not _saved)
    {
      uint32_t checksum = _size + _front + _tail;
      _fram->write32(pos +  0, _size );
      _fram->write32(pos +  4, _front);
      _fram->write32(pos +  8, _tail );
      _fram->write32(pos + 12, checksum);
      _saved = true;
    }
  }


  //  retrieve the internal variables + verify checksum.
  //  returns false if checksum fails ==> data inconsistent
  bool load()  
  {
    uint32_t pos = _start - 16;
    uint32_t checksum = 0;
    _size    = _fram->read32(pos +  0);
    _front   = _fram->read32(pos +  4);
    _tail    = _fram->read32(pos +  8);
    checksum = _fram->read32(pos + 12);
    //  restore count
    if (_front >= _tail) _count = _front - _tail;
    else                 _count = _front - _tail + _size;
    //  checksum test should be enough.
    //  optional these are possible
    //    (_start <= _front) && (_front < _start + _size);
    //    (_start <= _tail)  && (_tail < _start + _size);
    _saved = (checksum == _size + _front + _tail);
    return _saved;
  }


  //  remove all data from ringbuffer by overwriting the FRAM.
  void wipe()
  {
    uint32_t pos = _start - 16;       //  also overwrite metadata
    while (pos < _start + _size - 4)  //  prevent writing adjacent FRAM
    {
      _fram->write32(pos, 0xFFFFFFFF);
      pos += 4;
    }
    while (pos < _start + _size)      //  if _size not a multiple of 4.
    {
      _fram->write8(pos, 0xFF);
      pos++;
    }
  }


private:
  uint32_t _count = 0;        //  optimization == front - tail (+ size)
  uint32_t _size  = 0;
  uint32_t _start = 0;
  uint32_t _front = _start;
  uint32_t _tail  = _start;
  FRAM *   _fram;
  bool     _saved = false;
};


// -- END OF FILE --

