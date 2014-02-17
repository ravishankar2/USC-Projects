set srcdir=~csci570b/public/cs402/warmup1

#
# (B1)
# for the following commands, each correct answer gets 2 point
# "diff" command should produce nothing
#
/bin/rm -f f?.sort
foreach f (0 1 2 3 4 5 6 7 8 9 10 11 12 13 14)
    echo "===> $srcdir/f$f"
    ./warmup1 sort $srcdir/f$f > f$f.sort
    diff $srcdir/f$f.sort f$f.sort
end
/bin/rm -f f?.sort

#
# (B2)
# for the following commands, each correct answer gets 2 point
# "diff" command should produce nothing
#
/bin/rm -f f??.sort
    foreach f (15 16 17 18 19 20 21 22 23 24 25 26 27 28 29)
    echo "===> $srcdir/f$f"
    cat $srcdir/f$f | ./warmup1 sort > f$f.sort
    diff $srcdir/f$f.sort f$f.sort
end
/bin/rm -f f??.sort



