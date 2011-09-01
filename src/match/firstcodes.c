/*
  Copyright (c) 2011 Stefan Kurtz <kurtz@zbh.uni-hamburg.de>
  Copyright (c) 2011 Center for Bioinformatics, University of Hamburg

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

#include <math.h>
#include "core/encseq.h"
#include "core/codetype.h"
#include "core/arraydef.h"
#include "core/showtime.h"
#include "core/mathsupport.h"
#include "sfx-suffixer.h"
#include "firstcodes.h"

typedef struct
{
  unsigned long code, *ptr;
} GtIndexwithcode;

GT_DECLAREARRAYSTRUCT(GtIndexwithcode);

typedef struct
{
  unsigned int units, shiftright, offset;
  unsigned long entries;
  GtCodetype mask;
  GtBitsequence *bits;
} Gtmarksubstring;

typedef struct
{
  unsigned long differentcodes,
                firstcodehits,
                countsequences,
                numofsequences,
                binsearchcodebuffer_total,
                *allfirstcodes,
                *countocc;
  GtArrayGtIndexwithcode binsearchcache;
  unsigned int binsearchcache_depth,
               flushcount;
  GtArrayGtUlong binsearchcodebuffer;
  Gtmarksubstring markprefix,
                  marksuffix;
} GtFirstcodesinfo;

static void gt_storefirstcodes(void *processinfo,
                               GT_UNUSED bool firstinrange,
                               GT_UNUSED unsigned long pos,
                               GtCodetype code)
{
  GtFirstcodesinfo *firstcodesinfo = (GtFirstcodesinfo *) processinfo;

  gt_assert(firstinrange);
  gt_assert(firstcodesinfo->allfirstcodes != NULL &&
            firstcodesinfo->countsequences < firstcodesinfo->numofsequences);
  firstcodesinfo->allfirstcodes[firstcodesinfo->countsequences] = code;
  firstcodesinfo->countsequences++;
}

static unsigned long gt_remdups_in_sorted_array(
                                  GtFirstcodesinfo *firstcodesinfo)
{
  unsigned long *storeptr, *readptr;

  if (firstcodesinfo->numofsequences > 0)
  {
    unsigned long numofdifferentcodes;
    GtCodetype checkcode;

    firstcodesinfo->countocc
      = gt_calloc((size_t) firstcodesinfo->numofsequences,
                  sizeof (*firstcodesinfo->countocc));
    firstcodesinfo->countocc[0] = 1UL;
    checkcode = firstcodesinfo->allfirstcodes[0] >>
                firstcodesinfo->markprefix.shiftright;
    gt_assert(checkcode < firstcodesinfo->markprefix.entries);
    GT_SETIBIT(firstcodesinfo->markprefix.bits,checkcode);
    for (storeptr = firstcodesinfo->allfirstcodes,
         readptr = firstcodesinfo->allfirstcodes+1;
         readptr < firstcodesinfo->allfirstcodes +
                   firstcodesinfo->numofsequences;
         readptr++)
    {
      if (*storeptr != *readptr)
      {
        storeptr++;

        *storeptr = *readptr;
      }
      firstcodesinfo->countocc[(unsigned long)
                               (storeptr - firstcodesinfo->allfirstcodes)]++;
      checkcode = *readptr >> firstcodesinfo->markprefix.shiftright;
      if (!GT_ISIBITSET(firstcodesinfo->markprefix.bits,checkcode))
      {
        GT_SETIBIT(firstcodesinfo->markprefix.bits,checkcode);
      }
    }
    numofdifferentcodes
      = (unsigned long) (storeptr - firstcodesinfo->allfirstcodes + 1);
    if (numofdifferentcodes < firstcodesinfo->numofsequences)
    {
      /* reduce the memory requirement, as the duplicated elements are not
         needed */
      firstcodesinfo->allfirstcodes
        = gt_realloc(firstcodesinfo->allfirstcodes,
                     sizeof (*firstcodesinfo->allfirstcodes) *
                     numofdifferentcodes);
      firstcodesinfo->countocc
        = gt_realloc(firstcodesinfo->countocc,
                     sizeof (*firstcodesinfo->countocc) *
                     numofdifferentcodes);
    }
    gt_assert(firstcodesinfo->countocc != NULL);
    return numofdifferentcodes;
  }
  return 0;
}

