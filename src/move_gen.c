#include "core.h"
#include <string.h>

#include "string_processing.h"

// remove id from id list
// assume piece_type is set correctly
static void remove_id(int id) {
	PIECE pt = piece_type[id];
	ASSERT(pt != WK && pt != BK && pt > OFF_BOARD && pt <= BQ);

	int *pid_arr = piece_ids[pt - 2];
	int mc = --material_counts[pt - 2];
	for (int i = 0; i < mc; i++) {
		if (pid_arr[i] == id) {
			pid_arr[i] = pid_arr[mc];
			break;
		}
	}
}

// Return 1 if given square addr is attacked by color c
// Otherwise return 0
int is_square_attacked(int addr, COLOR c) {
	ASSERT(on_board(addr));
	ASSERT(c == WHITE || c == BLACK);

	if (c == WHITE) {
		if (pt_at_addr(addr + 9) == WP || pt_at_addr(addr + 11) == WP)
			return 1;
		if (		pt_at_addr(addr - 21) == WN || 
				pt_at_addr(addr - 19) == WN ||
				pt_at_addr(addr - 12) == WN || 
				pt_at_addr(addr -  8) == WN ||
				pt_at_addr(addr + 21) == WN || 
				pt_at_addr(addr + 19) == WN ||
				pt_at_addr(addr + 12) == WN || 
				pt_at_addr(addr +  8) == WN)
			return 1;
		if (		pt_at_addr(addr - 11) == WK || 
				pt_at_addr(addr - 10) == WK || 
				pt_at_addr(addr - 9) == WK || 
				pt_at_addr(addr - 1) == WK || 
				pt_at_addr(addr + 1) == WK ||
				pt_at_addr(addr + 9) == WK ||
				pt_at_addr(addr + 10) == WK ||
				pt_at_addr(addr + 11) == WK)
			return 1;

		// sliding checks
		PIECE p;
		int dir = -10;
		int c_addr = addr + dir;
		// up
		while (1) {
			p = pt_at_addr(c_addr);
			if (p == WQ || p == WR)
				return 1;
			if (p == EMPTY)
				c_addr += dir;
			else
				break;
		}
		// down
		dir = 10;
		c_addr = addr + dir;
		while (1) {
			p = pt_at_addr(c_addr);
			if (p == WQ || p == WR)
				return 1;
			if (p == EMPTY)
				c_addr += dir;
			else
				break;
		}
		// left
		dir = -1;
		c_addr = addr + dir;
		while (1) {
			p = pt_at_addr(c_addr);
			if (p == WQ || p == WR)
				return 1;
			if (p == EMPTY)
				c_addr += dir;
			else
				break;
		}
		// right
		dir = 1;
		c_addr = addr + dir;
		while (1) {
			p = pt_at_addr(c_addr);
			if (p == WQ || p == WR)
				return 1;
			if (p == EMPTY)
				c_addr += dir;
			else
				break;
		}
		// ul
		dir = -11;
		c_addr = addr + dir;
		while (1) {
			p = pt_at_addr(c_addr);
			if (p == WQ || p == WB)
				return 1;
			if (p == EMPTY)
				c_addr += dir;
			else
				break;
		}
		// ur
		dir = -9;
		c_addr = addr + dir;
		while (1) {
			p = pt_at_addr(c_addr);
			if (p == WQ || p == WB)
				return 1;
			if (p == EMPTY)
				c_addr += dir;
			else
				break;
		}
		// dl
		dir = 9;
		c_addr = addr + dir;
		while (1) {
			p = pt_at_addr(c_addr);
			if (p == WQ || p == WB)
				return 1;
			if (p == EMPTY)
				c_addr += dir;
			else
				break;
		}
		// dr
		dir = 11;
		c_addr = addr + dir;
		while (1) {
			p = pt_at_addr(c_addr);
			if (p == WQ || p == WB)
				return 1;
			if (p == EMPTY)
				c_addr += dir;
			else
				break;
		}
	} else {
		if (pt_at_addr(addr - 9) == BP || pt_at_addr(addr - 11) == BP)
			return 1;
		if (		pt_at_addr(addr - 21) == BN || 
				pt_at_addr(addr - 19) == BN ||
				pt_at_addr(addr - 12) == BN || 
				pt_at_addr(addr -  8) == BN ||
				pt_at_addr(addr + 21) == BN || 
				pt_at_addr(addr + 19) == BN ||
				pt_at_addr(addr + 12) == BN || 
				pt_at_addr(addr +  8) == BN)
			return 1;
		if (		pt_at_addr(addr - 11) == BK || 
				pt_at_addr(addr - 10) == BK || 
				pt_at_addr(addr - 9) == BK || 
				pt_at_addr(addr - 1) == BK || 
				pt_at_addr(addr + 1) == BK ||
				pt_at_addr(addr + 9) == BK ||
				pt_at_addr(addr + 10) == BK ||
				pt_at_addr(addr + 11) == BK)
			return 1;

		// sliding checks
		PIECE p;
		int dir = -10;
		int c_addr = addr + dir;
		// up
		while (1) {
			p = pt_at_addr(c_addr);
			if (p == BQ || p == BR)
				return 1;
			if (p == EMPTY)
				c_addr += dir;
			else
				break;
		}
		// down
		dir = 10;
		c_addr = addr + dir;
		while (1) {
			p = pt_at_addr(c_addr);
			if (p == BQ || p == BR)
				return 1;
			if (p == EMPTY)
				c_addr += dir;
			else
				break;
		}
		// left
		dir = -1;
		c_addr = addr + dir;
		while (1) {
			p = pt_at_addr(c_addr);
			if (p == BQ || p == BR)
				return 1;
			if (p == EMPTY)
				c_addr += dir;
			else
				break;
		}
		// right
		dir = 1;
		c_addr = addr + dir;
		while (1) {
			p = pt_at_addr(c_addr);
			if (p == BQ || p == BR)
				return 1;
			if (p == EMPTY)
				c_addr += dir;
			else
				break;
		}
		// ul
		dir = -11;
		c_addr = addr + dir;
		while (1) {
			p = pt_at_addr(c_addr);
			if (p == BQ || p == BB)
				return 1;
			if (p == EMPTY)
				c_addr += dir;
			else
				break;
		}
		// ur
		dir = -9;
		c_addr = addr + dir;
		while (1) {
			p = pt_at_addr(c_addr);
			if (p == BQ || p == BB)
				return 1;
			if (p == EMPTY)
				c_addr += dir;
			else
				break;
		}
		// dl
		dir = 9;
		c_addr = addr + dir;
		while (1) {
			p = pt_at_addr(c_addr);
			if (p == BQ || p == BB)
				return 1;
			if (p == EMPTY)
				c_addr += dir;
			else
				break;
		}
		// dr
		dir = 11;
		c_addr = addr + dir;
		while (1) {
			p = pt_at_addr(c_addr);
			if (p == BQ || p == BB)
				return 1;
			if (p == EMPTY)
				c_addr += dir;
			else
				break;
		}
	}
	return 0;
}

