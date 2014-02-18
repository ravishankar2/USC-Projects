set srcdir=~csci570b/public/cs402/warmup1

#
# If using C (well, you have to use C):
#
        /bin/rm -rf grading_$$
        mkdir grading_$$
        cd grading_$$
        cp ../my402list.c .
        cp $srcdir/cs402.h .
        cp $srcdir/my402list.h .
        cp $srcdir/listtest.c .
        cp $srcdir/Makefile .
        make

        #
        # for the following commands, each correct behavior gets 2 point
        # "./listtext" command should produce nothing
        #
        foreach f (0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19)
            echo "===> test_$f"
            ./listtest
        end
        cd ..
#
# Clean up temporary directory
#
/bin/rm -rf grading_$$



