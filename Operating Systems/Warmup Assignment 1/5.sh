                # need some sort of error output to indicate that the command
                #     cannot be completed, it does not have to exactly the
                #     error indicated below, but it MUST BE something reasonable
                #     to inform the user about the error condition
                # if a commandline is malformed, MUST give "usage" information
                #     as described in the spec
                # please note that commandline parsing is separate from
                #     other functionalities, so even though a student has
                #     declared that certain part of the assignments is not
                #     imlemented, commandling parsing still needs to be done
                #     for those commands
                set srcdir=~csci570b/public/cs402/warmup1

                ./warmup1
                    (malformed command)
                ./warmup1 -y sort
                    (malformed command)
                ./warmup1 xyz
                    (malformed command)
                ./warmup1 abc def ghi
                    (malformed command)
                ./warmup1 ksjdfjwiejofjasdfjowkejokjaosijfioejfsiejriwjeirjwier
                    (malformed command)

                ./warmup1 sort -x
                    (malformed command)
                ./warmup1 sort /usr/bin/xyzz
                    (input file /usr/bin/xyzz does not exist)
                ./warmup1 sort /etc/sysidcfg
                    (input file /etc/sysidcfg cannot be opened - access denies)
                ./warmup1 sort /etc/inet/secret/xyzz
                    (input file /etc/inet/secret/xyzz cannot be opened - access denies)
                ./warmup1 sort /etc
                    (input file /etc is a directory)
                ./warmup1 sort /etc/motd
                    (input file is not in the right format)
                ./warmup1 sort ~/.login
                    (input file is not in the right format)




                    