// Init move from addr sa to addr ta.
// Return 1 iff move is legal and was initialized
// Return 0 iff move is illegal and not initialzed
//
// Move memory is handled by caller.
// Promotion is always set to EMPTY and must be manually changed by
// caller if necassary.
int init_move(Move *move, int sa, int ta) {
	ASSERT(move);
	ASSERT(on_board(sa) && on_board(ta));
	ASSERT(piece_color[board[sa]] == side_to_move);
	ASSERT(piece_color[board[ta]] != side_to_move);

	move->cap_id = board[ta];
	// handle en passant capture
	if (ta == ep_addr) {
		if (pt_at_addr(sa) == WP) {
			ASSERT(pt_at_addr(ta + 10) == BP);
			move->cap_id = board[ta + 10];
		}
		else if (pt_at_addr(sa) == BP) {
			ASSERT(pt_at_addr(ta - 10) == WP);
			move->cap_id = board[ta - 10];
		}
	}
	// check for king in check through pseudo make move
	board[piece_addr[move->cap_id]] = E_ID;
	int sid = board[sa];
	board[sa] = E_ID;
	board[ta] = sid;
	int ka;
	int illegal = 0;

	if (side_to_move == WHITE) {
		ka = piece_addr[piece_ids[WK - 2][0]];
		if (ka == sa) ka = ta;
		if (is_square_attacked(ka, BLACK)) illegal = 1;
	} else {
		ka = piece_addr[piece_ids[BK - 2][0]];
		if (ka == sa) ka = ta;
		if (is_square_attacked(ka, WHITE)) illegal = 1;
	}

	board[ta] = E_ID;
	board[piece_addr[move->cap_id]] = move->cap_id;
	board[sa] = sid;
	if (illegal) return 0;

	move->mv.sa = sa;
	move->mv.ta = ta;
	move->castle_rights = castle_rights;
	move->ep_addr = ep_addr;
	move->halfmoves = halfmoves;
	move->mv.promo = EMPTY;
	move->game_hash = game_hash;
	return 1;
}





