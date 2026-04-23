#include "engine.h"
#include "core.h"
#include "string_processing.h"

#include <stdio.h>
#include <stdlib.h>


// Search in a seperate thread 
// Input search time in ms or 0 for infinite search
// If searching infinitely wait for halt command
static void run_search(void* ms) {
	iterative_ab_search(*(ULL*)ms);
	free(ms);
	uci_search_active = 0;
	platform_end_thread();
}

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
		fen[fen_pos + curr_len] = '\0';
		if (load_position(fen)) {
			uci_pos_loaded = 0;
			return;
		} else {
			uci_pos_loaded = 1;
		}
	}

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

	ULL wtime = 0ULL;
	ULL btime = 0ULL;
	ULL winc = 0ULL;
	ULL binc = 0ULL;
	ULL movetime = 0ULL;
	char infinite = 0;

	int ni = next_token_pos(cmd);
	while (ni != -1) {
		cmd += ni;
		if (next_token_eq(cmd, "movetime")) {
			ni = next_token_pos(cmd);
			if (ni == -1) break;
			cmd += ni;
			// extract 'movetime' number
			int ws = next_whitespace(cmd);
			int t = str_to_positive_int(cmd, ws);
			if (t == -1) break;
			else movetime = (ULL)t;

		} else if (next_token_eq(cmd, "wtime")) {
			ni = next_token_pos(cmd);
			if (ni == -1) break;
			cmd += ni;
			// extract 'wtime' number
			int ws = next_whitespace(cmd);
			int t = str_to_positive_int(cmd, ws);
			if (t == -1) break;
			else wtime = (ULL)t;

		} else if (next_token_eq(cmd, "btime")) {
			ni = next_token_pos(cmd);
			if (ni == -1) break;
			cmd += ni;
			// extract 'btime' number
			int ws = next_whitespace(cmd);
			int t = str_to_positive_int(cmd, ws);
			if (t == -1) break;
			else btime = (ULL)t;
		} else if (next_token_eq(cmd, "winc")) {
			ni = next_token_pos(cmd);
			if (ni == -1) break;
			cmd += ni;
			// extract 'winc' number
			int ws = next_whitespace(cmd);
			int t = str_to_positive_int(cmd, ws);
			if (t == -1) break;
			else winc = (ULL)t;
		} else if (next_token_eq(cmd, "binc")) {
			ni = next_token_pos(cmd);
			if (ni == -1) break;
			cmd += ni;
			// extract 'winc' number
			int ws = next_whitespace(cmd);
			int t = str_to_positive_int(cmd, ws);
			if (t == -1) break;
			else binc = (ULL)t;
		} else if (next_token_eq(cmd, "infinite")) {
			infinite = 1;
		}
		ni = next_token_pos(cmd);
	}

	if (infinite) {
		*ms = 0ULL;
	} else if (movetime > 0) {
		*ms = movetime;
	} else if (side_to_move == WHITE && wtime > 0) {
		*ms = wtime / 20 + winc / 20;
	} else if (side_to_move == BLACK && btime > 0) {
		*ms = btime / 20 + binc / 20;
	}

	platform_create_thread(run_search, (void*)ms);
}

////////////////////////////////////////////////////////////////////////////////
/// Engine API
/// 
/// Defined in engine.h
/// These functions are called by platform layer

// delete when done, used for perft test right now
#include "testing.c"
void engine_send_uci_command(const char* cmd) {
	ASSERT(cmd);
	while (1) {
		if (next_token_eq(cmd, "quit")) {
			exit(0);
		}
		if (!uci_cmd_received) {
			if (next_token_eq(cmd, "uci")) {
				uci_cmd_received = 1;
				platform_send_uci_command("id name squid-"ENGINE_VERSION"\nid author Luca Negris\n");
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
		} else if (!uci_search_active && next_token_eq(cmd, "testsuite")) {

			hash_check();
			perft_test(5);
			return;

		} else if (!uci_search_active && next_token_eq(cmd, "position")) {
			uci_position(cmd);
			return;

			// non standard extended command
		} else if (!uci_search_active && uci_pos_loaded && next_token_eq(cmd, "moves")) {
			uci_moves(cmd);
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
