import sortProcessWrapper
import warnings

# Monkeypatch warnings.showwarning for user-friendly output
# rather than dev-friendly output.
def showSimpleWarning(message, category, filename, lineno, file=None, line=None):
    print("WARNING: " + str(message))
warnings.showwarning = showSimpleWarning

def sortindexFile(filename, suffix_map, geneticize_hairs):
    # Get suffix, the last set of characters before the file suffix
    # and after an underscore (if there are any underscores).
    # In a_b_c.package, this is "c".
    # In A_B_C.mp.package, this is also "c".
    suffix = filename.split("_")[-1].split(".")[0].casefold()

    if suffix not in suffix_map:
        warnings.warn("Suffix '" + suffix + "' was not found in your map. " + filename + " will not be processed.")
        return False
    else:
        return sortProcessWrapper.sortindexFile(filename, suffix_map[suffix], geneticize_hairs)

# Tests
import unittest

class TestMapSorter(unittest.TestCase):

    def test_emptyFilename(self):
        self.assertWarns(UserWarning, sortindexFile, "", {"a":0xa}, false)

    def test_emptyMap(self):
        self.assertWarns(UserWarning, sortindexFile, "a_b.package", {}, false)

    def test_missingSuffix(self):
        self.assertWarns(UserWarning, sortindexFile, "a_b.package", {"a":0xa}, false)

    @unittest.expectedFailure
    def test_casefoldSuffix(self):
        # There SHOULD be a match between different cases,
        # so we expect this to fail.
        self.assertWarns(UserWarning, sortindexFile, "A_B.package", {"b":0xb}, false)

# Main; runs tests
if __name__ == "__main__":
    unittest.main()
