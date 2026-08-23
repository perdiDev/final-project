"""Tiga grafik tambahan untuk BAB III, dibaca langsung dari runtime_summary.csv
(keluaran aggregate_runtime.py) tanpa perlu menjalankan ulang seluruh pipeline agregasi
dari data mentah data/benchmark/ (skrip mandiri ini dipakai karena folder data mentah
tersebut tidak selalu tersedia di setiap mesin pengembangan, sedangkan
runtime_summary.csv yang sudah teragregasi tetap tersedia sebagai sumber data yang sah).

Keluaran:
  skripsi/eksperimen/plots/latency_decomposition_stacked_bar.png
  skripsi/eksperimen/plots/tracker_latency_comparison.png
  skripsi/eksperimen/plots/energy_per_frame.png

Tidak mengarang angka: seluruh nilai diambil langsung dari runtime_summary.csv, kolom
dan baris yang sama dengan yang sudah dipakai pada Tabel 3.2.3 / 3.4.1 / 3.4.3 di
BAB III.
"""

from __future__ import annotations

import argparse
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
import pandas as pd

# Warna kategorikal per model_config, urutan alfabetis konsisten dengan yang dipakai
# CATEGORICAL_COLORS pada aggregate_runtime.py / tradeoff_analysis.py, agar satu model
# selalu memakai warna yang sama di seluruh grafik BAB III.
MODEL_COLORS = {
    "yolov10n_kitti": "#4C72B0",
    "yolov26n_kitti": "#DD8452",
    "yolov8n_kitti": "#55A868",
    "yolov8n_kitti_efficientnms": "#C44E52",
    "yolov9t_kitti": "#8172B2",
    "yolov9t_kitti_efficientnms": "#937860",
}
DISPLAY_NAMES = {
    "yolov8n_kitti": "YOLOv8n",
    "yolov9t_kitti": "YOLOv9t",
    "yolov10n_kitti": "YOLOv10n",
    "yolov26n_kitti": "YOLO26n",
    "yolov8n_kitti_efficientnms": "YOLOv8n+EfficientNMS",
    "yolov9t_kitti_efficientnms": "YOLOv9t+EfficientNMS",
}
# Warna tracker tetap (dipakai lintas grafik 2 & 3): biru = NvDCF, merah = NvSORT.
TRACKER_COLORS = {"nvdcf": "#4C72B0", "nvsort": "#C44E52"}

LATENCY_COMPONENTS = [
    ("avg_Lat_PreMux_ms", "Pra-multiplexing"),
    ("avg_Lat_Mux_ms", "Multiplexing"),
    ("avg_Lat_Infer_ms", "Inferensi"),
    ("avg_Lat_Tracker_ms", "Tracking"),
    ("avg_Lat_PreOSD_ms", "Pra-OSD"),
    ("avg_Lat_OSD_ms", "OSD"),
    ("avg_Lat_Output_ms", "Output"),
]
COMPONENT_COLORS = ["#4C72B0", "#DD8452", "#55A868", "#C44E52", "#8172B2", "#937860", "#CCB974"]

BASELINE_MODELS_ORDER = ["yolov8n_kitti", "yolov9t_kitti", "yolov10n_kitti", "yolov26n_kitti"]
ALL_MODELS_ORDER = [
    "yolov8n_kitti",
    "yolov9t_kitti",
    "yolov10n_kitti",
    "yolov26n_kitti",
    "yolov8n_kitti_efficientnms",
    "yolov9t_kitti_efficientnms",
]


def plot_latency_decomposition(df: pd.DataFrame, out_path: Path) -> None:
    """Stacked bar dekomposisi latensi 7-komponen, 4 model *baseline* (tracker NvDCF) — Tabel 3.2.3."""
    sub = df[(df["model"].isin(BASELINE_MODELS_ORDER)) & (df["tracker"] == "nvdcf")]
    sub = sub.set_index("model").loc[BASELINE_MODELS_ORDER]

    fig, ax = plt.subplots(figsize=(8, 5.5))
    labels = [DISPLAY_NAMES[m] for m in BASELINE_MODELS_ORDER]
    bottom = [0.0] * len(BASELINE_MODELS_ORDER)
    for (col, comp_label), color in zip(LATENCY_COMPONENTS, COMPONENT_COLORS):
        values = sub[col].tolist()
        ax.bar(labels, values, bottom=bottom, label=comp_label, color=color, edgecolor="white", linewidth=0.5)
        bottom = [b + v for b, v in zip(bottom, values)]

    ax.set_ylabel("Latensi rata-rata (ms)")
    ax.set_title("Dekomposisi Latensi Per-Komponen — Model Baseline, Tracker NvDCF")
    ax.grid(axis="y", linestyle=":", alpha=0.4)
    ax.legend(loc="upper left", bbox_to_anchor=(1.0, 1.0), fontsize=8, title="Tahap pipeline")
    fig.tight_layout()
    out_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(out_path, dpi=150)
    plt.close(fig)


