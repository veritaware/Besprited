#!/usr/bin/env python3
# Besprited | Copyright (C) 2026 Veritaware
#
# Checks (and optionally fixes) the Veritaware copyright header of changed
# source files, mirroring .github/workflows/copyright-check.yml: within the
# first 10 lines there must be a "Copyright (C) [YYYY-]<current year>
# Veritaware" line. See the "Copyright headers" section of CONTRIBUTING.md.
#
# Usage:
#   tools/check_copyright.py [--fix] --staged          # files in the index
#   tools/check_copyright.py [--fix] --base origin/trunk
#   tools/check_copyright.py [--fix] FILE...
#
# Exit status: 0 when every file is fine (or was fixed with --fix), 1 when
# some file is missing a valid header and --fix was not given.

import argparse
import datetime
import fnmatch
import os
import re
import subprocess
import sys

HEADER_LINES = 10
EXEMPT_FILE = ".github/copyright-exempt.txt"
SKIP_PREFIXES = ("third_party/",)

LICENSE_BLURB = [
    "This program is free software; you can redistribute it and/or modify",
    "it under the terms of the GNU General Public License version 2 as",
    "published by the Free Software Foundation.",
]

# "Copyright (C) 2021-2026   Holder" with an optional "Name |" column before it.
COPYRIGHT_RE = re.compile(
    r"^(?P<lead>\s*(?://|#|<!--)\s*)"
    r"(?:(?P<name>[^|\s][^|]*?)\s*\|\s*)?"
    r"(?P<cpy>[Cc]opyright\s+\([Cc]\)\s+)"
    r"(?P<years>\d{4}(?:-\d{4})?)"
    r"(?P<gap>\s+)"
    r"(?P<holder>.*?)"
    r"(?P<tail>\s*-->)?\s*$"
)


def git(*args, check=True, text=True):
    return subprocess.run(["git", *args], check=check, capture_output=True, text=text).stdout


def is_checked(path):
    base = os.path.basename(path)
    if path.startswith(SKIP_PREFIXES):
        return False
    return base == "CMakeLists.txt" or base.endswith((".h", ".hpp", ".c", ".cpp", ".xml"))


def comment_style(path):
    base = os.path.basename(path)
    if base.endswith(".xml"):
        return "xml"
    if base == "CMakeLists.txt":
        return "hash"
    return "slash"


def load_exempt():
    try:
        with open(EXEMPT_FILE) as f:
            return {l.strip() for l in f if l.strip() and not l.startswith("#")}
    except OSError:
        return set()


def veritaware_ok(lines, year):
    pat = re.compile(r"[Cc]opyright\s+\([Cc]\)\s+\d{4}-" + str(year) + r"\s+Veritaware|"
                     r"[Cc]opyright\s+\([Cc]\)\s+" + str(year) + r"\s+Veritaware")
    return any(pat.search(l) for l in lines[:HEADER_LINES])


