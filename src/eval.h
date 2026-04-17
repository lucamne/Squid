#ifndef EVAL_H
#define EVAL_H

// Evaluate position from side_to_move's perspective
// in centipawns
int evaluate(void);

// Initialize evaluation data
// Should be called once at program intialization
void init_eval(void);

// Return 1 if position has already occurred
// Return 0 if it has not
int is_repeat_position(void);

#endif
