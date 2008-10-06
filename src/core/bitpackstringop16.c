/*
** autogenerated content - DO NOT EDIT
*/
/*
  Copyright (C) 2007 Thomas Jahns <Thomas.Jahns@gmx.net>

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
#include <limits.h>
#include <stdio.h>
#include "core/assert_api.h"
#include "core/minmax.h"
#include "core/bitpackstring.h"

uint16_t
gt_bsGetUInt16(constBitString str, BitOffset offset, unsigned numBits)
{
  unsigned long accum = 0;
  unsigned bitsLeft = numBits, bitTop = offset%bitElemBits;
  size_t elemStart = offset/bitElemBits;
  const BitElem *p = str + elemStart;
  gt_assert(str);
#ifndef NDEBUG
  if (numBits > sizeof (accum)*CHAR_BIT)
    fprintf(stderr, "numBits = %u\n", numBits);
#endif
  gt_assert(numBits <= sizeof (accum)*CHAR_BIT);
  if (bitTop)
  {
    unsigned long mask;
    unsigned bits2Read = MIN(bitElemBits - bitTop, bitsLeft);
    unsigned unreadRightBits = (bitElemBits - bitTop - bits2Read);
    mask = (~((~(unsigned long)0) << bits2Read)) << unreadRightBits;
    accum = ((*p++) & mask) >> unreadRightBits;
    bitsLeft -= bits2Read;
  }
  /* get bits from intervening elems */
  while (bitsLeft >= bitElemBits)
  {
    accum = accum << bitElemBits | (*p++);
    bitsLeft -= bitElemBits;
  }
  /* get bits from last elem */
  if (bitsLeft)
  {
    accum = accum << bitsLeft |
      (((*p) & ((~(unsigned long)0)<<(bitElemBits - bitsLeft)))
       >>(bitElemBits - bitsLeft));
  }
  return accum;
}

void
gt_bsStoreUInt16(BitString str, BitOffset offset,
                 unsigned numBits, uint16_t val)
{
  unsigned bitsLeft = numBits,
    bitTop = offset%bitElemBits;
  size_t elemStart = offset/bitElemBits;
  BitElem *p = str + elemStart;
  gt_assert(str);
  gt_assert(numBits <= sizeof (val)*CHAR_BIT);
  /* set bits of first element, accounting for bits to be preserved */
  if (bitTop)
  {
    unsigned long mask = ~(unsigned long)0;
    if (bitElemBits < (sizeof (unsigned long)*CHAR_BIT))
    {
      mask <<= bitElemBits;
    }
    else
    {
      mask = 0;
    }
    mask = (~mask) >> bitTop;
    if (numBits < bitElemBits - bitTop)
    {
      unsigned backShift = bitElemBits - numBits - bitTop;
      mask &= ~(unsigned long)0 << backShift;
      *p = (*p & ~mask) | ((val << backShift) & mask);
      /* TODO: try wether  r = a ^ ((a ^ b) & mask) is faster, see below */
      return;
    }
    else
    {
      bitsLeft -= bitElemBits - bitTop;
      *p = (*p & ~mask) | ((val >> bitsLeft) & mask);
      ++p;
    }
  }
  /* set bits for intervening elems */
  while (bitsLeft >= bitElemBits)
  {
    bitsLeft -= bitElemBits;
    *p++ = val >> bitsLeft;
  }
  /* set bits for last elem */
  if (bitsLeft)
  {
    unsigned long mask =
      ((~(unsigned long)0)<<(bitElemBits - bitsLeft));
    if (bitElemBits < (sizeof (unsigned long)*CHAR_BIT))
      mask &= (~(~(unsigned long)0<<bitElemBits));
    *p = (*p & ~mask) | ((val << (bitElemBits - bitsLeft)) & mask);
  }
}

