#include "engine.h"
#include "core.h"
#include "string_processing.h"
#include "eval.h"

#include <stdio.h>
#include <string.h>

///////////////////////////////////////////////////////////////////////////////
/// TYPES

// used for lower and upper bounds in search
// can be any value as long as sufficiently large
#define N_INF -999999
#define INF 999999
// program behavior is undefined if these are too small
#define MAX_VARIATION_LEN 256	// max variation length (depth)
#define INFO_BUFF_SIZE 256	// max size of info string output

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
	Variation* pv;	// output principle variation
	Variation* prio;// priority variation, check these moves first
	char is_top;	// is top level of search
} AB_Params;

// Result of search call
typedef struct {
	int eval;	// eval from side to move's perspective
	char halt;	// did search halt (either timeout or stop request)
} Search_Result;

////////////////////////////////////////////////////////////////////////////////
/// Static Search Variables
static ULL nodes_since_info_send;		// nodes since last time node info was sent
static ULL nodes_searched;			// number of nodes searched in current search
static ULL search_start_time;			// start time of search

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

////////////////////////////////////////////////////////////////////////////////
/// FUNCTIONS

// return table entry of current position
static TT_Entry tt_search(void) {
	return ttable[game_hash & TTMOD];
}

// add current position to transposition table
static void tt_add(NODE_TYPE nt, int depth, int eval) {
	ULL i = game_hash & TTMOD;
	if (ttable[i].depth < depth)
		ttable[i] = (TT_Entry){1, game_hash, (char)depth, nt, eval};
}

// sends just node info
static void send_node_info(void) {
	ULL time_elapsed = platform_get_time_ms() - search_start_time;
	if (!time_elapsed) time_elapsed = 1;
	ULL nps = nodes_searched * 1000ULL / time_elapsed;

	char info_buff[INFO_BUFF_SIZE];	
	// format info string
	snprintf(info_buff, INFO_BUFF_SIZE, "info nodes %lld nps %lld \n", nodes_searched, nps);
	platform_send_uci_command(info_buff);
}

