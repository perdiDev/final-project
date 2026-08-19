#!/usr/bin/env python3
"""Strip authorial meta-commentary (status blockquotes, [VERIFIKASI] tags) from a
BAB-*.md draft, producing a clean narrative version suitable for the merged docx.

Rules:
1. Remove standalone blockquote blocks (paragraphs where every line starts with "> ").
   These are always meta/revision-history notes addressed to the author, not thesis body
   content.
2. Remove the "## Judul (revisi)" section entirely (BAB I only) - it is meta commentary
   about the title change, not part of the chapter body.
3. Strip inline "`[VERIFIKASI]` " tags that appear mid-paragraph/mid-table-cell, keeping
   the rest of the sentence intact (no information is deleted, only the internal review
   flag marker).
4. Collapse resulting multiple blank lines into one.
"""
import re
import sys

INLINE_TAG_RE = re.compile(r"`\[VERIFIKASI\]`\s*")


def strip_judul_revisi_section(lines):
    out = []
    skipping = False
    for line in lines:
        if line.strip() == "## Judul (revisi)":
            skipping = True
            continue
        if skipping and line.startswith("## "):
            skipping = False
        if skipping:
            continue
        out.append(line)
    return out


def strip_blockquote_blocks(lines):
    out = []
    block = []

    def flush():
        if block:
            # entire block was blockquote lines -> drop it
            block.clear()

    for line in lines:
        if line.startswith(">"):
            block.append(line)
        else:
            flush()
            out.append(line)
    flush()
    return out


def strip_inline_tags(lines):
    return [INLINE_TAG_RE.sub("", line) for line in lines]


def collapse_blank_lines(text):
    return re.sub(r"\n{3,}", "\n\n", text)


def clean(text):
    lines = text.splitlines()
    lines = strip_judul_revisi_section(lines)
    lines = strip_blockquote_blocks(lines)
    lines = strip_inline_tags(lines)
    result = "\n".join(lines) + "\n"
    return collapse_blank_lines(result)


def main():
    src, dst = sys.argv[1], sys.argv[2]
    with open(src, encoding="utf-8") as f:
        text = f.read()
    cleaned = clean(text)
    with open(dst, "w", encoding="utf-8") as f:
        f.write(cleaned)
    print(f"Wrote {dst}")


if __name__ == "__main__":
    main()
