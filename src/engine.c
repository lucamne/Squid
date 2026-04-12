#include "engine.h"
#include "core.h"

#include <stdio.h>

// Converts promo PIECE to ascii char
static char promo_to_char(PIECE p) {
	switch(p) {
		case BR:
		case WR:
			return 'r';
		case BN:
		case WN:
			return 'n';
		case BB:
		case WB:
			return 'b';
		case BQ:
		case WQ:
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


// Search in a seperate thread until a stop command is received
static void search_infinite(void* unused) {
	Search_Result res = iterative_ab_search(0ULL);

	// send bestmove
	char info_buff[16];
	cpy_chars(info_buff, "bestmove ", 9);
	addr_to_str(res.sa, info_buff + 9);
	addr_to_str(res.ta, info_buff + 11);
	info_buff[13] = promo_to_char(res.promo);
	if (info_buff[13] == ' ') {
		info_buff[13] = '\n';
		info_buff[14] = '\0';
	} else {
		info_buff[14] = '\n';
		info_buff[15] = '\0';
	}
	platform_send_uci_command(info_buff);

	uci_search_active = 0;
	platform_end_thread();
}


// Return 1 if whitespace 0 if not
static int is_whitespace(char c) {
	return (c == ' ' || c == '\n' || c == '\f' || c == '\r' || c == '\t' || c == '\v');
}

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

// Return number of characters until whitespace
// Assume cmd is '\n' terminated
static int next_whitespace(const char* cmd) {
	ASSERT(cmd);
	for (int i = 0;; i++) {
		char c = cmd[i];
		if (is_whitespace(c))
			return i;
	}
}

// TODO remove when no longer needed
#include "testing.c"

static void uci_moves(const char* cmd) {
	ASSERT(next_token_eq(cmd, "moves"));
	while (1) {
		int ni = next_token_pos(cmd);
		if (ni == -1) return;
		cmd += ni;

		if (make_manual_move(cmd, next_whitespace(cmd)))
			return;
	}
}

// Process uci 'position' command
static void uci_position(const char* cmd) {
	ASSERT(next_token_eq(cmd, "position"));

	int ni = next_token_pos(cmd);
	if (ni == -1) {
		uci_pos_loaded = 0;
		return;
	}
	cmd += ni;

	// parse out startpos or fenstring
	if (next_token_eq(cmd, "startpos")) {
		if (load_position(STARTING_FEN)) {
			uci_pos_loaded = 0;
			return;
		} else {
			uci_pos_loaded = 1;
		}
	} else {
		// copy all 6 tokens of fen
		char fen[100];
		int fen_pos = 0;
		int curr_len = next_whitespace(cmd);

		for (int fen_tok = 0; fen_tok < 5; fen_tok++) {
			cpy_chars(fen + fen_pos, cmd, curr_len);
			fen[fen_pos + curr_len] = ' ';
			fen_pos += curr_len + 1;
			ni = next_token_pos(cmd);
			if (ni == -1) {
				uci_pos_loaded = 0;
				return;
			}
			cmd += ni;
			curr_len = next_whitespace(cmd);
		}

		cpy_chars(fen + fen_pos, cmd, curr_len);
		if (load_position(fen)) {
			uci_pos_loaded = 0;
			return;
		} else {
			uci_pos_loaded = 1;
		}
	}
	print_board();

	ni = next_token_pos(cmd);
	if (ni == -1) return;
	cmd += ni;
	if (!next_token_eq(cmd, "moves")) return;
	uci_moves(cmd);
}

////////////////////////////////////////////////////////////////////////////////
/// Engine API
/// 
/// Defined in engine.h
/// These functions are called by platform layer

void engine_send_uci_command(const char* cmd) {
	ASSERT(cmd);
	while (1) {

		if (!uci_cmd_received) {
			if (next_token_eq(cmd, "uci")) {
				uci_cmd_received = 1;
				platform_send_uci_command("id name RosieBot-1.0\nid author Luca Negris\n");
				platform_send_uci_command("uciok\n");
				return;
			}

		} else if (uci_pos_loaded && next_token_eq(cmd, "go")) {
			uci_search_active = 1;
			uci_halt_requested = 0;
			platform_create_thread(search_infinite, NULL);
			return;

		} else if (uci_search_active && next_token_eq(cmd, "stop")) {
			uci_halt_requested = 1;
			return;

		} else if (next_token_eq(cmd, "isready")) {
			platform_send_uci_command("readyok\n");
			return;

		} else if (!uci_search_active && next_token_eq(cmd, "ucinewgame")) {
			wipe_tt();
			return;

			// non standard testing command
		} else if (!uci_search_active && next_token_eq(cmd, "perftsuite")) {
			perft_test(5);
			return;

		} else if (!uci_search_active && next_token_eq(cmd, "position")) {
			uci_position(cmd);
			return;

			// non standard extended command
		} else if (!uci_search_active && uci_pos_loaded && next_token_eq(cmd, "moves")) {
			uci_moves(cmd);
			print_board();
			return;
		}

		int ni = next_token_pos(cmd);
		if (ni == -1)
			return;
		cmd += ni;
	}
}

void engine_init(void) {
	init();
}