// Generate all pawn promotion moves for pawn id to address ta.
// Return number of moves added
// Start adding moves at index 0 of ml.
// Assume ml is large enough to store all moves.
//
// NOTE: caller must account for added moves and adjust their move_list index
// accordingly. This function will ALWAYS generate 4 or 0 moves.
static int gen_promos(Move *ml, int id, int ta) {
	ASSERT(ml);
	ASSERT(piece_type[id] != EMPTY && piece_type[id] != OFF_BOARD);

	int mi = 0;
	int a = piece_addr[id];

	if (piece_color[id] == WHITE) {
		if (!init_move(ml + mi, a, ta))
			return 0;
		mi++;
		ml[mi - 1].mv.promo = WN;
		mi += init_move(ml + mi, a, ta);
		ml[mi - 1].mv.promo = WB;
		mi += init_move(ml + mi, a, ta);
		ml[mi - 1].mv.promo = WR;
		mi += init_move(ml + mi, a, ta);
		ml[mi - 1].mv.promo = WQ;
	} else {
		if (!init_move(ml + mi, a, ta))
			return 0;
		mi++;
		ml[mi - 1].mv.promo = BN;
		mi += init_move(ml + mi, a, ta);
		ml[mi - 1].mv.promo = BB;
		mi += init_move(ml + mi, a, ta);
		ml[mi - 1].mv.promo = BR;
		mi += init_move(ml + mi, a, ta);
		ml[mi - 1].mv.promo = BQ;
	}
	ASSERT(mi == 4);
	return mi;
}

// generate pseudo legal black pawn moves
// return number of moves generated
static int gen_bp_moves(Move *ml) {
	ASSERT(ml);
	ASSERT(side_to_move == BLACK);

	int mi = 0;
	int n_bp = p_count(BP);

	for (int i = 0; i < n_bp; i++) {
		int id = piece_ids[BP - 2][i];
		int a = piece_addr[id];
		ASSERT(addr_to_rank(a) <= 7);
		ASSERT(on_board(a + 10));

		// advance
		if (pt_at_addr(a + 10) == EMPTY) {
			if (addr_to_rank(a) == 2) {
				mi += gen_promos(ml + mi, id, a + 10);
			} else {
				mi += init_move(ml + mi, a, a + 10);
				if (addr_to_rank(a) == 7 && pt_at_addr(a + 20) == EMPTY) {
					mi += init_move(ml + mi, a, a + 20);
				}
			}
		}
		// captures
		if (piece_color[board[a + 9]] == WHITE || ep_addr == a + 9) {
			if (addr_to_rank(a) == 2) {
				mi += gen_promos(ml + mi, id, a + 9);
			} else {
				mi += init_move(ml + mi, a, a + 9);
			}
		}
		if (piece_color[board[a + 11]] == WHITE || ep_addr == a + 11) {
			if (addr_to_rank(a) == 2) {
				mi += gen_promos(ml + mi, id, a + 11);
			} else {
				mi += init_move(ml + mi, a, a + 11);
			}
		}
	}
	return mi;
}

// generate pseudo legal white pawn moves
// return number of moves generated
static int gen_wp_moves(Move *ml) {
	ASSERT(ml);
	ASSERT(side_to_move == WHITE);

	int mi = 0;
	int n_wp = p_count(WP);

	for (int i = 0; i < n_wp; i++) {
		int id = piece_ids[WP - 2][i];
		int a = piece_addr[id];
		ASSERT(addr_to_rank(a) <= 7);
		ASSERT(on_board(a - 10));

		// advance
		if (pt_at_addr(a - 10) == EMPTY) {
			if (addr_to_rank(a) == 7) {
				mi += gen_promos(ml + mi, id, a - 10);
			} else {
				mi += init_move(ml + mi, a, a - 10);
				if (addr_to_rank(a) == 2 && pt_at_addr(a - 20) == EMPTY) {
					mi += init_move(ml + mi, a, a - 20);
				}
			}
		}
		// captures
		if (piece_color[board[a - 9]] == BLACK || ep_addr == a - 9) {
			if (addr_to_rank(a) == 7) {
				mi += gen_promos(ml + mi, id, a - 9);
			} else {
				mi += init_move(ml + mi, a, a - 9);
			}
		}
		if (piece_color[board[a - 11]] == BLACK || ep_addr == a - 11) {
			if (addr_to_rank(a) == 7) {
				mi += gen_promos(ml + mi, id, a - 11);
			} else {
				mi += init_move(ml + mi, a, a - 11);
			}
		}
	}
	return mi;
}

