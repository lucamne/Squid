#include "core.h"
#include "eval.h"
#include "string_processing.h"

////////////////////////////////////////////////////////////////////////////////
/// PSEUDO-RANDOM NUMBER GENERATOR
///
/// Used to generate zobrist keys

#define PRNG_SEED 13503364562591410000 
#define ROTATE(x,k) (((x)<<(k))|((x)>>(64-(k))))

struct {
	ULL a;
	ULL b;
	ULL c;
	ULL d;
} prng_state;

// Return ull random number
static ULL prng(void) {
	ULL e = prng_state.a - ROTATE(prng_state.b, 7);
	prng_state.a = prng_state.b ^ ROTATE(prng_state.c, 13);
	prng_state.b = prng_state.c + ROTATE(prng_state.d, 37);
	prng_state.c = prng_state.d + e;
	prng_state.d = e + prng_state.a;
	return prng_state.d;
}

// Init prng
static void prng_init(void) {
	prng_state.a = 0xf1ea5eed;
	prng_state.b = prng_state.c = prng_state.d = PRNG_SEED;
	for (int i = 0; i < 20; i++) {
		prng();
	}
}

////////////////////////////////////////////////////////////////////////////////
/// EXTERN FUNCTIONS
///
/// Exposed in core.h

int load_position(char* fen) {
	ASSERT(fen);

	for (int i = 0; i < 16; i++)
		material_counts[i] = 0;

	// first two ids already occupied by empty and offboard
	game_hash = 0ull;

	int pos = 0;
	int x = 0;
	int y = 0;
	// parse pieces
	while (1) {
		PIECE p = EMPTY;
		switch (fen[pos]) {
			case 'P':
				p = WPAWN;
				break;
			case 'N':
				p = WKNIGHT;
				break;
			case 'B':
				p = WBISHOP;
				break;
			case 'R':
				p = WROOK;
				break;
			case 'Q':
				p = WQUEEN;
				break;
			case 'K':
				p = WKING;
				break;
			case 'p':
				p = BPAWN;
				break;
			case 'n':
				p = BKNIGHT;
				break;
			case 'b':
				p = BBISHOP;
				break;
			case 'r':
				p = BROOK;
				break;
			case 'q':
				p = BQUEEN;
				break;
			case 'k':
				p = BKING;
				break;
			case '/':
				if (x != 8) return 1;
				x = 0;
				y++;
				break;
			default:
				if (fen[pos] < '1' || fen[pos] > '9')
					return 1;
				// handle numbers for blank squares
				p = EMPTY;
				int num_empty_sq = fen[pos] - '0';
				for (int i = 0; i < num_empty_sq; i++) {
					board[10 * (y + 2) + x + 1] = 0;
					x++;
				}
		}

		if (p != EMPTY) {
			int addr = 10 * (y + 2) + x + 1;
			if (!on_board(addr)) return 1;

			board[addr] = p;
			++material_counts[p];
			piece_addr[p][material_counts[p] - 1] = addr;
			hash_piece(p, addr);

			x++;
		}
		pos++;

		// stop parsing piece positions
		if (y >= 7 && x>= 8) {
			break;
		}
	}

	// side to move
	if (fen[pos] != ' ') return 1;
	pos++;
	if (fen[pos] == ' ') return 1;

	if (fen[pos] == 'w') side_to_move = WHITE;
	else side_to_move = BLACK;
	pos++;
	if (fen[pos] != ' ') return 1;

	// castling rights
	pos++;
	if (fen[pos] == ' ') return 1;

	castle_rights = 0u;
	while(fen[pos] != ' ') {
		switch (fen[pos]) {
			case 'K':
				castle_rights |= K_CASTLE;
				break;
			case 'Q':
				castle_rights |= Q_CASTLE;
				break;
			case 'k':
				castle_rights |= k_CASTLE;
				break;
			case 'q':
				castle_rights |= q_CASTLE;
				break;
			default:;
		}
		pos++;
	}
	hash_castle();

	// en passant
	pos++;
	if (fen[pos] == ' ') return 1;

	if (fen[pos] != '-') {
		ep_addr = str_to_addr(fen + pos);
		pos++;
	} else {
		ep_addr = -1;
	}
	hash_ep();
	pos++;
	if (fen[pos] != ' ') return 1;

	// halfmove clock
	pos++;
	if (fen[pos] == ' ') return 1;
	int ws = next_whitespace(fen + pos);
	halfmoves = str_to_positive_int(fen + pos, ws);
	if (halfmoves == -1) return 1;
	pos += ws;

	// move counter
	pos++;
	if (fen[pos] == ' ') return 1;
	ws = next_whitespace(fen + pos);
	moves = str_to_positive_int(fen + pos, ws);
	if (moves == -1) return 1;

	// init history
	history[0] = game_hash;
	history_cnt = 1;

	return 0;
}


void init(void) {
	// init offboard
	for (int i = 0; i < 120; i++) {
		if (i < 21 || i > 98 || i % 10 == 0 || i % 10 == 9) {
			board[i] = OFF_BOARD;
		}
	}

	prng_init();
	for (int i = 0; i < NUM_ZOBRIST_KEYS; i++) {
		zobrist_keys[i] = prng();
	}

	uci_cmd_received = 0;
	uci_pos_loaded = 0;
	uci_search_active = 0;
	uci_halt_requested = 0;
	
	init_eval();
}