/**************************************************************************/
/* Merge bits from two values according to a mask                         */
/*                                                                        */
/* unsigned int a;       value to merge in non-masked bits                */
/* unsigned int b;       value to merge in masked bits                    */
/* unsigned int mask;    1 where bits from b should be selected;          */
/*                       0 where from a.                                  */
/* unsigned int r;       result of (a & ~mask) | (b & mask) goes here     */
/*                                                                        */
/* r = a ^ ((a ^ b) & mask);                                              */
/*                                                                        */
/* This shaves one operation from the obvious way of combining two sets   */
/* of bits according to a bit mask. If the mask is a constant, then there */
/* may be no advantage.                                                   */
/*                                                                        */
/* Ron Jeffery sent this to me on February 9, 2006.                       */
/**************************************************************************/

/*
 * include variations of vector operations
 */

/*
  Copyright (C) 2007 Thomas Jahns <Thomas.Jahns@gmx.net>

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

void
gt_bsGetUniformUInt16Array(constBitString str, BitOffset offset,
                           unsigned numBits, size_t numValues,
                           uint16_t val[])
{
  /* idea: read as much as possible from str in each iteration,
   * accumulate if bitsLeft < numBits */
  size_t j = 0;
  BitOffset totalBitsLeft = numValues * numBits;
  size_t elemStart = offset/bitElemBits;
  unsigned bitTop = offset%bitElemBits,
    bitsRead = 0; /*< how many bits in current *p are read */
  const BitElem *p = str + elemStart;
  unsigned bitsInAccum = 0;
  unsigned long accum = 0, valMask = ~(unsigned long)0;
  if (numBits < (sizeof (val[0])*CHAR_BIT))
    valMask = ~(valMask << numBits);
  gt_assert(str && val);
  gt_assert(numBits <= sizeof (val[0])*CHAR_BIT);
  /* user requested zero values, ugly but must be handled, since legal */
  if (!totalBitsLeft)
  {
    return;
  }
  /* get bits of first element if not aligned */
  if (bitTop)
  {
    unsigned long mask; /*< all of the bits we want to get from *p */
    unsigned bits2Read = MIN(bitElemBits - bitTop, totalBitsLeft);
    unsigned unreadRightBits = (bitElemBits - bitTop - bits2Read);
    mask = (~((~(unsigned long)0) << bits2Read)) << unreadRightBits;
    accum = ((*p++) & mask) >> unreadRightBits;
    bitsInAccum += bits2Read;
    totalBitsLeft -= bits2Read;
  }
  while (j < numValues)
  {
    while (bitsInAccum < numBits && totalBitsLeft)
    {
      unsigned bits2Read, bitsFree = sizeof (accum)*CHAR_BIT - bitsInAccum;
      unsigned long mask;
      bits2Read = MIN3(bitsFree, bitElemBits - bitsRead, totalBitsLeft);
      mask = (~((~(unsigned long)0) << bits2Read));
      accum = accum << bits2Read | (((*p) >> (bitElemBits
                                              - bits2Read - bitsRead)) & mask);
      bitsInAccum += bits2Read;
      totalBitsLeft -= bits2Read;
      /* all of *p consumed? */
      if ((bitsRead += bits2Read) == bitElemBits)
      {
        ++p, bitsRead = 0;
      }
    }
    /* now we have enough bits in accum */
    while (bitsInAccum >= numBits)
    {
      val[j++] = ((accum >> (bitsInAccum - numBits)) & valMask);
      bitsInAccum -= numBits;
    }
  }
}

