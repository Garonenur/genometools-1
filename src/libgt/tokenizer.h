/*
  Copyright (c) 2006-2007 Gordon Gremme <gremme@zbh.uni-hamburg.de>
  Copyright (c) 2006-2007 Center for Bioinformatics, University of Hamburg
  See LICENSE file or http://genometools.org/license.html for license details.
*/

#ifndef TOKENIZER_H
#define TOKENIZER_H

#include <stdbool.h>
#include <libgt/io.h>
#include <libgt/str.h>

typedef struct Tokenizer Tokenizer;

Tokenizer*    tokenizer_new(IO*, Env*); /* takes ownership */
/* activates the skipping of comment lines in the tokenizer (lines starting
   with '#') */
void          tokenizer_skip_comment_lines(Tokenizer*);
/* returns the current token */
Str*          tokenizer_get_token(Tokenizer*, Env*);
bool          tokenizer_has_token(Tokenizer*, Env*);
bool          tokenizer_line_start(const Tokenizer*);
void          tokenizer_next_token(Tokenizer*, Env*); /* go to the next token */
unsigned long tokenizer_get_line_number(const Tokenizer*);
const char*   tokenizer_get_filename(const Tokenizer*);
int           tokenizer_unit_test(Env*);
void          tokenizer_delete(Tokenizer*, Env*);

#endif
