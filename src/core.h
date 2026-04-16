#ifndef CORE_H
#define CORE_H
/* 
 * This is the core header of the engine. The global engine state is defined here.
 * This serves as a header for move_gen.c, board.c, and search.c. Also found here
 * are common types and static functions that are used throughout the source code.
 * The non static functions exposed in this header will typically be called from
 * engine.c to execute some UCI command.
 *
 * Author: Luca
 * Date: 4/10/2026
 */

#include <assert.h>

#define ASSERT assert
#define STARTING_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define THREEFOLD_BUG_FEN "1R6/4kp2/2p1r1p1/2b1B2p/5Pn1/P4K2/2P4P/3R4 w - - 6 29"
#define ULL unsigned long long

///////////////////////////////////////////////////////////////////////////////
/// Types

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

// Minimal info needed to describe a chess move
typedef struct {
	int sa;			// source address
	int ta;			// target address
	PIECE promo;		// promotion piece
} MMove;

// Info needed by the engine to execute a move
typedef struct {
	MMove mv;
	unsigned char castle_rights;
	ULL game_hash;
	int ep_addr;
	int cap_id;		// captured piece id
	int halfmoves;
} Move;

///////////////////////////////////////////////////////////////////////////////
/// Engine State
///
/// These variables are accessed and modified globally

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
ULL history[512];
int history_cnt;

// modify transposition table size to control memory usage
// table size must be power of 2 for fast modulo
// update TTMOD accordingly
#define TTABLE_SIZE 8388608
// bit mask to translate hash to address within TABLE_SIZE
// mask the lower part of hash with number of bits needed to represent
// TTABLE_SIZE
#define TTMOD 0b11111111111111111111111ull
TT_Entry ttable[TTABLE_SIZE];		// transposition table
					// should be wiped before new game

ULL nodes_searched;			// number of nodes searched in current search
					// should be reset at the start of search

/* used to track uci state*/
int uci_cmd_received;			// has UCI 'uci' command been received
int uci_pos_loaded;			// is a position loaded
int uci_search_active;			// is search running
int uci_halt_requested;			// search halt is requested

///////////////////////////////////////////////////////////////////////////////
/// Core API
///
/// Called by engine.c to execute UCI commands

/* move_gen.c */

ULL perft(int d);
// Return total nodes found to ply 'd' in current position

int is_square_attacked(int addr, COLOR c);
// Return 1 if given square addr is attacked by color c
// return 0 if not

int gen_moves(Move *ml);
// Generate legal moves in a position
// Input a sufficiently large move list
// Return number of moves found
//
// order of generated move is not always the same because
// adding and removing pieces changes the move gen order

void make_move(Move *move);
// make a move from mailbox address start_addr to end_addr
// verify that move does not leave king in check
//
// return 0 if move made and 1 if move failed due to leaving
// king in check
// assume addresses are valid

void unmake_move(Move *move);
// Revert board state to previous move using Move used to generate
// most recent move.
// Assume move is allocated and valid

int make_manual_move(const char* mv, int len);
// Make a move on internal board
// Input a move string and its length
// Return 1 if no move was made
// Return 0 if move was made
//
// Move string should be in long algebraic notation
// 	"e2e4", "e7e8q"...
// Move string will be validated so that any string
// can be passed in without danger

int init_move(Move *move, int sa, int ta);
// Init move from addr sa to addr ta.
// Return 1 iff move is legal and was initialized
// Return 0 iff move is illegal and not initialzed
//
// Move memory is handled by caller.
// Promotion is always set to EMPTY and must be manually changed by
// caller if necassary.

/* board.c */

int load_position(char* fen);
// Load position from FEN
// Return 1 on failure 0 on success

void wipe_tt(void);
// Wipe transposition table

void init(void);
// Initialize engine
// Call once before sending any commands

/* search.c */

MMove iterative_ab_search(ULL search_time);
// Search for best move in current position
// Input search time in ms or 0 to search until halt signal

///////////////////////////////////////////////////////////////////////////////
/// Engine Utilities

// return number of pieces of type p
static int p_count(PIECE p) {
	ASSERT(p >= WP && p <= BK);
	return material_counts[p - 2];
}

// return piece type at board address
static PIECE pt_at_addr(int addr) {
	return piece_type[board[addr]];
}

// return 1 iff addr is on playable board
static int on_board(int addr) {
	return addr > 20 && addr < 99 && addr % 10 != 0 && addr %10 != 9;
}

// Given mailbox address return real life rank [1-8] where
// rank 1 is on white's side of the board.
static int addr_to_rank(int addr) {
	ASSERT(on_board(addr));
	return 8 - (addr / 10 - 2);
}

// Given mailbox address return real life file [1-8] where
// file 1 is file 'a'.
static int addr_to_file(int addr) {
	return addr % 10;
}

// add or remove PIECE p at 120 based addr to hash
static void hash_piece(PIECE p, int addr) {
	ASSERT(p >= WP && p <= BK);
	ASSERT(on_board(addr));
	game_hash ^= zobrist_keys[120 * (p - 2) + addr];
}

static void hash_side_to_move(void) {
	game_hash ^= zobrist_keys[1440];
}

static void hash_castle(void) {
	game_hash ^= zobrist_keys[(int)(1441u + (unsigned int)castle_rights)];
}

static void hash_ep(void) {
	game_hash ^= zobrist_keys[1457 + (ep_addr % 10  - 1)];
}

// adds id to appropriate id list
// assume piece_type is already set
static void add_id(int id) {
	PIECE pt = piece_type[id];
	ASSERT(pt > OFF_BOARD && pt <= BK);
	ASSERT(material_counts[WK - 2] <= 1);
	ASSERT(material_counts[BK - 2] <= 1);

	int *pid_arr = piece_ids[pt - 2];
	int mc = ++material_counts[pt - 2];
	pid_arr[mc - 1] = id;
}

///////////////////////////////////////////////////////////////////////////////
/// String Processing

// Copy count chars from src to dest
// Assume valid bounds
static void cpy_chars(char* dest, const char* src, int count) {
	ASSERT(dest && src);
	for (int i = 0; i < count; i++) {
		dest[i] = src[i];
	}
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

// Convert move to algebraic notation and store in buffer
// Assume buffer is allocated and is at least two characters long
// Do not assume buffer is null terminated
// Assume addr is within range [0-63]
static void mmove_to_str(MMove mv, char* buffer) {
	ASSERT(buffer);
	addr_to_str(mv.sa, buffer);
	addr_to_str(mv.ta, buffer + 2);
	buffer[4] = promo_to_char(mv.promo);
}

#endif
