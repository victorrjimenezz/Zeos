/*
 * libc.h - macros per fer els traps amb diferents arguments
 *          definició de les crides a sistema
 */
 
#ifndef __LIBC_H__
#define __LIBC_H__

#include <stats.h>

/// ********* SYSCALLS *********** ///

/// Write to the system console.
/// @param fd: File descriptor.
/// @param buffer: Buffer to write.
/// @param size: Size of the buffer.
int write(int fd, char *buffer, int size);

/// Returns an unsigned int containing the current amount of clock ticks.
unsigned int gettime();


/// ********* System functions *********** ///

/// Prints the error message of the last errno.
void perror();

/// Turns the integer given in a to a char array in b.
void itoa(int a, char *b);

/// Calculates length of a char array.
int strlen(char *a);

int getpid();

int fork();

void exit();

#endif  /* __LIBC_H__ */
