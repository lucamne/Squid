#include "engine.h"
#include "core.h"

#include <stdio.h>

#include "eval.c"

////////////////////////////////////////////////////////////////////////////////
/// TYPES

// used for lower and upper bounds in search
// can be any value as long as sufficiently large
#define N_INF -999999
#define INF 999999


// search parameters
typedef struct {
	int depth;	// search depth
	int alpha;	// upper/lower bounds (don't set by hand)
	int beta;
	char is_timed;	// is search timed
	ULL ms_cutoff;	// if search is timed at what time to timeout
			// (compared to result of platform_get_time_ms())

	// characteristics to describe a priority move if any
	// addresses should be set to -1 if no priority move
	// priority move will be searched first
	int prio_sa;
	int prio_ta;
	PIECE prio_promo;
} AB_Params;

// return table entry of current position
static TT_Entry tt_search(void) {
	return ttable[game_hash & TTMOD];
}

// add current position to transposition table
static void tt_add(NODE_TYPE nt, int depth, int eval, int sa, int ta, PIECE promo) {
	ULL i = game_hash & TTMOD;
	ttable[i].active = 1;
	ttable[i].hash = game_hash;
	ttable[i].depth = (char)depth;
	ttable[i].nt = nt;
	ttable[i].eval = eval;
	ttable[i].sa = (char)sa;
	ttable[i].ta = (char)ta;
	ttable[i].promo = promo;
}

static int quiesce(int alpha, int beta, char is_timed, ULL ms_cutoff) {
	int static_eval = evaluate();

	// stand pat
	int best_value = static_eval;
	if (best_value >= beta)
		return best_value;
	if (best_value > alpha)
		alpha = best_value;

	if ((is_timed && platform_get_time_ms() > ms_cutoff) || uci_halt_requested)
		return best_value;

	Move ml[256];
	int n_moves = gen_moves(ml);
	for (int i = 0; i < n_moves; i++) {
		// stop once captures are all examined
		// assume MVV-LVA ordering
		if (ml[i].cap_id == EMPTY)
			break;

		make_move(ml + i);
		int score = -quiesce(-beta, -alpha, is_timed, ms_cutoff);
		unmake_move(ml + i);

		if (score >= beta)
			return score;
		if (score > best_value)
			best_value = score;
		if (score > alpha)
			alpha = score;
	}
	return best_value;
}

