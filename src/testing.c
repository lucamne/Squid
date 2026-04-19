#include <stdio.h>

/*
// convert mailbox address to chess square notation and store in buffer
// assume buffer is allocated and is at least two characters long
// do not assume buffer is null terminated
// assume addr is within range [0-63]
static void addr_to_str(int addr, char* buffer) {
	ASSERT(on_board(addr));
	ASSERT(buffer);

	char rank = (char)addr_to_rank(addr);
	char file = (char)addr_to_file(addr) - 1;
	file += 'a';

	buffer[0] = file;
	buffer[1] = rank + '0';
}
*/

// converts PIECE to ascii char
static char piece_to_char(PIECE p) {
	switch(p) {
		case WPAWN:
			return 'P';
		case WROOK:
			return 'R';
		case WKNIGHT:
			return 'N';
		case WBISHOP:
			return 'B';
		case WQUEEN:
			return 'Q';
		case WKING:
			return 'K';
		case BPAWN:
			return 'p';
		case BROOK:
			return 'r';
		case BKNIGHT:
			return 'n';
		case BBISHOP:
			return 'b';
		case BQUEEN:
			return 'q';
		case BKING:
			return 'k';
		default:
			return ' ';
	}
}

static void print_board(void) {
	printf("\n");
	for(int y = 0; y < 8; y++) {
		printf("   ---------------------------------\n");
		printf(" %i |", 8 - y);
		for (int x = 0; x < 8; x++) {
			printf(" %c |", piece_to_char(board2[(y + 2) * 10 + x + 1]));
		}
		printf("\n");
	}
	printf("   ---------------------------------\n");
	printf("     a   b   c   d   e   f   g   h\n\n");

	// side to move
	printf("To move: ");
	if (side_to_move == WHITE) printf("White\n");
	else printf("Black\n");

	// castling rights
	printf("Castling rights: ");
	if (castle_rights & K_CASTLE) printf("K");
	if (castle_rights & Q_CASTLE) printf("Q");
	if (castle_rights & k_CASTLE) printf("k");
	if (castle_rights & q_CASTLE) printf("q");
	printf("\n");

	// en passant
	printf("En passant square: "); 
	if (ep_addr == -1) {
		printf("-");
	} else {
		char ep_buff[2];
		addr_to_str(ep_addr, ep_buff);
		printf("%c%c", ep_buff[0], ep_buff[1]);
	}
	printf("\n");

	// move counters
	printf("Halfmove clock: %i\n", halfmoves);
	printf("Fullmove count: %i\n", moves);
	printf("\n");
}

/*
void print_move_list(Move *move_list, int num_moves) {
	ASSERT(move_list);
	char buff[2];

	for (int i = 0; i < num_moves; i++) {
		printf("%i: ", i + 1);
		addr_to_str(move_list[i].sa, buff);
		printf("%c%c->", buff[0], buff[1]);
		addr_to_str(move_list[i].ta, buff);
		printf("%c%c\n", buff[0], buff[1]);
	}
}

// play game against ai
// pass 0 to play as white and 1 as black
void play_against_ai(COLOR color) {
	ASSERT(color == WHITE || color == BLACK);
	load(BUG_FEN);

	Move move_list[256];
	char scan_buffer[4];
	int eval = 0;

	while(1) {
		// player turn
		if (side_to_move == color) {
			int n_moves = gen_moves(move_list);
			print_board();
			print_move_list(move_list, n_moves);

			printf("Enter move # to play:");
			scanf("%s", scan_buffer);
			int move_num = extract_int_from_str(scan_buffer, 0) - 1;
			if (move_num < 0 || move_num > n_moves - 1)
				continue;

			make_move(move_list + move_num);
		}
		// ai turn
		else {
			AB_Result res = iterative_ab_search(5000);
			eval = res.eval;

			if (res.sa == -1) {
				if (!eval) printf("Stalemate!\n");
				else printf("Checkmate!\n");
				break;
			}
			if (side_to_move == BLACK) {
				eval *= -1;
			}

			// TODO create a get move function to create a specific move
			Move ai_move;
			Move ml[256];
			int n_moves = gen_moves(ml);

			for (int i = 0; i < n_moves; i++) {
				if (ml[i].sa == res.sa && ml[i].ta == res.ta && ml[i].promo == res.promo) {
					ai_move = ml[i];
					break;
				}
			}

			make_move(&ai_move);
			char buff[2];
			addr_to_str(ai_move.sa, buff);
			printf("AI played %c%c->", buff[0], buff[1]);
			addr_to_str(ai_move.ta, buff);
			printf("%c%c with an evaluation of %.2f\n", buff[0], buff[1], (float)eval / 100.0f);
		}
	}
}

int main(int argc, char **argv) {
	// printf("%i\n", sizeof(TT_Entry));
	// play_against_ai(BLACK);
	// perft_test(5);
	// ASSERT(validate_search(5, STARTING_FEN));
	// search_test(9, STARTING_FEN);

	// load(HARD_PUZZLE_FEN);
	// Search_Result res;
	//int eval = alpha_beta(&res, 5);
	// printf("Eval:%i | %i -> %i\n", eval, res.move.sa, res.move.ta);

	return 0;
} */

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
		ULL st = platform_get_time_ms();
		unsigned long long n_nodes = perft(depth);
		ULL et = platform_get_time_ms();

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
