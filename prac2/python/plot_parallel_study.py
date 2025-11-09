#!/usr/bin/env python3
"""
Скрипт для построения графика зависимости времени работы параллельного алгоритма от Nproc
"""
import argparse
import csv
from pathlib import Path

import matplotlib.pyplot as plt
import numpy as np


def plot_parallel_study(csv_path: Path, output_path: Path, sequential_time: float = None) -> None:
    """Строит график зависимости времени работы от числа воркеров"""
    num_workers = []
    avg_times = []
    avg_costs = []
    min_costs = []
    
    with csv_path.open(newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            num_workers.append(int(row["num_workers"]))
            avg_times.append(float(row["avg_time_seconds"]))
            avg_costs.append(float(row["avg_cost"]))
            min_costs.append(float(row["min_cost"]))
    
    # Сортируем по числу воркеров
    sorted_data = sorted(zip(num_workers, avg_times, avg_costs, min_costs))
    num_workers, avg_times, avg_costs, min_costs = zip(*sorted_data)
    
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(16, 6))
    
    # График времени выполнения
    ax1.plot(num_workers, avg_times, 'o-', linewidth=2, markersize=8, label='Параллельный алгоритм')
    if sequential_time is not None:
        ax1.axhline(y=sequential_time, color='r', linestyle='--', linewidth=2, label='Последовательный алгоритм')
    
    ax1.set_xlabel('Число воркеров (Nproc)', fontsize=12)
    ax1.set_ylabel('Время выполнения (секунды)', fontsize=12)
    ax1.set_title('Зависимость времени работы от числа воркеров', fontsize=14, fontweight='bold')
    ax1.grid(True, alpha=0.3)
    ax1.legend(fontsize=11)
    ax1.set_xlim(0, max(num_workers) * 1.1)
    
    # Добавляем аннотации с ускорением
    if sequential_time is not None:
        for nw, time in zip(num_workers, avg_times):
            speedup = sequential_time / time
            ax1.annotate(f'{speedup:.2f}x', 
                        xy=(nw, time), 
                        xytext=(5, 5), 
                        textcoords='offset points',
                        fontsize=9,
                        bbox=dict(boxstyle='round,pad=0.3', facecolor='yellow', alpha=0.5))
    
    # График качества решения (стоимость)
    ax2.plot(num_workers, avg_costs, 'o-', linewidth=2, markersize=8, label='Средняя стоимость', color='blue')
    ax2.plot(num_workers, min_costs, 's-', linewidth=2, markersize=8, label='Минимальная стоимость', color='green')
    
    ax2.set_xlabel('Число воркеров (Nproc)', fontsize=12)
    ax2.set_ylabel('Стоимость решения (K2)', fontsize=12)
    ax2.set_title('Зависимость качества решения от числа воркеров', fontsize=14, fontweight='bold')
    ax2.grid(True, alpha=0.3)
    ax2.legend(fontsize=11)
    ax2.set_xlim(0, max(num_workers) * 1.1)
    
    # Используем экспоненциальную нотацию для больших чисел
    if max(avg_costs) > 1e10:
        ax2.ticklabel_format(style='scientific', axis='y', scilimits=(0,0))
    
    plt.tight_layout()
    plt.savefig(output_path, dpi=150, bbox_inches='tight')
    print(f"График сохранен в: {output_path}")


def main() -> None:
    parser = argparse.ArgumentParser(description="Построение графика исследования параллельного алгоритма")
    parser.add_argument("csv_path", type=Path, help="Путь к CSV файлу с результатами")
    parser.add_argument("output_path", type=Path, help="Путь для сохранения графика (PNG)")
    parser.add_argument("--sequential-time", type=float, help="Время выполнения последовательного алгоритма для сравнения")
    args = parser.parse_args()
    
    args.output_path.parent.mkdir(parents=True, exist_ok=True)
    plot_parallel_study(args.csv_path, args.output_path, args.sequential_time)


if __name__ == "__main__":
    main()

