#!/usr/bin/env ruby

require 'optparse'
require 'ostruct'

class Randomsequence
  def initialize(alphabet,myseed=nil)
    @alphabet = alphabet
    @asize = alphabet.length
    if myseed.nil?
      seed = Random.new_seed
    else
      seed = myseed
    end
    STDERR.puts "seed=#{seed}"
    @rgen = Random.new(seed)
  end
  def sequence(len)
    s = Array.new()
    0.upto(len-1).each do
      s.push(@alphabet[@rgen.rand * @asize])
    end
    return s.join
  end
  def mutate(sequence,errperc,alphabet)
    len = sequence.length
    err_prob = errperc.to_f/100.0
    asize = alphabet.length
    s = Array.new()
    i = 0
    loop do
      r = @rgen.rand
      if r <= err_prob
        r = @rgen.rand
        if r <= 0.8
          s.push(alphabet[@rgen.rand * asize])
          i += 1
        elsif r <= 0.9
          s.push(alphabet[@rgen.rand * asize])
        else
          i += 1
        end
      else
        s.push(sequence[i])
        i += 1
      end
      if i >= len
       break
      end
    end
    return s.join("")
  end
end

def parseargs(argv)
  options = OpenStruct.new
  mode = nil
  minminid = 70
  maxminid = 99
  defaultminid = 80
  indent = 37
  minidrange = "[#{minminid}..#{maxminid}]"
  options.seedlength = nil
  options.totallength = nil
  options.pairparam = false
  options.mirrored = false
  options.seeded = false
  options.namedfiles = false
  options.withwildcards = false
  options.minidentity = defaultminid
  options.seednumber = Random.new_seed
  options.reverse = false
  options.number = 1
  options.mems = false
  options.seedcoverage = 0
  opts = OptionParser.new
  opts.on("-m","--mode STRING","specify mode: mirrored|seeded|pair") do |x|
    mode = x
  end
  opts.on("-s","--seedlength NUM","specify seed length for mirrored sequences") do |x|
    options.seedlength = x.to_i
  end
  opts.on("-l","--length NUM","specify total length of sequences") do |x|
    options.totallength = x.to_i
  end
  opts.on("-i","--minidentity NUM","specify minimum identity percentage in\n" +
            (" " * indent) + "range " + "#{minidrange}, " +
            "default is #{defaultminid}") do |x|
    options.minidentity = x.to_i
  end
  opts.on("-n","--namedfiles","store seeded matches in files db.fna and\n" +
                              (" " * indent) + "query.fna") do |x|
    options.namedfiles = true
  end
  opts.on("-w","--withwildcards","store wildcards at end of extensions") do |x|
    options.withwildcards = true
  end
  opts.on("-r","--reverse","reverse the query sequence in query of\n" +
                           (" " * indent) + "seeded sequence pair") do |x|
    options.reverse = true
  end
  opts.on("--mems","generate mems, i.e. the seed are maximal") do |x|
    options.mems = true
  end
  opts.on("--number NUM","specify the number of seeded pairs to be\n" +
                         (" " * indent) + "generated (default 1)") do |x|
    options.number = x.to_i
  end
  opts.on("--seed NUM","specify the seed for the random number\n" +
                       (" " * indent) + "generator to make sequences reproducible") do |x|
    options.seednumber = x.to_i
  end
  opts.on("-c","--seedcoverage NUM","generate multiple seeds that cover the\n" +
         (" " * indent) + "specified number of bases") do |x|
    options.seedcoverage = x.to_i
  end
  rest = opts.parse(argv)
  if rest.length != 0
    STDERR.puts "Usage: #{$0} [options]"
    exit 1
  end
  if mode.nil?
    STDERR.puts "#{$0}: option -m is mandatory"
    exit 1
  end
  if mode == "mirrored"
    options.mirrored = true
  elsif mode == "seeded"
    options.seeded = true
  elsif mode == "pair"
    options.pairparam = true
  else
    STDERR.puts "#{$0}: possible modes are mirrored|seeded|pair"
    exit 1
  end
  if not (options.seeded or options.mirrored)
    if not options.seedlength.nil?
      STDERR.puts "#{$0}: option -s requires option -m seeded or -m mirrored"
      exit 1
    end
    if options.namedfiles
      STDERR.puts "#{$0}: option -n requires option -m seeded or -m mirrored"
      exit 1
    end
  end
  if options.totallength.nil?
    STDERR.puts "#{$0}: option --length is mandatory"
    exit 1
  end
  if (options.mirrored or options.seeded) and options.seedlength.nil?
    STDERR.puts "#{$0}: option -mode mirrored and -m seeded imply option -s"
    exit 1
  end
  if options.seedcoverage != 0 and not options.seeded
    STDERR.puts "#{$0}: option -c requires option -m seeded"
    exit 1
  end
  if not options.seedlength.nil? and options.seedlength >= options.totallength
    STDERR.puts "#{$0}: seedlength must not be larger than totallength"
    exit 1
  end
  if options.minidentity < minminid or options.minidentity > maxminid
    STDERR.puts "#{$0}: minidentity must be in range #{minidrange}"
    exit 1
  end
  if options.seedcoverage >= options.totallength
    STDERR.puts "#{$0}: seedcoverage must not be larger than totallength"
    exit 1
  end
  return options
