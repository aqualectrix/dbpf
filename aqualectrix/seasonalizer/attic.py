import os
import shutil

def box(name, dir):
    # We set root_dir to the parent of dir so that files will be
    # zipped into a folder, and hence unzipped into a folder.
    shutil.make_archive(name, 'zip', os.path.join(dir, os.pardir), dir)
    shutil.rmtree(dir)

def unbox(file, location):
    shutil.unpack_archive(file, location, 'zip')
    os.remove(file)

# Tests
# This is just an experimentation area to ensure I understand how
# make_archive / unpack_arcive work; there's not really any non-built-in
# functionality to test.
import unittest

class TestAtticStorage(unittest.TestCase):
    TEST_TMP = "test_tmp"
    PACKABLE_DIR = "numbers"

    def test_golden(self):
        box(self.PACKABLE_DIR, self.PACKABLE_DIR)
        self.assertTrue(os.path.isfile(self.PACKABLE_DIR + ".zip"))
        self.assertFalse(os.path.exists(self.PACKABLE_DIR))

        unbox("numbers.zip", os.curdir)
        self.assertTrue(os.path.exists(self.PACKABLE_DIR))
        self.assertFalse(os.path.exists(self.PACKABLE_DIR + ".zip"))

    @classmethod
    def setUpClass(cls):
        # Add directory for tests to play in
        os.mkdir(cls.TEST_TMP)
        os.chdir(cls.TEST_TMP)

    def setUp(self):
        # Create files and folders to manipulate

        # Files within a directory
        os.mkdir(self.PACKABLE_DIR)

        with open(os.path.join(self.PACKABLE_DIR, "123.txt"), "w") as one23:
            one23.write("contained file 123")

        with open(os.path.join(self.PACKABLE_DIR, "456.txt"), "w") as four56:
            four56.write("contained file 456")

    def tearDown(self):
        # Remove any manipulations, leaving test_tmp empty
        with os.scandir(os.curdir) as contents:
            for entry in contents:
                if os.path.isfile(entry):
                    os.remove(entry)
                else:
                    shutil.rmtree(entry)

    @classmethod
    def tearDownClass(cls):
        # Remove test_tmp directory
        os.chdir(os.pardir)
        shutil.rmtree(cls.TEST_TMP)

class TestAtticUnbox(unittest.TestCase):

    def setUpClass(self):
        pass


# Main; runs tests
if __name__ == "__main__":
    unittest.main()
