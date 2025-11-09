#!/bin/bash
set -euo pipefail

# Скрипт для исследования зависимости времени работы параллельного алгоритма от Nproc

readonly SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
readonly PROJECT_ROOT="$(dirname "${SCRIPT_DIR}")"
readonly PARALLEL_APP="${PROJECT_ROOT}/build/parallel_app"
readonly SEQUENTIAL_APP="$(dirname "${PROJECT_ROOT}")/sequential/build/sequential_app"

# Параметры по умолчанию
JOBS_FILE_DEFAULT="${PROJECT_ROOT}/../sequential/data/jobs_10000000.csv"
NUM_PROCESSORS_DEFAULT=40
COOLING_DEFAULT="logarithmic"
INITIAL_TEMP_DEFAULT=100.0
WORKERS_DEFAULT="1 2 4 8 16"
RUNS_DEFAULT=3
RESULTS_CSV_DEFAULT="${PROJECT_ROOT}/data/parallel_study.csv"

# Переопределение через переменные окружения
JOBS_FILE="${JOBS_FILE:-$JOBS_FILE_DEFAULT}"
NUM_PROCESSORS="${NUM_PROCESSORS:-$NUM_PROCESSORS_DEFAULT}"
COOLING="${COOLING:-$COOLING_DEFAULT}"
INITIAL_TEMP="${INITIAL_TEMP:-$INITIAL_TEMP_DEFAULT}"
WORKERS_STR="${WORKERS_STR:-$WORKERS_DEFAULT}"
RUNS="${RUNS:-$RUNS_DEFAULT}"
RESULTS_CSV="${RESULTS_CSV:-$RESULTS_CSV_DEFAULT}"

# Преобразуем строку воркеров в массив
IFS=' ' read -r -a WORKERS <<< "${WORKERS_STR}"

# Создаем директорию для данных
mkdir -p "$(dirname "${RESULTS_CSV}")"

# Проверяем наличие файла с данными
if [[ ! -f "${JOBS_FILE}" ]]; then
    echo "Error: Jobs file not found: ${JOBS_FILE}" >&2
    exit 1
fi

# Проверяем наличие исполняемых файлов
if [[ ! -f "${PARALLEL_APP}" ]]; then
    echo "Error: Parallel app not found: ${PARALLEL_APP}" >&2
    exit 1
fi

if [[ ! -f "${SEQUENTIAL_APP}" ]]; then
    echo "Error: Sequential app not found: ${SEQUENTIAL_APP}" >&2
    exit 1
fi

# Записываем заголовок в CSV
echo "num_workers,avg_time_seconds,avg_cost,min_cost" > "${RESULTS_CSV}"

echo "=== Исследование параллельного алгоритма ==="
echo "Jobs file: ${JOBS_FILE}"
echo "Number of processors (M): ${NUM_PROCESSORS}"
echo "Cooling schedule: ${COOLING}"
echo "Initial temperature: ${INITIAL_TEMP}"
echo "Workers to test: ${WORKERS_STR}"
echo "Runs per configuration: ${RUNS}"
echo "Results will be saved to: ${RESULTS_CSV}"
echo ""

# Сначала запускаем последовательную версию для сравнения
echo "=== Запуск последовательной версии для сравнения ==="
sequential_times=()
sequential_costs=()

for (( i=1; i<=${RUNS}; i++ )); do
    echo "-- Sequential run ${i}/${RUNS}"
    output=$("${SEQUENTIAL_APP}" "${JOBS_FILE}" "${NUM_PROCESSORS}" "${COOLING}" "${INITIAL_TEMP}")
    
    time_str=$(echo "${output}" | grep "Execution time" | sed 's/.*Execution time (seconds): //')
    cost_str=$(echo "${output}" | grep "Best solution cost" | sed 's/.*Best solution cost (K2 sum of completion times): //')
    
    if [[ -z "${time_str}" || -z "${cost_str}" ]]; then
        echo "Error: Failed to parse sequential output" >&2
        echo "${output}" >&2
        exit 1
    fi
    
    sequential_times+=( "${time_str}" )
    sequential_costs+=( "${cost_str}" )
done

avg_seq_time=$(printf "%s\n" "${sequential_times[@]}" | awk '{sum+=$1} END {if (NR>0) print sum/NR; else print 0}')
avg_seq_cost=$(printf "%s\n" "${sequential_costs[@]}" | awk '{sum+=$1} END {if (NR>0) print sum/NR; else print 0}')
min_seq_cost=$(printf "%s\n" "${sequential_costs[@]}" | awk 'BEGIN {min=""} {if (min=="" || $1<min) min=$1} END {print min}')

echo "Sequential average time: ${avg_seq_time}s"
echo "Sequential average cost: ${avg_seq_cost}"
echo "Sequential min cost: ${min_seq_cost}"
echo ""

# Теперь тестируем параллельную версию с разным числом воркеров
echo "=== Тестирование параллельной версии ==="
for num_workers in "${WORKERS[@]}"; do
    echo "=== Testing with ${num_workers} workers ==="
    run_times=()
    costs=()
    
    for (( i=1; i<=${RUNS}; i++ )); do
        echo "-- Parallel run ${i}/${RUNS} with ${num_workers} workers"
        output=$("${PARALLEL_APP}" "${JOBS_FILE}" "${NUM_PROCESSORS}" "${COOLING}" "${num_workers}" "${INITIAL_TEMP}")
        
        time_str=$(echo "${output}" | grep "Execution time" | sed 's/.*Execution time (seconds): //')
        cost_str=$(echo "${output}" | grep "Final best cost" | sed 's/.*Final best cost (K2 sum of completion times): //')
        
        if [[ -z "${time_str}" || -z "${cost_str}" ]]; then
            echo "Error: Failed to parse parallel output for ${num_workers} workers" >&2
            echo "${output}" >&2
            exit 1
        fi
        
        run_times+=( "${time_str}" )
        costs+=( "${cost_str}" )
    done
    
    avg_time=$(printf "%s\n" "${run_times[@]}" | awk '{sum+=$1} END {if (NR>0) print sum/NR; else print 0}')
    avg_cost=$(printf "%s\n" "${costs[@]}" | awk '{sum+=$1} END {if (NR>0) print sum/NR; else print 0}')
    min_cost=$(printf "%s\n" "${costs[@]}" | awk 'BEGIN {min=""} {if (min=="" || $1<min) min=$1} END {print min}')
    
    speedup=$(echo "${avg_seq_time} ${avg_time}" | awk '{print $1/$2}')
    echo "Average time: ${avg_time}s (speedup: ${speedup}x)"
    echo "Average cost: ${avg_cost}"
    echo "Min cost: ${min_cost}"
    echo ""
    
    echo "${num_workers},${avg_time},${avg_cost},${min_cost}" >> "${RESULTS_CSV}"
done

echo "=== Результаты ==="
echo "Sequential: ${avg_seq_time}s, cost: ${avg_seq_cost}"
echo "Results saved to: ${RESULTS_CSV}"