void
gt_bsGetNonUniformUInt16Array(
  constBitString str, BitOffset offset, size_t numValues,
  BitOffset numBitsTotal, unsigned numBitsList[], uint16_t val[])
{
  /* idea: read as much as possible from str in each iteration,
   * accumulate if bitsLeft < numBits */
  size_t j = 0;
  BitOffset totalBitsLeft = numBitsTotal;
  size_t elemStart = offset/bitElemBits;
  unsigned bitTop = offset%bitElemBits,
    bitsRead = 0; /*< how many bits in current *p are read */
  const BitElem *p = str + elemStart;
  unsigned bitsInAccum = 0;
  unsigned long accum = 0;
  gt_assert(str && val);
  /* user requested zero values, ugly but must be handled, since legal */
  if (!totalBitsLeft)
  {
    return;
  }
  /* get bits of first element if not aligned */
  if (bitTop)
  {
    unsigned long mask; /*< all of the bits we want to get from *p */
    unsigned bits2Read = MIN(bitElemBits - bitTop, totalBitsLeft);
    unsigned unreadRightBits = (bitElemBits - bitTop - bits2Read);
    mask = (~((~(unsigned long)0) << bits2Read)) << unreadRightBits;
    accum = ((*p++) & mask) >> unreadRightBits;
    bitsInAccum += bits2Read;
    totalBitsLeft -= bits2Read;
  }
  while (j < numValues)
  {
    while (bitsInAccum < numBitsList[j] && totalBitsLeft)
    {
      unsigned bits2Read, bitsFree = sizeof (accum)*CHAR_BIT - bitsInAccum;
      unsigned long mask;
      bits2Read = MIN3(bitsFree, bitElemBits - bitsRead, totalBitsLeft);
      mask = (~((~(unsigned long)0) << bits2Read));
      accum = accum << bits2Read | (((*p) >> (bitElemBits
                                              - bits2Read - bitsRead)) & mask);
      bitsInAccum += bits2Read;
      totalBitsLeft -= bits2Read;
      /* all of *p consumed? */
      if ((bitsRead += bits2Read) == bitElemBits)
      {
        ++p, bitsRead = 0;
      }
    }
    {
      unsigned numBits;
      /* now we have enough bits in accum */
      while (j < numValues && bitsInAccum >= (numBits = numBitsList[j]))
      {
        unsigned long valMask;
        gt_assert(numBits <= sizeof (val[0])*CHAR_BIT);
        valMask = ~(unsigned long)0;
        if (numBits < (sizeof (val[0])*CHAR_BIT))
          valMask = ~(valMask << numBits);
        val[j++] = ((accum >> (bitsInAccum - numBits)) & valMask );
        bitsInAccum -= numBits;
      }
    }
  }
}

void
gt_bsGetNonUniformInt16Array(
  constBitString str, BitOffset offset, size_t numValues,
  BitOffset numBitsTotal, unsigned numBitsList[], int16_t val[])
{
  /* idea: read as much as possible from str in each iteration,
   * accumulate if bitsLeft < numBits */
  size_t j = 0;
  BitOffset totalBitsLeft = numBitsTotal;
  size_t elemStart = offset/bitElemBits;
  unsigned bitTop = offset%bitElemBits,
    bitsRead = 0; /*< how many bits in current *p are read */
  const BitElem *p = str + elemStart;
  unsigned bitsInAccum = 0;
  unsigned long accum = 0;
  gt_assert(str && val);
  /* user requested zero values, ugly but must be handled, since legal */
  if (!totalBitsLeft)
  {
    return;
  }
  /* get bits of first element if not aligned */
  if (bitTop)
  {
    unsigned long mask; /*< all of the bits we want to get from *p */
    unsigned bits2Read = MIN(bitElemBits - bitTop, totalBitsLeft);
    unsigned unreadRightBits = (bitElemBits - bitTop - bits2Read);
    mask = (~((~(unsigned long)0) << bits2Read)) << unreadRightBits;
    accum = ((*p++) & mask) >> unreadRightBits;
    bitsInAccum += bits2Read;
    totalBitsLeft -= bits2Read;
  }
  while (j < numValues)
  {
    while (bitsInAccum < numBitsList[j] && totalBitsLeft)
    {
      unsigned bits2Read, bitsFree = sizeof (accum)*CHAR_BIT - bitsInAccum;
      unsigned long mask;
      bits2Read = MIN3(bitsFree, bitElemBits - bitsRead, totalBitsLeft);
      mask = (~((~(unsigned long)0) << bits2Read));
      accum = accum << bits2Read | (((*p) >> (bitElemBits
                                              - bits2Read - bitsRead)) & mask);
      bitsInAccum += bits2Read;
      totalBitsLeft -= bits2Read;
      /* all of *p consumed? */
      if ((bitsRead += bits2Read) == bitElemBits)
      {
        ++p, bitsRead = 0;
      }
    }
    {
      unsigned numBits;
      /* now we have enough bits in accum */
      while (j < numValues && bitsInAccum >= (numBits = numBitsList[j]))
      {
        unsigned long valMask = (numBits < 16)
          ? ~((~(unsigned long)0) << numBits) : ~(unsigned long)0;
        int16_t m = (int16_t)1 << (numBits - 1);
        gt_assert(numBits <= sizeof (val[0])*CHAR_BIT);
        val[j++] = ((((accum >> (bitsInAccum - numBits)) & valMask)
                              ^ m) - m);
        bitsInAccum -= numBits;
      }
    }
  }
}

