import os

from gooey import Gooey
from gooey import GooeyParser

def main(args):
    args = parse_args(args)

    print(args)

# @Gooey
def parse_args(args):
    parser = GooeyParser(description = "Zip up unseasonable folders and unzip seasonable folders.")
    parser.add_argument("season", choices=["Spring", "Summer", "Fall/Autumn", "Winter"], help = "The season to make clothes available for. Folders with 'Autumn' in the name are treated the same as folders with 'Fall' in the name.")
    parser.add_argument("folders", help = "Folders containing subfolders to seasonalize", nargs = "*", widget="MultiDirChooser")

    return parser.parse_args(args);

if __name__ == '__main__':
    import sys
    main(sys.argv[1:])
