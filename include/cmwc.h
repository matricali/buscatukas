/**
 * Complementary Multiply With Carry generator
 * https://en.wikipedia.org/wiki/Multiply-with-carry_pseudorandom_number_generator
 */

#ifndef CMWC_H
#define CMWC_H

#define CMWC_CYCLE 4096
#define CMWC_C_MAX 809430660

struct cmwc_state {
	uint32_t Q[CMWC_CYCLE];
	uint32_t c; // must be limited with CMWC_C_MAX
	unsigned i;
};

void cmwc_init(struct cmwc_state *state, unsigned int seed);

uint32_t cmwc_rand(struct cmwc_state *state);

uint32_t cmwc_rand_ts(struct cmwc_state *state);

#endif /* CMWC_H */
