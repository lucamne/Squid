#include "eval.h"
#include "core.h"

////////////////////////////////////////////////////////////////////////////////
/// CONSTANTS

// Value of pawns, knights, bishops, rooks, queens, and kings during middle and endgame
const int MG_PIECE_VALS[6] = {82, 337, 365, 477, 1025, 0};
const int EG_PIECE_VALS[6] = {94, 281, 297, 512, 936, 0};

// The presence of each type of piece increments the game phase score
// The lower the score, the closer the game is to engame
const int GAME_PHASE_INC[12] = {0, 0, 1, 1, 1, 1, 2, 2, 4, 4, 0, 0};

const int TRADE_DOWN = 10;	// Bonus for trading pieces when winning
const int NPP = 200;		// No pawns penalty

const int BISHOP_PAIR = 50;	// Bonus for having bishop pair
const int KNIGHT_PAIR = -50;	// Penalty for having knight pair
const int ROOK_PAIR = -50;	// Penalty for having rook pair

const int MOB = 10;		// Mobility bonus
const int DI = 50;		// Doubled or isolated pawns penalty

// Value modifiers for each piece type on each square in middle and endgame
int mg_pawn_table[64] = {
	0,   0,   0,   0,   0,   0,  0,   0,
	98, 134,  61,  95,  68, 126, 34, -11,
	-6,   7,  26,  31,  65,  56, 25, -20,
	-14,  13,   6,  21,  23,  12, 17, -23,
	-27,  -2,  -5,  12,  17,   6, 10, -25,
	-26,  -4,  -4, -10,   3,   3, 33, -12,
	-35,  -1, -20, -23, -15,  24, 38, -22,
	0,   0,   0,   0,   0,   0,  0,   0,
};

int eg_pawn_table[64] = {
	0,   0,   0,   0,   0,   0,   0,   0,
	178, 173, 158, 134, 147, 132, 165, 187,
	94, 100,  85,  67,  56,  53,  82,  84,
	32,  24,  13,   5,  -2,   4,  17,  17,
	13,   9,  -3,  -7,  -7,  -8,   3,  -1,
	4,   7,  -6,   1,   0,  -5,  -1,  -8,
	13,   8,   8,  10,  13,   0,   2,  -7,
	0,   0,   0,   0,   0,   0,   0,   0,
};

int mg_knight_table[64] = {
	-167, -89, -34, -49,  61, -97, -15, -107,
	-73, -41,  72,  36,  23,  62,   7,  -17,
	-47,  60,  37,  65,  84, 129,  73,   44,
	-9,  17,  19,  53,  37,  69,  18,   22,
	-13,   4,  16,  13,  28,  19,  21,   -8,
	-23,  -9,  12,  10,  19,  17,  25,  -16,
	-29, -53, -12,  -3,  -1,  18, -14,  -19,
	-105, -21, -58, -33, -17, -28, -19,  -23,
};

int eg_knight_table[64] = {
	-58, -38, -13, -28, -31, -27, -63, -99,
	-25,  -8, -25,  -2,  -9, -25, -24, -52,
	-24, -20,  10,   9,  -1,  -9, -19, -41,
	-17,   3,  22,  22,  22,  11,   8, -18,
	-18,  -6,  16,  25,  16,  17,   4, -18,
	-23,  -3,  -1,  15,  10,  -3, -20, -22,
	-42, -20, -10,  -5,  -2, -20, -23, -44,
	-29, -51, -23, -15, -22, -18, -50, -64,
};

int mg_bishop_table[64] = {
	-29,   4, -82, -37, -25, -42,   7,  -8,
	-26,  16, -18, -13,  30,  59,  18, -47,
	-16,  37,  43,  40,  35,  50,  37,  -2,
	-4,   5,  19,  50,  37,  37,   7,  -2,
	-6,  13,  13,  26,  34,  12,  10,   4,
	0,  15,  15,  15,  14,  27,  18,  10,
	4,  15,  16,   0,   7,  21,  33,   1,
	-33,  -3, -14, -21, -13, -12, -39, -21,
};

