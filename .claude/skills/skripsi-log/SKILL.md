---
name: skripsi-log
description: Use whenever creating, editing, reviewing, or discussing files under skripsi/ (Proposal, draft, referensi-skripsi, journal, eksperimen, log) in this project — loads the thesis context/rules and ensures skripsi/log/log-perubahan.md gets a dated entry after any change.
---

# Skripsi log & context skill

This project has a `skripsi/` folder where the user is drafting their thesis
("skripsi") on top of this DeepStream ADAS pipeline codebase.

## Before doing any work under `skripsi/`

1. Read `skripsi/PANDUAN-AI.md` first — it has thesis context (title, the
   Jetson AGX Orin → Orin Nano / DLA scope change that is still undecided),
   the folder structure, and rules for not fabricating experiment data.
2. Read the top entries of `skripsi/log/log-perubahan.md` to see what was
   done most recently, so you don't redo or overwrite prior work.

## After doing any work under `skripsi/`

Always append a new entry to the TOP of `skripsi/log/log-perubahan.md`
(entries are newest-first), formatted as:

```
## YYYY-MM-DD HH:MM (timezone)

- bullet list of what changed and why
```

Get the real timestamp by running `date "+%Y-%m-%d %H:%M:%S %Z"` — never
guess or fabricate the date/time. Do this for every change, even small ones
(a single file edit still gets a log entry).

## Content rules

- Never invent accuracy/FPS/latency numbers for the thesis. Pull real numbers
  from `../docs/`, `../data/`, or files the user has placed in
  `skripsi/eksperimen/`. If a number isn't available yet, write an explicit
  `TODO: ...` placeholder instead of guessing.
- Before writing new chapter ("BAB") content in `skripsi/draft/`, check
  whether `skripsi/Proposal/` and `skripsi/referensi-skripsi/` have content
  yet — use them to match the structure/style already approved at the
  proposal seminar ("seminar proposal") and the department's conventions.
- If the DLA vs. Jetson Orin Nano scope question (see `PANDUAN-AI.md`) hasn't
  been resolved yet and the current task touches BAB I (rumusan masalah/
  tujuan) or BAB III (metodologi), ask the user how they want to resolve it
  before finalizing that content — don't silently pick delete-vs-replace.