// generate pseudo legal knight moves
// return number of moves generated
// Assume ml is large enough.
// Caller is responsible for ml memory.
static int gen_knight_moves(Move *ml) {
	ASSERT(ml);
	ASSERT(side_to_move == WHITE || side_to_move == BLACK);

	int mi = 0;
	COLOR c = side_to_move;
	int np;
	int *pid_arr;
	if (c == WHITE) {
		np = p_count(WN);
		pid_arr = piece_ids[WN - 2];
	} else {
		np = p_count(BN);
		pid_arr = piece_ids[BN - 2];
	}

	for (int i = 0; i < np; i++) {
		int id = pid_arr[i];
		int a = piece_addr[id];

		if (piece_color[board[a - 21]] != c && pt_at_addr(a - 21) != OFF_BOARD)
			mi += init_move(ml + mi, a, a - 21);
		if (piece_color[board[a - 19]] != c && pt_at_addr(a - 19) != OFF_BOARD)
			mi += init_move(ml + mi, a, a - 19);
		if (piece_color[board[a - 12]] != c && pt_at_addr(a - 12) != OFF_BOARD)
			mi += init_move(ml + mi, a, a - 12);
		if (piece_color[board[a - 8]] != c && pt_at_addr(a - 8) != OFF_BOARD)
			mi += init_move(ml + mi, a, a - 8);
		if (piece_color[board[a + 21]] != c && pt_at_addr(a + 21) != OFF_BOARD)
			mi += init_move(ml + mi, a, a + 21);
		if (piece_color[board[a + 19]] != c && pt_at_addr(a + 19) != OFF_BOARD)
			mi += init_move(ml + mi, a, a + 19);
		if (piece_color[board[a + 12]] != c && pt_at_addr(a + 12) != OFF_BOARD)
			mi += init_move(ml + mi, a, a + 12);
		if (piece_color[board[a + 8]] != c && pt_at_addr(a + 8) != OFF_BOARD)
			mi += init_move(ml + mi, a, a + 8);
	}
	return mi;
}