int eg_bishop_table[64] = {
	-14, -21, -11,  -8, -7,  -9, -17, -24,
	-8,  -4,   7, -12, -3, -13,  -4, -14,
	2,  -8,   0,  -1, -2,   6,   0,   4,
	-3,   9,  12,   9, 14,  10,   3,   2,
	-6,   3,  13,  19,  7,  10,  -3,  -9,
	-12,  -3,   8,  10, 13,   3,  -7, -15,
	-14, -18,  -7,  -1,  4,  -9, -15, -27,
	-23,  -9, -23,  -5, -9, -16,  -5, -17,
};

int mg_rook_table[64] = {
	32,  42,  32,  51, 63,  9,  31,  43,
	27,  32,  58,  62, 80, 67,  26,  44,
	-5,  19,  26,  36, 17, 45,  61,  16,
	-24, -11,   7,  26, 24, 35,  -8, -20,
	-36, -26, -12,  -1,  9, -7,   6, -23,
	-45, -25, -16, -17,  3,  0,  -5, -33,
	-44, -16, -20,  -9, -1, 11,  -6, -71,
	-19, -13,   1,  17, 16,  7, -37, -26,
};

int eg_rook_table[64] = {
	13, 10, 18, 15, 12,  12,   8,   5,
	11, 13, 13, 11, -3,   3,   8,   3,
	7,  7,  7,  5,  4,  -3,  -5,  -3,
	4,  3, 13,  1,  2,   1,  -1,   2,
	3,  5,  8,  4, -5,  -6,  -8, -11,
	-4,  0, -5, -1, -7, -12,  -8, -16,
	-6, -6,  0,  2, -9,  -9, -11,  -3,
	-9,  2,  3, -1, -5, -13,   4, -20,
};

int mg_queen_table[64] = {
	-28,   0,  29,  12,  59,  44,  43,  45,
	-24, -39,  -5,   1, -16,  57,  28,  54,
	-13, -17,   7,   8,  29,  56,  47,  57,
	-27, -27, -16, -16,  -1,  17,  -2,   1,
	-9, -26,  -9, -10,  -2,  -4,   3,  -3,
	-14,   2, -11,  -2,  -5,   2,  14,   5,
	-35,  -8,  11,   2,   8,  15,  -3,   1,
	-1, -18,  -9,  10, -15, -25, -31, -50,
};

int eg_queen_table[64] = {
	-9,  22,  22,  27,  27,  19,  10,  20,
	-17,  20,  32,  41,  58,  25,  30,   0,
	-20,   6,   9,  49,  47,  35,  19,   9,
	3,  22,  24,  45,  57,  40,  57,  36,
	-18,  28,  19,  47,  31,  34,  39,  23,
	-16, -27,  15,   6,   9,  17,  10,   5,
	-22, -23, -30, -16, -16, -23, -36, -32,
	-33, -28, -22, -43,  -5, -32, -20, -41,
};

int mg_king_table[64] = {
	-65,  23,  16, -15, -56, -34,   2,  13,
	29,  -1, -20,  -7,  -8,  -4, -38, -29,
	-9,  24,   2, -16, -20,   6,  22, -22,
	-17, -20, -12, -27, -30, -25, -14, -36,
	-49,  -1, -27, -39, -46, -44, -33, -51,
	-14, -14, -22, -46, -44, -30, -15, -27,
	1,   7,  -8, -64, -43, -16,   9,   8,
	-15,  36,  12, -54,   8, -28,  24,  14,
};

int eg_king_table[64] = {
	-74, -35, -18, -18, -11,  15,   4, -17,
	-12,  17,  14,  17,  17,  38,  23,  11,
	10,  17,  23,  15,  20,  45,  44,  13,
	-8,  22,  24,  27,  26,  33,  26,   3,
	-18,  -4,  21,  24,  27,  23,   9, -11,
	-19,  -3,  11,  21,  23,  16,   7,  -9,
	-27, -11,   4,  13,  14,   4,  -5, -17,
	-53, -34, -21, -11, -28, -14, -24, -43
};

