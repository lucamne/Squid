#include "core.h"
#include <string.h>

#include "string_processing.h"

////////////////////////////////////////////////////////////////////////////////
/// Functions

// Return 1 if given square addr is attacked by color c
// Otherwise return 0
int is_square_attacked(int addr, COLOR c) {
	ASSERT(on_board(addr));
	ASSERT(c == WHITE || c == BLACK);

	if (c == WHITE) {
		if (board2[addr + 9] == WPAWN || board2[addr + 11] == WPAWN)
			return 1;
		if (		board2[addr - 21] == WKNIGHT || 
				board2[addr - 19] == WKNIGHT ||
				board2[addr - 12] == WKNIGHT || 
				board2[addr -  8] == WKNIGHT ||
				board2[addr + 21] == WKNIGHT || 
				board2[addr + 19] == WKNIGHT ||
				board2[addr + 12] == WKNIGHT || 
				board2[addr +  8] == WKNIGHT)
			return 1;
		if (		board2[addr - 11] == WKING || 
				board2[addr - 10] == WKING || 
				board2[addr - 9] == WKING || 
				board2[addr - 1] == WKING || 
				board2[addr + 1] == WKING ||
				board2[addr + 9] == WKING ||
				board2[addr + 10] == WKING ||
				board2[addr + 11] == WKING)
			return 1;

		// sliding checks
		PIECE p;
		int dir = -10;
		int c_addr = addr + dir;
		// up
		while (1) {
			p = board2[c_addr];
			if (p == WQUEEN || p == WROOK)
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
			p = board2[c_addr];
			if (p == WQUEEN || p == WROOK)
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
			p = board2[c_addr];
			if (p == WQUEEN || p == WROOK)
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
			p = board2[c_addr];
			if (p == WQUEEN || p == WROOK)
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
			p = board2[c_addr];
			if (p == WQUEEN || p == WBISHOP)
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
			p = board2[c_addr];
			if (p == WQUEEN || p == WBISHOP)
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
			p = board2[c_addr];
			if (p == WQUEEN || p == WBISHOP)
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
			p = board2[c_addr];
			if (p == WQUEEN || p == WBISHOP)
				return 1;
			if (p == EMPTY)
				c_addr += dir;
			else
				break;
		}
	} else {
		if (board2[addr - 9] == BPAWN || board2[addr - 11] == BPAWN)
			return 1;
		if (		board2[addr - 21] == BKNIGHT || 
				board2[addr - 19] == BKNIGHT ||
				board2[addr - 12] == BKNIGHT || 
				board2[addr -  8] == BKNIGHT ||
				board2[addr + 21] == BKNIGHT || 
				board2[addr + 19] == BKNIGHT ||
				board2[addr + 12] == BKNIGHT || 
				board2[addr +  8] == BKNIGHT)
			return 1;
		if (		board2[addr - 11] == BKING || 
				board2[addr - 10] == BKING || 
				board2[addr - 9] == BKING || 
				board2[addr - 1] == BKING || 
				board2[addr + 1] == BKING ||
				board2[addr + 9] == BKING ||
				board2[addr + 10] == BKING ||
				board2[addr + 11] == BKING)
			return 1;

		// sliding checks
		PIECE p;
		int dir = -10;
		int c_addr = addr + dir;
		// up
		while (1) {
			p = board2[c_addr];
			if (p == BQUEEN || p == BROOK)
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
			p = board2[c_addr];
			if (p == BQUEEN || p == BROOK)
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
			p = board2[c_addr];
			if (p == BQUEEN || p == BROOK)
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
			p = board2[c_addr];
			if (p == BQUEEN || p == BROOK)
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
			p = board2[c_addr];
			if (p == BQUEEN || p == BBISHOP)
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
			p = board2[c_addr];
			if (p == BQUEEN || p == BBISHOP)
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
			p = board2[c_addr];
			if (p == BQUEEN || p == BBISHOP)
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
			p = board2[c_addr];
			if (p == BQUEEN || p == BBISHOP)
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
	ASSERT(check_color(board2[sa], side_to_move));
	ASSERT(!check_color(board2[ta], side_to_move));

	move->cap_piece = board2[ta];
	int cap_addr = ta;
	// handle en passant capture
	if (ta == ep_addr) {
		if (board2[sa] == WPAWN) {
			ASSERT(board2[ta + 10] == BPAWN);
			move->cap_piece = BPAWN;
			cap_addr = ta + 10;
		}
		else if (board2[sa] == BPAWN) {
			ASSERT(board2[ta - 10] == WPAWN);
			move->cap_piece = WPAWN;
			cap_addr = ta - 10;
		}
	}

	// check for king in check through pseudo make move
	board2[cap_addr] = EMPTY;
	PIECE start_piece = board2[sa];
	board2[sa] = EMPTY;
	board2[ta] = start_piece;
	int ka;
	int illegal = 0;

	if (side_to_move == WHITE) {
		ka = wking_addr;
		if (ka == sa) ka = ta;
		if (is_square_attacked(ka, BLACK)) illegal = 1;
	} else {
		ka = bking_addr;
		if (ka == sa) ka = ta;
		if (is_square_attacked(ka, WHITE)) illegal = 1;
	}

	board2[ta] = EMPTY;
	board2[cap_addr] = move->cap_piece;
	board2[sa] = start_piece;
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





// Generate all pawn promotion moves for pawn at address sa to address ta.
// Return number of moves added
// Start adding moves at index 0 of ml.
// Assume ml is large enough to store all moves.
//
// NOTE: caller must account for added moves and adjust their move_list index
// accordingly. This function will ALWAYS generate 4 or 0 moves.
static int gen_promos(Move *ml, int sa, int ta) {
	ASSERT(ml);
	ASSERT(board2[sa] != EMPTY && board2[sa] != OFF_BOARD);

	int mi = 0;
	if (check_color(board2[sa], WHITE)) {
		if (!init_move(ml + mi, sa, ta))
			return 0;
		mi++;
		ml[mi - 1].mv.promo = WKNIGHT;
		mi += init_move(ml + mi, sa, ta);
		ml[mi - 1].mv.promo = WBISHOP;
		mi += init_move(ml + mi, sa, ta);
		ml[mi - 1].mv.promo = WROOK;
		mi += init_move(ml + mi, sa, ta);
		ml[mi - 1].mv.promo = WQUEEN;
	} else {
		if (!init_move(ml + mi, sa, ta))
			return 0;
		mi++;
		ml[mi - 1].mv.promo = BKNIGHT;
		mi += init_move(ml + mi, sa, ta);
		ml[mi - 1].mv.promo = BBISHOP;
		mi += init_move(ml + mi, sa, ta);
		ml[mi - 1].mv.promo = BROOK;
		mi += init_move(ml + mi, sa, ta);
		ml[mi - 1].mv.promo = BQUEEN;
	}
	ASSERT(mi == 4);
	return mi;
}

// generate pseudo legal moves for a black pawn at address a
// return number of moves generated
static int gen_bp_moves(Move *ml, int a) {
	ASSERT(ml);
	ASSERT(side_to_move == BLACK);
	ASSERT(check_color(board2[a], side_to_move));
	ASSERT(addr_to_rank(a) <= 7);
	ASSERT(on_board(a + 10));

	int mi = 0;
	// advance
	if (board2[a + 10] == EMPTY) {
		if (addr_to_rank(a) == 2) {
			mi += gen_promos(ml + mi, a, a + 10);
		} else {
			mi += init_move(ml + mi, a, a + 10);
			if (addr_to_rank(a) == 7 && board2[a + 20] == EMPTY) {
				mi += init_move(ml + mi, a, a + 20);
			}
		}
	}
	// captures
	PIECE tp = board2[a + 9];
	if (check_color(tp, WHITE) || ep_addr == a + 9) {
		if (addr_to_rank(a) == 2) {
			mi += gen_promos(ml + mi, a, a + 9);
		} else {
			mi += init_move(ml + mi, a, a + 9);
		}
	}
	tp = board2[a + 11];
	if (check_color(tp, WHITE) || ep_addr == a + 11) {
		if (addr_to_rank(a) == 2) {
			mi += gen_promos(ml + mi, a, a + 11);
		} else {
			mi += init_move(ml + mi, a, a + 11);
		}
	}
	return mi;
}

// generate pseudo legal moves for a white pawn at address a 
// return number of moves generated
static int gen_wp_moves(Move *ml, int a) {
	ASSERT(ml);
	ASSERT(side_to_move == WHITE);
	ASSERT(check_color(board2[a], side_to_move));
	ASSERT(addr_to_rank(a) <= 7);
	ASSERT(on_board(a - 10));

	int mi = 0;

	// advance
	if (board2[a - 10] == EMPTY) {
		if (addr_to_rank(a) == 7) {
			mi += gen_promos(ml + mi, a, a - 10);
		} else {
			mi += init_move(ml + mi, a, a - 10);
			if (addr_to_rank(a) == 2 && board2[a - 20] == EMPTY) {
				mi += init_move(ml + mi, a, a - 20);
			}
		}
	}
	// captures
	PIECE tp = board2[a - 9];
	if (check_color(tp, BLACK) || ep_addr == a - 9) {
		if (addr_to_rank(a) == 7) {
			mi += gen_promos(ml + mi, a, a - 9);
		} else {
			mi += init_move(ml + mi, a, a - 9);
		}
	}
	tp = board2[a - 11];
	if (check_color(tp, BLACK) || ep_addr == a - 11) {
		if (addr_to_rank(a) == 7) {
			mi += gen_promos(ml + mi, a, a - 11);
		} else {
			mi += init_move(ml + mi, a, a - 11);
		}
	}
	return mi;
}

// Return 1 if target address is a valid target for a piece of color c
// Return 0 if not
// DO NOT USE for pawns due to pawn capture rules
//
// Target is valid if it is empty or not color c
static int is_valid_target(int ta, COLOR c) {
	PIECE p = board2[ta];
	return (p != OFF_BOARD) && (!check_color(p, c) || p == EMPTY);
}

// generate pseudo legal moves for knight at address a
// return number of moves generated
// Assume ml is large enough.
// Caller is responsible for ml memory.
static int gen_knight_moves(Move *ml, int a) {
	ASSERT(ml);
	ASSERT(side_to_move == WHITE || side_to_move == BLACK);
	ASSERT(check_color(board2[a], side_to_move));

	int mi = 0;
	COLOR c = side_to_move;

	if (is_valid_target(a - 21, c))
		mi += init_move(ml + mi, a, a - 21);
	if (is_valid_target(a - 19, c))
		mi += init_move(ml + mi, a, a - 19);
	if (is_valid_target(a - 12, c))
		mi += init_move(ml + mi, a, a - 12);
	if (is_valid_target(a - 8, c))
		mi += init_move(ml + mi, a, a - 8);
	if (is_valid_target(a + 21, c))
		mi += init_move(ml + mi, a, a + 21);
	if (is_valid_target(a + 19, c))
		mi += init_move(ml + mi, a, a + 19);
	if (is_valid_target(a + 12, c))
		mi += init_move(ml + mi, a, a + 12);
	if (is_valid_target(a + 8, c))
		mi += init_move(ml + mi, a, a + 8);
	return mi;
}

// generate pseudo legal moves for a king at address a
// return number of moves generated
// Assume ml is large enough.
// Caller is responsible for ml memory.
static int gen_king_moves(Move *ml, int a) {
	ASSERT(ml);
	ASSERT(side_to_move == WHITE || side_to_move == BLACK);
	ASSERT(check_color(board2[a], side_to_move));

	int mi = 0;
	COLOR c = side_to_move;

	if (is_valid_target(a - 11, c))
		mi += init_move(ml + mi, a, a - 11);
	if (is_valid_target(a - 10, c))
		mi += init_move(ml + mi, a, a - 10);
	if (is_valid_target(a - 9, c))
		mi += init_move(ml + mi, a, a - 9);
	if (is_valid_target(a - 1, c))
		mi += init_move(ml + mi, a, a - 1);
	if (is_valid_target(a + 1, c))
		mi += init_move(ml + mi, a, a + 1);
	if (is_valid_target(a + 9, c))
		mi += init_move(ml + mi, a, a + 9);
	if (is_valid_target(a + 10, c))
		mi += init_move(ml + mi, a, a + 10);
	if (is_valid_target(a + 11, c))
		mi += init_move(ml + mi, a, a + 11);

	if (c == WHITE) {
		if (		castle_rights & K_CASTLE && 
				board2[a + 1] == EMPTY && 
				board2[a + 2] == EMPTY && 
				!is_square_attacked(a, BLACK) && 
				!is_square_attacked(a + 1, BLACK)) {
			ASSERT(board2[98] == WROOK);
			ASSERT(board2[95] == WKING);
			mi += init_move(ml + mi, a, a + 2);
		}
		if (		castle_rights & Q_CASTLE && 
				board2[a - 1] == EMPTY && 
				board2[a - 2] == EMPTY && 
				board2[a - 3] == EMPTY &&
				!is_square_attacked(a, BLACK) && 
				!is_square_attacked(a - 1, BLACK)) {
			ASSERT(board2[91] == WROOK);
			ASSERT(board2[95] == WKING);
			mi += init_move(ml + mi, a, a - 2);
		}
	} else {
		if (		castle_rights & k_CASTLE && 
				board2[a + 1] == EMPTY && 
				board2[a + 2] == EMPTY && 
				!is_square_attacked(a, WHITE) && 
				!is_square_attacked(a + 1, WHITE)) {
			ASSERT(board2[28] == BROOK);
			ASSERT(board2[25] == BKING);
			mi += init_move(ml + mi, a, a + 2);
		}
		if (		castle_rights & q_CASTLE && 
				board2[a - 1] == EMPTY && 
				board2[a - 2] == EMPTY && 
				board2[a - 3] == EMPTY &&
				!is_square_attacked(a, WHITE) && 
				!is_square_attacked(a - 1, WHITE)) {
			ASSERT(board2[21] == BROOK);
			ASSERT(board2[25] == BKING);
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
	ASSERT(check_color(board2[a], side_to_move));
	ASSERT(ml);
	ASSERT(on_board(a));
	ASSERT(		board2[a] == WQUEEN || 
			board2[a] == BQUEEN || 
			board2[a] == WBISHOP || 
			board2[a] == BBISHOP || 
			board2[a] == WROOK || 
			board2[a] == BROOK);

	COLOR c = side_to_move ^ 1;
	int mi = 0;
	int off = d;
	while (1) {
		PIECE p = board2[a + off];
		if (p == OFF_BOARD) {
			break;
		} else if (p == EMPTY) {
			mi += init_move(ml + mi, a, a + off);
			off += d;
		} else if (check_color(p, c)) {
			mi += init_move(ml + mi, a, a + off);
			break;
		} else {
			break;
		}
	}
	return mi;
}

// generate pseudo legal moves for a bishop at address a
// return number of moves generated
// Assume ml is large enough.
// Caller is responsible for ml memory.
static int gen_bishop_moves(Move *ml, int a) {
	ASSERT(ml);
	int mi = 0;
	mi += gen_slide_move(ml + mi, a, UR);
	mi += gen_slide_move(ml + mi, a, UL);
	mi += gen_slide_move(ml + mi, a, DR);
	mi += gen_slide_move(ml + mi, a, DL);
	return mi;
}

// generate pseudo legal moves for a rook at address a
// return number of moves generated
// Assume ml is large enough.
// Caller is responsible for ml memory.
static int gen_rook_moves(Move *ml, int a) {
	ASSERT(ml);
	int mi = 0;
	mi += gen_slide_move(ml + mi, a, U);
	mi += gen_slide_move(ml + mi, a, D);
	mi += gen_slide_move(ml + mi, a, L);
	mi += gen_slide_move(ml + mi, a, R);
	return mi;
}

// generate pseudo legal moves for a queen at address a
// return number of moves generated
// Assume ml is large enough.
// Caller is responsible for ml memory.
static int gen_queen_moves(Move *ml, int a) {
	ASSERT(ml);
	int mi = 0;
	mi += gen_slide_move(ml + mi, a, UR);
	mi += gen_slide_move(ml + mi, a, UL);
	mi += gen_slide_move(ml + mi, a, DR);
	mi += gen_slide_move(ml + mi, a, DL);
	mi += gen_slide_move(ml + mi, a, U);
	mi += gen_slide_move(ml + mi, a, D);
	mi += gen_slide_move(ml + mi, a, L);
	mi += gen_slide_move(ml + mi, a, R);
	return mi;
}

// Sort the first mi entries in Move list ml by most valuable victim
// then by least valuable attacker
static void mvv_lva_quicksort(Move *ml, int mi) {
	ASSERT(ml);

	if (mi <= 1) return;

	int pivot = 0;
	int rear_swap = mi - 1;

	while (pivot < rear_swap) {
		PIECE v1 = ml[pivot].cap_piece;
		PIECE v2 = ml[pivot + 1].cap_piece;
		PIECE a1 = board2[ml[pivot].mv.sa];
		PIECE a2 = board2[ml[pivot + 1].mv.sa];
		// swap left if pivot + 1 is a greater capture
		if (v2 > v1 || (v1 == v2 && a2 < a1)) {
			Move t = ml[pivot];
			ml[pivot] = ml[pivot + 1];
			ml[pivot + 1] = t;
			pivot++;
		// swap right if rear_swap != pivot + 1
		} else if (pivot + 1 < rear_swap) {
			Move t = ml[pivot + 1];
			ml[pivot + 1] = ml[rear_swap];
			ml[rear_swap] = t;
			rear_swap--;
		// all elements have been checked
		} else {
			break;
		}
	}

	// sort front
	mvv_lva_quicksort(ml, pivot);
	// sort back if pivot is a capture piece
	// don't care about order of non captures right now
	if (ml[pivot].cap_piece >= WPAWN) 
		mvv_lva_quicksort(ml + pivot + 1, mi - pivot - 1);

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
	for (int addr = 21; addr <= 98; addr++) {
		PIECE p = board2[addr];
		if (p < WPAWN || !check_color(p, side_to_move))
			continue;
		switch (p) {
			case WPAWN:
				mi += gen_wp_moves(ml + mi, addr);
				break;
			case BPAWN:
				mi += gen_bp_moves(ml + mi, addr);
				break;
			case WKNIGHT:
			case BKNIGHT:
				mi += gen_knight_moves(ml + mi, addr);
				break;
			case WBISHOP:
			case BBISHOP:
				mi += gen_bishop_moves(ml + mi, addr);
				break;
			case WROOK:
			case BROOK:
				mi += gen_rook_moves(ml + mi, addr);
				break;
			case WQUEEN:
			case BQUEEN:
				mi += gen_queen_moves(ml + mi, addr);
				break;
			case WKING:
			case BKING:
				mi += gen_king_moves(ml + mi, addr);
				break;
			default:
				ASSERT(0);
		}
	}

	// place a non capture at front of list to be pivot in quicksort
	for (int i = 0; i < mi; i++) {
		if (ml[i].cap_piece < WPAWN) {
			Move t = ml[0];
			ml[0] = ml[i];
			ml[i] = t;
			break;
		}
	}

	mvv_lva_quicksort(ml, mi);
	return mi;
}

// move piece at source address to target address
// helper function for make_move()
static void move_piece(int sa, int ta) {
	ASSERT(on_board(ta));
	ASSERT(on_board(sa));

	PIECE p = board2[sa];
	hash_piece(p, sa);
	board2[ta] = p;
	board2[sa] = EMPTY;
	hash_piece(p, ta);
}

// move piece from source address to target address
// helper function for unmake_move()
//
// same as move_piece except without hashing as
// unmake_move() handles that itself
static void unmove_piece(int sa, int ta) {
	ASSERT(on_board(ta));
	ASSERT(on_board(sa));
	board2[ta] = board2[sa];
	board2[sa] = EMPTY;
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
	PIECE ept = board2[e];

	if (ept == WKING) wking_addr = s;
	else if (ept == BKING) bking_addr = s;

	ASSERT(on_board(s));
	ASSERT(on_board(e));
	ASSERT(board2[s] == EMPTY);
	ASSERT(ept >= WPAWN && ept <= BKING);

	// ummove piece and handle special cases
	//
	// promotion
	if (move->mv.promo != EMPTY) {
		ASSERT(move->mv.promo != OFF_BOARD);
		COLOR c = CMASK & ept;
		ASSERT(c == WHITE || c == BLACK);
		ASSERT(ept > WPAWN || ept <= BQUEEN);

		--material_counts[ept];
		if (c == WHITE) {
			board2[e] = WPAWN;
			++material_counts[WPAWN];
		} else {
			board2[e] = BPAWN;
			++material_counts[BPAWN];
		}
	} 
	// undo kingside castle
	else if ((ept == WKING || ept == BKING) && e == s + 2) {
		unmove_piece(e - 1, e + 1);
	}
	// undo queenside castle
	else if ((ept == WKING || ept == BKING) && e == s - 2) {
		unmove_piece(e + 1, e - 2);
	}
	// standard move
	unmove_piece(e, s);

	// uncapture piece (or replace with NONE if no capture occurred)
	PIECE cap_piece = move->cap_piece;
	if (cap_piece != EMPTY){
		ASSERT(cap_piece != OFF_BOARD);
		++material_counts[cap_piece];

		int cap_addr = e;
		if (e == ep_addr) {
			ASSERT(board2[s] == WPAWN || board2[s] == BPAWN);
			ASSERT(cap_piece == WPAWN || cap_piece == BPAWN);
			if (check_color(cap_piece, WHITE)) cap_addr -= 10;
			else cap_addr += 10;
		}
		board2[cap_addr] = cap_piece;
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
	PIECE spt = board2[s];

	ASSERT(on_board(s));
	ASSERT(on_board(e));

	// track king square
	if (spt == WKING) wking_addr = e;
	else if (spt == BKING) bking_addr = e;

	// capture piece
	if (move->cap_piece != EMPTY) {
		ASSERT(		move->cap_piece >= WPAWN && 
				move->cap_piece <= BKING &&
				!check_color(move->cap_piece, side_to_move));

		int cap_addr = e;
		if (e == ep_addr) {
			if (side_to_move == WHITE) cap_addr += 10;
			else cap_addr -= 10;
			ASSERT((addr_to_rank(cap_addr) == 5 && side_to_move == WHITE) || 
					(addr_to_rank(cap_addr) == 4 && side_to_move == BLACK));
		}
		hash_piece(move->cap_piece, cap_addr);
		board2[cap_addr] = EMPTY;
		--material_counts[move->cap_piece];
	}
	// move piece in mailbox and handle special cases
	//
	// promotion
	PIECE promo = move->mv.promo;
	if (promo != EMPTY) {
		// white assertions
		ASSERT(		side_to_move == BLACK || 
				(spt == WPAWN && 
				 addr_to_rank(s) == 7 && 
				 addr_to_rank(e) == 8));
		ASSERT(		side_to_move == BLACK || 
				(promo >= WKNIGHT && 
				 promo <= WQUEEN && 
				 check_color(promo, WHITE)));
		// black assertions
		ASSERT(		side_to_move == WHITE || 
				(spt == BPAWN && 
				 addr_to_rank(s) == 2  && 
				 addr_to_rank(e) == 1));
		ASSERT(		side_to_move == WHITE || 
				(promo >= BKNIGHT && 
				 promo <= BQUEEN &&
				 check_color(promo, BLACK)));

		move_piece(s, e);
		// hash out piece because we are promoting
		hash_piece(spt, e);
		--material_counts[spt];

		board2[e] = promo;
		++material_counts[promo];
		hash_piece(promo, e);
	} 
	// white kingside castle
	else if (spt == WKING && e == s + 2) {
		ASSERT(castle_rights & K_CASTLE);
		ASSERT(board2[s + 1] == EMPTY && board2[s + 2] == EMPTY);
		ASSERT(s == 95);
		ASSERT(board2[98] == WROOK);

		move_piece(e + 1, e - 1);
		move_piece(s, e);

		hash_castle();
		castle_rights ^= K_CASTLE;
		castle_rights &= ~Q_CASTLE;
		hash_castle();
	}
	// white queenside castle
	else if (spt == WKING && e == s - 2) {
		ASSERT(castle_rights & Q_CASTLE);
		ASSERT(board2[s - 1] == EMPTY && 
				board2[s - 2] == EMPTY && 
				board2[s - 3] == EMPTY);
		ASSERT(s == 95);
		ASSERT(board2[91] == WROOK);

		move_piece(e - 2, e + 1);
		move_piece(s, e);

		hash_castle();
		castle_rights ^= Q_CASTLE;
		castle_rights &= ~K_CASTLE;
		hash_castle();
	}
	// black kingside castle
	else if (spt == BKING && e == s + 2) {
		ASSERT(castle_rights & k_CASTLE);
		ASSERT(board2[s + 1] == EMPTY && board2[s + 2] == EMPTY);
		ASSERT(s == 25);
		ASSERT(board2[28] == BROOK);

		move_piece(e + 1, e - 1);
		move_piece(s, e);

		hash_castle();
		castle_rights ^= k_CASTLE;
		castle_rights &= ~q_CASTLE;
		hash_castle();
	}
	// black queenside castle
	else if (spt == BKING && e == s - 2) {
		ASSERT(castle_rights & q_CASTLE);
		ASSERT(board2[s - 1] == EMPTY && board2[s - 2] == EMPTY && board2[s - 3] == EMPTY);
		ASSERT(s == 25);
		ASSERT(board2[21] == BROOK);

		move_piece(e - 2, e + 1);
		move_piece(s, e);

		hash_castle();
		castle_rights ^= q_CASTLE;
		castle_rights &= ~k_CASTLE;
		hash_castle();
	}
	// standard move
	else {
		move_piece(s, e);
	}
	ASSERT(board2[s] == EMPTY);

	// increment fullmove and halfmove if applicable
	if (side_to_move == BLACK) moves++;
	ASSERT(move->cap_piece != OFF_BOARD);
	if (move->cap_piece != EMPTY || spt == WPAWN || spt == BPAWN)
		halfmoves = 0;
	else
		halfmoves++;

	// update en passant square
	if (ep_addr != -1) hash_ep();
	if (spt == WPAWN && addr_to_rank(s) == 2 && e == s - 20) {
		ep_addr = s - 10;
		hash_ep();
	} else if (spt == BPAWN && addr_to_rank(s) == 7 && e == s + 20) {
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
		if (spt == WKING) {
			castle_rights &= ~K_CASTLE;
			castle_rights &= ~Q_CASTLE;
		} else if (spt == BKING) {
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
	side_to_move ^= 1u;

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
				wp = WQUEEN;
				bp = BQUEEN;
				break;
			case 'r':
				wp = WROOK;
				bp = BROOK;
				break;
			case 'b':
				wp = WBISHOP;
				bp = BBISHOP;
				break;
			case 'n':
				wp = WKNIGHT;
				bp = BKNIGHT;
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
