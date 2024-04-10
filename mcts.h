#pragma once

#define ITERATIONS 100000
// Precalculated sqrt(2) -> fix point representation
#define EXPLORATION_FACTOR 0x5a82799a

int mcts(char *table, char player);
