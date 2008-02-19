/*
  Copyright (c) 2007 Stefan Kurtz <kurtz@zbh.uni-hamburg.de>
  Copyright (c) 2007 Center for Bioinformatics, University of Hamburg

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

#ifndef SFX_OUTLCP_H
#define SFX_OUTLCP_H
#include <stdio.h>
#include "libgtcore/error.h"
#include "libgtcore/str.h"
#include "encseq-def.h"
#include "intcode-def.h"
#include "seqpos-def.h"

typedef struct Outlcpinfo Outlcpinfo;

Outlcpinfo *newlcpoutfileinfo(const Str *indexname,
                              unsigned int prefixlength,
                              unsigned int numofchars,
                              Seqpos totallength,
                              Error *err);
Seqpos getnumoflargelcpvalues(const Outlcpinfo *outlcpinfo);
Seqpos getmaxbranchdepth(const Outlcpinfo *outlcpinfo);
void freeoutlcptab(Outlcpinfo **outlcpinfo);

void sortallbuckets(Seqpos *suftabptr,
                    const Encodedsequence *encseq,
                    Readmode readmode,
                    Codetype mincode,
                    Codetype maxcode,
                    Seqpos totalwidth,
                    Seqpos previoussuffix,
                    const Seqpos *leftborder,
                    const Seqpos *countspecialcodes,
                    unsigned int numofchars,
                    unsigned int prefixlength,
                    const unsigned long **distpfxidx_startpointers,
                    const Codetype *filltable,
                    Outlcpinfo *outlcpinfo);

#endif
