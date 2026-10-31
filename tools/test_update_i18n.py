# Copyright (C) 2026 Veritaware
# This file is released under the terms of the MIT license

import tempfile
import unittest
from pathlib import Path

from update_i18n import I18nExtractor


class ExtractorTest(unittest.TestCase):
    def setUp(self):
        self._tmp = tempfile.TemporaryDirectory()
        self.root = Path(self._tmp.name)
        (self.root / "src").mkdir()
        (self.root / "data" / "widgets").mkdir(parents=True)

    def tearDown(self):
        self._tmp.cleanup()

    def test_cpp_literals_are_extracted(self):
        (self.root / "src" / "a.cpp").write_text(
            'auto a = app::i18n("Shortcut: ");\n'
            'auto b = i18n("Plain", "fallback");\n'
            'auto c = app::i18n("Say \\"hi\\"");\n'
            'auto d = app::i18n("123");\n'
        )
        strings = I18nExtractor(self.root).extract_from_cpp()
        self.assertEqual({"Shortcut: ", "Plain", 'Say "hi"'}, strings)

    def test_non_literal_cpp_arguments_are_ignored(self):
        (self.root / "src" / "a.cpp").write_text("auto a = app::i18n(name);\n")
        self.assertEqual(set(), I18nExtractor(self.root).extract_from_cpp())

    def test_xml_text_is_extracted_and_opt_out_skipped(self):
        (self.root / "data" / "widgets" / "w.xml").write_text(
            '<gui><label text="&amp;Open"/>'
            '<entry text="Keep" i18n="false"/></gui>'
        )
        self.assertEqual({"Open"}, I18nExtractor(self.root).extract_from_xml())


if __name__ == "__main__":
    unittest.main()
