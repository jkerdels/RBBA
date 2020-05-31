#ifndef TEXT_BUFFER_H
#define TEXT_BUFFER_H

#include <stdint.h>
#include <string.h>

class TextBuffer {

public:

  virtual char* get_buffer() = 0;

  virtual void put(const char *text) = 0;

};

template<uint8_t BufferSize>
class TextBufferImpl : public TextBuffer {

  char buffer[BufferSize+1];

public:

  TextBufferImpl() {
    memset(buffer,0,BufferSize+1);
  }

  char* get_buffer() { return buffer; }

  void put(const char *text) {
    strncpy(buffer,text,BufferSize);
  }

};

#endif
