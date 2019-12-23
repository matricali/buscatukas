#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/**
 * Complementary Multiply With Carry generator
 * https://en.wikipedia.org/wiki/Multiply-with-carry_pseudorandom_number_generator
 */
#include "cmwc.h"

pthread_mutex_t cmwc_lock;

/**
 * Make 32 bit random number (some systems use 16 bit RAND_MAX [Visual C 2012 uses 15 bits!])
 */
static uint32_t rand32(void)
{
	uint32_t result = rand();
	return result << 16 | rand();
}

/* Init the state with seed */
void cmwc_init(struct cmwc_state *state, unsigned int seed)
{
	if (pthread_mutex_init(&cmwc_lock, NULL) != 0) {
		fprintf(stderr, "mutex init has failed\n");
		exit(EXIT_FAILURE);
	}

	srand(seed);
	for (int i = 0; i < CMWC_CYCLE; i++)
		state->Q[i] = rand32();
	do
		state->c = rand32();
	while (state->c >= CMWC_C_MAX);
	state->i = CMWC_CYCLE - 1;
}

/* CMWC engine */
uint32_t cmwc_rand(struct cmwc_state *state)
{
	uint64_t const a = 18782;
	uint32_t const m = 0xfffffffe;
	uint64_t t;
	uint32_t x;

	state->i = (state->i + 1) & (CMWC_CYCLE - 1);
	t = a * state->Q[state->i] + state->c;
	/* Let c = t / 0xffffffff, x = t mod 0xffffffff */
	state->c = t >> 32;
	x = t + state->c;
	if (x < state->c) {
		x++;
		state->c++;
	}
	return state->Q[state->i] = m - x;
}

uint32_t cmwc_rand_ts(struct cmwc_state *state)
{
	uint32_t ret;
	pthread_mutex_lock(&cmwc_lock);
	ret = cmwc_rand(state);
	pthread_mutex_unlock(&cmwc_lock);
	return ret;
}
