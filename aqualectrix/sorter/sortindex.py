import sortProcessWrapper
import mapReader
import mapSorter

from gooey import Gooey
from gooey import GooeyParser

def main(args):
    args = parse_args(args)

    if args.Index:
        for f in args.Filenames:
            sortProcessWrapper.sortindexFile(f, args.Index)

    if args.Mapfile:
        suffix_map = mapReader.parseMapFile(args.Mapfile)
        for f in args.Filenames:
            mapSorter.sortindexFile(f, suffix_map)

@Gooey(
    # General
    advanced = True,
    program_name = "SortIndexer",
    show_restart_button = False
)
def parse_args(args):
    parser = GooeyParser(description = "Sort any BodyShop content. Higher numbers come earlier in the catalog!")

    indexGroup = parser.add_mutually_exclusive_group(required = True)
    indexGroup.add_argument("-i", "--Index", type=hex, help = "Value to apply to sortindex, in hex.")
    indexGroup.add_argument("-m", "--Mapfile", help = "File listing suffix:index pairs. Sortindex to apply is determined by the suffix (characters after the last _ in the file name) of the file.", widget = "FileChooser")

    parser.add_argument("Filenames", help = "File(s) to process", nargs = "*", widget="MultiFileChooser")

    return parser.parse_args(args);

# hex type for argparsing hex_index
def hex(hex_string):
    value = int(hex_string, 16)
    return value

if __name__ == '__main__':
    import sys
    main(sys.argv[1:])
