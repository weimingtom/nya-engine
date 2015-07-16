//https://code.google.com/p/nya-engine/

#pragma once

#include <string>

std::string utf8_from_utf16le(const void *data,unsigned int size);
std::string utf8_from_shiftjis(const void *data,unsigned int size);
