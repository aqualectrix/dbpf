import sortProcessWrapper
import mapReader

from warnings import warn

from gooey import Gooey
from gooey import GooeyParser

def main(args):
    args = parse_args(args)

    if args.index:
        for f in args.filenames:
            sortProcessWrapper.sortindexFile(f, args.hex_index)

    if args.mapfile:
        suffix_map = mapReader.parseMapFile(args.mapfile)
        for f in args.filenames:
            suffix = f.split("_")[-1].split(".")[0].casefold()
            if not suffix_map[suffix]:
                warn("Suffix " + suffix + " was not found in your map. " + f + " will not be sorted.")
            else:
                sortProcessWrapper.sortindexFile(f, suffix_map[suffix])

@Gooey
def parse_args(args):
    parser = GooeyParser(description = "Change the sortindex of the given files to the given hex value.")

    indexGroup = parser.add_mutually_exclusive_group(required = True)
    indexGroup.add_argument("-i", "--index", type=hex, help = "Value to apply to sortindex, in hex.")
    indexGroup.add_argument("-m", "--mapfile", help = "File listing suffix:index pairs. Sortindex to apply is determined by the suffix (characters after the last _ in the file name) of the file.", widget = "FileChooser")

    parser.add_argument("filenames", help = "File(s) to process", nargs = "*", widget="MultiFileChooser")

    return parser.parse_args(args);

# hex type for argparsing hex_index
def hex(hex_string):
    value = int(hex_string, 16)
    return value

if __name__ == '__main__':
    import sys
    main(sys.argv[1:])