// generate pseudo legal king moves
// return number of moves generated
// Assume ml is large enough.
// Caller is responsible for ml memory.
static int gen_king_moves(Move *ml) {
	ASSERT(ml);
	ASSERT(side_to_move == WHITE || side_to_move == BLACK);

	int mi = 0;
	COLOR c = side_to_move;
	int id;
	if (c == WHITE) {
		ASSERT(material_counts[WK - 2]);
		id = piece_ids[WK - 2][0];
	} else {
		ASSERT(material_counts[BK - 2]);
		id = piece_ids[BK - 2][0];
	}
	int a = piece_addr[id];

	if (piece_color[board[a - 11]] != c && pt_at_addr(a - 11) != OFF_BOARD)
		mi += init_move(ml + mi, a, a - 11);
	if (piece_color[board[a - 10]] != c && pt_at_addr(a - 10) != OFF_BOARD)
		mi += init_move(ml + mi, a, a - 10);
	if (piece_color[board[a - 9]] != c && pt_at_addr(a - 9) != OFF_BOARD)
		mi += init_move(ml + mi, a, a - 9);
	if (piece_color[board[a - 1]] != c && pt_at_addr(a - 1) != OFF_BOARD)
		mi += init_move(ml + mi, a, a - 1);
	if (piece_color[board[a + 1]] != c && pt_at_addr(a + 1) != OFF_BOARD)
		mi += init_move(ml + mi, a, a + 1);
	if (piece_color[board[a + 9]] != c && pt_at_addr(a + 9) != OFF_BOARD)
		mi += init_move(ml + mi, a, a + 9);
	if (piece_color[board[a + 10]] != c && pt_at_addr(a + 10) != OFF_BOARD)
		mi += init_move(ml + mi, a, a + 10);
	if (piece_color[board[a + 11]] != c && pt_at_addr(a + 11) != OFF_BOARD)
		mi += init_move(ml + mi, a, a + 11);

	if (c == WHITE) {
		if (		castle_rights & K_CASTLE && 
				pt_at_addr(a + 1) == EMPTY && 
				pt_at_addr(a + 2) == EMPTY && 
				!is_square_attacked(a, BLACK) && 
				!is_square_attacked(a + 1, BLACK)) {
			ASSERT(pt_at_addr(98) == WR);
			ASSERT(pt_at_addr(95) == WK);
			mi += init_move(ml + mi, a, a + 2);
		}
		if (		castle_rights & Q_CASTLE && 
				pt_at_addr(a - 1) == EMPTY && 
				pt_at_addr(a - 2) == EMPTY && 
				pt_at_addr(a - 3) == EMPTY &&
				!is_square_attacked(a, BLACK) && 
				!is_square_attacked(a - 1, BLACK)) {
			ASSERT(pt_at_addr(91) == WR);
			ASSERT(pt_at_addr(95) == WK);
			mi += init_move(ml + mi, a, a - 2);
		}
	} else {
		if (		castle_rights & k_CASTLE && 
				pt_at_addr(a + 1) == EMPTY && 
				pt_at_addr(a + 2) == EMPTY && 
				!is_square_attacked(a, WHITE) && 
				!is_square_attacked(a + 1, WHITE)) {
			ASSERT(pt_at_addr(28) == BR);
			ASSERT(pt_at_addr(25) == BK);
			mi += init_move(ml + mi, a, a + 2);
		}
		if (		castle_rights & q_CASTLE && 
				pt_at_addr(a - 1) == EMPTY && 
				pt_at_addr(a - 2) == EMPTY && 
				pt_at_addr(a - 3) == EMPTY &&
				!is_square_attacked(a, WHITE) && 
				!is_square_attacked(a - 1, WHITE)) {
			ASSERT(pt_at_addr(21) == BR);
			ASSERT(pt_at_addr(25) == BK);
			mi += init_move(ml + mi, a, a - 2);
		}
	}
	return mi;
}

// Generate sliding moves for a piece located at a moving in DIRECTION d 
// and add to move_list.
// Return number of moves added to move_list
// Moves will be added to move_list starting at index 0
//
// Assume move_list is initialized and has enough space
// Aassume a bishop, rook, or queen is located at addr
static int gen_slide_move(Move *ml, int a, DIRECTION d) {
	ASSERT(ml);
	ASSERT(on_board(a));
	ASSERT(		pt_at_addr(a) == WQ || 
			pt_at_addr(a) == BQ || 
			pt_at_addr(a) == WB || 
			pt_at_addr(a) == BB || 
			pt_at_addr(a) == WR || 
			pt_at_addr(a) == BR);

	COLOR c;
	if (side_to_move == WHITE) c = BLACK;
	else c = WHITE;

	int mi = 0;
	int off = d;
	while (1) {
		int id = board[a + off];
		if (piece_type[id] == EMPTY) {
			mi += init_move(ml + mi, a, a + off);
			off += d;
		} else if (piece_color[id] == c) {
			mi += init_move(ml + mi, a, a + off);
			break;
		} else {
			break;
		}
	}
	return mi;
}

// generate pseudo legal bishop moves
// return number of moves generated
// Assume ml is large enough.
// Caller is responsible for ml memory.
static int gen_bishop_moves(Move *ml) {
	ASSERT(ml);

	int mi = 0;
	COLOR c = side_to_move;
	int np;
	int *pid_arr;
	if (c == WHITE) {
		np = p_count(WB);
		pid_arr = piece_ids[WB - 2];
	} else {
		np = p_count(BB);
		pid_arr = piece_ids[BB - 2];
	}

	for (int i = 0; i < np; i++) {
		int id = pid_arr[i];
		int a = piece_addr[id];
		mi += gen_slide_move(ml + mi, a, UR);
		mi += gen_slide_move(ml + mi, a, UL);
		mi += gen_slide_move(ml + mi, a, DR);
		mi += gen_slide_move(ml + mi, a, DL);
	}
	return mi;
}

