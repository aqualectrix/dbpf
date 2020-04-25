import warnings

# Monkeypatch warnings.showwarning for user-friendly output
# rather than dev-friendly output.
def showSimpleWarning(message, category, filename, lineno, file=None, line=None):
    print("WARNING: " + str(message))
warnings.showwarning = showSimpleWarning

def parseMapFile(filename):
    with open(filename, 'r') as f:
        return parseLines(f.readlines())

def parseLines(lines):
    keymap = {}
    linenum = 0
    for line in lines:
        linenum += 1
        line = line.strip()

        # Don't process empty or comment lines
        if not line or line[0] == '#':
            continue

        # Otherwise, lowercase to ignore case
        line = line.casefold()

        # Split on ':' to get name and hexindex
        parts = [p.strip() for p in line.split(':')]

        # provide helpful error messages
        if not parts or not parts[0]:
            warnings.warn("Key name missing on line " + str(linenum) + ". Skipping this line.", stacklevel=2)
            continue
        if len(parts) < 2 or not parts[1]:
            warnings.warn("Index missing on line " + str(linenum) + ". Skipping this line; " + parts[0] + " will not be sorted.", stacklevel=2)
            continue

        name = parts[0]
        hexstring = parts[1]

        # massage hexindex; allow both OxABC and ABC
        if not hexstring.startswith('0x'):
            hexstring = "0x" + hexstring

        try:
            index = int(hexstring, 16)
        except ValueError:
            warnings.warn("Invalid index " + hexstring + " on line " + str(linenum) + "Skipping this line; " + parts[0] + " will not be sorted.", stacklevel=2)
            continue

        # add to map
        keymap[name] = index

    return keymap

# Tests
import unittest

class TestMapParser(unittest.TestCase):

    def test_empty(self):
        keymap = parseLines([])
        self.assertEqual(keymap, {})

    def test_emptyLines(self):
        keymap = parseLines(["\n", "  ", "\t"])
        self.assertEqual(keymap, {})

    def test_comments(self):
        keymap = parseLines(["#", "  #", "#  "])
        self.assertEqual(keymap, {})

    def test_parseAsHex(self):
        keymap = parseLines(["red : 0x123",
                          "blue : 456",
                          "green : 0xabc",
                          "yellow : def"])
        expected = {"red": 0x123,
                  "blue": 0x456,
                  "green": 0xABC,
                  "yellow": 0xDEF}
        self.assertEqual(keymap, expected)

    def test_invalidHex(self):
        self.assertWarns(UserWarning, parseLines, ["red : efg"])

    def test_golden(self):
        keymap = parseLines(["red : 123",
                            "blue : abc"])
        expected = {"red": 0x123,
                   "blue": 0xABC}
        self.assertEqual(keymap, expected)

    def test_whitespaceStripping(self):
        keymap = parseLines(["red:123",
                            "  blue:456",
                            "green:789\t ",
                            "  yellow\t:    abc"])
        expected = {"red":    0x123,
                  "blue":   0x456,
                  "green":  0x789,
                  "yellow": 0xABC}
        self.assertEqual(keymap, expected)

    def test_noName(self):
        self.assertWarns(UserWarning, parseLines, [": 123"])

    def test_noIndex(self):
        self.assertWarns(UserWarning, parseLines, ["red:"])


# Main; runs tests
if __name__ == "__main__":
    unittest.main()