// Convert a 120 based address to a 64 based address
int s120_to_64[120] = {
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1,  0,  1,  2,  3,  4,  5,  6,  7, -1, 
	-1,  8,  9, 10, 11, 12, 13, 14, 15, -1, 
	-1, 16, 17, 18, 19, 20, 21, 22, 23, -1, 
	-1, 24, 25, 26, 27, 28, 29, 30, 31, -1, 
	-1, 32, 33, 34, 35, 36, 37, 38, 39, -1, 
	-1, 40, 41, 42, 43, 44, 45, 46, 47, -1, 
	-1, 48, 49, 50, 51, 52, 53, 54, 55, -1, 
	-1, 56, 57, 58, 59, 60, 61, 62, 63, -1, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
	-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 
};

////////////////////////////////////////////////////////////////////////////////
/// SHARED EVAL MEMORY

// Square eval for each piece
// Pieces are ordered WP, BP, WN, BN, WB, BB, WR, BR, WQ, BQ, WK, BK
int mg_piece_sq[12][64];
int eg_piece_sq[12][64];

int* mg_piece_tables[6] = {
	mg_pawn_table,
	mg_knight_table,
	mg_bishop_table,
	mg_rook_table,
	mg_queen_table,
	mg_king_table
};

int* eg_piece_tables[6] = {
	eg_pawn_table,
	eg_knight_table,
	eg_bishop_table,
	eg_rook_table,
	eg_queen_table,
	eg_king_table
};

////////////////////////////////////////////////////////////////////////////////
/// STATIC FUNCTIONS

// calculate the number of pseudo legal white pawn moves 
static int wp_mobility(void) {
	int n_moves = 0;
	PIECE p = WPAWN;
	int mc = material_counts[p];
	int* pa_arr = piece_addr[p];

	for (int i = 0; i < mc; i++) {
		int a = pa_arr[i];

		ASSERT(board[a] == WPAWN);
		ASSERT(addr_to_rank(a) <= 7);
		ASSERT(on_board(a - 10));

		// advance
		if (board[a - 10] == EMPTY) {
			n_moves++;
			if (addr_to_rank(a) == 7)
				n_moves += 3;
			if (addr_to_rank(a) == 2 && board[a - 20] == EMPTY) {
				n_moves++;
			}
		}
		// captures
		PIECE tp = board[a - 9];
		if (check_color(tp, BLACK) || ep_addr == a - 9) {
			n_moves++;
			if (addr_to_rank(a) == 7)
				n_moves += 3;
		}
		tp = board[a - 11];
		if (check_color(tp, BLACK) || ep_addr == a - 11) {
			n_moves++;
			if (addr_to_rank(a) == 7)
				n_moves += 3;
		}
	}
	return n_moves;
}

// calculate the number of pseudo legal black pawn moves 
static int bp_mobility(void) {
	int n_moves = 0;
	PIECE p = BPAWN;
	int mc = material_counts[p];
	int* pa_arr = piece_addr[p];

	for (int i = 0; i < mc; i++) {
		int a = pa_arr[i];

		ASSERT(board[a] == BPAWN);
		ASSERT(addr_to_rank(a) <= 7);
		ASSERT(on_board(a + 10));

		// advance
		if (board[a + 10] == EMPTY) {
			n_moves++;
			if (addr_to_rank(a) == 2)
				n_moves += 3;
			if (addr_to_rank(a) == 7 && board[a + 20] == EMPTY) {
				n_moves++;
			}
		}
		// captures
		PIECE tp = board[a + 9];
		if (check_color(tp, WHITE) || ep_addr == a + 9) {
			if (addr_to_rank(a) == 2)
				n_moves += 3;
			n_moves++;
		}
		tp = board[a + 11];
		if (check_color(tp, WHITE) || ep_addr == a + 11) {
			if (addr_to_rank(a) == 2)
				n_moves += 3;
			n_moves++;
		}
	}
	return n_moves;
}