def plot_tracker_latency_comparison(df: pd.DataFrame, out_path: Path) -> None:
    """Grouped bar latensi tahap tracking NvDCF vs. NvSORT, skala-y log — Tabel 3.4.1."""
    import numpy as np

    labels = [DISPLAY_NAMES[m] for m in ALL_MODELS_ORDER]
    nvdcf_vals = [
        df[(df["model"] == m) & (df["tracker"] == "nvdcf")]["avg_Lat_Tracker_ms"].iloc[0]
        for m in ALL_MODELS_ORDER
    ]
    nvsort_vals = [
        df[(df["model"] == m) & (df["tracker"] == "nvsort")]["avg_Lat_Tracker_ms"].iloc[0]
        for m in ALL_MODELS_ORDER
    ]

    x = np.arange(len(labels))
    width = 0.38
    fig, ax = plt.subplots(figsize=(9, 5.5))
    ax.bar(x - width / 2, nvdcf_vals, width, label="NvDCF", color=TRACKER_COLORS["nvdcf"])
    ax.bar(x + width / 2, nvsort_vals, width, label="NvSORT", color=TRACKER_COLORS["nvsort"])
    ax.set_yscale("log")
    ax.set_ylabel("Latensi tahap tracking rata-rata (ms, skala logaritmik)")
    ax.set_title("Perbandingan Latensi Tahap Tracking: NvDCF vs. NvSORT")
    ax.set_xticks(x)
    ax.set_xticklabels(labels, rotation=20, ha="right")
    ax.grid(axis="y", linestyle=":", alpha=0.4, which="both")
    ax.legend()
    fig.tight_layout()
    out_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(out_path, dpi=150)
    plt.close(fig)


def plot_energy_per_frame(df: pd.DataFrame, out_path: Path) -> None:
    """Grouped bar estimasi energi per-*frame* (daya sistem total / FPS) — Tabel 3.4.3."""
    import numpy as np

    labels = [DISPLAY_NAMES[m] for m in ALL_MODELS_ORDER]
    energy = {}
    for tracker in ("nvdcf", "nvsort"):
        vals = []
        for m in ALL_MODELS_ORDER:
            row = df[(df["model"] == m) & (df["tracker"] == tracker)].iloc[0]
            vals.append(row["avg_VDD_IN_mW"] / row["avg_fps"])
        energy[tracker] = vals

    x = np.arange(len(labels))
    width = 0.38
    fig, ax = plt.subplots(figsize=(9, 5.5))
    ax.bar(x - width / 2, energy["nvdcf"], width, label="NvDCF", color=TRACKER_COLORS["nvdcf"])
    ax.bar(x + width / 2, energy["nvsort"], width, label="NvSORT", color=TRACKER_COLORS["nvsort"])
    ax.set_ylabel("Estimasi energi per frame (mJ)")
    ax.set_title("Estimasi Energi Per-Frame: NvDCF vs. NvSORT")
    ax.set_xticks(x)
    ax.set_xticklabels(labels, rotation=20, ha="right")
    ax.grid(axis="y", linestyle=":", alpha=0.4)
    ax.legend()
    fig.tight_layout()
    out_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(out_path, dpi=150)
    plt.close(fig)


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--runtime-summary",
        type=Path,
        default=Path("skripsi/eksperimen/runtime_summary.csv"),
    )
    parser.add_argument("--out-dir", type=Path, default=Path("skripsi/eksperimen"))
    return parser


def main(argv=None) -> int:
    args = build_parser().parse_args(argv)
    if not args.runtime_summary.is_file():
        print(f"[ERROR] '{args.runtime_summary}' tidak ditemukan. Jalankan aggregate_runtime.py dulu.")
        return 1

    df = pd.read_csv(args.runtime_summary)
    plots_dir = args.out_dir / "plots"

    plot_latency_decomposition(df, plots_dir / "latency_decomposition_stacked_bar.png")
    plot_tracker_latency_comparison(df, plots_dir / "tracker_latency_comparison.png")
    plot_energy_per_frame(df, plots_dir / "energy_per_frame.png")
    print(f"[OK] 3 grafik tambahan ditulis ke {plots_dir}/")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
