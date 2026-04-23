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
#include <stdint.h>

#define ASSERT assert
#define STARTING_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define ULL uint_fast64_t

///////////////////////////////////////////////////////////////////////////////
/// TYPES

// Piece Types
#define PIECE unsigned int
#define EMPTY 0b0000u
#define OFF_BOARD 0b0010u
#define WPAWN 0b0100u
#define BPAWN 0b0101u
#define WKNIGHT 0b0110u
#define BKNIGHT 0b0111u
#define WBISHOP 0b1000u
#define BBISHOP 0b1001u
#define WROOK 0b1010u
#define BROOK 0b1011u
#define WQUEEN 0b1100u
#define BQUEEN 0b1101u
#define WKING 0b1110u
#define BKING 0b1111u
#define CMASK 0b1u		// mask to extract color from piece

// Colors
#define COLOR unsigned int
#define WHITE 0u
#define BLACK 1u

// evaluation type of node
typedef enum {
	PV = 0,		// exact eval
	UB,		// upper bound
	LB		// lower bound
} NODE_TYPE;


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
	PIECE cap_piece;	// captured piece
	int halfmoves;
} Move;

///////////////////////////////////////////////////////////////////////////////
///  GLOBAL ENGINE MEMORY
///
/// These variables are accessed and modified globally

/* Represents a 10x12 board. The playable chess board is 8x8 and the extra
 * squares are used to calculate board edges.
 * The board is indexed first left to right then top to bottom.
 * ie: a8 index < a1 index < h1 index.
 * An excel sheet diagraming the board is stored in the project directory. 
 *
 * Each address contains a bitfield of the where the rightmost bit is color
 * (0 = white, 1 = black) and the next 3 bits determine piece type. 
 * Use macros to extract info. */
unsigned int board[120];

/* Addresses of each piece on the board
 * The first four arrays are EMPTY, UNUSED, OFFBOARD, UNUSED
 * so that macros can be used directly.
 * piece_addr does not get initiliazed because access is 
 * based on material counts.*/
int piece_addr[16][10];
int material_counts[16];	// count of each piece type
				// use 16 indices so PIECE macros
				// can be used directly to index array

COLOR side_to_move;		// 1 = black, 0 = white
int halfmoves;			// halfmoves since last capture or pawn advance
int moves;			// fullmove counter
int ep_addr;			// en passant address

unsigned char castle_rights;	// [0-15] all combinations of castling rights
#define K_CASTLE 0b0001		// use castle masks to set and extract castle rights
#define Q_CASTLE 0b0010
#define k_CASTLE 0b0100
#define q_CASTLE 0b1000


/*
 * keys for generating position hash
 * 1 key for each piece at each square 
 * {WP, BP, WN, BN, WB, BB, WR, 
 * BR, WQ, BQ, WK, BK} x 120
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

/* used to track uci state*/
int uci_cmd_received;			// has UCI 'uci' command been received
int uci_pos_loaded;			// is a position loaded
int uci_search_active;			// is search running
int uci_halt_requested;			// search halt is requested

///////////////////////////////////////////////////////////////////////////////
/// CORE API
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
// Expect fen to be null terminated
// Return 1 on failure 0 on success

void wipe_tt(void);
// Wipe transposition table

void init(void);
// Initialize engine
// Call once before sending any commands

/* search.c */

void iterative_ab_search(ULL search_time);
// Search for best move in current position
// Input search time in ms or 0 to search until halt signal

///////////////////////////////////////////////////////////////////////////////
/// STATIC ENGINE UTILITIES
///
/// Frequently accessed simple functions

// Return 1 if piece p is color c
// Return 0 if piece is EMPTY/OFFBOARD
static int check_color(PIECE p, COLOR c) {
	return p >= WPAWN && (CMASK & p) == c;
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
	ASSERT(p >= WPAWN && p <= BKING );
	ASSERT(on_board(addr));
	game_hash ^= zobrist_keys[120 * (p - WPAWN) + addr];
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

#endif
