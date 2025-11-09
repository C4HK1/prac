import argparse
import csv
import random
from pathlib import Path


def generate_dataset(
    path: Path, num_jobs: int, min_duration: int, max_duration: int
) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", newline="") as file:
        writer = csv.writer(file)
        writer.writerow(["Job ID", "Duration"])
        for job_id in range(1, num_jobs + 1):
            duration = random.randint(min_duration, max_duration)
            writer.writerow([f"Job_{job_id}", duration])


def build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        description="Генерация CSV с заданиями для имитации отжига."
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=Path("jobs.csv"),
        help="Путь к результирующему CSV.",
    )
    parser.add_argument("--jobs", type=int, default=1000, help="Количество работ.")
    parser.add_argument(
        "--min-duration", type=int, default=1, help="Минимальная длительность работы."
    )
    parser.add_argument(
        "--max-duration",
        type=int,
        default=100,
        help="Максимальная длительность работы.",
    )
    return parser


def main() -> None:
    parser = build_parser()
    args = parser.parse_args()
    generate_dataset(args.output, args.jobs, args.min_duration, args.max_duration)
    print(f"Dataset saved to {args.output.resolve()}")


if __name__ == "__main__":
    main()
