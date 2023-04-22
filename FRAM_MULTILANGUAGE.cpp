//
//    FILE: FRAM_MULTILANGUAGE.cpp
//  AUTHOR: Rob Tillaart
//    DATE: 2023-04-22
// PURPOSE: Arduino library for I2C FRAM based multi language table
//     URL: https://github.com/RobTillaart/FRAM_I2C
//


#include "FRAM_MULTILANGUAGE.h"   //   https://github.com/RobTillaart/FRAM_I2C


//  CONFIG
#define FRAM_ML_MAX_LANGUAGES          5
#define FRAM_ML_MAX_COUNTRY_CODE       3
#define FRAM_ML_METADATA_SIZE          40


FRAM_ML::FRAM_ML()
{
}


uint32_t FRAM_ML::begin(FRAM *fram, uint32_t memAddr, uint8_t languages, uint8_t strings, uint8_t maxLength)
{
  _fram      = fram;
  _baseAddr  = memAddr;
  _languages = languages;
  if (_languages > FRAM_ML_MAX_LANGUAGES)
  {
    _languages = FRAM_ML_MAX_LANGUAGES;
  }
  _strings   = strings;
  _maxLength = maxLength;

  //  write configuration to FRAM.
  _fram->write8(_baseAddr + 0, _version);       //  room for appl name?
  _fram->write8(_baseAddr + 20, _languages);
  _fram->write8(_baseAddr + 21, _strings);
  _fram->write8(_baseAddr + 22, _maxLength);

  setLanguage(0);

  return _baseAddr + _languages * _strings * _maxLength + FRAM_ML_METADATA_SIZE;
}


uint32_t FRAM_ML::begin(FRAM *fram, uint32_t memAddr)
{
  _fram      = fram;
  _baseAddr  = memAddr;

  _languages = _fram->read8(_baseAddr + 20);
  _strings   = _fram->read8(_baseAddr + 21);
  _maxLength = _fram->read8(_baseAddr + 22);

  setLanguage(0);

  return _baseAddr + _languages * _strings * _maxLength + FRAM_ML_METADATA_SIZE;
}


int FRAM_ML::getMaxLanguage()
{
  return _languages;
}


int FRAM_ML::getMaxStrings()
{
  return _strings;
}


int FRAM_ML::getMaxLength()
{
  return _maxLength;
}


/////////////////////////////////////////////////////////////////////
//
//
//
int FRAM_ML::setLanguageName(uint8_t index, const char * str)
{
  if (index > FRAM_ML_MAX_LANGUAGES) return FRAM_ML_INDEX_OUT_OF_RANGE;
  uint8_t len = strlen(str);
  if (len > FRAM_ML_MAX_COUNTRY_CODE) return FRAM_ML_TEXT_TOO_LONG;
  _fram->write(_baseAddr + index * (FRAM_ML_MAX_COUNTRY_CODE + 1), (uint8_t*) str, len);
  //  add separator.
  _fram->write8(_baseAddr + index * (FRAM_ML_MAX_COUNTRY_CODE + 1) + len, '\n');
  return FRAM_ML_OK;
}


int FRAM_ML::getLanguageName(uint8_t index, char * str)
{
  if (index > FRAM_ML_MAX_LANGUAGES) return FRAM_ML_INDEX_OUT_OF_RANGE;
  _fram->readUntil(_baseAddr + index * (FRAM_ML_MAX_COUNTRY_CODE + 1), str, (FRAM_ML_MAX_COUNTRY_CODE + 1), '\n');
  return FRAM_ML_OK;
}


/////////////////////////////////////////////////////////////////////
//
//
//
int FRAM_ML::setLanguage(uint8_t index)
{
  if (index >= FRAM_ML_MAX_LANGUAGES) return FRAM_ML_INDEX_OUT_OF_RANGE;
  if (index > _languages) return FRAM_ML_INDEX_OUT_OF_RANGE;
  _currentLanguage = index;
  //  set language address too.
  _langAddr = _baseAddr + FRAM_ML_METADATA_SIZE + _currentLanguage * _strings * _maxLength;
  return FRAM_ML_OK;
}


int FRAM_ML::getLanguage()
{
  return _currentLanguage;
}


/////////////////////////////////////////////////////////////////////
//
//  TEXT TABLES
//
int FRAM_ML::setText(uint8_t index, const char * text)
{
  if (index > _strings) return FRAM_ML_INDEX_OUT_OF_RANGE;
  uint8_t len = strlen(text);
  if (len > _maxLength) return FRAM_ML_TEXT_TOO_LONG;

  _fram->write(_langAddr + index * _maxLength, (uint8_t*) text, len);
  //  add separator.
  _fram->write8(_langAddr + index * _maxLength + len, '\n');
  return FRAM_ML_OK;
}


int FRAM_ML::getText(uint8_t index, char * text)
{
  if (index > _strings) return FRAM_ML_INDEX_OUT_OF_RANGE;
  _fram->readUntil(_langAddr + index * _maxLength, text, _maxLength, '\n');
  return FRAM_ML_OK;
}


//  -- END OF FILE --