// generate pseudo legal rooks moves
// return number of moves generated
// Assume ml is large enough.
// Caller is responsible for ml memory.
static int gen_rook_moves(Move *ml) {
	ASSERT(ml);

	int mi = 0;
	COLOR c = side_to_move;
	int np;
	int *pid_arr;
	if (c == WHITE) {
		np = p_count(WR);
		pid_arr = piece_ids[WR - 2];
	} else {
		np = p_count(BR);
		pid_arr = piece_ids[BR - 2];
	}

	for (int i = 0; i < np; i++) {
		int id = pid_arr[i];
		int a = piece_addr[id];
		mi += gen_slide_move(ml + mi, a, U);
		mi += gen_slide_move(ml + mi, a, D);
		mi += gen_slide_move(ml + mi, a, L);
		mi += gen_slide_move(ml + mi, a, R);
	}
	return mi;
}

// generate pseudo legal queen moves
// return number of moves generated
// Assume ml is large enough.
// Caller is responsible for ml memory.
static int gen_queen_moves(Move *ml) {
	ASSERT(ml);

	int mi = 0;
	COLOR c = side_to_move;
	int np;
	int *pid_arr;
	if (c == WHITE) {
		np = p_count(WQ);
		pid_arr = piece_ids[WQ - 2];
	} else {
		np = p_count(BQ);
		pid_arr = piece_ids[BQ - 2];
	}

	for (int i = 0; i < np; i++) {
		int id = pid_arr[i];
		int a = piece_addr[id];
		mi += gen_slide_move(ml + mi, a, UR);
		mi += gen_slide_move(ml + mi, a, UL);
		mi += gen_slide_move(ml + mi, a, DR);
		mi += gen_slide_move(ml + mi, a, DL);
		mi += gen_slide_move(ml + mi, a, U);
		mi += gen_slide_move(ml + mi, a, D);
		mi += gen_slide_move(ml + mi, a, L);
		mi += gen_slide_move(ml + mi, a, R);
	}
	return mi;
}

// generate legal moves in a position
// caller is responsible for ml memory
// assume ml is large enough
// above assumptions are true for all move gen helper functions
//
// order of generated move is not always the same because
// adding and removing pieces changes the move gen order
int gen_moves(Move *ml) {
	ASSERT(ml);
	ASSERT(side_to_move == WHITE || side_to_move == BLACK);
	int mi = 0;

	if (side_to_move == WHITE) {
		mi += gen_wp_moves(ml + mi);
	} else {
		mi += gen_bp_moves(ml + mi);
	}
	mi += gen_knight_moves(ml + mi);
	mi += gen_bishop_moves(ml + mi);
	mi += gen_rook_moves(ml + mi);
	mi += gen_queen_moves(ml + mi);
	mi += gen_king_moves(ml + mi);

	// MVV-LVA ordering
	// NOTE moves already generated in least valuable aggressor order 
	int test_i = 1;
	int compare_i = 0;
	while (test_i < mi) {
		PIECE curr_victim = piece_type[ml[test_i].cap_id];
		PIECE compare_victim = piece_type[ml[compare_i].cap_id];

		// find swap index
		while (curr_victim > compare_victim) {
			if (--compare_i < 0)
				break;
			compare_victim = piece_type[ml[compare_i].cap_id];
		}

		ASSERT(compare_i < test_i && compare_i >= -1);
		// ordering is incorrect
		if (compare_i < test_i - 1) {
			Move temp_move = ml[test_i];
			memmove(ml + compare_i + 2, ml + compare_i + 1, sizeof(Move) * (test_i - (compare_i + 1)));
			ml[compare_i + 1] = temp_move;
		}
		compare_i = test_i++;
	}

	return mi;
}

// move piece id to target address
// helper function for make_move()
static void move_piece(int id, int ta) {
	ASSERT(on_board(ta));
	hash_piece(piece_type[id], piece_addr[id]);

	board[ta] = id;
	board[piece_addr[id]] = E_ID;
	piece_addr[id] = ta;

	hash_piece(piece_type[id], ta);
}

// move piece id to target address
// helper function for unmake_move()
//
// same as move_piece except without hashing as
// unmake_move() handles that itself
static void unmove_piece(int id, int ta) {
	ASSERT(on_board(ta));
	board[ta] = id;
	board[piece_addr[id]] = E_ID;
	piece_addr[id] = ta;
}

