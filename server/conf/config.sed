###############################################################################
#                                                                             #
# config.sed                                                                  #
#                                                                             #
# OS9                                                                         #
#                                                                             #
# 05.09.93                                                                    #
#                                                                             #
###############################################################################
1 i\
char configuration[]="\\
$ a\
";
/^$/ d
s!"!\\"!g
s!$!\\n\\!