// Return 1 if target address is a valid target for a piece of color c
// Return 0 if not
// DO NOT USE for pawns due to pawn capture rules
//
// Target is valid if it is empty or not color c
// TODO There is an identical function in move_gen.c
static int is_valid_target(int ta, COLOR c) {
	PIECE p = board[ta];
	return (p != OFF_BOARD) && (!check_color(p, c) || p == EMPTY);
}

// calculate number of pseudo legal knight moves of color c
static int knight_mobility(COLOR c) {
	int n_moves = 0;
	PIECE p = WKNIGHT | c;
	int mc = material_counts[p];
	int* pa_arr = piece_addr[p];

	for (int i = 0; i < mc; i++) {
		int a = pa_arr[i];
		ASSERT(board[a] == WKNIGHT || board[a] == BKNIGHT);

		if (is_valid_target(a - 21, c))
			n_moves++;
		if (is_valid_target(a - 19, c))
			n_moves++;
		if (is_valid_target(a - 12, c))
			n_moves++;
		if (is_valid_target(a - 8, c))
			n_moves++;
		if (is_valid_target(a + 21, c))
			n_moves++;
		if (is_valid_target(a + 19, c))
			n_moves++;
		if (is_valid_target(a + 12, c))
			n_moves++;
		if (is_valid_target(a + 8, c))
			n_moves++;
	}
	return n_moves;
}

// calculate number of pseudo legal king moves of COLOR c
static int king_mobility(COLOR c) {
	int n_moves = 0;
	PIECE p = WKING | c;
	int a = piece_addr[p][0];
	ASSERT(board[a] == WKING || board[a] == BKING);

	if (is_valid_target(a - 11, c))
		n_moves++;
	if (is_valid_target(a - 10, c))
		n_moves++;
	if (is_valid_target(a - 9, c))
		n_moves++;
	if (is_valid_target(a - 1, c))
		n_moves++;
	if (is_valid_target(a + 1, c))
		n_moves++;
	if (is_valid_target(a + 9, c))
		n_moves++;
	if (is_valid_target(a + 10, c))
		n_moves++;
	if (is_valid_target(a + 11, c))
		n_moves++;

	if (c == WHITE) {
		if (		castle_rights & K_CASTLE && 
				board[a + 1] == EMPTY && 
				board[a + 2] == EMPTY) {
			ASSERT(board[98] == WROOK);
			ASSERT(board[95] == WKING);
			n_moves++;
		}
		if (		castle_rights & Q_CASTLE && 
				board[a - 1] == EMPTY && 
				board[a - 2] == EMPTY && 
				board[a - 3] == EMPTY) {
			ASSERT(board[91] == WROOK);
			ASSERT(board[95] == WKING);
			n_moves++;
		}
	} else {
		if (		castle_rights & k_CASTLE && 
				board[a + 1] == EMPTY && 
				board[a + 2] == EMPTY) {
			ASSERT(board[28] == BROOK);
			ASSERT(board[25] == BKING);
			n_moves++;
		}
		if (		castle_rights & q_CASTLE && 
				board[a - 1] == EMPTY && 
				board[a - 2] == EMPTY && 
				board[a - 3] == EMPTY) {
			ASSERT(board[21] == BROOK);
			ASSERT(board[25] == BKING);
			n_moves++;
		}
	}
	return n_moves;
}


// calculate number of pseudo legal sliding moves for piece at address 1
// moving in direction d
// Aassume a bishop, rook, or queen is located at addr
static int slide_mobility(int a, DIRECTION d) {
	ASSERT(on_board(a));
	ASSERT(		board[a] == WQUEEN || 
			board[a] == BQUEEN || 
			board[a] == WBISHOP || 
			board[a] == BBISHOP || 
			board[a] == WROOK || 
			board[a] == BROOK);

	COLOR c = board[a] & CMASK;
	int n_moves = 0;
	int off = d;
	while (1) {
		PIECE p = board[a + off];
		if (p == OFF_BOARD) {
			break;
		} else if (p == EMPTY) {
			n_moves++;
			off += d;
		} else if (!check_color(p, c)){
			n_moves++;
			break;
		} else {
			break;
		}
	}
	return n_moves;
}

