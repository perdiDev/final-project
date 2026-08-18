"""Agregasi fps.csv + hardware_analysis.csv dari seluruh run benchmark menjadi
tabel ringkasan dan grafik untuk BAB IV / docs/06_runtime_results.md.

Alur:
  1. scripts/run_all_benchmark.sh  -> data/benchmark/<model>_<tracker>/<run_id>/{fps.csv,
                                       hardware_analysis.csv, run_info.txt}
  2. script ini                   -> skripsi/eksperimen/runtime_summary.csv
                                      skripsi/eksperimen/plots/fps_boxplot_by_model.png
                                      skripsi/eksperimen/plots/fps_boxplot_by_tracker.png

Tidak mengarang angka: kalau data/benchmark/ kosong atau belum berisi run yang valid,
script berhenti dengan pesan jelas dan TIDAK menulis file output kosong/nol-baris.

Lihat docs/06_runtime_results.md untuk penjelasan metodologi tiap statistik.
"""

from __future__ import annotations

import argparse
from pathlib import Path
from typing import List, Optional, Sequence, Tuple

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
import pandas as pd

from common import (
    RunInfo,
    cpu_core_columns,
    discover_runs,
    load_fps_csv,
    load_hardware_csv,
    power_rail_columns,
    run_throughput_fps,
    trim_warmup,
)

# Warna kategorikal tetap (bukan colormap otomatis/rainbow) — dipertahankan konsisten
# antar grafik pada dokumen yang sama. Lihat skill dataviz: "assign categorical hues
# in fixed order, never cycled".
CATEGORICAL_COLORS = ["#4C72B0", "#DD8452", "#55A868", "#C44E52", "#8172B2", "#937860"]


def _run_metrics(run: RunInfo, warmup_s: float) -> Optional[dict]:
    fps_df = load_fps_csv(run.run_dir)
    if fps_df is None:
        print(f"[WARN] {run.run_dir}: fps.csv hilang/kosong, skip run ini.")
        return None
    fps_df = trim_warmup(fps_df, warmup_s)
    if len(fps_df) < 2:
        print(f"[WARN] {run.run_dir}: kurang dari 2 baris setelah buang warm-up, skip.")
        return None

    metrics = {
        "run_dir": str(run.run_dir),
        "throughput_fps": run_throughput_fps(fps_df),
        "latency_mean_ms": fps_df["Latency_ms"].mean(),
        "latency_median_ms": fps_df["Latency_ms"].median(),
        "latency_p95_ms": fps_df["Latency_ms"].quantile(0.95),
        "n_frames": len(fps_df),
    }
    for component in (
        "Lat_PreMux_ms",
        "Lat_Mux_ms",
        "Lat_Infer_ms",
        "Lat_Tracker_ms",
        "Lat_PreOSD_ms",
        "Lat_OSD_ms",
        "Lat_Output_ms",
    ):
        if component in fps_df.columns:
            metrics[f"{component}_mean"] = fps_df[component].mean()

    hw_df = load_hardware_csv(run.run_dir)
    if hw_df is not None:
        hw_df = trim_warmup(hw_df, warmup_s, elapsed_col="Hardware_Elapsed_ms")
        if "GPU_Persen" in hw_df.columns:
            metrics["gpu_pct_mean"] = hw_df["GPU_Persen"].mean()
        if "RAM_MB" in hw_df.columns:
            metrics["ram_mb_mean"] = hw_df["RAM_MB"].mean()
        for rail in power_rail_columns(hw_df):
            metrics[f"{rail}_mean"] = hw_df[rail].mean()
        for core_col in cpu_core_columns(hw_df):
            metrics[f"{core_col}_mean"] = hw_df[core_col].mean()
    else:
        print(f"[WARN] {run.run_dir}: hardware_analysis.csv hilang/kosong, statistik hardware run ini dilewati.")

    return metrics