static void gt_firstcodes_halves_rek(GtFirstcodesinfo *firstcodesinfo,
                                     unsigned long left,unsigned long right,
                                     unsigned int depth,unsigned int maxdepth)
{
  unsigned long mid;

  gt_assert(left <= right);
  mid = left + GT_DIV2(right-left);
  if (depth < maxdepth)
  {
    gt_firstcodes_halves_rek(firstcodesinfo,left,mid-1,depth+1,maxdepth);
  }
  gt_assert(firstcodesinfo->binsearchcache.nextfreeGtIndexwithcode <
            firstcodesinfo->binsearchcache.allocatedGtIndexwithcode);
  firstcodesinfo->binsearchcache.spaceGtIndexwithcode[
                  firstcodesinfo->binsearchcache.nextfreeGtIndexwithcode]
                  .ptr = firstcodesinfo->allfirstcodes + mid;
  firstcodesinfo->binsearchcache.spaceGtIndexwithcode[
                  firstcodesinfo->binsearchcache.nextfreeGtIndexwithcode++]
                  .code = firstcodesinfo->allfirstcodes[mid];
  if (depth < maxdepth)
  {
    gt_firstcodes_halves_rek(firstcodesinfo,mid+1,right,depth+1,maxdepth);
  }
}

static void gt_firstcodes_halves(GtFirstcodesinfo *firstcodesinfo,
                                 unsigned int maxdepth)
{
  firstcodesinfo->binsearchcache.nextfreeGtIndexwithcode = 0;
  firstcodesinfo->binsearchcache.allocatedGtIndexwithcode
    = (1UL << (maxdepth+1)) - 1;
  if (firstcodesinfo->binsearchcache.allocatedGtIndexwithcode <
      firstcodesinfo->differentcodes)
  {
    size_t allocbytes
      = sizeof (*firstcodesinfo->binsearchcache.spaceGtIndexwithcode)
                * firstcodesinfo->binsearchcache.allocatedGtIndexwithcode;
    printf("size of binsearch cache: %lu\n",(unsigned long) allocbytes);
    firstcodesinfo->binsearchcache.spaceGtIndexwithcode = gt_malloc(allocbytes);
    gt_assert(firstcodesinfo->differentcodes > 0);
    gt_firstcodes_halves_rek(firstcodesinfo,0,
                             firstcodesinfo->differentcodes - 1,0,maxdepth);
    gt_assert(firstcodesinfo->binsearchcache.nextfreeGtIndexwithcode
              == firstcodesinfo->binsearchcache.allocatedGtIndexwithcode);
#undef FIRSTCODEDEBUG
#ifdef FIRSTCODEDEBUG
    {
      unsigned long idx;

      for (idx=0; idx < firstcodesinfo->binsearchcache.nextfreeGtIndexwithcode;
           idx++)
      {
        printf("%lu %lu\n",
             (unsigned long)
             (firstcodesinfo->binsearchcache.spaceGtIndexwithcode[idx].ptr -
             firstcodesinfo->allfirstcodes),
             firstcodesinfo->binsearchcache.spaceGtIndexwithcode[idx].code);
      }
    }
#endif
  }
}

const unsigned long *gt_firstcodes_find(const GtFirstcodesinfo *firstcodesinfo,
                                        unsigned long code)
{
  const unsigned long *leftptr = NULL, *midptr, *rightptr = NULL;
  unsigned int depth;

  if (firstcodesinfo->binsearchcache.spaceGtIndexwithcode != NULL)
  {
    const GtIndexwithcode *leftic, *midic, *rightic;

    leftic = firstcodesinfo->binsearchcache.spaceGtIndexwithcode;
    rightic = firstcodesinfo->binsearchcache.spaceGtIndexwithcode +
              firstcodesinfo->binsearchcache.nextfreeGtIndexwithcode - 1;
    for (depth = 0; /* Nothing */; depth++)
    {
      midic = leftic + GT_DIV2((unsigned long) (rightic-leftic));
      if (code < midic->code)
      {
        if (depth < firstcodesinfo->binsearchcache_depth)
        {
          rightic = midic - 1;
        } else
        {
          gt_assert(leftic->ptr != NULL && rightic->ptr != NULL);
          if (leftic > firstcodesinfo->binsearchcache.spaceGtIndexwithcode)
          {
            leftptr = (leftic-1)->ptr + 1;
          } else
          {
            leftptr = firstcodesinfo->allfirstcodes;
          }
          rightptr = rightic->ptr - 1;
          break;
        }
      } else
      {
        if (code > midic->code)
        {
          if (depth < firstcodesinfo->binsearchcache_depth)
          {
            leftic = midic + 1;
          } else
          {
            gt_assert(leftic->ptr != NULL && rightic->ptr != NULL);
            leftptr = leftic->ptr + 1;
            if (rightic < firstcodesinfo->binsearchcache.spaceGtIndexwithcode +
                     firstcodesinfo->binsearchcache.nextfreeGtIndexwithcode-1)
            {
              rightptr = (rightic+1)->ptr - 1;
            } else
            {
              rightptr = firstcodesinfo->allfirstcodes +
                         firstcodesinfo->differentcodes - 1;
            }
            break;
          }
        } else
        {
          return midic->ptr;
        }
      }
    }
    gt_assert(leftptr != NULL && rightptr != NULL);
  } else
  {
    depth = 0;
    leftptr = firstcodesinfo->allfirstcodes;
    rightptr = firstcodesinfo->allfirstcodes +
               firstcodesinfo->differentcodes - 1;
  }
  while (leftptr <= rightptr)
  {
    midptr = leftptr + GT_DIV2((unsigned long) (rightptr-leftptr));
    if (code < *midptr)
    {
      rightptr = midptr - 1;
    } else
    {
      if (code > *midptr)
      {
        leftptr = midptr + 1;
      } else
      {
        return midptr;
      }
    }
    depth++;
  }
  return NULL;
}