// Revert board state to previous move using Move used to generate
// most recent move.
// Assume move is allocated and valid
void unmake_move(Move *move) {
	ASSERT(move);
	// reset game state
	ep_addr = move->ep_addr;
	halfmoves = move->halfmoves;
	castle_rights = move->castle_rights;
	game_hash = move->game_hash;

	const int s = move->mv.sa;
	const int e = move->mv.ta;
	int eid = board[e];
	PIECE ept = piece_type[eid];

	ASSERT(on_board(s));
	ASSERT(on_board(e));
	ASSERT(board[s] == E_ID);
	ASSERT(ept >= WP && ept <= BK);

	// ummove piece and handle special cases
	//
	// promotion
	if (move->mv.promo != EMPTY) {
		ASSERT(move->mv.promo != OFF_BOARD);
		COLOR c = piece_color[eid];
		ASSERT(c == WHITE || c == BLACK);
		ASSERT(piece_type[eid] > WP || piece_type[eid] <= BQ);

		remove_id(eid);
		if (c == WHITE) piece_type[eid] = WP;
		else piece_type[eid] = BP;
		add_id(eid);
	} 
	// undo kingside castle
	else if ((ept == WK || ept == BK) && e == s + 2) {
		unmove_piece(board[e - 1], e + 1);
	}
	// undo queenside castle
	else if ((ept == WK || ept == BK) && e == s - 2) {
		unmove_piece(board[e + 1], e - 2);
	}
	// standard move
	unmove_piece(eid, s);

	// uncapture piece (or replace with NONE if no capture occurred)
	if (move->cap_id > OFF_BOARD){
		add_id(move->cap_id);
		board[piece_addr[move->cap_id]] = move->cap_id;
	}

	if (side_to_move == WHITE) {
		moves--;
		side_to_move = BLACK;
	} else {
		side_to_move = WHITE;
	}

	// undo history
	history_cnt--;
}

