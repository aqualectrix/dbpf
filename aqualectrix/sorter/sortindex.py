import sortProcessWrapper
import mapReader
import mapSorter
import os

from gooey import Gooey
from gooey import GooeyParser

def main(args):
    args = parse_args(args)

    sort_success = {}
    total = len(args.Filenames)

    print("Sorting...")

    if args.Index:
        for n, f in enumerate(args.Filenames, start=1):
            print("Processing: " + str(n) + "/" + str(total))
            sort_success[f] = sortProcessWrapper.sortindexFile(f, args.Index, args.GeneticizeHairs)

    if args.Mapfile:
        suffix_map = mapReader.parseMapFile(args.Mapfile)
        for n, f in enumerate(args.Filenames, start=1):
            print("Processing: " + str(n) + "/" + str(total))
            sort_success[f] = mapSorter.sortindexFile(f, suffix_map, args.GeneticizeHairs)

    printSummary(sort_success)

@Gooey(
    # General
    advanced = True,
    program_name = "SortIndexer",
    show_restart_button = False,

    # Progress
    progress_regex=r"^Processing: (?P<current>\d+)/(?P<total>\d+)$",
    progress_expr="current / total * 100",
    hide_progress_msg = True
)
def parse_args(args):
    parser = GooeyParser(description = "Sort any BodyShop content. Higher numbers come earlier in the catalog!")

    parser.add_argument("-g", "--GeneticizeHairs", action = "store_true", default = False, help = "For any files where the Property Set includes a hairtone, set the hairtone to a genetic value based on the sortindex.")

    indexGroup = parser.add_mutually_exclusive_group(required = True)
    indexGroup.add_argument("-i", "--Index", type=hex, help = "Value to apply to sortindex, in hex.")
    indexGroup.add_argument("-m", "--Mapfile", help = "File listing suffix:index pairs. Sortindex to apply is determined by the suffix (characters after the last _ in the file name) of the file.", widget = "FileChooser")

    parser.add_argument("Filenames", help = "File(s) to process", nargs = "*", widget="MultiFileChooser")

    return parser.parse_args(args);

# hex type for argparsing hex_index
def hex(hex_string):
    value = int(hex_string, 16)
    return value

def printSummary(sort_success):
    if any(success for success in sort_success.values()):
        print("Sorted:")
        for f, s in filter(lambda item: item[1], sort_success.items()):
            print(os.path.basename(f))
    if any(not success for success in sort_success.values()):
        print("Skipped:")
        for f, s in filter(lambda item: not item[1], sort_success.items()):
            print(os.path.basename(f))

if __name__ == '__main__':
    import sys
    main(sys.argv[1:])