#ifdef  QSORTNAME
#undef  QSORTNAME
#endif

#define QSORTNAME(NAME)                        firstcodes_##NAME
#define firstcodes_ARRAY_GET(ARR,RELIDX)       ARR[RELIDX]
#define firstcodes_ARRAY_SET(ARR,RELIDX,VALUE) ARR[RELIDX] = VALUE

typedef unsigned long QSORTNAME(Sorttype);

#include "match/qsort-direct.gen"

static void firstcodesaccum_flush(GtFirstcodesinfo *firstcodesinfo)
{
  const unsigned long *ptr;
  unsigned long *vptr;

  gt_assert(firstcodesinfo->allfirstcodes != NULL);
  QSORTNAME(gt_direct_qsort)
            (6UL, false,
             firstcodesinfo->binsearchcodebuffer.spaceGtUlong,
             firstcodesinfo->binsearchcodebuffer.nextfreeGtUlong);
  firstcodesinfo->binsearchcodebuffer_total
    += firstcodesinfo->binsearchcodebuffer.nextfreeGtUlong;
  for (vptr = firstcodesinfo->binsearchcodebuffer.spaceGtUlong;
       vptr < firstcodesinfo->binsearchcodebuffer.spaceGtUlong +
              firstcodesinfo->binsearchcodebuffer.nextfreeGtUlong;
       vptr++)
  {
    ptr = gt_firstcodes_find(firstcodesinfo,*vptr);
    if (ptr != NULL)
    {
      firstcodesinfo->countocc[(unsigned long)
                               (ptr - firstcodesinfo->allfirstcodes)]++;
      firstcodesinfo->firstcodehits++;
    }
  }
  printf("%u ",firstcodesinfo->flushcount++);
  (void) fflush(stdout);
  firstcodesinfo->binsearchcodebuffer.nextfreeGtUlong = 0;
}

static void gt_accumulateallfirstcodeocc(void *processinfo,
                                         GT_UNUSED bool firstinrange,
                                         GT_UNUSED unsigned long pos,
                                         GtCodetype code)
{
  GtFirstcodesinfo *firstcodesinfo = (GtFirstcodesinfo *) processinfo;
  GtCodetype checkcode;

  gt_assert(firstcodesinfo->countocc != NULL);
  if (firstcodesinfo->binsearchcodebuffer.nextfreeGtUlong  ==
      firstcodesinfo->binsearchcodebuffer.allocatedGtUlong)
  {
    firstcodesaccum_flush(firstcodesinfo);
  }
  checkcode = code >> (GtCodetype) firstcodesinfo->markprefix.shiftright;
  if (GT_ISIBITSET(firstcodesinfo->markprefix.bits,checkcode))
  {
    gt_assert (firstcodesinfo->binsearchcodebuffer.nextfreeGtUlong <
               firstcodesinfo->binsearchcodebuffer.allocatedGtUlong);
    firstcodesinfo->binsearchcodebuffer.spaceGtUlong[
      firstcodesinfo->binsearchcodebuffer.nextfreeGtUlong++] = code;
  }
}

static void gt_marksubstring_init(Gtmarksubstring *mark,unsigned int numofchars,
                                  unsigned int kmersize, unsigned int offset,
                                  unsigned int units)
{
  gt_assert(kmersize >= units);
  mark->units = units;
  mark->offset = offset;
  mark->entries = gt_power_for_small_exponents(numofchars,units);
  if (offset > 0)
  {
    mark->mask = (GtCodetype) ~mark->entries;
  } else
  {
    mark->mask = 0;
  }
  mark->shiftright = GT_MULT2(kmersize - units);
  GT_INITBITTAB(mark->bits,mark->entries);
}