/*
  Copyright (C) 2007 Thomas Jahns <Thomas.Jahns@gmx.net>

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

#include "core/unused_api.h"

void
gt_bsStoreUniformUInt16Array(BitString str, BitOffset offset,
                                 unsigned numBits, size_t numValues,
                                 const uint16_t val[])
{
  /* idea: read as much as possible from val in each iteration,
   * accumulate if bitsInAccum < bitElemBits */
  size_t j = 0;
  BitOffset totalBitsLeft = numValues * numBits;
  unsigned bitTop = offset%bitElemBits,
    bitsLeft; /*< how many bits in currentVal == val[j] are left */
  BitElem *p = str + offset/bitElemBits;
  unsigned bitsInAccum;
  unsigned long accum, valMask = ~(unsigned long)0, currentVal;
  if (numBits < (sizeof (val[0])*CHAR_BIT))
    valMask = ~(valMask << numBits);
  gt_assert(str && val);
  gt_assert(numBits <= sizeof (val[0])*CHAR_BIT);
  /* user requested zero values, ugly but must be handled, since legal */
  if (!totalBitsLeft)
  {
    return;
  }
  accum = val[0] & valMask;
  totalBitsLeft -= bitsInAccum = numBits;
  if (totalBitsLeft)
  {
    currentVal = val[++j] & valMask;
    totalBitsLeft -= bitsLeft = numBits;
  }
  else
  {
    currentVal = 0;
    bitsLeft = 0;
  }
  /* set bits of first element if not aligned */
  if (bitTop)
  {
    BitElem mask = ~(~(unsigned long)0 << (bitElemBits - bitTop));
    while ((totalBitsLeft || bitsLeft) && bitsInAccum < bitElemBits - bitTop)
    {
      unsigned bits2Read, bitsFree = sizeof (accum)*CHAR_BIT - bitsInAccum;

      if ((bits2Read = MIN(bitsFree, bitsLeft)) < sizeof (accum)*CHAR_BIT)
      {
        accum = accum << bits2Read
          | ((currentVal) >> (bitsLeft - bits2Read));
      }
      else
        accum = currentVal;

      /* all of val[j] consumed? */
      bitsInAccum += bits2Read;
      if (!(bitsLeft -= bits2Read) && totalBitsLeft)
        currentVal = val[++j] & valMask, totalBitsLeft -= bitsLeft = numBits;
    }
    /* at this point accum holds as many bits as we could get
     * to fill the first BitElem in str, but did we get enough? */
    if (bitsInAccum < bitElemBits - bitTop)
    {
      /* no there's not enough */
      unsigned backShift = bitElemBits - bitsInAccum - bitTop;
      mask &= ~(unsigned long)0 << backShift;
      *p = (*p & ~mask) | ((accum << backShift) & mask);
      /* TODO: try wether  r = a ^ ((a ^ b) & mask) is faster, see below */
      return; /* if we couldn't gather more bits, there's none left */
    }
    else
    {
      /* yep, just or with accumVals */
      *p = (*p & ~mask) | (accum >> (bitsInAccum - bitElemBits + bitTop));
      ++p;
      bitsInAccum -= bitElemBits - bitTop;
    }
  }

  while (totalBitsLeft || (bitsInAccum + bitsLeft) > bitElemBits)
  {
    while ((totalBitsLeft || bitsLeft)
          && ((bitsInAccum < bitElemBits)
              || (bitsLeft < sizeof (accum)*CHAR_BIT - bitsInAccum)))
    {
      unsigned bits2Read, bitsFree = sizeof (accum)*CHAR_BIT - bitsInAccum;
      if ((bits2Read = MIN(bitsFree, bitsLeft)) < sizeof (accum)*CHAR_BIT)
      {
        unsigned long mask = ~((~(unsigned long)0) << bits2Read);
        accum = accum << bits2Read
          | ((currentVal >> (bitsLeft - bits2Read)) & mask);
      }
      else
        accum = currentVal;
      bitsInAccum += bits2Read;
      /* all of currentVal == val[j] consumed? */
      if (bits2Read == bitsLeft && totalBitsLeft)
        currentVal = val[++j] & valMask, totalBitsLeft -= bitsLeft = numBits;
      else
        bitsLeft -= bits2Read;
    }
    /* now we have enough bits in accum */
    while (bitsInAccum >= bitElemBits)
    {
      *p++ = accum >> (bitsInAccum - bitElemBits);
      bitsInAccum -= bitElemBits;
    }
  }
  /* write the rest bits left in accum and currentVal */
  accum = (accum << bitsLeft)
    | (currentVal & (valMask >> (numBits - bitsLeft)));
  bitsInAccum += bitsLeft;
  while (bitsInAccum >= bitElemBits)
  {
    *p++ = accum >> (bitsInAccum - bitElemBits);
    bitsInAccum -= bitElemBits;
  }
  if (bitsInAccum)
  {
    unsigned long mask =
      ~(unsigned long)0 << (bitElemBits - bitsInAccum);
    *p = (*p & ~mask) | ((accum << (bitElemBits - bitsInAccum))& mask);
  }
}

