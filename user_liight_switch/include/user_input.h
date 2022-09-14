#ifndef _USER_INPUT_H
#define _USER_INPUT_H

#include <stdint.h>
typedef struct
{
  uint8_t   buffer[256];
  uint16_t  read_index;
  uint16_t  write_index;
}small_buffer_t;
void rtt_input_init();

#endif