// perform alpha beta search to depth, stopping if ms_cutoff is reached
// see AB_Params definition in "search.h" for description of parameters
Search_Result ab_search(AB_Params params) {
	// arguments
	int alpha = params.alpha;
	int beta = params.beta;
	int depth = params.depth;
	char is_timed = params.is_timed;
	ULL ms_cutoff = params.ms_cutoff;
	int prio_sa = params.prio_sa;
	int prio_ta = params.prio_ta;
	PIECE prio_promo = params.prio_promo;

	ASSERT(is_timed == 0 || ms_cutoff > 0);
	Search_Result res = {0, 0, -1, -1, EMPTY};

	if (is_repeat_position()) {
		res.eval = 0;
		return res;
	}
	if (depth == 0) {
		res.eval = quiesce(alpha, beta, is_timed, ms_cutoff);
		return res;
	}
	if ((is_timed && platform_get_time_ms() > ms_cutoff) || uci_halt_requested) {
		res.halt = 1;
		return res;
	}

	// check transposition table
	TT_Entry te = tt_search();
	if (te.active && te.hash == game_hash && te.depth >= depth) {
		if (te.nt == PV) {
			res.eval = te.eval;
			res.sa = te.sa;
			res.ta = te.ta;
			res.promo = te.promo;
			return res;
		} else if (te.nt == LB && te.eval >= beta) {
			res.eval = te.eval;
			res.sa = te.sa;
			res.ta = te.ta;
			res.promo = te.promo;
			return res;
		} else if (te.nt == UB && te.eval < alpha) {
			res.eval = te.eval;
			res.sa = te.sa;
			res.ta = te.ta;
			res.promo = te.promo;
			return res;
		}
	}

	NODE_TYPE nt = PV;
	int best_score = N_INF - depth;
	Move ml[256];
	int n_moves = gen_moves(ml);

	// setup priority move
	if (prio_sa != -1) {
		ASSERT(on_board(prio_sa) && on_board(prio_ta));
		ASSERT(prio_promo >= EMPTY && prio_promo <= BK);

		if (init_move(ml + n_moves, prio_sa, prio_ta)) {
			ml[n_moves].promo = prio_promo;
		}

		Move t = ml[0];
		ml[0] = ml[n_moves];
		ml[n_moves] = t;
		n_moves++;
	}

	// maximize eval
	for (int i = 0; i < n_moves; i++) {
		// skip second instance of prio move
		if (		i > 0 && prio_sa == ml[i].sa && 
				prio_ta == ml[i].ta && prio_promo == ml[i].promo)
			continue;

		make_move(ml + i);
		AB_Params p = {
			depth - 1, 
			-beta, 
			-alpha, 
			is_timed, 
			ms_cutoff, 
			-1, 
			-1, 
			EMPTY};
		Search_Result temp_res = ab_search(p);
		unmake_move(ml + i);

		if (temp_res.halt) {
			res.halt = 1;
			return res;
		}

		int score = -temp_res.eval;
		if (score > best_score) {
			best_score = score;
			res.sa = ml[i].sa;
			res.ta = ml[i].ta;
			res.promo = ml[i].promo;
			if (score > alpha)
				alpha = score;
		}

		if (score >= beta) {
			if (side_to_move == WHITE)
				nt = LB;
			else
				nt = UB;
			break;
		}
	}

	// no moves and king not attacked is stalemate
	if (!n_moves) {
		if (side_to_move == WHITE && !is_square_attacked(piece_addr[piece_ids[WK - 2][0]], BLACK)) {
			best_score = 0;
		} else if (side_to_move == BLACK && !is_square_attacked(piece_addr[piece_ids[BK - 2][0]], WHITE)) {
			best_score = 0;
		}
	}
	res.eval = best_score;
	tt_add(nt, depth, best_score, res.sa, res.ta, res.promo);

	return res;
}

Search_Result iterative_ab_search(ULL search_time) {
	ULL ms_cutoff = search_time + platform_get_time_ms();
	char is_timed = 1;
	if (search_time == 0)
		is_timed = 0;
	int depth = 1;
	Search_Result prev_res = {0, 1, -1, -1, EMPTY};

	const int asp_window = 50;
	int alpha = N_INF - depth - 1 - asp_window;
	int beta = INF + depth + 1 + asp_window;

	char info_buff[32];	
	while(1) {
		AB_Params p = {
			depth, 
			alpha,
			beta,
			is_timed, 
			ms_cutoff, 
			prev_res.sa, 
			prev_res.ta, 
			prev_res.promo};

		Search_Result res = ab_search(p);


		// need to return something even if halt occurs
		if (depth == 1)
			prev_res = res;

		if (res.halt) {
			// send info
			float cp = (float)prev_res.eval / 100.0f;
			int d = depth;
			if (side_to_move == BLACK) cp = -cp;
			if (d > 1) d--;
			snprintf(info_buff, 32, "info depth %d cp %.2f\n", d, cp);
			platform_send_uci_command(info_buff);

			return prev_res;
		}

		if (res.eval <= alpha) {
			alpha -= asp_window;
			continue;
		}

		if (res.eval >= beta) {
			beta += asp_window;
			continue;
		}

		// send info
		float cp = (float)res.eval / 100.0f;
		if (side_to_move == BLACK)
			cp = -cp;
		snprintf(info_buff, 32, "info depth %d cp %.2f\n", depth, cp);
		platform_send_uci_command(info_buff);

		prev_res = res;
		alpha = res.eval - asp_window;
		beta = res.eval + asp_window;
		// checkmate
		if (res.eval >= INF || res.eval <= N_INF)
			return res;
		depth++;
	}
}
