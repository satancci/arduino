#ifndef LIBMAIN_H
#define LIBMAIN_H

#include <stdint.h>
#include <stdbool.h>

void pinMode(unsigned int pino, const char* modo);
bool digitalRead(unsigned int pino);
void digitalWrite(unsigned int pino, bool estado);
void digitalWrite8(uint8_t valor, uint8_t pino_base);

#endif // LIBMAIN_H
