#include "platform.h"
#include "engine.h"
#include "prng.h"
#include <stdio.h>

///////////////////////////////////////////////////////////////////////////////
/// TYPES

// piece types
typedef enum {
	EMPTY = 0,
	OFF_BOARD,
	WP,
	WN,
	WB,
	WR,
	WQ,
	WK,
	BP,
	BN,
	BB,
	BR,
	BQ,
	BK
} PIECE;

typedef enum {
	NC = 0,			// No color
	WHITE,
	BLACK
} COLOR;

// evaluation type of node
typedef enum {
	PV = 0,		// exact eval
	UB,		// upper bound
	LB		// lower bound
} NODE_TYPE;

// transposition table entry
typedef struct {
	char active;	// is entry populated with real data
	ULL hash;	// node hash
	char depth;	// depth of node
	NODE_TYPE nt;
	int eval;	// node eval 
	char sa;	// starting address and target address of best move
	char ta;
	PIECE promo;	// promotion piece if any
} TT_Entry;

// direction of movement for a sliding piece
// in address offsets
typedef enum {
	U = -10,
	D = 10,
	L = -1,
	R = 1,
	UR = -9,
	UL = -11,
	DR = 11,
	DL = 9
} DIRECTION;

///////////////////////////////////////////////////////////////////////////////
/// ENGINE STATE GLOBALS

/* Represents a 10x12 board. The playable chess board is 8x8 and the extra
 * squares are used to calculate board edges.
 * The board is indexed first left to right then top to bottom.
 * ie: a8 index < a1 index < h1 index.
 * An excel sheet diagraming the board is stored in the project directory. 
 *
 * Each address contains an id to a piece. 
 * Use that id to find out info about piece.*/
int board[120];

COLOR side_to_move;
int halfmoves;			// halfmoves since last capture or pawn advance
int moves;			// fullmove counter
int ep_addr;			// en passant address

unsigned char castle_rights;	// [0-15] all combinations of castling rights
#define K_CASTLE 0b0001		// use castle masks to set and extract castle rights
#define Q_CASTLE 0b0010
#define k_CASTLE 0b0100
#define q_CASTLE 0b1000

int next_id;			// next available piece id
int material_counts[12];	// count of each piece non empty/offboard piece

// piece attributes indexed by id
// indices 0 and 1 are EMPTY and OFF_BOARD
// all EMPTY and OFF_BOARD pieces share the same ids
COLOR piece_color[66];		// color of piece indexed by id
PIECE piece_type[66];		// type of piece indexed by id
int piece_addr[66];		// address of piece indexed by id
#define E_ID 0			// EMPTY ID
#define OB_ID 1			// OFF_BOARD ID

// ids of all playable pieces
// dimension 1 = piece type; dimension 2 = ids
// piece types index order {WP, WN, WB, WR, WQ, WK, BP, BN, BB, BR, BQ, BK}
int piece_ids[12][64];

/*
 * keys for generating position hash
 * 1 key for each piece at each square 
 * {WP, WN, WB, WR, WQ, WK, BP, BN, BB, BR, BQ, BK} x 120
 * 1 key for side to move is black (i = 1440)
 * 16 keys for all possible castling rights (i = 1441)
 * 1 key for each file of en passant square if any {8} (i = 1457) 
 *
 * Off board addresses are included in zobrist keys to avoid converting
 * from 120 based addresses to 64 based addresses.*/
#define NUM_ZOBRIST_KEYS 1465
ULL zobrist_keys[NUM_ZOBRIST_KEYS];
ULL game_hash;

// history of position hashes used for repetition checking
// index 1 is populated when position is loaded
ULL history[256];
int history_cnt;

// modify transposition table size to control memory usage
// table size must be power of 2 for fast modulo
// update TTMOD accordingly
#define TTABLE_SIZE 8388608
// bit mask to translate hash to address within TABLE_SIZE
// mask the lower part of hash with number of bits needed to represent
// TTABLE_SIZE
#define TTMOD 0b11111111111111111111111ull
static TT_Entry ttable[TTABLE_SIZE];	// transposition table
					// should be wiped before new game

////////////////////////////////////////////////////////////////////////////////
/// UTILITY FUNCTIONS
#include "util.c"

////////////////////////////////////////////////////////////////////////////////
// MOVE GENERATION
 #include "move_gen.c"

////////////////////////////////////////////////////////////////////////////////
// EVALUATION
// #include "eval.c"

////////////////////////////////////////////////////////////////////////////////
// SEARCH
// #include "search.c"

////////////////////////////////////////////////////////////////////////////////
// ENGINE CONTROL
void wipe_tt(void) {
	for (int i = 0; i < TTABLE_SIZE; i++) {
		ttable[i].active = 0;
	}
}

// one time initialization
void init_engine(void) {
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

	// init zobrist keys and hash
	prng_init();
	for (int i = 0; i < NUM_ZOBRIST_KEYS; i++) {
		zobrist_keys[i] = prng();
	}
}

// register piece with stack data structures
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

