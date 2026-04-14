// calculate the number of pseudo legal white pawn moves
static int wp_mobility(void) {
	int n_moves = 0;
	int n_wp = p_count(WP);

	for (int i = 0; i < n_wp; i++) {
		int id = piece_ids[WP - 2][i];
		int a = piece_addr[id];
		ASSERT(addr_to_rank(a) <= 7);
		ASSERT(on_board(a - 10));

		// advance
		if (pt_at_addr(a - 10) == EMPTY) {
			if (addr_to_rank(a) == 7) {
				n_moves += 4;
			} else {
				n_moves++;
				if (addr_to_rank(a) == 2 && pt_at_addr(a - 20) == EMPTY) {
					n_moves++;
				}
			}
		}
		// captures
		if (piece_color[board[a - 9]] == BLACK || ep_addr == a - 9) {
			if (addr_to_rank(a) == 7) {
				n_moves += 4;
			} else {
				n_moves++;
			}
		}
		if (piece_color[board[a - 11]] == BLACK || ep_addr == a - 11) {
			if (addr_to_rank(a) == 7) {
				n_moves += 4;
			} else {
				n_moves++;
			}
		}
	}
	return n_moves;
}

// calculate the number of pseudo legal black pawn moves
static int bp_mobility(void) {
	int n_moves = 0;
	int n_bp = p_count(BP);

	for (int i = 0; i < n_bp; i++) {
		int id = piece_ids[BP - 2][i];
		int a = piece_addr[id];
		ASSERT(addr_to_rank(a) <= 7);
		ASSERT(on_board(a + 10));

		// advance
		if (pt_at_addr(a + 10) == EMPTY) {
			if (addr_to_rank(a) == 2) {
				n_moves += 4;
			} else {
				n_moves++;
				if (addr_to_rank(a) == 7 && pt_at_addr(a + 20) == EMPTY) {
					n_moves++;
				}
			}
		}
		// captures
		if (piece_color[board[a + 9]] == WHITE || ep_addr == a + 9) {
			if (addr_to_rank(a) == 2) {
				n_moves += 4;
			} else {
				n_moves++;
			}
		}
		if (piece_color[board[a + 11]] == WHITE || ep_addr == a + 11) {
			if (addr_to_rank(a) == 2) {
				n_moves += 4;
			} else {
				n_moves++;
			}
		}
	}
	return n_moves;
}

// calculate number of pseudo legal knight moves for side color
static int knight_mobility(COLOR c) {
	ASSERT(c == WHITE || c == BLACK);

	int n_moves = 0;
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
			n_moves++;
		if (piece_color[board[a - 19]] != c && pt_at_addr(a - 19) != OFF_BOARD)
			n_moves++;
		if (piece_color[board[a - 12]] != c && pt_at_addr(a - 12) != OFF_BOARD)
			n_moves++;
		if (piece_color[board[a - 8]] != c && pt_at_addr(a - 8) != OFF_BOARD)
			n_moves++;
		if (piece_color[board[a + 21]] != c && pt_at_addr(a + 21) != OFF_BOARD)
			n_moves++;
		if (piece_color[board[a + 19]] != c && pt_at_addr(a + 19) != OFF_BOARD)
			n_moves++;
		if (piece_color[board[a + 12]] != c && pt_at_addr(a + 12) != OFF_BOARD)
			n_moves++;
		if (piece_color[board[a + 8]] != c && pt_at_addr(a + 8) != OFF_BOARD)
			n_moves++;
	}
	return n_moves;
}

