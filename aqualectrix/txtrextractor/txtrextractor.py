import txtrExtractProcessWrapper
import os.path

from gooey import Gooey
from gooey import GooeyParser

def main(args):
	args = parse_args(args)

	prefix = get_prefix(args.filenames)
	if not prefix:
		prefix = "Extracted_"

	num_extracted = txtrExtractProcessWrapper.extractFromFiles(args.filenames, prefix)

@Gooey(
	program_name = "Texture Extractor"
)
def parse_args(args):
	parser = GooeyParser(description = "Extract all TXTR resources from the files and save them to a new file.")

	parser.add_argument("filenames", help="Files from which TXTRs should be extracted.", nargs = "*", widget = "MultiFileChooser")

	return parser.parse_args(args)

def get_prefix(filenames):
	basenames = [os.path.basename(file) for file in filenames]
	return os.path.commonprefix(basenames)

if __name__ == '__main__':
	import sys
	main(sys.argv[1:])
