/* shell.h - declares the shell functions */

#ifndef SHELL_H
#define SHELL_H

#include "types.h"

#define MAX_INPUT_BUFFER_SIZE    128

int shell_loop();

int command_list(const uint8_t * params, uint32_t paramsize);

int command_cat(const uint8_t * params, uint32_t paramsize);

int command_rtc(const uint8_t * params, uint32_t paramsize);

#endif /* SHELL_H */