// calculate number of pseudo legal king moves for side color
static int king_mobility(COLOR c) {
	ASSERT(c == WHITE || c == BLACK);

	int n_moves = 0;
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
		n_moves++;
	if (piece_color[board[a - 10]] != c && pt_at_addr(a - 10) != OFF_BOARD)
		n_moves++;
	if (piece_color[board[a - 9]] != c && pt_at_addr(a - 9) != OFF_BOARD)
		n_moves++;
	if (piece_color[board[a - 1]] != c && pt_at_addr(a - 1) != OFF_BOARD)
		n_moves++;
	if (piece_color[board[a + 1]] != c && pt_at_addr(a + 1) != OFF_BOARD)
		n_moves++;
	if (piece_color[board[a + 9]] != c && pt_at_addr(a + 9) != OFF_BOARD)
		n_moves++;
	if (piece_color[board[a + 10]] != c && pt_at_addr(a + 10) != OFF_BOARD)
		n_moves++;
	if (piece_color[board[a + 11]] != c && pt_at_addr(a + 11) != OFF_BOARD)
		n_moves++;

	if (c == WHITE) {
		if (		castle_rights & K_CASTLE && 
				pt_at_addr(a + 1) == EMPTY && 
				pt_at_addr(a + 2) == EMPTY && 
				!is_square_attacked(a, BLACK) && 
				!is_square_attacked(a + 1, BLACK)) {
			ASSERT(pt_at_addr(98) == WR);
			ASSERT(pt_at_addr(95) == WK);
			n_moves++;
		}
		if (		castle_rights & Q_CASTLE && 
				pt_at_addr(a - 1) == EMPTY && 
				pt_at_addr(a - 2) == EMPTY && 
				pt_at_addr(a - 3) == EMPTY &&
				!is_square_attacked(a, BLACK) && 
				!is_square_attacked(a - 1, BLACK)) {
			ASSERT(pt_at_addr(91) == WR);
			ASSERT(pt_at_addr(95) == WK);
			n_moves++;
		}
	} else {
		if (		castle_rights & k_CASTLE && 
				pt_at_addr(a + 1) == EMPTY && 
				pt_at_addr(a + 2) == EMPTY && 
				!is_square_attacked(a, WHITE) && 
				!is_square_attacked(a + 1, WHITE)) {
			ASSERT(pt_at_addr(28) == BR);
			ASSERT(pt_at_addr(25) == BK);
			n_moves++;
		}
		if (		castle_rights & q_CASTLE && 
				pt_at_addr(a - 1) == EMPTY && 
				pt_at_addr(a - 2) == EMPTY && 
				pt_at_addr(a - 3) == EMPTY &&
				!is_square_attacked(a, WHITE) && 
				!is_square_attacked(a - 1, WHITE)) {
			ASSERT(pt_at_addr(21) == BR);
			ASSERT(pt_at_addr(25) == BK);
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
	ASSERT(		pt_at_addr(a) == WQ || 
			pt_at_addr(a) == BQ || 
			pt_at_addr(a) == WB || 
			pt_at_addr(a) == BB || 
			pt_at_addr(a) == WR || 
			pt_at_addr(a) == BR);

	COLOR c = piece_color[board[a]];
	int n_moves = 0;
	int off = d;
	while (1) {
		int id = board[a + off];
		if (piece_type[id] == EMPTY) {
			n_moves++;
			off += d;
		} else if (piece_color[id] == c) {
			n_moves++;
			break;
		} else {
			break;
		}
	}
	return n_moves;
}

// calculate number of pseudo legal bishop moves for side color
static int bishop_mobility(COLOR c) {
	int n_moves = 0;
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
		n_moves += slide_mobility(a, UR);
		n_moves += slide_mobility(a, UL);
		n_moves += slide_mobility(a, DR);
		n_moves += slide_mobility(a, DL);
	}
	return n_moves;
}

// calculate number of pseudo legal rook moves for side color
static int rook_mobility(COLOR c) {
	int n_moves = 0;
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
		n_moves += slide_mobility(a, U);
		n_moves += slide_mobility(a, D);
		n_moves += slide_mobility(a, L);
		n_moves += slide_mobility(a, R);
	}
	return n_moves;
}

// calculate number of pseudo legal queen moves for side color
static int queen_mobility(COLOR c) {
	int n_moves = 0;
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
	wm += king_mobility(WHITE);
	bm += king_mobility(BLACK);
	wm += bishop_mobility(WHITE);
	bm += bishop_mobility(BLACK);
	wm += rook_mobility(WHITE);
	bm += rook_mobility(BLACK);
	wm += queen_mobility(WHITE);
	bm += queen_mobility(BLACK);

	return wm - bm;
}

// return 1 iff position current position has already been reached
// otherwise return 0
static int is_repeat_position(void) {
	// only need to check since last capture or pawn advance
	// only need to check every other position
	if (halfmoves < 4) return 0;

	// i = number of moves back
	for (int i = 2; i <= halfmoves; i += 2) {
		if (history[history_cnt - 1 - i] < 0)
			return 0;
		if (history[history_cnt - 1 - i] == game_hash)
			return 1;
	}
	return 0;
}

// Return an avaluation of the pawn structure in centipawns
// Evaluation is from color 'c's perspective
static int pawn_structure_eval(COLOR c) {
	ASSERT(c == WHITE || c == BLACK);
	int *pid_arr;
	int np;
	if (c == WHITE) {
		pid_arr = piece_ids[WP - 2];
		np = p_count(WP);
	} else {
		pid_arr = piece_ids[BP - 2];
		np = p_count(BP);
	}

	char file_count[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	int score = 0;
	for (int i = 0; i < np; i++) {
		int id = pid_arr[i];
		int a = piece_addr[id];
		int rank = addr_to_rank(a);
		if (c == BLACK) rank -= 9;
		file_count[addr_to_file(a) - 1]++;

		switch (rank) {
			case 4:
				score += 1;
			case 5:
				score += 2;
			case 6:
				score += 4;
			case 7:
				score += 6;
			default:;
		}
	}

	const int doubled = 5;
	const int isolated = 5;
	for (int i = 0; i < 8; i++) {
		int cnt = file_count[i];
		if (cnt) {
			score -= (cnt - 1) * doubled;

			if (i == 0 && !file_count[1])
				score -= cnt * isolated;
			else if (i == 7 && !file_count[6])
				score -= cnt * isolated;
			else if (!file_count[i - 1] && !file_count[i + 1])
				score -= cnt * isolated;
		}
	}
	return score;
}

// Evaluate position from side_to_move's perspective
// in centipawns
// TODO there is redundant processing here
// 	I have left it for simplicity but optimize when needed
int evaluate(void) {
	// material values
	const int K = 20000;
	const int Q = 900;
	const int R = 500;
	const int B = 300;
	const int N = 300;
	const int P = 100;
	const int MOB = 5;

	if (halfmoves == 50) return 0;
	int score = K * (material_counts[WK - 2] - material_counts[BK - 2]) +
		Q * (material_counts[WQ - 2] - material_counts[BQ - 2]) +
		R * (material_counts[WR - 2] - material_counts[BR - 2]) +
		B * (material_counts[WB - 2] - material_counts[BB - 2]) +
		N * (material_counts[WN - 2] - material_counts[BN - 2]) +
		P * (material_counts[WP - 2] - material_counts[BP - 2]);

	score += MOB * mobility();

	score += pawn_structure_eval(WHITE);
	score -= pawn_structure_eval(BLACK);

	if (side_to_move == BLACK) score *= -1;
	return score;
}
