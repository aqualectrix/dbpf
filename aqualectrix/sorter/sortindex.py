import argparse
import sortProcessWrapper

from gooey import Gooey

def main(args):
    args = parse_args(args)

    for f in args.filenames:
        sortProcessWrapper.sortindexFile(f, args.hex_index)

@Gooey
def parse_args(args):
    parser = argparse.ArgumentParser(description = "Change the sortindex of the given files to the given hex value.")
    parser.add_argument("hex_index", type=hex, help = "Value to apply to sortindex, in hex.")
    parser.add_argument("filenames", help = "File(s) to process", nargs = "*")

    return parser.parse_args(args);

# hex type for argparsing hex_index
def hex(hex_string):
    value = int(hex_string, 16)
    return value

if __name__ == '__main__':
    import sys
    main(sys.argv[1:])
