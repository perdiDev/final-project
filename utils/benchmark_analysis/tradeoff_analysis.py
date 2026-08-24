"""Gabungkan runtime_summary.csv (aggregate_runtime.py) dengan akurasi KITTI
(accuracy_reference.csv, ditranskrip dari docs/05_accuracy_results.md) menjadi tabel
dan grafik trade-off akurasi-vs-kecepatan-vs-daya untuk BAB IV / docs/07_tradeoff_analysis.md.

Alur:
  1. aggregate_runtime.py  -> skripsi/eksperimen/runtime_summary.csv + runtime_per_run.csv
  2. script ini            -> skripsi/eksperimen/tradeoff_summary.csv
                               skripsi/eksperimen/plots/tradeoff_map_vs_fps.png
                               skripsi/eksperimen/plots/tradeoff_map_vs_power.png

Tidak mengarang angka: kalau runtime_summary.csv belum ada, script berhenti dan minta
menjalankan aggregate_runtime.py dulu.
"""

from __future__ import annotations

import argparse
from pathlib import Path
from typing import Optional, Sequence

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
import pandas as pd

# Nama tampilan, urutan model, dan warna tracker diimpor dari extra_plots.py supaya
# konsisten dengan grafik lain di BAB III (mis. TRACKER_COLORS biru=NvDCF/merah=NvSORT
# dipakai juga pada tracker_latency_comparison.png dan energy_per_frame.png).
from extra_plots import ALL_MODELS_ORDER, DISPLAY_NAMES, TRACKER_COLORS

# Kolom rail daya total (dipilih sebagai representasi "Avg Power") — sesuaikan dengan
# nama rail yang benar-benar muncul di runtime_summary.csv Anda (lihat catatan
# docs/06_runtime_results.md §6.2: nama rail bergantung platform Jetson, mis. VDD_IN,
# VDD_CPU_GPU_CV, dst). Diselesaikan otomatis lewat --power-column atau deteksi kolom
# pertama yang mengandung "IN" / "TOTAL" bila tidak diberikan.
def _guess_power_column(columns: Sequence[str]) -> Optional[str]:
    candidates = [c for c in columns if c.startswith("avg_") and c.endswith("_mW")]
    if not candidates:
        return None
    for preferred in ("avg_VDD_IN_mW", "avg_VIN_SYS_5V0_mW"):
        if preferred in candidates:
            return preferred
    for c in candidates:
        if "IN" in c.upper() or "TOTAL" in c.upper() or "SYS" in c.upper():
            return c
    return sorted(candidates)[0]


def load_accuracy_reference(path: Path) -> pd.DataFrame:
    return pd.read_csv(path, comment="#")


def build_tradeoff_table(runtime_summary: pd.DataFrame, accuracy: pd.DataFrame) -> pd.DataFrame:
    merged = runtime_summary.merge(accuracy, left_on="model", right_on="model_config", how="left")
    missing = merged[merged["display_name"].isna()]["model"].unique()
    if len(missing) > 0:
        print(
            f"[WARN] Model tanpa data akurasi di accuracy_reference.csv (dilewati dari plot): {sorted(missing)}"
        )
    return merged


