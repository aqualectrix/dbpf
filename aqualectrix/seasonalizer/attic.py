import operator
import os
import shutil

from functools import reduce

Seasons = {"Spring": ["Spring"],
           "Summer": ["Summer"],
           "Fall/Autumn": ["Fall", "Autumn"],
           "Winter": ["Winter"]}

SeasonSet = set(reduce(operator.add, Seasons.values()))

def box(name, dir):
    '''
    Make a zip archive of the given dir, with the given name,
    then deletes the dir.
    '''
    # We set root_dir to the parent of dir so that files will be
    # zipped into a folder, and hence unzipped into a folder.
    shutil.make_archive(name, 'zip', os.path.join(dir, os.pardir), dir)
    shutil.rmtree(dir)

def unbox(file, location):
    '''
    Unzips the given archive file to the given location,
    then deletes the file.
    '''
    shutil.unpack_archive(file, location, 'zip')
    os.remove(file)

def isEligible(name):
    '''
    Returns true iff the name contains one of the entries in Seasons,
    in any case.
    '''
    return any(seasonname.casefold() in name.casefold()
               for seasonname in SeasonSet)

def isSeasonable(season, name):
    '''
    Returns true if the name contains the season (in any case).
    'Fall' and 'Autumn' are equivalent.
    '''
    return any(seasonname.casefold() in name.casefold()
               for seasonname in Seasons[season])


def pack_away_unseasonables(season, dir):
    '''
    Boxes up any subdirectories of dir that do *not* contain the season
    in their names (but do contain *some* season in their names).
    '''
    wd = os.getcwd()
    os.chdir(dir)
    try:
        with os.scandir(os.curdir) as entries:
            for entry in entries:
                if entry.is_dir() and isEligible(entry.name) and not isSeasonable(season, entry.name):
                    box(entry.name, entry.name)
    finally:
        os.chdir(wd)


def unpack_seasonables(season, dir):
    '''
    Unboxes any .zip files in dir that *do* contain the season in their names.
    '''
    wd = os.getcwd()
    os.chdir(dir)
    try:
        with os.scandir(os.curdir) as entries:
            for entry in entries:
                if entry.is_file() and entry.name.split('.')[-1] == 'zip' and isSeasonable(season, entry.name):
                    unbox(entry.name, os.curdir)
    finally:
        os.chdir(wd)

# Tests
import unittest

class TestAtticBoxes(unittest.TestCase):
    '''
    Basically just an experimentation area to ensure I understand how
    make_archive / unpack_archive work; there's not really any non-built-in
    functionality to test for box/unbox.
    '''

    TEST_TMP = "test_tmp"
    PACKABLE_DIR = "numbers"

    def test_goldenBoxUnbox(self):
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

class TestAtticNameChecks(unittest.TestCase):
    '''
    Test isEligible() and isSeasonable().
    '''

    def test_isEligibleRejectsNonSeasonables(self):
        self.assertFalse(isEligible("Nonsense name"))

    def test_isEligibleIgnoresCase(self):
        self.assertTrue(isEligible("SPRING"))

    def test_isEligibleAcceptsSeasonables(self):
        self.assertTrue(isEligible("FALL"))

    def test_isSeasonableRejectsIneligibles(self):
        self.assertFalse(isSeasonable("Spring", "Nonsense name"))

    def test_isSeasonableIgnoresCase(self):
        self.assertTrue(isSeasonable("Spring", "SPRING"))

    def test_isSeasonableHandlesFallAndAutumn(self):
        self.assertTrue(isSeasonable("Fall/Autumn", "Fall"))
        self.assertTrue(isSeasonable("Fall/Autumn", "Autumn"))