end

def gen_mirrored(fpdb,fpquery,rseq,options,alphabet,errperc)
  extendlength = (options.totallength - options.seedlength)/2
  seedstring = rseq.sequence(options.seedlength)
  leftcontext1 = rseq.sequence(extendlength)
  leftcontext2 = rseq.mutate(leftcontext1,errperc,alphabet)
  fpdb.puts ">seedlength=#{options.seedlength},extendlength=#{extendlength}," +
       "errperc=#{errperc}"
  fpdb.puts "#{leftcontext1}"
  fpdb.puts "#{seedstring}"
  fpdb.puts "#{leftcontext1.reverse}"
  fpquery.puts ">seedlength=#{options.seedlength},extendlength=#{extendlength}," +
       "errperc=#{errperc}"
  fpquery.puts "#{leftcontext2}"
  fpquery.puts "#{seedstring}"
  fpquery.puts "#{leftcontext2.reverse}"
end

def headkey(options,extendlength,errperc)
  return "seedlength=#{options.seedlength},extendlength=#{extendlength}," +
         "errperc=#{errperc}"
end

def gen_seeded(fpdb,fpquery,fpquery_r,rseq,options,alphabet,errperc)
  extendlength = (options.totallength - options.seedlength)/2
  seedstring = rseq.sequence(options.seedlength)
  leftcontext1 = rseq.sequence(extendlength)
  leftcontext2 = rseq.mutate(leftcontext1,errperc,alphabet)
  rightcontext1 = rseq.sequence(extendlength)
  rightcontext2 = rseq.mutate(rightcontext1,errperc,alphabet)
  if options.withwildcards
    wildcard = "N"
  else
    wildcard = ""
  end
  fpdb.puts ">db: #{headkey(options,extendlength,errperc)}"
  if options.mems
    left1 = "a"
    left2 = "c"
    right1 = "g"
    right2 = "t"
  else
    left1 = ""
    left2 = ""
    right1 = ""
    right2 = ""
  end
  fpdb.puts "#{leftcontext1}#{wildcard}#{left1}"
  fpdb.puts "#{seedstring}"
  fpdb.puts "#{right1}#{rightcontext1}"
  fpquery.puts ">query: #{headkey(options,extendlength,errperc)}"
  queryseq = "#{leftcontext2}#{left2}\n#{seedstring}\n#{right2}#{rightcontext2}"
  fpquery.puts queryseq
  if options.reverse
    fpquery_r.puts ">query-r: #{headkey(options,extendlength,errperc)}"
    fpquery_r.puts queryseq.reverse
  end
