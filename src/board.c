#include "core.h"

////////////////////////////////////////////////////////////////////////////////
/// Pseudo-Random Number Generator
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
/// load_position() Helpers

// Register new piece 
static void register_piece(PIECE p, int addr) {
	ASSERT(p >= WP && p <= BK);
	ASSERT(on_board(addr));
	ASSERT(next_id < 64);

	int id = next_id++;
	board[addr] = id;
	piece_addr[id] = addr;
	piece_type[id] = p;
	if (p <= OFF_BOARD)
		piece_color[id] = NC;
	else if (p >= WP && p <= WK)
		piece_color[id] = WHITE;
	else
		piece_color[id] = BLACK;
	add_id(id);

	hash_piece(p, addr);
}


// Extract an integer from a string given a string and 
// starting position of number withing string
// Assume number is terminated by a space or null terminating byte
static int extract_int_from_str(char* str, int pos) {
	ASSERT(str[pos] != ' ');

	int num_len = 0;
	while (str[pos] != ' ' && str[pos] != '\0') {
		num_len++;
		pos++;
	}
	pos -= num_len;

	int out = 0;
	for (;num_len > 0; num_len--) {
		int digit = str[pos] - '0';
		for (int i = 0; i < num_len - 1; i++) 
			digit *= 10;

		out += digit;
		pos++;
	}
	return out;
}

////////////////////////////////////////////////////////////////////////////////
/// Public Functions

int load_position(char* fen) {
	ASSERT(fen);

	for (int i = 0; i < 12; i++) {
		material_counts[i] = 0;
	}
	// first two ids already occupied by empty and offboard
	next_id = 2;
	game_hash = 0ull;

	int pos = 0;
	int x = 0;
	int y = 0;
	// parse pieces
	while (1) {
		PIECE p = EMPTY;
		switch (fen[pos]) {
			case 'P':
				p = WP;
				break;
			case 'N':
				p = WN;
				break;
			case 'B':
				p = WB;
				break;
			case 'R':
				p = WR;
				break;
			case 'Q':
				p = WQ;
				break;
			case 'K':
				p = WK;
				break;
			case 'p':
				p = BP;
				break;
			case 'n':
				p = BN;
				break;
			case 'b':
				p = BB;
				break;
			case 'r':
				p = BR;
				break;
			case 'q':
				p = BQ;
				break;
			case 'k':
				p = BK;
				break;
			case '/':
				if (x != 8) return 1;
				x = 0;
				y++;
				break;
			default:
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
			register_piece(p, addr);
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
	halfmoves = extract_int_from_str(fen, pos);

	while (fen[++pos] != ' ');	// jump to end of integer

	// move counter
	pos++;
	if (fen[pos] == ' ') return 1;
	moves = extract_int_from_str(fen, pos);

	// init history
	history[0] = game_hash;
	history_cnt = 1;

	return 0;
}

void wipe_tt(void) {
	for (int i = 0; i < TTABLE_SIZE; i++) {
		ttable[i].active = 0;
	}
}

void init(void) {
	// init offboard
	for (int i = 0; i < 120; i++) {
		if (i < 21 || i > 98 || i % 10 == 0 || i % 10 == 9) {
			board[i] = OB_ID;
		}
	}
	// first two ids are always Empty and Offboard
	piece_color[E_ID] = piece_color[OB_ID] = NC;
	piece_type[E_ID] = EMPTY;
	piece_type[OB_ID] = OFF_BOARD;
	piece_addr[E_ID] = piece_addr[OB_ID] = -1;

	prng_init();
	for (int i = 0; i < NUM_ZOBRIST_KEYS; i++) {
		zobrist_keys[i] = prng();
	}

	uci_cmd_received = 0;
	uci_pos_loaded = 0;
	uci_search_active = 0;
	uci_halt_requested = 0;

	nodes_searched = 0;
}