void
gt_bsStoreNonUniformUInt16Array(
  BitString str, BitOffset offset, GT_UNUSED size_t numValues,
  BitOffset totalBitsLeft, unsigned numBitsList[], const uint16_t val[])
{
  /* idea: read as much as possible from val in each iteration,
   * accumulate if bitsInAccum < bitElemBits */
  size_t j = 0;
  unsigned bitTop = offset%bitElemBits,
    bitsLeft; /*< how many bits in currentVal == val[j] are left */
  BitElem *p = str + offset/bitElemBits;
  unsigned bitsInAccum;
  unsigned long accum, valMask = ~(unsigned long)0, currentVal;
  if (numBitsList[0] < (sizeof (val[0])*CHAR_BIT))
    valMask = ~(valMask << numBitsList[0]);
  gt_assert(str && val);
  gt_assert(numBitsList[0] <= sizeof (val[0])*CHAR_BIT);
  /* user requested zero values, ugly but must be handled, since legal */
  if (!totalBitsLeft)
  {
    return;
  }
  accum = val[0] & valMask;
  totalBitsLeft -= bitsInAccum = numBitsList[0];
  if (totalBitsLeft)
  {
    unsigned numBits = numBitsList[++j];
    gt_assert(numBits <= sizeof (val[0])*CHAR_BIT);
    valMask = (numBits < 16)?
      ~((~(uint16_t)0) << numBits):~(uint16_t)0;
    currentVal = val[j] & valMask;
    totalBitsLeft -= bitsLeft = numBits;
  }
  else
  {
    currentVal = 0;
    bitsLeft = 0;
  }
  /* set bits of first element if not aligned */
  if (bitTop)
  {
    BitElem mask = ~(~(unsigned long)0 << (bitElemBits - bitTop));
    while ((totalBitsLeft || bitsLeft) && bitsInAccum < bitElemBits - bitTop)
    {
      unsigned bits2Read, bitsFree = sizeof (accum)*CHAR_BIT - bitsInAccum;

      if ((bits2Read = MIN(bitsFree, bitsLeft)) < sizeof (accum)*CHAR_BIT)
      {
        accum = accum << bits2Read
          | ((currentVal) >> (bitsLeft - bits2Read));
      }
      else
        accum = currentVal;

      /* all of val[j] consumed? */
      bitsInAccum += bits2Read;
      if (!(bitsLeft -= bits2Read) && totalBitsLeft)
      {
        unsigned numBits = numBitsList[++j];
        gt_assert(numBits <= sizeof (val[0])*CHAR_BIT);
        valMask = (numBits < 16)?
          ~((~(uint16_t)0) << numBits):~(uint16_t)0;
        currentVal = val[j] & valMask, totalBitsLeft -= bitsLeft = numBits;
      }
    }
    /* at this point accum holds as many bits as we could get
     * to fill the first BitElem in str, but did we get enough? */
    if (bitsInAccum < bitElemBits - bitTop)
    {
      /* no there's not enough */
      unsigned backShift = bitElemBits - bitsInAccum - bitTop;
      mask &= ~(unsigned long)0 << backShift;
      *p = (*p & ~mask) | ((accum << backShift) & mask);
      /* TODO: try wether  r = a ^ ((a ^ b) & mask) is faster, see below */
      return; /* if we couldn't gather more bits, there's none left */
    }
    else
    {
      /* yep, just or with accumVals */
      *p = (*p & ~mask) | (accum >> (bitsInAccum - bitElemBits + bitTop));
      ++p;
      bitsInAccum -= bitElemBits - bitTop;
    }
  }

  while (totalBitsLeft || (bitsInAccum + bitsLeft) > bitElemBits)
  {
    while ((totalBitsLeft || bitsLeft)
          && ((bitsInAccum < bitElemBits)
              || (bitsLeft < sizeof (accum)*CHAR_BIT - bitsInAccum)))
    {
      unsigned bits2Read, bitsFree = sizeof (accum)*CHAR_BIT - bitsInAccum;
      if ((bits2Read = MIN(bitsFree, bitsLeft)) < sizeof (accum)*CHAR_BIT)
      {
        unsigned long mask = ~((~(unsigned long)0) << bits2Read);
        accum = accum << bits2Read
          | ((currentVal >> (bitsLeft - bits2Read)) & mask);
      }
      else
        accum = currentVal;
      bitsInAccum += bits2Read;
      /* all of currentVal == val[j] consumed? */
      if (bits2Read == bitsLeft && totalBitsLeft)
      {
        unsigned numBits = numBitsList[++j];
        gt_assert(numBits <= sizeof (val[0])*CHAR_BIT);
        valMask = (numBits < 16)?
          ~((~(uint16_t)0) << numBits):~(uint16_t)0;
        currentVal = val[j] & valMask, totalBitsLeft -= bitsLeft = numBits;
      }
      else
        bitsLeft -= bits2Read;
    }
    /* now we have enough bits in accum */
    while (bitsInAccum >= bitElemBits)
    {
      *p++ = accum >> (bitsInAccum - bitElemBits);
      bitsInAccum -= bitElemBits;
    }
  }
  /* write the rest bits left in accum and currentVal */
  accum = (accum << bitsLeft)
    | (currentVal & (valMask >> (numBitsList[j] - bitsLeft)));
  bitsInAccum += bitsLeft;
  while (bitsInAccum >= bitElemBits)
  {
    *p++ = accum >> (bitsInAccum - bitElemBits);
    bitsInAccum -= bitElemBits;
  }
  if (bitsInAccum)
  {
    unsigned long mask =
      ~(unsigned long)0 << (bitElemBits - bitsInAccum);
    *p = (*p & ~mask) | ((accum << (bitElemBits - bitsInAccum))& mask);
  }
}

