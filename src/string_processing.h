#ifndef STRING_PROCESSING_H
#define STRING_PROCESSING_H

#include "core.h"
/* 
 * Header only string processing module.
 *
 * Author: Luca
 * Date: 4/1y/2026
 */

// Converts promo PIECE to ascii char
static char promo_to_char(PIECE p) {
	switch(p) {
		case BROOK:
		case WROOK:
			return 'r';
		case BKNIGHT:
		case WKNIGHT:
			return 'n';
		case BBISHOP:
		case WBISHOP:
			return 'b';
		case BQUEEN:
		case WQUEEN:
			return 'q';
		default:
			return ' ';
	}
}

// Convert mailbox address to chess square notation and store in buffer
// Assume buffer is allocated and is at least two characters long
// Do not assume buffer is null terminated
// Assume addr is within range [0-63]
static void addr_to_str(int addr, char* buffer) {
	ASSERT(on_board(addr));
	ASSERT(buffer);

	char rank = (char)addr_to_rank(addr);
	char file = (char)addr_to_file(addr) - 1;
	file += 'a';

	buffer[0] = file;
	buffer[1] = rank + '0';
}

// Convert move to algebraic notation and store in buffer
// Assume buffer is allocated and is at least 5 characters long
// Do not assume buffer is null terminated
// Assume addr is within range [0-63]
static void mmove_to_str(MMove mv, char* buffer) {
	ASSERT(buffer);
	addr_to_str(mv.sa, buffer);
	addr_to_str(mv.ta, buffer + 2);
	buffer[4] = promo_to_char(mv.promo);
}


// Convert chess square notation to board address
// Assume buffer is not null
// Return -1 if buffer does not contain valid square
static int str_to_addr(const char* buffer) {
	ASSERT(buffer);

	char file = buffer[0];
	char rank = buffer[1];
	if (file < 'a' || file > 'h' || rank < '0' || rank > '8')
		return -1;

	int addr = 9 - (rank - '0' - 1);
	addr *= 10;
	addr += file - 'a' + 1;
	return addr;
}

// Convert a max of 'max' digits of i to chars
// and store in dest
// If i has more digits than max, leftmost digits
// are cut off
// Return characters written
static int int_to_str(char *dest, int i, int max) {
	ASSERT(dest);
	int num_digs = 0;
	int t = i;

	while (t != 0 && num_digs <= max) {
		num_digs++;
		t /= 10;
	}

	if (num_digs < max && i < 0) 
		num_digs++;

	for (t = 0; t < num_digs; t++) {
		if (i == 0)
			dest[num_digs - t -1] = '-';
		else
			dest[num_digs - t - 1] = '0' + (i % 10);
		i /= 10;
	}
	return num_digs;
}

// Copy count chars from src to dest
// Assume valid bounds
static void cpy_chars(char* dest, const char* src, int count) {
	ASSERT(dest && src);
	for (int i = 0; i < count; i++) {
		dest[i] = src[i];
	}
}

// Convert a positive string integer to an int
// Input len of string integere
// Return -1 if str is not a positive integer
static int str_to_positive_int(const char* str, int len) {
	ASSERT(str);
	int out = 0;
	for (int i = 0; i < len; i++) {
		if (str[i] < '0' || str[i] > '9')
			return -1;
		out *= 10;
		out += str[i] - '0';
	}
	return out;
}

// Return 1 if whitespace 0 if not
static int is_whitespace(char c) {
	return (c == ' ' || c == '\n' || c == '\f' || c == '\r' || c == '\t' || c == '\v');
}

// Return number of characters until whitespace or '\0'
static int next_whitespace(const char* cmd) {
	ASSERT(cmd);
	for (int i = 0;; i++) {
		char c = cmd[i];
		if (is_whitespace(c) || c == '\0')
			return i;
	}
}

///////////////////////////////////////////////////////////////////////////////
/// Token Processing
/// Used by engine.c to process uci commands

// Return 1 if next token in 'cmd' equals 'str'
// Return 0 if not equal
// Assume str is null terminated
// Assume cmd is terminated by some type of whitespace
static int next_token_eq(const char* cmd, const char* str) {
	ASSERT(cmd && str);
	for (int i = 0;; i++) {
		if (is_whitespace(cmd[i])) {
			if (str[i] == '\0')
				return 1;
			return 0;
		}
		if (cmd[i] != str[i])
			return 0;
	}
}

// Return the index of next token
// Assume cmd is '\n' terminated
// Return -1 if no further tokens
static int next_token_pos(const char* cmd) {
	ASSERT(cmd);
	int i = 0;
	int whitespace_found = 0;
	while (1) {
		char c = cmd[i];
		if (c == '\n')
			return -1;
		if (is_whitespace(c))
			whitespace_found = 1;
		else if (whitespace_found)
			return i;
		i++;
	}
}

#endif
