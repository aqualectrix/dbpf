import texRefProcessWrapper
import fileHandler
import os

from gooey import Gooey
from gooey import GooeyParser

def main(args):
    args = parse_args(args)

    texref_success = {}
    ref_library = {}

    print(args)

    if args.ReferenceFiles:
        print("Gathering Reference Data...")

        for n, f in enumerate(args.ReferenceFiles, start = 1):
            print("Reading: " + str(n) + "/" + str(len(args.ReferenceFiles)))
            fileHandler.addToRefMap(f, ref_library)

        print()

    if args.FilesToUseReferences:
        print("Texture Referencing...")

        for n, f in enumerate(args.FilesToUseReferences, start = 1):
            print("Processing: " + str(n) + "/" + str(len(args.FilesToUseReferences)))
            texref_success[f] = fileHandler.texRefFile(f, ref_library, args.SubsetToReference, args.AlsoReferenceBumpmap)

        printSummary(texref_success)

@Gooey(
    # General
    advanced = True,
    program_name = "Texture Referencer",
    show_restart_button = False,

    # Progress
    progress_regex=r"^Processing: (?P<current>\d+)/(?P<total>\d+)$",
    progress_expr="current / total * 100",
    hide_progress_msg = True
)
def parse_args(args):
    parser = GooeyParser(description = "Texture reference one set of files to another.")

    parser.add_argument("-r", "--ReferenceFiles", help = "Files which hold the reference textures. These files will not be changed.", nargs = "*", widget = "MultiFileChooser")
    parser.add_argument("-f", "--FilesToUseReferences", help = "Files which will point to the reference textures. These files will be changed, and will then require their reference files to show up correctly in game.", nargs = "*", widget = "MultiFileChooser")

    parser.add_argument("-s", "--SubsetToReference", help = "The subset to texture reference (e.g. body, top, bottom, body_alpha, etc.")

    parser.add_argument("-b", "--AlsoReferenceBumpmap", help = "Also replace the bumpmap, if there is one, with a reference.", action='store_true', gooey_options={'initial_value': True})

    return parser.parse_args(args);

def printSummary(texref_success):
    if any(success for success in texref_success.values()):
        print("Texture Referenced:")
        for f, s in filter(lambda item: item[1], texref_success.items()):
            print(os.path.basename(f))
    if any(not success for success in texref_success.values()):
        print("Skipped:")
        for f, s in filter(lambda item: not item[1], texref_success.items()):
            print(os.path.basename(f))

if __name__ == '__main__':
    import sys
    main(sys.argv[1:])