/*
  Copyright (C) 2007 Thomas Jahns <Thomas.Jahns@gmx.net>

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

void
gt_bsGetUniformUInt16ArrayAdd(constBitString str, BitOffset offset,
                           unsigned numBits, size_t numValues,
                           uint16_t val[])
{
  /* idea: read as much as possible from str in each iteration,
   * accumulate if bitsLeft < numBits */
  size_t j = 0;
  BitOffset totalBitsLeft = numValues * numBits;
  size_t elemStart = offset/bitElemBits;
  unsigned bitTop = offset%bitElemBits,
    bitsRead = 0; /*< how many bits in current *p are read */
  const BitElem *p = str + elemStart;
  unsigned bitsInAccum = 0;
  unsigned long accum = 0, valMask = ~(unsigned long)0;
  if (numBits < (sizeof (val[0])*CHAR_BIT))
    valMask = ~(valMask << numBits);
  gt_assert(str && val);
  gt_assert(numBits <= sizeof (val[0])*CHAR_BIT);
  /* user requested zero values, ugly but must be handled, since legal */
  if (!totalBitsLeft)
  {
    return;
  }
  /* get bits of first element if not aligned */
  if (bitTop)
  {
    unsigned long mask; /*< all of the bits we want to get from *p */
    unsigned bits2Read = MIN(bitElemBits - bitTop, totalBitsLeft);
    unsigned unreadRightBits = (bitElemBits - bitTop - bits2Read);
    mask = (~((~(unsigned long)0) << bits2Read)) << unreadRightBits;
    accum = ((*p++) & mask) >> unreadRightBits;
    bitsInAccum += bits2Read;
    totalBitsLeft -= bits2Read;
  }
  while (j < numValues)
  {
    while (bitsInAccum < numBits && totalBitsLeft)
    {
      unsigned bits2Read, bitsFree = sizeof (accum)*CHAR_BIT - bitsInAccum;
      unsigned long mask;
      bits2Read = MIN3(bitsFree, bitElemBits - bitsRead, totalBitsLeft);
      mask = (~((~(unsigned long)0) << bits2Read));
      accum = accum << bits2Read | (((*p) >> (bitElemBits
                                              - bits2Read - bitsRead)) & mask);
      bitsInAccum += bits2Read;
      totalBitsLeft -= bits2Read;
      /* all of *p consumed? */
      if ((bitsRead += bits2Read) == bitElemBits)
      {
        ++p, bitsRead = 0;
      }
    }
    /* now we have enough bits in accum */
    while (bitsInAccum >= numBits)
    {
      val[j++] += ((accum >> (bitsInAccum - numBits)) & valMask);
      bitsInAccum -= numBits;
    }
  }
}

