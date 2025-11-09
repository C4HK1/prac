#!/usr/bin/env python3
import argparse
import csv
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np


def read_metrics(path: Path, value_field: str):
    with path.open(newline="") as f:
        reader = csv.DictReader(f)
        data = {}
        jobs = set()
        procs = set()
        for row in reader:
            job = int(row["jobs"])
            proc = int(row["processors"])
            if value_field not in row:
                raise KeyError(f"Column '{value_field}' not found in {path}")
            value = float(row[value_field])
            data[(job, proc)] = value
            jobs.add(job)
            procs.add(proc)
    jobs = sorted(jobs)
    procs = sorted(procs)
    matrix = np.full((len(procs), len(jobs)), np.nan)
    for i, proc in enumerate(procs):
        for j, job in enumerate(jobs):
            if (job, proc) in data:
                matrix[i, j] = data[(job, proc)]
    return jobs, procs, matrix


def format_jobs(jobs):
    formatted = []
    for job in jobs:
        if job >= 1_000_000:
            formatted.append(f"{job/1_000_000:.1f}M")
        elif job >= 1_000:
            formatted.append(f"{job/1_000:.0f}k")
        else:
            formatted.append(str(job))
    return formatted


def main():
    parser = argparse.ArgumentParser(
        description="Plot heat map for sequential annealing runtimes."
    )
    parser.add_argument("metrics", type=Path, help="CSV produced by run_heatmap.sh")
    parser.add_argument("output", type=Path, help="Path to save PNG heatmap")
    parser.add_argument(
        "--field",
        default="avg_time_seconds",
        help="Column to visualize (default: avg_time_seconds)",
    )
    parser.add_argument("--title", default="Heat map")
    parser.add_argument("--cmap", default="viridis")
    args = parser.parse_args()

    jobs, procs, matrix = read_metrics(args.metrics, args.field)

    fig, ax = plt.subplots(figsize=(1.5 * len(jobs), 1.2 * len(procs)))
    im = ax.imshow(matrix, cmap=args.cmap, aspect="auto", origin="lower")

    ax.set_xticks(np.arange(len(jobs)))
    ax.set_xticklabels(format_jobs(jobs), rotation=45, ha="right")
    ax.set_yticks(np.arange(len(procs)))
    ax.set_yticklabels([str(p) for p in procs])
    ax.set_xlabel("Jobs (N)")
    ax.set_ylabel("Processors (M)")
    ax.set_title(args.title)

    # Определяем формат чисел: для cost используем экспоненциальную нотацию
    use_scientific = "cost" in args.field.lower() or np.nanmax(matrix) > 1e10

    for i in range(len(procs)):
        for j in range(len(jobs)):
            value = matrix[i, j]
            if np.isnan(value):
                continue
            # Для больших чисел используем экспоненциальную нотацию
            if use_scientific:
                text = f"{value:.2e}"
            else:
                text = f"{value:.1f}"
            ax.text(j, i, text, ha="center", va="center", color="white", fontsize=9)

    fig.colorbar(im, ax=ax, label=args.field)
    fig.tight_layout()
    fig.savefig(args.output, dpi=150)
    print(f"Heatmap saved to {args.output}")


if __name__ == "__main__":
    main()
