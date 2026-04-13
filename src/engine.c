#include "engine.h"
#include "core.h"

#include <stdio.h>
#include <stdlib.h>


// Search in a seperate thread 
// Input search time in ms or 0 for infinite search
// If searching infinitely wait for halt command
static void run_search(void* ms) {
	MMove bestmove = iterative_ab_search(*(ULL*)ms);
	free(ms);

	// send bestmove
	char info_buff[16];
	cpy_chars(info_buff, "bestmove ", 9);
	move_to_str(bestmove, info_buff + 9);
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

// Process uci 'moves' command
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

// Process uci 'go' command
static void uci_go(const char* cmd) {
	ASSERT(next_token_eq(cmd, "go"));
	uci_search_active = 1;
	uci_halt_requested = 0;
	// ms freed in run search
	ULL* ms = malloc(sizeof(ULL));
	*ms = 0ULL;

	int ni = next_token_pos(cmd);
	if (ni != -1) {
		cmd += ni;
		if (next_token_eq(cmd, "movetime")) {
			ni = next_token_pos(cmd);
			if (ni != -1) {
				cmd += ni;
				// extract 'movetime' number
				int ws = next_whitespace(cmd);
				for (int i = 0; i < ws; i++) {
					if (cmd[i] < '0' || cmd[i] > '9') {
						*ms = 0ULL;
						break;
					}
					*ms *= 10;
					*ms += cmd[i] - '0';
				}
			}
		}
	}
	platform_create_thread(run_search, (void*)ms);
}

////////////////////////////////////////////////////////////////////////////////
/// Engine API
/// 
/// Defined in engine.h
/// These functions are called by platform layer

void engine_send_uci_command(const char* cmd) {
	ASSERT(cmd);
	while (1) {
		if (next_token_eq(cmd, "quit")) {
			exit(0);
		}
		if (!uci_cmd_received) {
			if (next_token_eq(cmd, "uci")) {
				uci_cmd_received = 1;
				platform_send_uci_command("id name squid-alpha\nid author Luca Negris\n");
				platform_send_uci_command("uciok\n");
				return;
			}

		} else if (uci_pos_loaded && next_token_eq(cmd, "go")) {
			uci_go(cmd);
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