void
gt_bsGetNonUniformUInt16ArrayAdd(
  constBitString str, BitOffset offset, size_t numValues,
  BitOffset numBitsTotal, unsigned numBitsList[], uint16_t val[])
{
  /* idea: read as much as possible from str in each iteration,
   * accumulate if bitsLeft < numBits */
  size_t j = 0;
  BitOffset totalBitsLeft = numBitsTotal;
  size_t elemStart = offset/bitElemBits;
  unsigned bitTop = offset%bitElemBits,
    bitsRead = 0; /*< how many bits in current *p are read */
  const BitElem *p = str + elemStart;
  unsigned bitsInAccum = 0;
  unsigned long accum = 0;
  gt_assert(str && val);
  /* user requested zero values, ugly but must be handled, since legal */
  if (!totalBitsLeft)
  {
    return;
  }
  /* get bits of first element if not aligned */
  if (bitTop)
  {
    unsigned long mask; /*< all of the bits we want to get from *p */
    unsigned bits2Read = MIN(bitElemBits - bitTop, totalBitsLeft);
    unsigned unreadRightBits = (bitElemBits - bitTop - bits2Read);
    mask = (~((~(unsigned long)0) << bits2Read)) << unreadRightBits;
    accum = ((*p++) & mask) >> unreadRightBits;
    bitsInAccum += bits2Read;
    totalBitsLeft -= bits2Read;
  }
  while (j < numValues)
  {
    while (bitsInAccum < numBitsList[j] && totalBitsLeft)
    {
      unsigned bits2Read, bitsFree = sizeof (accum)*CHAR_BIT - bitsInAccum;
      unsigned long mask;
      bits2Read = MIN3(bitsFree, bitElemBits - bitsRead, totalBitsLeft);
      mask = (~((~(unsigned long)0) << bits2Read));
      accum = accum << bits2Read | (((*p) >> (bitElemBits
                                              - bits2Read - bitsRead)) & mask);
      bitsInAccum += bits2Read;
      totalBitsLeft -= bits2Read;
      /* all of *p consumed? */
      if ((bitsRead += bits2Read) == bitElemBits)
      {
        ++p, bitsRead = 0;
      }
    }
    {
      unsigned numBits;
      /* now we have enough bits in accum */
      while (j < numValues && bitsInAccum >= (numBits = numBitsList[j]))
      {
        unsigned long valMask;
        gt_assert(numBits <= sizeof (val[0])*CHAR_BIT);
        valMask = ~(unsigned long)0;
        if (numBits < (sizeof (val[0])*CHAR_BIT))
          valMask = ~(valMask << numBits);
        val[j++] += ((accum >> (bitsInAccum - numBits)) & valMask );
        bitsInAccum -= numBits;
      }
    }
  }
}

