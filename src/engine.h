#ifndef ENGINE_H
#define ENGINE_H
/* 
 * An interface between the hardware dependant and non hardware dependent
 * code. It is designed so the engine can be ported to an arbitrary system. The 
 * platform code should implement the Platform API and the program entry point which 
 * commands the engine via UCI commands.
 *
 * Author: Luca
 * Date: 4/10/2026
 */

#include <stdint.h>

#define ULL uint_fast64_t
#define RB_MUTEX void*
#define RB_HANDLE void*

////////////////////////////////////////////////////////////////////////////////
/// Platform API

int platform_create_thread(void (*func)(void *), void* arg);
// Create thread that starts executing at 'func' with arguments 'arg'
// Return -1 on failure and handle to thread on success

void platform_end_thread(void);
// End calling thread

RB_MUTEX platform_create_mutex(void);
// Return mutex handle

int platform_wait_mutex(RB_MUTEX handle);
// Wait for mutex to be available then lock it
// Return 1 on error

int platform_release_mutex(RB_MUTEX handle);
// Release mutex
// Return 1 on failure 0 on success

ULL platform_get_time_ms(void);
// Get time in ms with respect to a constant reference point

void platform_send_uci_command(const char* cmd);
// Send UCI command to platform
// Assume command is terminated by '\n'

////////////////////////////////////////////////////////////////////////////////
/// Engine API

void engine_init(void);
// Initialize engine
// Call once before sending any commands

void engine_send_uci_command(const char* cmd);
// Send UCI command to engine
// Assume command is terminated by '\n'
// Invalid commands fail silently

#endif
