#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
SEQ_DIR=$(cd "${SCRIPT_DIR}/.." && pwd)
AIO_DIR=$(cd "${SEQ_DIR}/.." && pwd)
PROJECT_ROOT=$(cd "${AIO_DIR}/.." && pwd)
PYTHON_BIN=${PYTHON_BIN:-python3}

# Чтобы модули python/ были доступны для импортов.
export PYTHONPATH="${PROJECT_ROOT}:${PYTHONPATH:-}"

# Наборы значений N (число работ) и M (число процессоров).
# Можно переопределить через переменные окружения JOB_VALUES_STR и PROC_VALUES_STR.
JOB_VALUES_STR=${JOB_VALUES_STR:-"2000000 4000000 6000000"}
PROC_VALUES_STR=${PROC_VALUES_STR:-"20 30 40"}
IFS=' ' read -r -a JOB_VALUES <<< "${JOB_VALUES_STR}"
IFS=' ' read -r -a PROC_VALUES <<< "${PROC_VALUES_STR}"

COOLING=${COOLING:-cauchy}
RUNS=${RUNS:-5}
DATA_DIR="${SEQ_DIR}/data"
RESULTS_CSV="${RESULTS_CSV:-${DATA_DIR}/heatmap_metrics.csv}"
SEQUENTIAL_BIN="${SEQ_DIR}/build/sequential_app"
GENERATOR_MODULE="python.data_generator.run_experiments"

if [[ ! -x "${SEQUENTIAL_BIN}" ]]; then
  echo "error: ${SEQUENTIAL_BIN} not found. Build the project first." >&2
  echo "       cmake -S ${SEQ_DIR} -B ${SEQ_DIR}/build" >&2
  echo "       cmake --build ${SEQ_DIR}/build --config Release" >&2
  exit 1
fi

echo "jobs,processors,avg_time_seconds,avg_cost,min_cost" > "${RESULTS_CSV}"

for jobs in "${JOB_VALUES[@]}"; do
  dataset="${DATA_DIR}/jobs_${jobs}.csv"
  if [[ ! -f "${dataset}" ]]; then
    echo "[generator] jobs=${jobs}" >&2
    ${PYTHON_BIN} -m ${GENERATOR_MODULE} --output "${dataset}" --jobs "${jobs}" --min-duration 1 --max-duration 100
  fi

  for procs in "${PROC_VALUES[@]}"; do
    echo "=== jobs=${jobs}, processors=${procs}, cooling=${COOLING} ==="
    run_times=()
    costs=()

    for run in $(seq 1 "${RUNS}"); do
      echo "-- run ${run}/${RUNS}" >&2
      output=$("${SEQUENTIAL_BIN}" "${dataset}" "${procs}" "${COOLING}")
      echo "${output}" >&2
      run_time=$(echo "${output}" | awk -F': ' '/Execution time \(seconds\)/ {print $2}')
      cost=$(echo "${output}" | awk -F': ' '/Best solution cost/ {print $2}')
      if [[ -z "${run_time}" || -z "${cost}" ]]; then
        echo "error: failed to parse output" >&2
        exit 1
      fi
      run_times+=("${run_time}")
      costs+=("${cost}")
    done

    avg_time=$(${PYTHON_BIN} -c 'import sys; vals = list(map(float, sys.argv[1:])); print(sum(vals)/len(vals) if vals else 0.0)' "${run_times[@]}")

    avg_cost=$(${PYTHON_BIN} -c 'import sys; vals = list(map(float, sys.argv[1:])); print(sum(vals)/len(vals) if vals else 0.0)' "${costs[@]}")

    min_cost=$(${PYTHON_BIN} -c 'import sys; vals = list(map(float, sys.argv[1:])); print(min(vals) if vals else 0.0)' "${costs[@]}")

    echo "${jobs},${procs},${avg_time},${avg_cost},${min_cost}" >> "${RESULTS_CSV}"
  done
done

echo "Heatmap metrics saved to ${RESULTS_CSV}"
