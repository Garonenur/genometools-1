/*
  Copyright (c) 2006 Gordon Gremme <gremme@zbh.uni-hamburg.de>
  Copyright (c) 2006 Center for Bioinformatics, University of Hamburg
  See LICENSE file or http://genometools.org/license.html for license details.
*/

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <libgt/compare.h>
#include <libgt/genome_feature_type.h>

/*
  Keep in sync with ``genome_feature_type'' in ``genome_feature.h''!
  The feature types have to be sorted. We assume ASCII encoding.
*/

static const char *genome_feature_type_strings[] = { "CDS",
                                                     "TF_binding_site",
                                                     "exon",
                                                     "gene",
                                                     "intron",
                                                     "mRNA" };

int genome_feature_type_get(GenomeFeatureType *type, char *gft_string)
{
  void *result;

  assert(type && gft_string);

  result = bsearch(&gft_string,
                   genome_feature_type_strings,
                   sizeof (genome_feature_type_strings) /
                   sizeof (genome_feature_type_strings[0]),
                   sizeof (char*),
                   compare);

  if (result) {
    *type = (GenomeFeatureType)
            ((char**) result - (char**) genome_feature_type_strings);
    return 0;
  }
  /* else type not found */
  return -1;
}

const char* genome_feature_type_get_cstr(GenomeFeatureType type)
{
  return genome_feature_type_strings[type];
}

unsigned long genome_feature_type_num_of_features(void)
{
  return sizeof (genome_feature_type_strings) /
         sizeof (genome_feature_type_strings[0]);
}