void
gt_bsGetNonUniformInt16ArrayAdd(
  constBitString str, BitOffset offset, size_t numValues,
  BitOffset numBitsTotal, unsigned numBitsList[], int16_t val[])
{
  /* idea: read as much as possible from str in each iteration,
   * accumulate if bitsLeft < numBits */
  size_t j = 0;
  BitOffset totalBitsLeft = numBitsTotal;
  size_t elemStart = offset/bitElemBits;
  unsigned bitTop = offset%bitElemBits,
    bitsRead = 0; /*< how many bits in current *p are read */
  const BitElem *p = str + elemStart;
  unsigned bitsInAccum = 0;
  unsigned long accum = 0;
  gt_assert(str && val);
  /* user requested zero values, ugly but must be handled, since legal */
  if (!totalBitsLeft)
  {
    return;
  }
  /* get bits of first element if not aligned */
  if (bitTop)
  {
    unsigned long mask; /*< all of the bits we want to get from *p */
    unsigned bits2Read = MIN(bitElemBits - bitTop, totalBitsLeft);
    unsigned unreadRightBits = (bitElemBits - bitTop - bits2Read);
    mask = (~((~(unsigned long)0) << bits2Read)) << unreadRightBits;
    accum = ((*p++) & mask) >> unreadRightBits;
    bitsInAccum += bits2Read;
    totalBitsLeft -= bits2Read;
  }
  while (j < numValues)
  {
    while (bitsInAccum < numBitsList[j] && totalBitsLeft)
    {
      unsigned bits2Read, bitsFree = sizeof (accum)*CHAR_BIT - bitsInAccum;
      unsigned long mask;
      bits2Read = MIN3(bitsFree, bitElemBits - bitsRead, totalBitsLeft);
      mask = (~((~(unsigned long)0) << bits2Read));
      accum = accum << bits2Read | (((*p) >> (bitElemBits
                                              - bits2Read - bitsRead)) & mask);
      bitsInAccum += bits2Read;
      totalBitsLeft -= bits2Read;
      /* all of *p consumed? */
      if ((bitsRead += bits2Read) == bitElemBits)
      {
        ++p, bitsRead = 0;
      }
    }
    {
      unsigned numBits;
      /* now we have enough bits in accum */
      while (j < numValues && bitsInAccum >= (numBits = numBitsList[j]))
      {
        unsigned long valMask = (numBits < 16)
          ? ~((~(unsigned long)0) << numBits) : ~(unsigned long)0;
        int16_t m = (int16_t)1 << (numBits - 1);
        gt_assert(numBits <= sizeof (val[0])*CHAR_BIT);
        val[j++] += ((((accum >> (bitsInAccum - numBits)) & valMask)
                              ^ m) - m);
        bitsInAccum -= numBits;
      }
    }
  }
}