// convert chess square notation to board address
// assume buffer is not null
// return -1 if buffer does not contain valid square
static int str_to_addr(char* buffer) {
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

// extract an integer from a string given a string and 
// starting position of number withing string
// assume number is terminated by a space or null terminating byte
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

// setup engine state from FEN
int load_position(char* fen) {
	ASSERT(fen);
	// reset material counts
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
				// ASSERT(x == 8);
				if (x != 8) return 1;
				x = 0;
				y++;
				break;
				// handle numbers for blank squares
			default:
				// convert ascii num to int
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
	// ASSERT(fen[pos] == ' ');
	if (fen[pos] != ' ') return 1;
	pos++;
	// ASSERT(fen[pos] != ' ');
	if (fen[pos] == ' ') return 1;

	if (fen[pos] == 'w') side_to_move = WHITE;
	else side_to_move = BLACK;
	pos++;
	// ASSERT(fen[pos] == ' ');
	if (fen[pos] != ' ') return 1;

	// castling rights
	pos++;
	// ASSERT(fen[pos] != ' ');
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
	// ASSERT(fen[pos] != ' ');
	if (fen[pos] == ' ') return 1;

	if (fen[pos] != '-') {
		ep_addr = str_to_addr(fen + pos);
		pos++;
	} else {
		ep_addr = -1;
	}
	hash_ep();
	pos++;
	// ASSERT(fen[pos] == ' ');
	if (fen[pos] != ' ') return 1;

	// halfmove clock
	pos++;
	// ASSERT(fen[pos] != ' ');
	if (fen[pos] == ' ') return 1;
	halfmoves = extract_int_from_str(fen, pos);

	while (fen[++pos] != ' ');	// jump to end of integer

	// move counter
	pos++;
	// ASSERT(fen[pos] != ' ');
	if (fen[pos] == ' ') return 1;
	moves = extract_int_from_str(fen, pos);

	// init history
	history[0] = game_hash;
	history_cnt = 1;

	return 0;
}

// container for perft test info
typedef struct perft {
	char *fen;
	int depths;
	unsigned long long nodes_at_depth[10];
} Perft;

// Run perft of multiple positions to max depth
// NOTE: some positions do not have data up to max_depth
void perft_test(int max_depth) {
	// setup perft data
	Perft p1 = {STARTING_FEN, 10, {20u, 400u, 8902u, 197281u, 4865609u, 
		119060324u, 3195901860u, 84998978956u, 2439530234167u, 69352859712417u}};
	Perft p2 = {"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1",
		6, {48u, 2039u, 97862u, 4085603u, 193690690u, 8031647685u}};
	Perft p3 = {"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 8, {14u, 191u, 2812u, 
		43238u, 674624u, 11030083u, 178633661u, 3009794393u}};
	Perft p4 = {"r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
		6, {6u, 264u, 9467u, 422333u, 15833292u, 706045033u}};
	Perft p5 = {"rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 5,
		{44u, 1486u, 62379u, 2103487u, 89941194u}};
	Perft p6 = {"r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10",
		9, {46u, 2079u, 89890u, 3894594u, 164075551u, 6923051137u, 287188994746u, 
			11923589843526u, 490154852788714u}};
	int n_perft = 6;
	Perft positions[6] = {p1, p2, p3, p4, p5, p6};

	// performance metrics
	ULL total_secs = 0;
	ULL total_milli = 0;
	unsigned long long total_nodes = 0;

	printf("***********************\nPERFT\n***********************\n");
	for (int i = 0; i < n_perft; i++) {
		int depth;
		if (positions[i].depths < max_depth)
			depth = positions[i].depths;
		else
			depth = max_depth;

		printf("Position %i to depth %i\nFEN: %s\n\n", i + 1, depth, positions[i].fen);

		load_position(positions[i].fen);
		ULL st = get_time_ms();
		unsigned long long n_nodes = perft(depth);
		ULL et = get_time_ms();

		total_nodes += n_nodes;
		ULL secs_elapsed = (et - st) / 1000ull;
		ULL milli_elapsed = (et - st) % 1000ull;

		total_secs += secs_elapsed;
		total_milli += milli_elapsed;
		if (total_milli >= 1000) {
			total_secs += total_milli / 1000;
			total_milli %= 1000;
		}

		if (n_nodes != positions[i].nodes_at_depth[depth - 1]) {
			printf("PERFT FAILED!\nExpected nodes: %lli\nNodes found: %lli\n", 
					positions[i].nodes_at_depth[depth - 1], n_nodes);
			break;
		} else {
			printf("Nodes found: %lli\n", n_nodes);
			printf("Time elapsed: %llu.%llu\n", secs_elapsed, milli_elapsed);
			printf("-----------------------\n");
		}
	}

	// overflows are very possible here
	printf("***********************\nPERFT PASSED!\n");
	printf("\nTotal nodes found: %lli\n", total_nodes);
	printf("Total time elapsed: %llu.%llu\n", total_secs, total_milli);
	printf("Nodes/sec: ~%lli\n", 1000 * (total_nodes / (1000 * total_secs + total_milli)));
	printf("***********************\n\n");
}