// "quiet search"
static int quiesce(int alpha, int beta, char is_timed, ULL ms_cutoff) {
	int static_eval = evaluate();
	nodes_searched++;
	if(++nodes_since_info_send >= 10000000ULL) {
		nodes_since_info_send = 0ULL;
		send_node_info();
	}

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
		if ((is_timed && platform_get_time_ms() > ms_cutoff) || uci_halt_requested)
			return best_value;
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
	Variation* prio = params.prio;

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

	// setup priority move
	// maintain captures at front of move list
	if (prio->count > 0 && n_moves > 0) {
		int prio_pos = -1;
		int non_cap_pos = -1;
		Move t = ml[0];
		for (int i = 0; i < n_moves; i++) {
			if (ml[i].cap_id == E_ID) {
				non_cap_pos = i;
			}

			if (		ml[i].mv.sa == prio->line[0].sa && 
					ml[i].mv.ta == prio->line[0].ta &&
					ml[i].mv.promo == prio->line[0].promo) {
				prio_pos = i;
				break;
			}
		}

		if (prio_pos == -1) {
			prio->count = 0;
		} else {
			ml[0] = ml[prio_pos];
			if (non_cap_pos <= 0) {
				ml[prio_pos] = t;
			} else {
				Move t2 = ml[non_cap_pos];
				ml[non_cap_pos] = t;
				ml[prio_pos] = t2;
			}
			prio->count--;
			memmove(prio->line, prio->line + 1, prio->count* sizeof(MMove));
		}
	}


	// maximize eval
	for (int i = 0; i < n_moves; i++) {


		if (params.is_top && depth > 6) {
			char info_buff[INFO_BUFF_SIZE];
			snprintf(info_buff, INFO_BUFF_SIZE, "info currmove ");
			mmove_to_str(ml[i].mv, info_buff + 14);

			int p;
			if (ml[i].mv.promo == EMPTY) p = 0;
			else p = 1;
			snprintf(info_buff + 19 + p, INFO_BUFF_SIZE, "currmovenumber %d\n", i + 1);
			platform_send_uci_command(info_buff);
		}

		make_move(ml + i);
		AB_Params p = {
			depth - 1, 
			-beta, 
			-alpha, 
			is_timed, 
			ms_cutoff,
			&curr_var,
			prio,
			0};
		Search_Result temp_res = ab_search(p);
		unmake_move(ml + i);

		if (temp_res.halt) {
			res.halt = 1;
			if (params.is_top)
				res.eval = best_score;
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

// send search info
static void send_pv(int depth, int cp, ULL time_elapsed, ULL nodes, ULL nps, Variation* pv) {
	ASSERT(pv);
	char info_buff[INFO_BUFF_SIZE];	

	// format info string
	snprintf(info_buff, INFO_BUFF_SIZE, "info depth %d score cp %d time %lld nodes %lld nps %lld pv ",
			depth, cp, time_elapsed, nodes, nps);

	// extract principle variation
	int pos = strlen(info_buff);
	for (int i = 0; i < pv->count; i++) {
		mmove_to_str(pv->line[i], info_buff + pos);
		if (info_buff[pos + 4] == ' ') {
			pos += 5;
		} else {
			info_buff[pos + 5] = ' ';
			pos += 6;
		}
	}
	info_buff[pos - 1] = '\n';
	info_buff[pos] = '\0';

	platform_send_uci_command(info_buff);
}

static void send_bestmove(MMove bestmove) {
	char info_buff[16];
	cpy_chars(info_buff, "bestmove ", 9);
	mmove_to_str(bestmove, info_buff + 9);
	if (info_buff[13] == ' ') {
		info_buff[13] = '\n';
		info_buff[14] = '\0';
	} else {
		info_buff[14] = '\n';
		info_buff[15] = '\0';
	}
	platform_send_uci_command(info_buff);
}

void iterative_ab_search(ULL search_time) {
	nodes_searched = 0ULL;
	search_start_time = platform_get_time_ms();
	nodes_since_info_send = 0ULL;

	ULL ms_cutoff = search_time + search_start_time;

	char is_timed = 1;
	if (search_time == 0) is_timed = 0;
	int depth = 1;
	const int asp_window = 35;
	int alpha = N_INF - depth - 1 - asp_window;
	int beta = INF + depth + 1 + asp_window;

	Search_Result prev_res = {0, 1};
	Variation prev_pv;
	prev_pv.count = 0;
	prev_pv.line[0] = (MMove){-1, -1, EMPTY};

	while(1) {
		Variation curr_pv;
		curr_pv.count = 0;
		Variation prio_pv;
		memcpy(prio_pv.line, prev_pv.line, prev_pv.count * sizeof(MMove));
		prio_pv.count = prev_pv.count;

		AB_Params p = {
			depth, 
			alpha,
			beta,
			is_timed, 
			ms_cutoff,
			&curr_pv,
			&prio_pv,
			1};
		Search_Result res = ab_search(p);

		// statistics
		ULL time_elapsed = platform_get_time_ms() - search_start_time;
		if (!time_elapsed) time_elapsed = 1;
		ULL nps = nodes_searched * 1000ULL / time_elapsed;
		int cp = res.eval;

		if (res.halt) {
			if (curr_pv.count < depth) {
				res = prev_res;
				memcpy(curr_pv.line, prev_pv.line, prev_pv.count * sizeof(MMove));
				curr_pv.count = prev_pv.count;
				cp = res.eval;
			}

			send_pv(depth, cp, time_elapsed, nodes_searched, nps, &curr_pv);
			send_bestmove(curr_pv.line[0]);
			return;
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
		memcpy(prev_pv.line, curr_pv.line, curr_pv.count * sizeof(MMove));
		prev_pv.count = curr_pv.count;

		// send info
		send_pv(depth, cp, time_elapsed, nodes_searched, nps, &curr_pv);

		// checkmate
		if (res.eval >= INF || res.eval <= N_INF) {
			send_bestmove(curr_pv.line[0]);
			return;
		}

		alpha = res.eval - asp_window;
		beta = res.eval + asp_window;
		depth++;
	}
}

void wipe_tt(void) {
	for (int i = 0; i < TTABLE_SIZE; i++) {
		ttable[i].active = 0;
	}
}
