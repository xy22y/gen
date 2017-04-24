# Password generator

This program will generate a random password, using pseudorandom data supplied by getentropy().
Command-line parameters can be used to specify a particular password policy, and the same
arguments can be placed into ~/.genrc to create a persistent configuration.

# Command line usage

```
Usage:
  gen <options>
    -u [count-spec]: specify number of upper case characters (default=unbounded)
    -l [count-spec]: specify number of lower case characters (default=unbounded)
    -n [count-spec]: specify number of numeric characters (default=unbounded)
    -s [count-spec]: specify number of special characters (default=unbounded)
    -L: specify password length (default=41)
    -x: hex characters only: 0=no, 1(default)=lower, 2=upper, 3=both (default=0)
    -c: exclude hard-to-read characters from password (like 0 and O) (default=no)
    -p <chars>: specify a custom character pool (default=none)
    -e <chars>: specify a custom set of exclude characters (default=none)
    -f <chars>: specify requirements for first character (default=none)
    -v: show verbose output

Note:
  [count-spec] is specified in "[min]-[max]" format.
```

# Configuration file

The configuration file should be placed in ~/.genrc. Sample content:

```
# this configuration generates passwords with 256 bits of entropy
-u -   # requirements for upper case letters [lowcount]-[highcount]
       #                                     dash alone means don't care
-l -   # "            "   lower case letters
-n -   # "            "   number characters
-s -   # "            "   special characters
-L 40  # "            "   password length
# -x [n]                                    # hex characters only
#                                           #   (1=lower,2=upper,3=mixed)
# -c                                        # exclude hard-to-read characters
# -p abcdefghijlkmnopqrstuvwxyz0123456789   # explicit character pool
# -e $%                                     # explicit exclude pool
# -f abcdefghijklmnopqrstuvwxyz             # first char must be
# -v                                        # verbose output
```