// calculate number of pseudo legal bishop moves of color c
static int bishop_mobility(COLOR c) {
	int n_moves = 0;
	PIECE p = WBISHOP | c;
	int mc = material_counts[p];
	int* pa_arr = piece_addr[p];

	for (int i = 0; i < mc; i++) {
		int a = pa_arr[i];
		ASSERT(board[a] == WBISHOP || board[a] == BBISHOP);

		n_moves += slide_mobility(a, UR);
		n_moves += slide_mobility(a, UL);
		n_moves += slide_mobility(a, DR);
		n_moves += slide_mobility(a, DL);
	}
	return n_moves;
}

// calculate number of pseudo legal rook moves for of COLOR c
static int rook_mobility(COLOR c) {
	int n_moves = 0;
	PIECE p = WROOK | c;
	int mc = material_counts[p];
	int* pa_arr = piece_addr[p];

	for (int i = 0; i < mc; i++) {
		int a = pa_arr[i];
		ASSERT(board[a] == WROOK || board[a] == BROOK);

		n_moves += slide_mobility(a, U);
		n_moves += slide_mobility(a, D);
		n_moves += slide_mobility(a, L);
		n_moves += slide_mobility(a, R);
	}
	return n_moves;
}

// calculate number of pseudo legal queen moves of COLOR c
static int queen_mobility(COLOR c) {
	int n_moves = 0;
	PIECE p = WQUEEN | c;
	int mc = material_counts[p];
	int* pa_arr = piece_addr[p];

	for (int i = 0; i < mc; i++) {
		int a = pa_arr[i];
		ASSERT(board[a] == WQUEEN || board[a] == BQUEEN);

		n_moves += slide_mobility(a, UR);
		n_moves += slide_mobility(a, UL);
		n_moves += slide_mobility(a, DR);
		n_moves += slide_mobility(a, DL);
		n_moves += slide_mobility(a, U);
		n_moves += slide_mobility(a, D);
		n_moves += slide_mobility(a, L);
		n_moves += slide_mobility(a, R);
	}
	return n_moves;
}

// calculate number of moves for white minus number of moves for black 
// in current position
static int mobility(void) {
	int wm = 0;	// white mobility
	int bm = 0;	// black mobility

	wm += wp_mobility();
	bm += bp_mobility();
	wm += knight_mobility(WHITE);
	bm += knight_mobility(BLACK);
	wm += bishop_mobility(WHITE);
	bm += bishop_mobility(BLACK);
	wm += rook_mobility(WHITE);
	bm += rook_mobility(BLACK);
	wm += queen_mobility(WHITE);
	bm += queen_mobility(BLACK);
	wm += king_mobility(WHITE);
	bm += king_mobility(BLACK);

	return wm - bm;
}

