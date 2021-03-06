/*
  Copyright (c) 2014 Dirk Willrodt <willrodt@zbh.uni-hamburg.de>
  Copyright (c) 2014-2016 Center for Bioinformatics, University of Hamburg

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

/*
  THIS FILE IS GENERATED by
  scripts/gen-intsets.rb.
  DO NOT EDIT.
*/

#ifndef INTSET_32_H
#define INTSET_32_H

#include "core/types_api.h"
#include "extended/intset_rep.h"

/* The <GtIntset32> class implements the <GtIntset> interface.
   This class only works if <GtUword> is larger than 32 bits! */
typedef struct GtIntset32 GtIntset32;

/* Map static local methods to interface */
const     GtIntsetClass* gt_intset_32_class(void);

/* Return a new <GtIntset> object, the implementation beeing of type
   <GtIntset32>.
   Fails if 32 >= bits for (GtUword). */
GtIntset* gt_intset_32_new(GtUword maxelement, GtUword num_of_elems);

/* Returns true, if the <type> read from a file storing a <GtIntset> indicates
   the type of this implementation. */
bool      gt_intset_32_file_is_type(GtUword type);

/* Return a new <GtIntset> object, with data read from <fp> */
GtIntset* gt_intset_32_new_from_file(FILE *fp, GtError *err);

/* Add <elem> to <intset>. <elem> has to be larger than the previous <elem>
   added. */
void      gt_intset_32_add(GtIntset *intset, GtUword elem);

/* Returns the element at index <idx> in the sorted set <intset>. */
GtUword   gt_intset_32_get(GtIntset *intset, GtUword idx);

/* Returns actual number of stored elements */
GtUword   gt_intset_32_size(GtIntset *intset);

/* Returns <true> if <elem> is a member of the set <intset>. */
bool      gt_intset_32_is_member(GtIntset *intset, GtUword elem);

/* Returns the number of the element in <intset> that is the smallest element
   larger than or equal <value> or <num_of_elems> if there is no such <element>.
   */
GtUword   gt_intset_32_get_idx_smallest_geq(GtIntset *intset, GtUword value);

/* Write <intset> to file <fp>. Returns <NULL> on error (<intset> will be
   freed). */
GtIntset* gt_intset_32_write(GtIntset *intset, FILE *fp, GtError *err);

/* IO-function to be used if <intset> is part of a larger structure. If <intset>
   is NULL, will attempt to allocate memory and fill a new <GtIntset32>
   object by reading from <fp>. If <intset> is not NULL, will attempt to write
   its content to <fp>.
   Returns <NULL> on error (<intset> will be freed) and sets <err>. */
GtIntset* gt_intset_32_io(GtIntset *intset, FILE *fp, GtError *err);

/* Deletes <intset> and frees all associated space. */
void      gt_intset_32_delete(GtIntset *intset);

/* Returns the size of the representation of an intset with given number of
   elements <num_of_elems> and maximum value <maxelement>, in bytes. This does
   not include the size of the structure.
   Fails if 32 >= bits for (GtUword). */
size_t    gt_intset_32_size_of_rep(GtUword maxelement, GtUword num_of_elems);

/* Returns the size in bytes of the <GtIntset32>-structure. */
size_t    gt_intset_32_size_of_struct(void);

int gt_intset_32_unit_test(GtError *err);
#endif
