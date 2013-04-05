/*
  Copyright (c) 2013 Stefan Kurtz <kurtz@zbh.uni-hamburg.de>
  Copyright (c) 2013 Center for Bioinformatics, University of Hamburg

  Permission to use, copy, modify, and distribute this software for any
  purpose with or without fee is hereby granted, provided that the above
  copyright notice and this permission notice appear in all copies.

  THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
  WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
  ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
  WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
  ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
  OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include <inttypes.h>
#include "core/ma_api.h"
#include "core/assert_api.h"
#include "bitbuffer.h"

#define GT_BITSINBYTEBUFFER 64U

struct GtBitbuffer
{
  unsigned int remainingbitsinbuffer,
               bitsperentry,
               bits2store;
  unsigned long currentvalue;
  uint64_t currentbitbuffer,
           numberofallelements,
           wordswritten;
  FILE *outfp;
};

GtBitbuffer *gt_bitbuffer_new(FILE *outfp,uint8_t bitsperentry,
                              uint64_t numberofallelements)
{
  GtBitbuffer *bitbuffer = gt_malloc(sizeof *bitbuffer);

  gt_assert(outfp != NULL);
  (void) fwrite(&numberofallelements,sizeof numberofallelements,(size_t) 1,
                outfp);
  (void) fwrite(&bitsperentry,sizeof bitsperentry,(size_t) 1,outfp);
  gt_assert((unsigned int) bitsperentry < GT_BITSINBYTEBUFFER);
  bitbuffer->numberofallelements = numberofallelements;
  bitbuffer->bitsperentry = (unsigned int) bitsperentry;
  bitbuffer->currentbitbuffer = 0;
  bitbuffer->currentvalue = 0;
  bitbuffer->outfp = outfp;
  bitbuffer->remainingbitsinbuffer = GT_BITSINBYTEBUFFER;
  bitbuffer->bits2store = 0;
  bitbuffer->wordswritten = 0;
  return bitbuffer;
}

void gt_bitbuffer_next_value (GtBitbuffer *bb, unsigned long value)
{
  while (true)
  {
    if (bb->remainingbitsinbuffer == 0)
    {
      (void) fwrite(&bb->currentbitbuffer,sizeof bb->currentbitbuffer,
                    (size_t) 1,bb->outfp);
      bb->wordswritten++;
      bb->currentbitbuffer = 0;
      bb->remainingbitsinbuffer = GT_BITSINBYTEBUFFER;
    } else
    {
      if (bb->bits2store == 0)
      {
        bb->currentvalue = value;
        bb->bits2store = bb->bitsperentry;
      } else
      {
        if (bb->remainingbitsinbuffer >= bb->bits2store)
        {
          unsigned int shiftleft = bb->remainingbitsinbuffer -
                                   bb->bits2store;
          /* use bb->remainingbitsinbuffer after subtraction for shiftleft */
          bb->currentbitbuffer
            |= (uint64_t) (bb->currentvalue << shiftleft);
          bb->remainingbitsinbuffer -= bb->bits2store;
          bb->bits2store = 0;
          break;
        } else
        {
          unsigned long maskright = (1UL << bb->bits2store) - 1;
          unsigned int shiftright = bb->bits2store -
                                    bb->remainingbitsinbuffer;
          /* use bb->bits2store after subtraction for shiftright */
          /* maskright is not necessary */
          bb->currentbitbuffer
            |= ((uint64_t) (bb->currentvalue & maskright) >> shiftright);
          bb->bits2store -= bb->remainingbitsinbuffer;
          gt_assert(bb->bits2store > 0);
          bb->remainingbitsinbuffer = 0;
        }
      }
    }
  }
}

void gt_bitbuffer_next_uint32tab(GtBitbuffer *bb,const uint32_t *tab,
                                 unsigned long len)
{
  const uint32_t *uintptr;

  gt_assert (tab != NULL);
  for (uintptr = tab; uintptr < tab + len; uintptr++)
  {
    gt_bitbuffer_next_value (bb, (unsigned long) *uintptr);
  }
}

void gt_bitbuffer_next_ulongtab(GtBitbuffer *bb,
                                const unsigned long *tab,
                                unsigned long len)
{
  const unsigned long *ulongptr;

  gt_assert (tab != NULL);
  for (ulongptr = tab; ulongptr < tab + len; ulongptr++)
  {
    gt_bitbuffer_next_value (bb, *ulongptr);
  }
}

void gt_bitbuffer_delete(GtBitbuffer *bb)
{
  uint64_t expectedwords,
           totalbits = bb->numberofallelements *
                       bb->bitsperentry;
  const int log2of64 = 6;

  expectedwords = totalbits >> log2of64; /* div by bitsize of uint64_t */
  if ((totalbits & (uint64_t) 63) > 0)  /* mod by bitsize of uint64_t */
  {
    expectedwords++;
  }
  if (bb->outfp != NULL)
  {
    if (bb->remainingbitsinbuffer < GT_BITSINBYTEBUFFER)
    {
      (void) fwrite(&bb->currentbitbuffer,
                    sizeof bb->currentbitbuffer,
                    (size_t) 1,bb->outfp);
      bb->wordswritten++;
    }
    if (bb->wordswritten != expectedwords)
    {
      fprintf(stderr,"wordswritten = %lu != %lu = expextedwords\n",
               (unsigned long) bb->wordswritten,
               (unsigned long) expectedwords);
    }
    gt_assert(bb->wordswritten == expectedwords);
  }
  gt_free(bb);
}