void storefirstcodes_getencseqkmers_twobitencoding(const GtEncseq *encseq,
                                                   unsigned int kmersize)
{
  GtTimer *timer = NULL;
  GtFirstcodesinfo firstcodesinfo;
  size_t sizeforcodestable;
  unsigned int numofchars = gt_encseq_alphabetnumofchars(encseq);
  const unsigned int markprefixunits = 14U;

  if (gt_showtime_enabled())
  {
    timer = gt_timer_new_with_progress_description("insert first codes into "
                                                   "array");
    gt_timer_start(timer);
  }
  firstcodesinfo.numofsequences = gt_encseq_num_of_sequences(encseq);
  gt_assert(numofchars == 4U);
  sizeforcodestable = sizeof (*firstcodesinfo.allfirstcodes) *
                      firstcodesinfo.numofsequences;
  printf("# use array of size %lu\n",(unsigned long) sizeforcodestable);
  firstcodesinfo.allfirstcodes = gt_malloc(sizeforcodestable);
  firstcodesinfo.differentcodes = 0;
  firstcodesinfo.countsequences = 0;
  firstcodesinfo.firstcodehits = 0;
  GT_INITARRAY(&firstcodesinfo.binsearchcache,GtIndexwithcode);
  GT_INITARRAY(&firstcodesinfo.binsearchcodebuffer,GtUlong);
  getencseqkmers_twobitencoding(encseq,
                                GT_READMODE_FORWARD,
                                kmersize,
                                kmersize,
                                true,
                                gt_storefirstcodes,
                                &firstcodesinfo,
                                NULL,
                                NULL);
  gt_assert(firstcodesinfo.numofsequences == firstcodesinfo.countsequences);
  if (timer != NULL)
  {
    gt_timer_show_progress(timer, "sorting the codes",stdout);
  }
  QSORTNAME(gt_direct_qsort)
             (6UL, false,
             firstcodesinfo.allfirstcodes,firstcodesinfo.numofsequences);
  gt_marksubstring_init(&firstcodesinfo.markprefix,numofchars,kmersize,0,
                        markprefixunits);
  firstcodesinfo.differentcodes = gt_remdups_in_sorted_array(&firstcodesinfo);
  printf("# number of different codes=%lu (%.4f) in %lu sequences\n",
          firstcodesinfo.differentcodes,
          (double) firstcodesinfo.differentcodes/firstcodesinfo.numofsequences,
          firstcodesinfo.countsequences);
  firstcodesinfo.binsearchcache_depth = 15U;
  firstcodesinfo.flushcount = 0;
  firstcodesinfo.binsearchcodebuffer_total = 0;
  gt_firstcodes_halves(&firstcodesinfo,firstcodesinfo.binsearchcache_depth);
  firstcodesinfo.binsearchcodebuffer.allocatedGtUlong = 3000000UL;
  firstcodesinfo.binsearchcodebuffer.nextfreeGtUlong = 0;
  firstcodesinfo.binsearchcodebuffer.spaceGtUlong
    = gt_malloc(sizeof (*firstcodesinfo.binsearchcodebuffer.spaceGtUlong)
                          * firstcodesinfo.binsearchcodebuffer.
                                           allocatedGtUlong);
  gt_assert(firstcodesinfo.binsearchcodebuffer.allocatedGtUlong > 0);
  if (timer != NULL)
  {
    gt_timer_show_progress(timer, "accumulate counts",stdout);
  }
  getencseqkmers_twobitencoding(encseq,
                                GT_READMODE_FORWARD,
                                kmersize,
                                45U,
                                false,
                                gt_accumulateallfirstcodeocc,
                                &firstcodesinfo,
                                NULL,
                                NULL);
  firstcodesaccum_flush(&firstcodesinfo);
  printf("\nbinsearchbuffer_total=%lu\n",
          firstcodesinfo.binsearchcodebuffer_total);
  printf("# firstcodehits=%lu (%.2f)\n",firstcodesinfo.firstcodehits,
                                      (double) firstcodesinfo.firstcodehits/
                                      gt_encseq_total_length(encseq));
  GT_FREEARRAY(&firstcodesinfo.binsearchcache,GtIndexwithcode);
  GT_FREEARRAY(&firstcodesinfo.binsearchcodebuffer,GtUlong);
  gt_free(firstcodesinfo.allfirstcodes);
  gt_free(firstcodesinfo.countocc);
  gt_free(firstcodesinfo.markprefix.bits);
  if (timer != NULL)
  {
    gt_timer_show_progress_final(timer, stdout);
    gt_timer_delete(timer);
  }
}
