#include "engine.h"
#include "core.h"

#include <stdio.h>
#include <string.h>

#include "eval.c"

////////////////////////////////////////////////////////////////////////////////
/// TYPES

// used for lower and upper bounds in search
// can be any value as long as sufficiently large
#define N_INF -999999
#define INF 999999
#define INFO_BUFF_SIZE 256	// size of buffer for info output commands
#define MAX_VARIATION_LEN 16	// max variation length (depth)

// Contains variation line
typedef struct {
	int count;
	MMove line[MAX_VARIATION_LEN];
} Variation;

// search parameters
typedef struct {
	int depth;	// search depth
	int alpha;	// upper/lower bounds (don't set by hand)
	int beta;
	char is_timed;	// is search timed
	ULL ms_cutoff;	// if search is timed at what time to timeout
			// (compared to result of platform_get_time_ms())
	Variation* pv;	// principle variation
} AB_Params;

// Result of search call
typedef struct {
	int eval;	// eval from side to move's perspective
	char halt;	// did search halt (either timeout or stop request)
} Search_Result;

// return table entry of current position
static TT_Entry tt_search(void) {
	return ttable[game_hash & TTMOD];
}

// add current position to transposition table
static void tt_add(NODE_TYPE nt, int depth, int eval) {
	ULL i = game_hash & TTMOD;
	ttable[i].active = 1;
	ttable[i].hash = game_hash;
	ttable[i].depth = (char)depth;
	ttable[i].nt = nt;
	ttable[i].eval = eval;
}

static int quiesce(int alpha, int beta, char is_timed, ULL ms_cutoff) {
	int static_eval = evaluate();
	nodes_searched++;

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
	Variation* pv = params.pv;

	ASSERT(is_timed == 0 || ms_cutoff > 0);
	Search_Result res = {0, 0};

	if (is_repeat_position()) {
		res.eval = 0;
		pv->count = 0;
		return res;
	}
	if (depth == 0) {
		res.eval = quiesce(alpha, beta, is_timed, ms_cutoff);
		pv->count = 0;
		return res;
	}
	if ((is_timed && platform_get_time_ms() > ms_cutoff) || uci_halt_requested) {
		res.halt = 1;
		pv->count = 0;
		return res;
	}

	// check transposition table
	TT_Entry te = tt_search();
	if (te.active && te.hash == game_hash && te.depth >= depth) {
		if (te.nt == LB && te.eval >= beta) {
			res.eval = te.eval;
			pv->count = 0;
			return res;
		} else if (te.nt == UB && te.eval < alpha) {
			res.eval = te.eval;
			pv->count = 0;
			return res;
		}
	}

	NODE_TYPE nt = PV;
	int best_score = N_INF - depth;
	Move ml[256];
	Variation curr_var;
	int n_moves = gen_moves(ml);

	/*
	 * TODO replace
	// setup priority move
	if (prio_sa != -1) {
		ASSERT(on_board(prio_sa) && on_board(prio_ta));
		ASSERT(prio_promo >= EMPTY && prio_promo <= BK);

		if (init_move(ml + n_moves, prio_sa, prio_ta)) {
			ml[n_moves].mv.promo = prio_promo;
		}

		Move t = ml[0];
		ml[0] = ml[n_moves];
		ml[n_moves] = t;
		n_moves++;
	}
	*/

	// maximize eval
	for (int i = 0; i < n_moves; i++) {
		/*
		 * TODO replace
		// skip second instance of prio move
		if (		i > 0 && prio_sa == ml[i].mv.sa && 
				prio_ta == ml[i].mv.ta && prio_promo == ml[i].mv.promo)
			continue;
			*/

		make_move(ml + i);
		AB_Params p = {
			depth - 1, 
			-beta, 
			-alpha, 
			is_timed, 
			ms_cutoff,
			&curr_var};
		Search_Result temp_res = ab_search(p);
		unmake_move(ml + i);

		if (temp_res.halt) {
			res.halt = 1;
			return res;
		}

		int score = -temp_res.eval;
		if (score > best_score) {
			best_score = score;
			pv->line[0] = (MMove){ml[i].mv.sa, ml[i].mv.ta, ml[i].mv.promo};
			memcpy(pv->line + 1, curr_var.line, curr_var.count * sizeof(MMove));
			pv->count = curr_var.count + 1;

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
	tt_add(nt, depth, best_score);

	return res;
}

MMove iterative_ab_search(ULL search_time) {
	nodes_searched = 0;
	const ULL start_time = platform_get_time_ms();
	ULL ms_cutoff = search_time + start_time;

	char is_timed = 1;
	if (search_time == 0) is_timed = 0;
	int depth = 1;
	const int asp_window = 50;
	int alpha = N_INF - depth - 1 - asp_window;
	int beta = INF + depth + 1 + asp_window;

	Search_Result prev_res = {0, 1};
	Variation prev_pv;
	prev_pv.count = 1;
	prev_pv.line[0] = (MMove){-1, -1, EMPTY};

	char info_buff[INFO_BUFF_SIZE];	
	while(1) {
		Variation curr_pv;
		AB_Params p = {
			depth, 
			alpha,
			beta,
			is_timed, 
			ms_cutoff,
			&curr_pv};
		Search_Result res = ab_search(p);
		// statistics
		ULL time_elapsed = platform_get_time_ms() - start_time;
		if (!time_elapsed) time_elapsed = 1;
		ULL nps = nodes_searched * 1000ULL / time_elapsed;
		float cp = (float)res.eval / 100.0f;
		if (side_to_move == BLACK) cp = -cp;

		if (res.halt) {
			// send info
			if (curr_pv.line[0].sa == -1 || curr_pv.line[0].ta == -1) {
				res = prev_res;
				curr_pv = prev_pv;
				cp = (float)res.eval / 100.0f;
				if (side_to_move == BLACK) cp = -cp;
			}

			snprintf(info_buff, INFO_BUFF_SIZE, "info depth %d score cp %.2f time %lld nodes %lld nps %lld\n", 
					--depth, cp, time_elapsed, nodes_searched, nps);
			platform_send_uci_command(info_buff);
			return curr_pv.line[0];
		}

		if (res.eval <= alpha) {
			alpha -= asp_window;
			continue;
		}
		if (res.eval >= beta) {
			beta += asp_window;
			continue;
		}
		prev_res = res;
		prev_pv = curr_pv;

		// send info
		snprintf(info_buff, INFO_BUFF_SIZE, "info depth %d score cp %.2f time %lld nodes %lld nps %lld pv ", 
				depth, cp, time_elapsed, nodes_searched, nps);
		int info_pos = strlen(info_buff);
		for (int pv_move = 0; pv_move < curr_pv.count; pv_move++) {
			move_to_str(curr_pv.line[pv_move], info_buff + info_pos);
			if (info_buff[info_pos + 4] == ' ')
				info_pos += 5;
			else
				info_pos += 6;
		}
		info_buff[info_pos - 1] = '\n';
		info_buff[info_pos] = '\0';

		platform_send_uci_command(info_buff);
		// checkmate
		if (res.eval >= INF || res.eval <= N_INF) 
			return curr_pv.line[0];

		alpha = res.eval - asp_window;
		beta = res.eval + asp_window;
		depth++;
	}
}