end

def gen_seeded_with_coverage(fpdb,fpquery,fpquery_r,rseq,options,alphabet,errperc)
  # build array with random seed positions covering >= seedcoverage positions
  pos = Array.new
  rgen = Random.new(options.seednumber)
  while pos.length < options.seedcoverage
    idx = rgen.rand * (options.totallength - options.seedlength + 1)
    idx = idx.to_i
    pos.concat((idx...idx + options.seedlength).to_a).sort!.uniq!
  end
  # convert array to ranges
  ranges = pos.inject([]) do |spans, n|
    if spans.empty? || spans.last.last != n - 1
      spans + [n..n]
    else
      spans[0..-2] + [spans.last.first..n]
    end
  end

  # generate sequences
  context1 = Array.new
  seedstring = Array.new
  prev = 0
  for range in ranges
    context1.push("#{rseq.sequence(range.begin - prev)}")
    range_size = range.last - range.first + 1
    seedstring.push("#{rseq.sequence(range_size)}")
    prev = range.last + 1
  end
  context1.push("#{rseq.sequence(options.totallength - prev)}")
  context2 = context1.map{|sequence| rseq.mutate(sequence, errperc, alphabet)}

  if options.withwildcards
    wildcard = "N"
  else
    wildcard = ""
  end
  if options.mems
    left1 = "a"
    left2 = "c"
    right1 = "g"
    right2 = "t"
  else
    left1 = ""
    left2 = ""
    right1 = ""
    right2 = ""
  end

  # print sequences
  extendlength = context1.map{|seq| seq.length}.join("+")
  fpdb.puts ">db: #{headkey(options,extendlength,errperc)}"
  dbseq = ""
  for i in (0...seedstring.length)
    dbseq += "#{context1[i]}#{wildcard}#{left1}\n#{seedstring[i]}\n#{right1}"
  end
  dbseq += "#{context1[seedstring.length]}"
  fpdb.puts dbseq

  extendlength = context2.map{|seq| seq.length}.join("+")
  fpquery.puts ">query: #{headkey(options,extendlength,errperc)}"
  queryseq = ""
  for i in (0...seedstring.length)
    queryseq += "#{context2[i]}#{left2}\n#{seedstring[i]}\n#{right2}"
  end
  queryseq += "#{context2[seedstring.length]}"
  fpquery.puts queryseq

  if options.reverse
    fpquery_r.puts ">query-r: #{headkey(options,extendlength,errperc)}"
    fpquery_r.puts queryseq.reverse
  end
end


def openoutfile(filename)
begin
  fp = File.new(filename,"w")
rescue => err
  STDERR.puts "cannot open #{filename}"
  exit 1
end
return fp
end

options = parseargs(ARGV)
alphabet = "acgt"
errperc = 100 - options.minidentity
rseq = Randomsequence.new(alphabet,options.seednumber)
if options.namedfiles
  fpdb = openoutfile("db.fna")
  fpquery = openoutfile("query.fna")
  if options.reverse
    fpquery_r = openoutfile("query-r.fna")
  else
    fpquery_r = nil
  end
else
  fpdb = STDOUT
  fpquery = STDOUT
  fpquery_r = STDOUT
end

if options.mirrored
  gen_mirrored(fpdb,fpquery,rseq,options,alphabet,errperc)
elsif options.seeded
  options.number.times do
    if options.seedcoverage == 0
      gen_seeded(fpdb,fpquery,fpquery_r,rseq,options,alphabet,errperc)
    else
      gen_seeded_with_coverage(fpdb,fpquery,fpquery_r,rseq,options,alphabet,errperc)
    end
  end
elsif options.pairparam
  seq1 = rseq.sequence(options.totallength)
  seq2 = rseq.sequence(options.totallength)
  puts "#{seq1} #{seq2}"
end
