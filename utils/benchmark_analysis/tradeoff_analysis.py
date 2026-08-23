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
from typing import Dict, List, Optional, Sequence

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt
import pandas as pd

CATEGORICAL_COLORS = ["#4C72B0", "#DD8452", "#55A868", "#C44E52", "#8172B2", "#937860"]

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


def _place_labels_without_overlap(
    points: Sequence[tuple],
    x_span: float,
    y_span: float,
    cluster_threshold: float = 0.1,
    row_gap_frac: float = 0.065,
) -> List[float]:
    """Hitung offset-y label supaya tidak tumpang-tindih pada titik yang berdekatan.

    `points` berisi tuple (x, y, text) terurut sembarang. Karena beberapa skenario
    (mis. varian *baseline* vs. EfficientNMS berbobot sama) berbagi akurasi/FPS/daya
    yang nyaris identik, titik-titik ini divisualisasikan sebagai satu marker yang
    "menumpuk" rapat. Titik dikelompokkan memakai jarak Euclidean **ternormalisasi**
    (x dan y masing-masing dibagi rentangnya sendiri, agar kedua sumbu sebanding
    meski satuan/skalanya berbeda jauh, mis. FPS vs. mAP) via *union-find* sederhana,
    lalu label dalam satu klaster disebar vertikal berjarak tetap
    `row_gap_frac * y_span`, dipusatkan pada rata-rata y klaster tersebut. Klaster
    berisi satu titik tidak digeser sama sekali. Mengembalikan daftar offset-y
    (data-unit) sejajar urutan input `points`.
    """
    n = len(points)
    x_span = max(x_span, 1e-9)
    y_span = max(y_span, 1e-9)
    parent = list(range(n))

    def _find(i: int) -> int:
        while parent[i] != i:
            parent[i] = parent[parent[i]]
            i = parent[i]
        return i

    def _union(i: int, j: int) -> None:
        ri, rj = _find(i), _find(j)
        if ri != rj:
            parent[ri] = rj

    for i in range(n):
        for j in range(i + 1, n):
            dx = (points[i][0] - points[j][0]) / x_span
            dy = (points[i][1] - points[j][1]) / y_span
            if (dx * dx + dy * dy) ** 0.5 < cluster_threshold:
                _union(i, j)

    clusters: Dict[int, List[int]] = {}
    for i in range(n):
        clusters.setdefault(_find(i), []).append(i)

    offsets = [0.0] * n
    row_gap = row_gap_frac * y_span
    for members in clusters.values():
        if len(members) < 2:
            continue
        members = sorted(members, key=lambda i: -points[i][1])
        base_y = sum(points[i][1] for i in members) / len(members)
        for rank, idx in enumerate(members):
            slot = rank - (len(members) - 1) / 2
            offsets[idx] = (base_y + slot * row_gap) - points[idx][1]
    return offsets


def plot_scatter(
    df: pd.DataFrame,
    x_col: str,
    y_col: str,
    out_path: Path,
    title: str,
    xlabel: str,
    ylabel: str,
) -> None:
    plot_df = df.dropna(subset=[x_col, y_col, "display_name"])
    if plot_df.empty:
        print(f"[WARN] Tidak ada baris valid untuk plot '{title}', dilewati.")
        return

    fig, ax = plt.subplots(figsize=(10, 6.5))
    models = sorted(plot_df["model"].unique())
    for i, model in enumerate(models):
        sub = plot_df[plot_df["model"] == model]
        ax.scatter(
            sub[x_col],
            sub[y_col],
            color=CATEGORICAL_COLORS[i % len(CATEGORICAL_COLORS)],
            label=model,
            s=60,
            edgecolors="white",
            linewidths=0.5,
            zorder=3,
        )

    x_span = float(plot_df[x_col].max() - plot_df[x_col].min())
    y_span = float(plot_df[y_col].max() - plot_df[y_col].min())
    points = [
        (row[x_col], row[y_col], f"{row['display_name']} ({row['tracker']})")
        for _, row in plot_df.iterrows()
    ]
    y_offsets = _place_labels_without_overlap(points, x_span=x_span, y_span=y_span)
    for (x, y, text), y_off in zip(points, y_offsets):
        label_y = y + y_off
        if abs(y_off) > y_span * 0.01:
            ax.plot(
                [x, x],
                [y, label_y],
                color="0.6",
                linewidth=0.6,
                zorder=2,
            )
        ax.annotate(
            text,
            (x, label_y),
            fontsize=7,
            xytext=(6, 0),
            textcoords="offset points",
            va="center",
            zorder=4,
        )
    ax.set_xlabel(xlabel)
    ax.set_ylabel(ylabel)
    ax.set_title(title)
    ax.grid(linestyle=":", alpha=0.4)
    ax.margins(x=0.18)
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
    plot_scatter(
        tradeoff,
        x_col="avg_fps",
        y_col="map50_95",
        out_path=plots_dir / "tradeoff_map_vs_fps.png",
        title="Trade-off Akurasi vs. Kecepatan (Jetson Orin Nano)",
        xlabel="Avg FPS",
        ylabel="mAP50-95 (KITTI val, Tesla T4)",
    )

    power_col = args.power_column or _guess_power_column(tradeoff.columns)
    if power_col is None:
        print("[WARN] Tidak ada kolom 'avg_*_mW' di runtime_summary.csv, plot vs power dilewati.")
    else:
        plot_scatter(
            tradeoff,
            x_col=power_col,
            y_col="map50_95",
            out_path=plots_dir / "tradeoff_map_vs_power.png",
            title="Trade-off Akurasi vs. Daya (Jetson Orin Nano)",
            xlabel=f"{power_col} (mW)",
            ylabel="mAP50-95 (KITTI val, Tesla T4)",
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
