/*
  Copyright (c) 2003-2010 Gordon Gremme <gremme@zbh.uni-hamburg.de>
  Copyright (c) 2003-2008 Center for Bioinformatics, University of Hamburg

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

#ifndef PARSE_OPTIONS_H
#define PARSE_OPTIONS_H

#include "core/option.h"
#include "gth/call_info.h"
#include "gth/input.h"
#include "gth/jump_table.h"
#include "gth/stat.h"

#define FROMPOS_OPT_CSTR  "frompos"

/* The following function parses the arguments given by the <argv> array of
   length <argc>. */
GtOPrval gth_parse_options(GthCallInfo*, GthInput*, int *parsed_args, int argc,
                           const char **argv, bool gthconsensus_parsing,
                           GtStrArray *consensusfiles, GthStat*,
                           void(*showverbose)(const char *),
                           void(*showverboseVM)(char *),
                           GtShowVersionFunc, GthJumpTableNew, GtError*);

#endif