def plot_tradeoff_dumbbell(
    df: pd.DataFrame,
    x_col: str,
    y_col: str,
    out_path: Path,
    title: str,
    xlabel: str,
    delta_fmt: str = "{:+.1f}",
) -> None:
    """Dumbbell plot: satu baris per model, dua titik (NvDCF/NvSORT) dihubungkan garis.

    `y_col` (mAP50-95) tidak berubah antar-*tracker* untuk model yang sama — bobot
    deteksi identik, EfficientNMS/*tracker* hanya mengubah eksekusi *runtime*, bukan
    akurasi (lihat catatan Tabel 3.5.1). Karena itu memplot ke-12 baris pada satu
    sumbu-y kontinu (pendekatan lama) hanya menumpuk titik yang sebenarnya berbagi
    y persis sama, dipisahkan artifisial lewat garis pemandu label. Bentuk yang
    cocok untuk struktur data ini adalah **pasangan sebelum/sesudah per model**: satu
    baris kategorikal per model (diurutkan naik menurut akurasi), dua titik
    (NvDCF/NvSORT, warna tetap konsisten dengan grafik lain di bab ini) dihubungkan
    garis yang panjangnya *langsung* menunjukkan selisih `x_col` akibat pergantian
    *tracker* — persis pola yang dibahas di teks (§3.6.1: titik NvSORT konsisten
    bergeser ke satu arah dibanding pasangan NvDCF-nya pada akurasi yang identik).
    """
    plot_df = df.dropna(subset=[x_col, y_col, "display_name"])
    if plot_df.empty:
        print(f"[WARN] Tidak ada baris valid untuk plot '{title}', dilewati.")
        return

    rows = []
    for model in ALL_MODELS_ORDER:
        sub = plot_df[plot_df["model"] == model]
        if sub.empty:
            continue
        by_tracker = sub.set_index("tracker")
        acc = sub[y_col].iloc[0]
        rows.append((model, acc, by_tracker))
    rows.sort(key=lambda r: r[1])  # akurasi naik -> model akurasi tertinggi di baris paling atas

    fig, ax = plt.subplots(figsize=(9, 5.5))
    ytick_labels = []
    for y, (model, acc, by_tracker) in enumerate(rows):
        xs = {t: by_tracker.loc[t, x_col] for t in ("nvdcf", "nvsort") if t in by_tracker.index}
        if len(xs) == 2:
            ax.plot(
                [xs["nvdcf"], xs["nvsort"]],
                [y, y],
                color="0.75",
                linewidth=2.2,
                solid_capstyle="round",
                zorder=1,
            )
            delta = xs["nvsort"] - xs["nvdcf"]
            mid_x = (xs["nvdcf"] + xs["nvsort"]) / 2
            ax.annotate(
                delta_fmt.format(delta),
                (mid_x, y),
                fontsize=7.5,
                color="0.35",
                ha="center",
                va="bottom",
                xytext=(0, 6),
                textcoords="offset points",
                zorder=2,
            )
        for tracker, x in xs.items():
            ax.scatter(
                [x],
                [y],
                color=TRACKER_COLORS[tracker],
                s=85,
                edgecolors="white",
                linewidths=0.8,
                zorder=3,
                label=tracker,
            )
        ytick_labels.append(f"{DISPLAY_NAMES[model]}\nmAP50-95 {acc:.3f}")

    ax.set_yticks(range(len(rows)))
    ax.set_yticklabels(ytick_labels, fontsize=8.5)
    ax.set_xlabel(xlabel)
    ax.set_title(title)
    ax.grid(axis="x", linestyle=":", alpha=0.4)
    ax.margins(y=0.1, x=0.12)

    handles, labels = ax.get_legend_handles_labels()
    by_label = dict(zip(labels, handles))
    order = [t for t in ("nvdcf", "nvsort") if t in by_label]
    ax.legend(
        [by_label[t] for t in order],
        ["NvDCF" if t == "nvdcf" else "NvSORT" for t in order],
        title="Tracker",
        loc="best",
        fontsize=8.5,
    )
    fig.tight_layout()
    out_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(out_path, dpi=150)
    plt.close(fig)