// make a move from mailbox address start_addr to end_addr
// verify that move does not leave king in check
//
// return 0 if move made and 1 if move failed due to leaving
// king in check
// assume addresses are valid
void make_move(Move *move) {
	// start and end squares
	int s = move->mv.sa;
	int e = move->mv.ta;
	int sid = board[s];
	PIECE spt = piece_type[sid];

	ASSERT(on_board(s));
	ASSERT(on_board(e));

	// capture piece
	if (move->cap_id > OB_ID) {
		ASSERT(		piece_type[move->cap_id] >= WP && 
				piece_type[move->cap_id] <= BK &&
				piece_color[move->cap_id] != side_to_move);

		hash_piece(piece_type[move->cap_id], piece_addr[move->cap_id]);
		board[piece_addr[move->cap_id]] = E_ID;
		remove_id(move->cap_id);
	}
	// move piece in mailbox and handle special cases
	//
	// promotion
	if (move->mv.promo != EMPTY) {
		// white assertions
		ASSERT(		side_to_move == BLACK || 
				(spt == WP && 
				 addr_to_rank(s) == 7 && 
				 addr_to_rank(e) == 8));
		ASSERT(		side_to_move == BLACK || 
				(move->mv.promo >= WN && 
				 move->mv.promo <= WQ));
		// black assertions
		ASSERT(		side_to_move == WHITE || 
				(spt == BP && 
				 addr_to_rank(s) == 2  && 
				 addr_to_rank(e) == 1));
		ASSERT(		side_to_move == WHITE || 
				(move->mv.promo >= BN && 
				 move->mv.promo <= BQ));

		move_piece(sid, e);
		hash_piece(piece_type[sid], e);

		remove_id(sid);
		piece_type[sid] = move->mv.promo;
		add_id(sid);

		hash_piece(piece_type[sid], e);
	} 
	// white kingside castle
	else if (spt == WK && e == s + 2) {
		ASSERT(castle_rights & K_CASTLE);
		ASSERT(board[s + 1] == E_ID && board[s + 2] == E_ID);
		ASSERT(s == 95);
		ASSERT(pt_at_addr(98) == WR);

		int rook_id = board[e + 1];
		move_piece(rook_id, e - 1);
		move_piece(sid, e);

		hash_castle();
		castle_rights ^= K_CASTLE;
		castle_rights &= ~Q_CASTLE;
		hash_castle();
	}
	// white queenside castle
	else if (spt == WK && e == s - 2) {
		ASSERT(castle_rights & Q_CASTLE);
		ASSERT(board[s - 1] == E_ID && board[s - 2] == E_ID && board[s - 3] == E_ID);
		ASSERT(s == 95);
		ASSERT(pt_at_addr(91) == WR);

		int rook_id = board[e - 2];
		move_piece(rook_id, e + 1);
		move_piece(sid, e);

		hash_castle();
		castle_rights ^= Q_CASTLE;
		castle_rights &= ~K_CASTLE;
		hash_castle();
	}
	// black kingside castle
	else if (spt == BK && e == s + 2) {
		ASSERT(castle_rights & k_CASTLE);
		ASSERT(board[s + 1] == E_ID && board[s + 2] == E_ID);
		ASSERT(s == 25);
		ASSERT(pt_at_addr(28) == BR);

		int rook_id = board[e + 1];
		move_piece(rook_id, e - 1);
		move_piece(sid, e);

		hash_castle();
		castle_rights ^= k_CASTLE;
		castle_rights &= ~q_CASTLE;
		hash_castle();
	}
	// black queenside castle
	else if (spt == BK && e == s - 2) {
		ASSERT(castle_rights & q_CASTLE);
		ASSERT(board[s - 1] == E_ID && board[s - 2] == E_ID && board[s - 3] == E_ID);
		ASSERT(s == 25);
		ASSERT(pt_at_addr(21) == BR);

		int rook_id = board[e - 2];
		move_piece(rook_id, e + 1);
		move_piece(sid, e);

		hash_castle();
		castle_rights ^= q_CASTLE;
		castle_rights &= ~k_CASTLE;
		hash_castle();
	}
	// standard move
	else {
		move_piece(sid, e);
	}
	ASSERT(board[s] == E_ID);

	// increment fullmove and halfmove if applicable
	if (side_to_move == BLACK) moves++;
	ASSERT(move->cap_id != OB_ID);
	if (move->cap_id != E_ID || spt == WP || spt == BP)
		halfmoves = 0;
	else
		halfmoves++;

	// update en passant square
	if (ep_addr != -1) hash_ep();
	if (spt == WP && addr_to_rank(s) == 2 && e == s - 20) {
		ep_addr = s - 10;
		hash_ep();
	} else if (spt == BP && addr_to_rank(s) == 7 && e == s + 20) {
		ep_addr = s + 10;
		hash_ep();
	} else {
		ep_addr = -1;
	}

	ASSERT(ep_addr == -1 || 
			addr_to_rank(ep_addr) == 3 || 
			addr_to_rank(ep_addr) == 6);

	// update castling rights
	if (castle_rights) {
		if (spt == WK) {
			castle_rights &= ~K_CASTLE;
			castle_rights &= ~Q_CASTLE;
		} else if (spt == BK) {
			castle_rights &= ~k_CASTLE;
			castle_rights &= ~q_CASTLE;
		}
		if (s == 98 || e == 98) castle_rights &= ~K_CASTLE;
		if (s == 91 || e == 91) castle_rights &= ~Q_CASTLE;
		if (s == 28 || e == 28) castle_rights &= ~k_CASTLE;
		if (s == 21 || e == 21) castle_rights &= ~q_CASTLE;
	}

	// update side to move
	hash_side_to_move();
	if (side_to_move == WHITE) side_to_move = BLACK;
	else side_to_move = WHITE;

	// update history
	history[history_cnt++] = game_hash;
}

ULL perft(int d) {
	Move move_list[256];
	int n_moves = gen_moves(move_list);
	if (d == 1) return n_moves;

	unsigned long long c = 0;
	for (int i = 0; i < n_moves; i++) {
		make_move(move_list + i);
		c += perft(d - 1);
		unmake_move(move_list + i);
	}
	return c;
}

int make_manual_move(const char* mv, int len) {
	if (!mv) return 1;
	if (len < 4 || len > 5) return 1;

	int sa = str_to_addr(mv);
	int ta = str_to_addr(mv + 2);
	PIECE wp = EMPTY;
	PIECE bp = EMPTY;
	if (len == 5) {
		switch (mv[4]) {
			case 'q':
				wp = WQ;
				bp = BQ;
				break;
			case 'r':
				wp = WR;
				bp = BR;
				break;
			case 'b':
				wp = WB;
				bp = BB;
				break;
			case 'n':
				wp = WN;
				bp = BN;
				break;
			default:
				return 1;
		}
	}

	Move ml[256];
	int mi = gen_moves(ml);
	for (int i = 0; i < mi; i++) {
		if (		ml[i].mv.sa == sa && 
				ml[i].mv.ta == ta && 
				(ml[i].mv.promo == wp || 
				 ml[i].mv.promo == bp)) {
			make_move(ml + i);
			return 0;
		}
	}
	return 1;
}
