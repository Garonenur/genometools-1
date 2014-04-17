rm = gt.region_mapping_new_seqfile("LmjF_v6.1_all_20131105.fa")

derives_from = {}

describe.feature("gene", function(gene)
  it("should contain transcripts", function()
    expect(gene:has_child_of_type("mRNA")
             or gene:has_child_of_type("tRNA")
             or gene:has_child_of_type("snRNA")
             or gene:has_child_of_type("snoRNA")
             or gene:has_child_of_type("rRNA")
             or gene:has_child_of_type("SLRNA")
             or gene:has_child_of_type("ncRNA")).should_be(true)
  end)

  it("should adhere to a ID naming scheme, if coding", function()
    if gene:has_child_of_type("mRNA") then
      expect(gene:get_attribute("ID")).should_match("^"..gene:get_seqid()..".%d%d%d%d")
    else
      expect(gene:get_attribute("ID")).should_match("RNA")
    end
  end)

  it("should not be suspiciously short (>30nt)", function()
    local rng = gene:get_range()
    expect(rng:get_end() - rng:get_start() + 1).should_be_larger_than(30)
  end)

  it("should not span a contig separator sequence", function()
    local seq = string.lower(gene:extract_sequence("gene", false, rm))
    expect(seq).should_not_match(string.rep("n", 100))
  end)
end)

describe.feature("mRNA", function(mrna)
  it("should have a coding sequence", function()
    expect(mrna:has_child_of_type("CDS")).should_be(true)
  end)
end)

describe.feature("CDS", function(cds)
  if not (cds:appears_as_child_of_type("ncRNA")
            or cds:appears_as_child_of_type("snRNA")
            or cds:appears_as_child_of_type("snoRNA")
            or cds:appears_as_child_of_type("tRNA")
            or cds:appears_as_child_of_type("rRNA")) then
    it("should not contain internal stop codons", function()
      local seq = cds:extract_sequence("CDS", false, rm)
      expect(gt.translate_dna(string.sub(seq, 1, -3))).should_not_match("*")
    end)
  end
end)

describe.feature("polypeptide", function(pp)
  it("should be unique in its Derives_from", function()
    local dfrom = pp:get_attribute("Derives_from")
    expect(dfrom).should_not_be(nil)
    expect(derives_from).should_not_have_key(dfrom)
    derives_from[dfrom] = true
  end)
end)

describe.meta(function(meta)
  it("should have a valid URL in the feature-ontology directive", function()
    if meta:get_directive() == "feature-ontology" then
      expect(meta:get_data()).should_match("https?://[%w-_%.%?%.:/%+=&]+")
    end
  end)
end)

describe.region(function(region)
  it("should have sequence IDs start with organism shorthand", function()
    expect(region:get_seqid()).should_match("^LmjF")
  end)
end)

describe.comment(function(comment)
  it("should not be longer than 80 characters", function()
    expect(string.len(comment:get_comment())).should_be_smaller_than(80)
  end)
end)