def compare_models(per_run: pd.DataFrame, spec_a: str, spec_b: str) -> None:
    """Uji signifikansi (Welch's t-test) antara distribusi FPS dua skenario.

    `spec_a`/`spec_b` berformat "model" atau "model:tracker". Kalau tracker tidak
    diberikan, seluruh tracker untuk model itu digabung.
    """
    from scipy import stats  # import lokal: scipy opsional, hanya dibutuhkan di sini

    def _select(spec: str) -> pd.Series:
        if ":" in spec:
            model, tracker = spec.split(":", 1)
            mask = (per_run["model"] == model) & (per_run["tracker"] == tracker)
        else:
            mask = per_run["model"] == spec
        return per_run.loc[mask, "throughput_fps"].dropna()

    fps_a, fps_b = _select(spec_a), _select(spec_b)
    if len(fps_a) < 2 or len(fps_b) < 2:
        print(
            f"[WARN] Butuh minimal 2 run per skenario untuk uji signifikansi "
            f"(dapat {len(fps_a)} untuk '{spec_a}', {len(fps_b)} untuk '{spec_b}'). Lewati."
        )
        return

    t_stat, p_value = stats.ttest_ind(fps_a, fps_b, equal_var=False)
    verdict = "SIGNIFIKAN (p < 0.05)" if p_value < 0.05 else "tidak signifikan (p >= 0.05)"
    print(
        f"[SIGNIFIKANSI] '{spec_a}' (n={len(fps_a)}, mean={fps_a.mean():.2f} FPS) vs "
        f"'{spec_b}' (n={len(fps_b)}, mean={fps_b.mean():.2f} FPS): "
        f"t={t_stat:.3f}, p={p_value:.4f} -> {verdict}"
    )


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--runtime-summary",
        type=Path,
        default=Path("skripsi/eksperimen/runtime_summary.csv"),
        help="Output aggregate_runtime.py",
    )
    parser.add_argument(
        "--runtime-per-run",
        type=Path,
        default=Path("skripsi/eksperimen/runtime_per_run.csv"),
        help="Output aggregate_runtime.py (dibutuhkan hanya untuk --significance)",
    )
    parser.add_argument(
        "--accuracy-reference",
        type=Path,
        default=Path("utils/benchmark_analysis/accuracy_reference.csv"),
        help="Transkrip akurasi dari docs/05_accuracy_results.md",
    )
    parser.add_argument("--out-dir", type=Path, default=Path("skripsi/eksperimen"))
    parser.add_argument(
        "--power-column",
        type=str,
        default=None,
        help="Nama kolom avg_<rail>_mW di runtime_summary.csv untuk plot vs power (auto-detect bila kosong)",
    )
    parser.add_argument(
        "--significance",
        nargs=2,
        metavar=("MODEL_A", "MODEL_B"),
        default=None,
        help="Uji Welch's t-test FPS antara dua skenario 'model' atau 'model:tracker' (opsional)",
    )
    return parser


def main(argv: Optional[Sequence[str]] = None) -> int:
    args = build_parser().parse_args(argv)

    if not args.runtime_summary.is_file():
        print(
            f"[INFO] '{args.runtime_summary}' belum ada. Jalankan "
            "utils/benchmark_analysis/aggregate_runtime.py dulu. Tidak ada file output yang dibuat."
        )
        return 1
    if not args.accuracy_reference.is_file():
        print(f"[ERROR] File referensi akurasi tidak ditemukan: {args.accuracy_reference}")
        return 1

    runtime_summary = pd.read_csv(args.runtime_summary)
    accuracy = load_accuracy_reference(args.accuracy_reference)
    tradeoff = build_tradeoff_table(runtime_summary, accuracy)

    args.out_dir.mkdir(parents=True, exist_ok=True)
    tradeoff_path = args.out_dir / "tradeoff_summary.csv"
    tradeoff.to_csv(tradeoff_path, index=False)
    print(f"[OK] {len(tradeoff)} baris trade-off ditulis ke {tradeoff_path}")

    plots_dir = args.out_dir / "plots"
    plot_tradeoff_dumbbell(
        tradeoff,
        x_col="avg_fps",
        y_col="map50_95",
        out_path=plots_dir / "tradeoff_map_vs_fps.png",
        title="Trade-off Akurasi vs. Kecepatan (Jetson Orin Nano)",
        xlabel="Avg FPS",
    )

    power_col = args.power_column or _guess_power_column(tradeoff.columns)
    if power_col is None:
        print("[WARN] Tidak ada kolom 'avg_*_mW' di runtime_summary.csv, plot vs power dilewati.")
    else:
        plot_tradeoff_dumbbell(
            tradeoff,
            x_col=power_col,
            y_col="map50_95",
            out_path=plots_dir / "tradeoff_map_vs_power.png",
            title="Trade-off Akurasi vs. Daya (Jetson Orin Nano)",
            xlabel=f"Daya sistem total, {power_col.removeprefix('avg_').removesuffix('_mW')} (mW)",
            delta_fmt="{:+.0f}",
        )
    print(f"[OK] Grafik trade-off ditulis ke {plots_dir}/")

    if args.significance is not None:
        if not args.runtime_per_run.is_file():
            print(
                f"[WARN] '{args.runtime_per_run}' tidak ada, tidak bisa jalankan --significance "
                "(jalankan ulang aggregate_runtime.py untuk membuatnya)."
            )
        else:
            per_run = pd.read_csv(args.runtime_per_run)
            compare_models(per_run, args.significance[0], args.significance[1])

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
