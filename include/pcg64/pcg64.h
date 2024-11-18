/*
 * PCG64 Random Number Generation for C.
 *
 * Copyright 2014 Melissa O'Neill <oneill@pcg-random.org>
 * Copyright 2015 Robert Kern <robert.kern@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * For additional information about the PCG random number generation scheme,
 * including its license and other licensing options, visit
 *
 *     http://www.pcg-random.org
 *
 * Relicensed MIT in May 2019
 *
 * The MIT License
 *
 * PCG Random Number Generation for C.
 *
 * Copyright 2014 Melissa O'Neill <oneill@pcg-random.org>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef PCG64_H_INCLUDED
#define PCG64_H_INCLUDED 1

#include <inttypes.h>
#if __GNUC_GNU_INLINE__ && !defined(__cplusplus)
#error Nonstandard GNU inlining semantics. Compile with -std=c99 or better.
#endif

#if __cplusplus
extern "C"
{
#endif

  typedef __uint128_t pcg128_t;
#define PCG_128BIT_CONSTANT(high, low) (((pcg128_t)(high) << 64) + low)

  typedef struct
  {
    pcg128_t state;
  } pcg_state_128;

  typedef struct
  {
    pcg128_t state;
    pcg128_t inc;
  } pcg_state_setseq_128;

#define PCG_DEFAULT_MULTIPLIER_128 \
  PCG_128BIT_CONSTANT(2549297995355413924ULL, 4865540595714422341ULL)
#define PCG_DEFAULT_INCREMENT_128 \
  PCG_128BIT_CONSTANT(6364136223846793005ULL, 1442695040888963407ULL)
#define PCG_STATE_SETSEQ_128_INITIALIZER \
  {                                      \
      PCG_128BIT_CONSTANT(0x979c9a98d8462005ULL, 0x7d3e9cb6cfe0549bULL), PCG_128BIT_CONSTANT(0x0000000000000001ULL, 0xda3e39cb94b95bdbULL)}

  static inline uint64_t pcg_rotr_64(uint64_t value, unsigned int rot)
  {
    return (value >> rot) | (value << ((-rot) & 63));
  }

  static inline uint64_t pcg_output_xsl_rr_128_64(pcg128_t state)
  {
    return pcg_rotr_64(((uint64_t)(state >> 64u)) ^ (uint64_t)state,
                       state >> 122u);
  }

  extern inline uint64_t
  pcg_setseq_128_xsl_rr_64_random_r(pcg_state_setseq_128 *rng);

  extern inline void pcg_setseq_128_srandom_r(pcg_state_setseq_128 *rng,
                                              pcg128_t initstate,
                                              pcg128_t initseq);

  static inline uint64_t
  pcg_setseq_128_xsl_rr_64_boundedrand_r(pcg_state_setseq_128 *rng,
                                         uint64_t bound)
  {
    uint64_t threshold = -bound % bound;
    for (;;)
    {
      uint64_t r = pcg_setseq_128_xsl_rr_64_random_r(rng);
      if (r >= threshold)
        return r % bound;
    }
  }

  extern pcg128_t pcg_advance_lcg_128(pcg128_t state, pcg128_t delta,
                                      pcg128_t cur_mult, pcg128_t cur_plus);

  static inline void pcg_setseq_128_advance_r(pcg_state_setseq_128 *rng,
                                              pcg128_t delta)
  {
    rng->state = pcg_advance_lcg_128(rng->state, delta,
                                     PCG_DEFAULT_MULTIPLIER_128, rng->inc);
  }

  typedef pcg_state_setseq_128 pcg64_random_t;
#define pcg64_random_r pcg_setseq_128_xsl_rr_64_random_r
#define pcg64_boundedrand_r pcg_setseq_128_xsl_rr_64_boundedrand_r
#define pcg64_srandom_r pcg_setseq_128_srandom_r
#define pcg64_advance_r pcg_setseq_128_advance_r
#define PCG64_INITIALIZER PCG_STATE_SETSEQ_128_INITIALIZER
  typedef struct s_pcg64_state
  {
    pcg64_random_t *pcg_state;
    int has_uint32;
    uint32_t uinteger;
  } pcg64_state;

  static inline uint64_t pcg64_next64(pcg64_state *state)
  {
    return pcg64_random_r(state->pcg_state);
  }

  static inline double pcg64_next_double(pcg64_state *state)
  {
    return (pcg64_next64(state) >> 11) * (1.0 / 9007199254740992.0);
  }

  static inline uint32_t pcg64_next32(pcg64_state *state)
  {
    uint64_t next;
    if (state->has_uint32)
    {
      state->has_uint32 = 0;
      return state->uinteger;
    }
    next = pcg64_random_r(state->pcg_state);
    state->has_uint32 = 1;
    state->uinteger = (uint32_t)(next >> 32);
    return (uint32_t)(next & 0xffffffff);
  }

  void pcg64_advance(pcg64_state *state, uint64_t *step);

  void pcg64_set_seed(pcg64_state *state, uint64_t *seed, uint64_t *inc);

  void pcg64_get_state(pcg64_state *state, uint64_t *state_arr, int *has_uint32,
                       uint32_t *uinteger);

  void pcg64_set_state(pcg64_state *state, uint64_t *state_arr, int has_uint32,
                       uint32_t uinteger);
#if __cplusplus
}
#endif
#endif /* PCG64_H_INCLUDED */