class TestAtticPacking(unittest.TestCase):
    '''
    Tests packing unseasonables and unpacking seasonables.
    '''
    TEST_TMP = "season_tmp"

    def test_packDoesNotPackStrayFiles(self):
        season = "Summer"
        stray = "Just some file.txt"
        with open(stray, "w") as f:
            f.write("Single files don't get packed.")

        pack_away_unseasonables(season, os.curdir)

        self.assertFalse(isEligible(stray))
        self.assertTrue(os.path.exists(stray))
        self.assertFalse(os.path.exists(stray + ".zip"))

    def test_packDoesNotPackStrayUnseasonableFiles(self):
        season = "Summer"
        unseasonable = "Spring.txt"
        with open(unseasonable, "w") as f:
            f.write("Single files, even if unseasonable, don't get packed.")

        pack_away_unseasonables(season, os.curdir)

        self.assertTrue(isEligible(unseasonable))
        self.assertFalse(isSeasonable(season, unseasonable))
        self.assertTrue(os.path.exists(unseasonable))
        self.assertFalse(os.path.exists(unseasonable + ".zip"))

    def test_packDoesNotPackUninvolvedFolders(self):
        season = "Summer"
        uninvolved = "All"
        os.mkdir(uninvolved)
        with open(os.path.join(uninvolved, uninvolved + ".txt"), "w") as f:
            f.write("This file's folder shouldn't be touched since it has no season names in it.")

        pack_away_unseasonables(season, os.curdir)

        self.assertFalse(isEligible(uninvolved))
        self.assertTrue(os.path.exists(uninvolved))
        self.assertFalse(os.path.exists(uninvolved + ".zip"))

    def test_unpackDoesNotUnpackUninvolvedZips(self):
        season = "Summer"
        uninvolved = "Backups.zip"
        with open(uninvolved, "w") as f:
            f.write("This is not a real zipfile :)")

        unpack_seasonables(season, os.curdir)

        self.assertFalse(isEligible(uninvolved))
        self.assertFalse(isSeasonable(season, uninvolved)) # trivial since ineligible
        self.assertTrue(os.path.exists(uninvolved))
        self.assertFalse(os.path.exists(uninvolved[:-4]))

    def test_packDoesNotPackSeasonableFolders(self):
        season = "Summer"
        seasonable = season + " + Spring"
        os.mkdir(seasonable)
        with open(os.path.join(seasonable, seasonable + ".txt"), "w") as f:
            f.write("This file's folder shouldn't be touched since it's seasonable.")

        pack_away_unseasonables(season, os.curdir)

        self.assertTrue(isEligible(seasonable))
        self.assertTrue(isSeasonable(season, seasonable))
        self.assertTrue(os.path.exists(seasonable))
        self.assertFalse(os.path.exists(seasonable + ".txt"))

    def test_unpackUnpacksSeasonableZips(self):
        season = "Summer"
        seasonable = season + " + Fall"
        os.mkdir(seasonable)
        with open(os.path.join(seasonable, seasonable + ".txt"), "w") as f:
            f.write("Summer & Fall")
        box(seasonable, seasonable)

        unpack_seasonables(season, os.curdir)

        self.assertTrue(isEligible(seasonable))
        self.assertTrue(isSeasonable(season, seasonable))
        self.assertTrue(os.path.exists(seasonable))
        self.assertFalse(os.path.exists(seasonable + ".zip"))

    def test_packPacksUnseasonableFolders(self):
        season = "Summer"
        unseasonable = "Fall & Winter"
        os.mkdir(unseasonable)
        with open(os.path.join(unseasonable, unseasonable + ".txt"), "w") as f:
            f.write("This file's folder should get packed!")

        pack_away_unseasonables(season, os.curdir)

        self.assertTrue(isEligible(unseasonable))
        self.assertFalse(isSeasonable(season, unseasonable))
        self.assertFalse(os.path.exists(unseasonable))
        self.assertTrue(os.path.exists(unseasonable + ".zip"))

    def test_unpackDoesNotUnpackUnseasonableZips(self):
        season = "Summer"
        unseasonable = "Fall & Winter.zip"
        with open(unseasonable, "w") as f:
            f.write("This is not a real zip file.")

        unpack_seasonables(season, os.curdir)

        self.assertTrue(isEligible(unseasonable))
        self.assertFalse(isSeasonable(season, unseasonable))
        self.assertTrue(os.path.exists(unseasonable))
        self.assertFalse(os.path.exists(unseasonable[:-4]))

    @classmethod
    def setUpClass(cls):
        # Add directory for tests to play in
        os.mkdir(cls.TEST_TMP)
        os.chdir(cls.TEST_TMP)

    def tearDown(self):
        # Remove any manipulations, leaving TEST_TMP empty
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

# Main; runs tests
if __name__ == "__main__":
    unittest.main()