// Return number of isolated, doubled, and blocked pawns of COLOR c
// If pawns are doubled only one of them is counted as bad
static int bad_pawns(COLOR c) {
	ASSERT(c == WHITE || c == BLACK);
	int *pa_arr;
	int np;
	if (c == WHITE) {
		pa_arr = piece_addr[WPAWN];
		np = material_counts[WPAWN];
	} else {
		pa_arr = piece_addr[BPAWN];
		np = material_counts[BPAWN];
	}

	int bad_pawns = 0;

	char file_count[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	for (int i = 0; i < np; i++) {
		int a = pa_arr[i];
		file_count[addr_to_file(a) - 1]++;
	}

	for (int i = 0; i < 8; i++) {
		int cnt = file_count[i];
		if (cnt) {
			bad_pawns += cnt - 1;
			if (i == 0 && !file_count[1])
				bad_pawns += cnt;
			else if (i == 7 && !file_count[6])
				bad_pawns += cnt;
			else if (!file_count[i - 1] && !file_count[i + 1])
				bad_pawns += cnt;
		}
	}
	return bad_pawns;
}

// Return 1 if position is most likely a draw
// Return 0 if not
static int is_draw(void) {
	if (halfmoves == 50) return 1;

	if (is_repeat_position()) return 1;

	// catches most draws by insufficient material
	if (!material_counts[WPAWN] + !material_counts[BPAWN]) {
		int score = 9 * (material_counts[WQUEEN] - material_counts[BQUEEN]) + 
			5 * (material_counts[WROOK] - material_counts[BROOK]) +
			3 * (material_counts[WBISHOP] - material_counts[BBISHOP]) +
			3 * (material_counts[WKNIGHT] - material_counts[BKNIGHT]);

		if (score > -4 && score < 4)
			return 1;
	}
	return 0;
}

// Evaluate piece square tables
static int piece_square_eval(void) {
	int game_phase = 0;
	int mg_score = 0;
	int eg_score = 0;

	for (int addr = 21; addr <= 98; addr++) {
		PIECE p = board[addr];
		if (p <= OFF_BOARD)
			continue;

		game_phase += GAME_PHASE_INC[p - WPAWN];

		int a = s120_to_64[addr];
		int ms = mg_piece_sq[p - WPAWN][a];
		int es = eg_piece_sq[p - WPAWN][a];

		if (check_color(p, WHITE)) {
			mg_score += ms;
			eg_score += es;
		} else {
			mg_score -= ms;
			eg_score -= es;
		}
	}
	if (game_phase > 24) game_phase = 24;
	return (mg_score * game_phase + eg_score *(24 - game_phase)) / 24;
}

////////////////////////////////////////////////////////////////////////////////
/// EXTERNAL FUNCTIONS
///
/// Exposed in eval.h

int evaluate(void) {
	if (is_draw()) return 0;

	int score = piece_square_eval();

	// trade down pieces when winning
	int minor_major_cnt = 	material_counts[WKNIGHT] + material_counts[WBISHOP] + 
		material_counts[WROOK] + material_counts[WQUEEN] +
		material_counts[BKNIGHT] + material_counts[BBISHOP] + 
		material_counts[BROOK] + material_counts[BQUEEN];
	if (minor_major_cnt <= 14) {
		if (score > 0)
			score += TRADE_DOWN * (14 - minor_major_cnt);
		else if (score < 0)
			score -= TRADE_DOWN * (14 - minor_major_cnt);
	}

	if (!material_counts[WPAWN]) score -= NPP;
	if (!material_counts[BPAWN]) score += NPP;

	// pairs
	if (material_counts[WBISHOP] >= 2) score += BISHOP_PAIR;
	if (material_counts[BBISHOP] >= 2) score -= BISHOP_PAIR;
	if (material_counts[WKNIGHT] >= 2) score += KNIGHT_PAIR;
	if (material_counts[BKNIGHT] >= 2) score -= KNIGHT_PAIR;
	if (material_counts[WROOK] >= 2) score += ROOK_PAIR;
	if (material_counts[BROOK] >= 2) score -= ROOK_PAIR;

	score += MOB * mobility();
	score -= DI * (bad_pawns(WHITE) - bad_pawns(BLACK));

	if (side_to_move == BLACK) score *= -1;
	return score;
}


// initialize piece square tables
void init_eval(void) {
	for (int i = 0; i < 6; i++) {
		for (int n = 0; n < 64; n++) {
			mg_piece_sq[2 * i][n] = mg_piece_tables[i][n] + MG_PIECE_VALS[i];
			eg_piece_sq[2 * i][n] = eg_piece_tables[i][n] + EG_PIECE_VALS[i];
			mg_piece_sq[2 * i + 1][n] = mg_piece_tables[i][n ^ 56] + MG_PIECE_VALS[i];
			eg_piece_sq[2 * i + 1][n] = eg_piece_tables[i][n ^ 56] + EG_PIECE_VALS[i];
		}
	}
}

int is_repeat_position(void) {
	if (halfmoves < 4) return 0;
	// only need to check since last capture or pawn advance
	// only need to check every other position
	for (int i = 2; i <= halfmoves; i += 2) {
		if (history[history_cnt - 1 - i] < 0)
			return 0;
		if (history[history_cnt - 1 - i] == game_hash)
			return 1;
	}
	return 0;
}