def fix_text(text, path, year, is_new):
    """Returns the text with a valid Veritaware header (unchanged if it has one)."""
    eol = "\r\n" if "\r\n" in text[:4096] else "\n"
    lines = text.split(eol)
    head = lines[:HEADER_LINES]
    if veritaware_ok(head, year):
        return text

    style = comment_style(path)
    entries = []  # (index, match) of the copyright lines in the header
    for i, line in enumerate(head):
        m = COPYRIGHT_RE.match(line)
        if m:
            entries.append((i, m))

    # 1. An outdated Veritaware line: extend its year range to the current year.
    for i, m in entries:
        if m.group("holder").strip().startswith("Veritaware"):
            start = m.group("years")[:4]
            new_years = str(year) if int(start) >= year else f"{start}-{year}"
            span = len(m.group("years")) + len(m.group("gap"))
            gap = " " * max(1, span - len(new_years))
            line = head[i]
            s, e = m.start("years"), m.end("gap")
            lines[i] = line[:s] + new_years + gap + line[e:]
            return eol.join(lines)

    # 2. Other authors' lines exist: add an aligned Besprited line after them.
    if entries:
        last_i, last = entries[-1]
        named = [m for _, m in entries if m.group("name")]
        if named:
            name_w = max(len(m.group("name")) for m in named + [last])
            name_w = max(name_w, len("Besprited"))
            years_w = max(len(m.group("years")) for _, m in entries)
            years_w = max(years_w, len(str(year)))
            new = (f"{last.group('lead')}{'Besprited'.ljust(name_w)} | {last.group('cpy')}"
                   f"{str(year).ljust(years_w)} Veritaware")
        else:
            new = f"{last.group('lead')}{last.group('cpy')}{year} Veritaware"
        if style == "xml":
            new += " -->"
        lines.insert(last_i + 1, new)
        return eol.join(lines)

    # 3. No copyright line at all: add one at the top.
    insert_at = 0
    if style == "xml" and lines and lines[0].startswith("<?xml"):
        insert_at = 1
    if style == "xml":
        block = [f"<!-- Copyright (C) {year} Veritaware -->"]
    elif style == "hash":
        block = [f"# Copyright (C) {year} Veritaware"]
    else:
        block = [f"// Copyright (C) {year} Veritaware"]
        if is_new:
            block += ["//"] + [f"// {l}" for l in LICENSE_BLURB] + [""]
    lines[insert_at:insert_at] = block
    return eol.join(lines)


def staged_files():
    out = git("diff", "--cached", "--name-only", "--diff-filter=ACMR", "-z")
    added = set(git("diff", "--cached", "--name-only", "--diff-filter=A", "-z").split("\0"))
    return [p for p in out.split("\0") if p], added


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawTextHelpFormatter)
    ap.add_argument("files", nargs="*")
    ap.add_argument("--fix", action="store_true", help="update the headers in place")
    ap.add_argument("--staged", action="store_true", help="check the staged files (fixes the index too)")
    ap.add_argument("--base", help="check the files changed since this ref")
    ap.add_argument("--year", type=int, default=datetime.date.today().year)
    args = ap.parse_args()

    os.chdir(git("rev-parse", "--show-toplevel").strip())
    added = set()
    if args.staged:
        files, added = staged_files()
    elif args.base:
        mb = git("merge-base", args.base, "HEAD").strip()
        files = [p for p in git("diff", "--name-only", "--diff-filter=ACMR", "-z", mb).split("\0") if p]
        added = set(git("diff", "--name-only", "--diff-filter=A", "-z", mb).split("\0"))
    else:
        files = args.files

    exempt = load_exempt()
    bad = 0
    for path in files:
        if not is_checked(path) or path in exempt or not os.path.isfile(path):
            continue
        is_new = path in added

        def read_worktree():
            with open(path, encoding="utf-8", errors="surrogateescape", newline="") as f:
                return f.read()

        if args.staged:
            text = git("show", f":{path}", text=False).decode("utf-8", "surrogateescape")
        else:
            text = read_worktree()

        fixed = fix_text(text, path, args.year, is_new)
        if fixed == text:
            continue

        if not args.fix:
            print(f"copyright: {path}: missing a Veritaware {args.year} copyright header", file=sys.stderr)
            bad += 1
            continue

        print(f"copyright: updated the Veritaware header of {path}", file=sys.stderr)
        if args.staged:
            blob = subprocess.run(["git", "hash-object", "-w", "--stdin"],
                                  input=fixed.encode("utf-8", "surrogateescape"),
                                  check=True, capture_output=True).stdout.decode().strip()
            mode = git("ls-files", "-s", "--", path).split()[0]
            git("update-index", "--cacheinfo", f"{mode},{blob},{path}")
            # Mirror the edit into the working tree (which may hold unstaged changes).
            wt = read_worktree()
            wt_fixed = fix_text(wt, path, args.year, is_new)
            if wt_fixed != wt:
                with open(path, "w", encoding="utf-8", errors="surrogateescape", newline="") as f:
                    f.write(wt_fixed)
        else:
            with open(path, "w", encoding="utf-8", errors="surrogateescape", newline="") as f:
                f.write(fixed)

    if bad:
        print(f"copyright: {bad} file(s) need a header; run with --fix", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
