Name "gt parser test (open match format)"
Keywords "gt_matchtool"
Test do
  run_test "#{$bin}gt matchtool -matchfile #{$testdata}matchtool_open.match"
  run "diff #{last_stdout} #{$testdata}matchtool_open.out"
end

Name "gt parser test (open match format corrupt)"
Keywords "gt_matchtool"
Test do
  run_test "#{$bin}gt matchtool -matchfile #{$testdata}matchtool_open_corrupt.match",
  :retval => 1
  grep last_stderr, /invalid format/
end

Name "gt parser test (open match format empty)"
Keywords "gt_matchtool"
Test do
  run_test "#{$bin}gt matchtool -matchfile #{$testdata}matchtool_open_empty.match"
  run "diff #{last_stdout} #{$testdata}matchtool_open_empty.out"
end

Name "gt parser test (open match format neg value)"
Keywords "gt_matchtool"
Test do
  run_test "#{$bin}gt matchtool -matchfile #{$testdata}matchtool_open_neg.match",
  :retval => 1
  grep last_stderr, /non-negative/
end

Name "gt parser test (open match format too few columns)"
Keywords "gt_matchtool"
Test do
  run_test "#{$bin}gt matchtool -matchfile #{$testdata}matchtool_open_too_few.match",
  :retval => 1
  grep last_stderr, /invalid format/
end

Name "gt parser test (open match format too many columns)"
Keywords "gt_matchtool"
Test do
  run_test "#{$bin}gt matchtool -matchfile #{$testdata}matchtool_open_too_many.match",
  :retval => 1
  grep last_stderr, /invalid format/
end

Name "gt parser test (open match format gz)"
Keywords "gt_matchtool"
Test do
  run_test "#{$bin}gt matchtool -matchfile #{$testdata}matchtool_open.match.gz"
  run "diff #{last_stdout} #{$testdata}matchtool_open.out"
end

Name "gt parser test (open match format bz2)"
Keywords "gt_matchtool"
Test do
  run_test "#{$bin}gt matchtool -matchfile #{$testdata}matchtool_open.match.bz2"
  run "diff #{last_stdout} #{$testdata}matchtool_open.out"
end

Name "gt parser test (blast match format)"
Keywords "gt_matchtool"
Test do
  run_test "#{$bin}gt matchtool -type BLASTOUT -matchfile #{$testdata}matchtool_blast.match"
  run "diff #{last_stdout} #{$testdata}matchtool_blast.out"
end

Name "gt parser test (blast match format corrupt)"
Keywords "gt_matchtool"
Test do
  run_test "#{$bin}gt matchtool -type BLASTOUT -matchfile #{$testdata}matchtool_blast_corrupt.match",
  :retval => 1
  grep last_stderr, /invalid format/
end

Name "gt parser test (blast match format empty)"
Keywords "gt_matchtool"
Test do
  run_test "#{$bin}gt matchtool -type BLASTOUT -matchfile #{$testdata}matchtool_blast_empty.match"
  run "diff #{last_stdout} #{$testdata}matchtool_blast_empty.out"
end

Name "gt parser test (blast match format too few columns)"
Keywords "gt_matchtool"
Test do
  run_test "#{$bin}gt matchtool -type BLASTOUT -matchfile #{$testdata}matchtool_blast_too_few.match",
  :retval => 1
  grep last_stderr, /invalid format/
end

Name "gt parser test (blast match format too many columns)"
Keywords "gt_matchtool"
Test do
  run_test "#{$bin}gt matchtool -type BLASTOUT -matchfile #{$testdata}matchtool_blast_too_many.match",
  :retval => 1
  grep last_stderr, /invalid format/
end

Name "gt parser test (blast match format neg values)"
Keywords "gt_matchtool"
Test do
  run_test "#{$bin}gt matchtool -type BLASTOUT -matchfile #{$testdata}matchtool_blast_neg.match",
  :retval => 1
  grep last_stderr, /non-negative/
end

Name "gt parser test (blast match format gz)"
Keywords "gt_matchtool"
Test do
  run_test "#{$bin}gt matchtool -type BLASTOUT -matchfile #{$testdata}matchtool_blast.match.gz"
  run "diff #{last_stdout} #{$testdata}matchtool_blast.out"
end

Name "gt parser test (blast match format bz2)"
Keywords "gt_matchtool"
Test do
  run_test "#{$bin}gt matchtool -type BLASTOUT -matchfile #{$testdata}matchtool_blast.match.bz2"
  run "diff #{last_stdout} #{$testdata}matchtool_blast.out"
end

#Name "gt parser test (blastn match process)"
#Keywords "gt_matchtool"
#Test do
  #run_test "#{$bin}gt matchtool -type BLASTN -db #{$testdata}matchtool_blastn.db -query #{$testdata}matchtool_blastn.query"
  #run "diff #{last_stdout} #{$testdata}matchtool_blastn.out"
#end

#Name "gt parser test (blastalln match process)"
#Keywords "gt_matchtool"
#Test do
  #run_test "#{$bin}gt matchtool -type BLASTALLN -db #{$testdata}matchtool_blastn.db -query #{$testdata}matchtool_blastn.query"
  #run "diff #{last_stdout} #{$testdata}matchtool_blastalln.out"
#end

#Name "gt parser test (blastp match process)"
#Keywords "gt_matchtool"
#Test do
  #run_test "#{$bin}gt matchtool -type BLASTP -db #{$testdata}matchtool_blastp.db -query #{$testdata}matchtool_blastp.query"
  #run "diff #{last_stdout} #{$testdata}matchtool_blastp.out"
#end

#Name "gt parser test (blastallp match process)"
#Keywords "gt_matchtool"
#Test do
  #run_test "#{$bin}gt matchtool -type BLASTALLP -db #{$testdata}matchtool_blastp.db -query #{$testdata}matchtool_blastp.query"
  #run "diff #{last_stdout} #{$testdata}matchtool_blastallp.out"
#end
