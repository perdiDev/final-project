"""Helper bersama untuk aggregate_runtime.py dan tradeoff_analysis.py.

Membaca struktur hasil `scripts/run_benchmark.sh` / `scripts/run_all_benchmark.sh`:

    data/benchmark/<model>_<tracker>/<run_id>/
        fps.csv                 -> header persis src/main.cpp:884-885
        hardware_analysis.csv   -> header dinamis, dibuat oleh src/log_parser.cpp:71-86
        run_info.txt            -> metadata "key : value" per baris (run_benchmark.sh:298-317)

Nama folder gabungan `<model>_<tracker>` ambigu untuk di-split langsung (tracker
`nvdcf_perf` sendiri mengandung underscore) — model_config diturunkan dengan
menghapus suffix `_<tracker>` dari field `model` di run_info.txt, memakai field
`tracker` yang sudah pasti diketahui dari run_info.txt itu sendiri.
"""

from __future__ import annotations

import re
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List, Optional

import pandas as pd

FPS_CSV_NAME = "fps.csv"
HARDWARE_CSV_NAME = "hardware_analysis.csv"
RUN_INFO_NAME = "run_info.txt"

_POWER_RAIL_RE = re.compile(r"^(.+)_mW$")
_CPU_CORE_RE = re.compile(r"^CPU_Core_(\d+)_(Persen|Freq_MHz)$")


@dataclass(frozen=True)
class RunInfo:
    run_dir: Path
    combined_name: str
    model_config: str
    tracker: str


def parse_run_info(path: Path) -> Dict[str, str]:
    """Parse file `run_info.txt` (format `key : value` per baris)."""
    fields: Dict[str, str] = {}
    for raw_line in path.read_text().splitlines():
        if ":" not in raw_line:
            continue
        key, _, value = raw_line.partition(":")
        fields[key.strip()] = value.strip()
    return fields


def discover_runs(bench_root: Path) -> List[RunInfo]:
    """Cari semua run di `bench_root/<combined_name>/<run_id>/run_info.txt`.

    Run tanpa `run_info.txt` yang bisa diparse (field `model`/`tracker` hilang)
    dilewati dengan pesan peringatan ke stderr, bukan menghentikan seluruh proses.
    """
    runs: List[RunInfo] = []
    if not bench_root.is_dir():
        return runs

    for run_info_path in sorted(bench_root.glob("*/*/" + RUN_INFO_NAME)):
        run_dir = run_info_path.parent
        try:
            fields = parse_run_info(run_info_path)
            combined_name = fields["model"]
            tracker_raw = fields["tracker"]
            # Tracker di run_info.txt bisa berupa path (mis. config/tracker_nvdcf.yml).
            # Kita ambil basename-nya lalu bersihkan prefix/suffix agar konsisten
            # dengan field model (model_config + "_" + tracker).
            tracker = Path(tracker_raw).stem
            if tracker.startswith("tracker_"):
                tracker = tracker[len("tracker_") :]
        except (OSError, KeyError) as exc:
            print(f"[WARN] Lewati {run_dir}: gagal baca run_info.txt ({exc})")
            continue

        if not combined_name.endswith(f"_{tracker}"):
            print(
                f"[WARN] Lewati {run_dir}: field model='{combined_name}' tidak "
                f"berakhiran '_{tracker}', tidak bisa menurunkan model_config dengan aman."
            )
            continue

        model_config = combined_name[: -(len(tracker) + 1)]
        runs.append(
            RunInfo(
                run_dir=run_dir,
                combined_name=combined_name,
                model_config=model_config,
                tracker=tracker,
            )
        )
    return runs


def load_fps_csv(run_dir: Path) -> Optional[pd.DataFrame]:
    path = run_dir / FPS_CSV_NAME
    if not path.is_file():
        return None
    try:
        df = pd.read_csv(path)
    except (OSError, pd.errors.EmptyDataError, pd.errors.ParserError) as exc:
        print(f"[WARN] Gagal baca {path}: {exc}")
        return None
    return df if not df.empty else None


def load_hardware_csv(run_dir: Path) -> Optional[pd.DataFrame]:
    path = run_dir / HARDWARE_CSV_NAME
    if not path.is_file():
        return None
    try:
        df = pd.read_csv(path)
    except (OSError, pd.errors.EmptyDataError, pd.errors.ParserError) as exc:
        print(f"[WARN] Gagal baca {path}: {exc}")
        return None
    return df if not df.empty else None


def trim_warmup(df: pd.DataFrame, warmup_s: float, elapsed_col: str = "Elapsed_ms") -> pd.DataFrame:
    """Buang baris dengan `elapsed_col` di bawah `warmup_s` detik pertama."""
    if elapsed_col not in df.columns:
        return df
    return df[df[elapsed_col] >= warmup_s * 1000].copy()


def power_rail_columns(df: pd.DataFrame) -> List[str]:
    """Kolom rail daya `<NAMA>_mW` yang benar-benar ada di DataFrame ini."""
    return [c for c in df.columns if _POWER_RAIL_RE.match(c)]


def cpu_core_columns(df: pd.DataFrame) -> List[str]:
    """Kolom per-core CPU (`CPU_Core_<n>_Persen` / `_Freq_MHz`) yang ada di DataFrame ini."""
    return [c for c in df.columns if _CPU_CORE_RE.match(c)]


def run_throughput_fps(df: pd.DataFrame, elapsed_col: str = "Elapsed_ms") -> float:
    """Throughput satu run: jumlah frame dibagi rentang wall-clock (bukan mean kolom FPS).

    Kolom `FPS` pada fps.csv adalah nilai window satu detik yang diulang tiap baris —
    merata-ratakannya langsung memberi bobot lebih ke window ber-FPS tinggi. Lihat
    docs/06_runtime_results.md §6.3.
    """
    if len(df) < 2:
        return float("nan")
    elapsed_span_s = (df[elapsed_col].max() - df[elapsed_col].min()) / 1000
    if elapsed_span_s <= 0:
        return float("nan")
    return (len(df) - 1) / elapsed_span_s