def build_summary(runs: List[RunInfo], warmup_s: float) -> Tuple[pd.DataFrame, pd.DataFrame]:
    rows = []
    for run in runs:
        metrics = _run_metrics(run, warmup_s)
        if metrics is None:
            continue
        metrics["model"] = run.model_config
        metrics["tracker"] = run.tracker
        rows.append(metrics)

    if not rows:
        return pd.DataFrame(), pd.DataFrame()

    per_run = pd.DataFrame(rows)

    # Kolom yang sudah bernama "..._mean" adalah rata-rata *dalam satu run*
    # (dihitung di _run_metrics); di sini kita ambil rata-rata *antar run* dari
    # nilai-nilai itu, lalu ganti nama jadi "avg_..." supaya tidak ada "mean_mean".
    per_run_mean_cols = [c for c in per_run.columns if c.endswith("_mean")]

    named_agg = {
        "n_runs": pd.NamedAgg(column="run_dir", aggfunc="count"),
        "avg_fps": pd.NamedAgg(column="throughput_fps", aggfunc="mean"),
        "median_fps": pd.NamedAgg(column="throughput_fps", aggfunc="median"),
        "std_fps": pd.NamedAgg(column="throughput_fps", aggfunc="std"),
        "avg_latency_ms": pd.NamedAgg(column="latency_mean_ms", aggfunc="mean"),
        "avg_p95_latency_ms": pd.NamedAgg(column="latency_p95_ms", aggfunc="mean"),
    }
    for col in per_run_mean_cols:
        out_name = "avg_" + col[: -len("_mean")]
        named_agg[out_name] = pd.NamedAgg(column=col, aggfunc="mean")

    grouped = per_run.groupby(["model", "tracker"]).agg(**named_agg)
    grouped = grouped.reset_index().sort_values(["model", "tracker"])
    return grouped, per_run


def plot_fps_boxplot(per_run: pd.DataFrame, group_col: str, out_path: Path, title: str) -> None:
    groups = sorted(per_run[group_col].dropna().unique())
    if not groups:
        return
    data = [per_run.loc[per_run[group_col] == g, "throughput_fps"].dropna() for g in groups]

    fig, ax = plt.subplots(figsize=(max(6, len(groups) * 1.2), 5))
    box = ax.boxplot(data, labels=groups, patch_artist=True)
    for i, patch in enumerate(box["boxes"]):
        patch.set_facecolor(CATEGORICAL_COLORS[i % len(CATEGORICAL_COLORS)])
        patch.set_alpha(0.7)
    ax.set_ylabel("FPS (per run)")
    ax.set_title(title)
    ax.grid(axis="y", linestyle=":", alpha=0.4)
    plt.setp(ax.get_xticklabels(), rotation=30, ha="right")
    fig.tight_layout()
    out_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(out_path, dpi=150)
    plt.close(fig)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--bench-root", type=Path, default=Path("data/benchmark"), help="Folder hasil scripts/run_all_benchmark.sh")
    parser.add_argument("--warmup-s", type=float, default=10.0, help="Detik pertama yang dibuang sebelum agregasi (default 10)")
    parser.add_argument(
        "--out-dir",
        type=Path,
        default=Path("skripsi/eksperimen"),
        help="Folder output (runtime_summary.csv + plots/) — default skripsi/eksperimen",
    )
    return parser


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = build_parser().parse_args(argv)

    runs = discover_runs(args.bench_root)
    if not runs:
        print(
            f"[INFO] Belum ada data benchmark valid di '{args.bench_root}'. "
            "Jalankan scripts/run_all_benchmark.sh dulu (lihat docs/04_benchmark_protocol.md), "
            "lalu jalankan ulang script ini. Tidak ada file output yang dibuat."
        )
        return 1

    summary, per_run = build_summary(runs, args.warmup_s)
    if summary.empty:
        print(
            "[INFO] Ditemukan folder run, tapi tidak ada run dengan fps.csv valid setelah "
            f"buang warm-up {args.warmup_s}s. Tidak ada file output yang dibuat."
        )
        return 1

    args.out_dir.mkdir(parents=True, exist_ok=True)
    summary_path = args.out_dir / "runtime_summary.csv"
    summary.to_csv(summary_path, index=False)
    print(f"[OK] {len(summary)} skenario (model x tracker) ditulis ke {summary_path}")

    # Data mentah per-run (bukan cuma ringkasan) — dipakai tradeoff_analysis.py
    # untuk uji signifikansi (scipy.stats.ttest_ind butuh distribusi, bukan rata-rata).
    per_run_path = args.out_dir / "runtime_per_run.csv"
    per_run.drop(columns=["n_frames"], errors="ignore").to_csv(per_run_path, index=False)
    print(f"[OK] {len(per_run)} baris data per-run ditulis ke {per_run_path}")

    plots_dir = args.out_dir / "plots"
    plot_fps_boxplot(per_run, "model", plots_dir / "fps_boxplot_by_model.png", "Distribusi FPS per Model (Jetson Orin Nano)")
    plot_fps_boxplot(per_run, "tracker", plots_dir / "fps_boxplot_by_tracker.png", "Distribusi FPS per Tracker (Jetson Orin Nano)")
    print(f"[OK] Grafik boxplot ditulis ke {plots_dir}/")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
