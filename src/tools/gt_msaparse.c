/*
  Copyright (c) 2007-2008 Gordon Gremme <gremme@zbh.uni-hamburg.de>
  Copyright (c) 2007-2008 Center for Bioinformatics, University of Hamburg

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

#include "core/fileutils.h"
#include "core/option.h"
#include "core/versionfunc.h"
#include "extended/msa.h"
#include "tools/gt_msaparse.h"

typedef struct {
  bool show,
       consensus,
       sumofpairs;
} MSAparse_arguments;

static OPrval parse_options(int *parsed_args, MSAparse_arguments *arguments,
                            int argc, const char **argv, GtError *err)
{
  GtOptionParser *op;
  GtOption *o;
  OPrval oprval;
  gt_error_check(err);
  op = gt_option_parser_new("[option ...] MSA_file",
                         "Parse multiple sequence alignment (MSA) file and "
                         "optionally show score(s).");
  /* -show */
  o = gt_option_new_bool("show", "show the parsed MSA on stdout",
                         &arguments->show,
                         false);
  gt_option_parser_add_option(op, o);
  /* -consensus */
  o = gt_option_new_bool("consensus", "show consensus distance",
                      &arguments->consensus, false);
  gt_option_parser_add_option(op, o);
  /* -sumofpairs */
  o = gt_option_new_bool("sumofpairs", "show optimal sum of pairwise scores",
                      &arguments->sumofpairs, false);
  gt_option_parser_add_option(op, o);
  /* parse */
  gt_option_parser_set_min_max_args(op, 1, 1);
  oprval = gt_option_parser_parse(op, parsed_args, argc, argv, gt_versionfunc,
                                  err);
  gt_option_parser_delete(op);
  return oprval;
}

int gt_msaparse(int argc, const char **argv, GtError *err)
{
  MSAparse_arguments arguments;
  int parsed_args, had_err = 0;
  GtMSA *msa = NULL;
  gt_error_check(err);

  /* option parsing */
  switch (parse_options(&parsed_args, &arguments, argc, argv, err)) {
    case OPTIONPARSER_OK: break;
    case OPTIONPARSER_ERROR: return -1;
    case OPTIONPARSER_REQUESTS_EXIT: return 0;
  }

  /* make sure sequence_file exists */
  gt_assert(parsed_args < argc);
  if (!gt_file_exists(argv[parsed_args])) {
    gt_error_set(err, "MSA_file '%s' does not exist", argv[parsed_args]);
    had_err = -1;
  }

  if (!had_err) {
    /* multiple sequence alignment construction */
    msa = gt_msa_new(argv[parsed_args], err);
    if (!msa)
      had_err = -1;
  }

  if (!had_err) {
    /* output */
    if (arguments.show)
      gt_msa_show(msa);
    if (arguments.consensus)
      printf("consensus distance: %lu\n", gt_msa_consensus_distance(msa));
    if (arguments.sumofpairs) {
      printf("sum of pairwise scores: %lu\n",
             gt_msa_sum_of_pairwise_scores(msa));
    }
  }

  /* free */
  gt_msa_delete(msa);

  return had_err;
}
