# Инструкции по тестированию алгоритма имитации отжига

## Структура проекта

```
АИО/
├── sequential/          # Последовательная реализация
│   ├── build/           # Собранное приложение
│   ├── data/            # Данные и результаты
│   ├── include/         # Заголовочные файлы
│   ├── src/             # Исходные файлы
│   ├── scripts/         # Скрипты для исследований
│   └── CMakeLists.txt
├── parallel/            # Параллельная реализация
│   ├── build/           # Собранное приложение
│   ├── data/            # Данные и результаты
│   ├── include/         # Заголовочные файлы
│   ├── src/             # Исходные файлы
│   ├── scripts/         # Скрипты для исследований
│   └── CMakeLists.txt
python/                  # Python скрипты
├── data_generator/      # Генерация данных
└── plot_*.py            # Построение графиков
docs/                    # Документация
```

## 1. Сборка проектов

### Последовательная версия
```bash
cd АИО/sequential
cmake -S . -B build
cmake --build build
```

### Параллельная версия
```bash
cd АИО/parallel
cmake -S . -B build
cmake --build build
```

## 2. Генерация данных

```bash
cd python/data_generator
python run_experiments.py --output ../../АИО/sequential/data/jobs_10000.csv --jobs 10000 --min-duration 1 --max-duration 100
```

## 3. Запуск последовательной версии

### Базовый запуск
```bash
cd АИО/sequential
./build/sequential_app data/jobs_10000.csv 10 logarithmic 100
```

Параметры:
1. `data/jobs_10000.csv` — файл с данными
2. `10` — число процессоров (M)
3. `logarithmic` — закон охлаждения (boltzmann/cauchy/logarithmic)
4. `100` — начальная температура (опционально)

### С отрицанием критерия (для отладки)
```bash
./build/sequential_app data/jobs_1000.csv 1 logarithmic 10000 --negate-cost
```

## 4. Запуск параллельной версии

```bash
cd АИО/parallel
./build/parallel_app ../sequential/data/jobs_10000.csv 10 logarithmic 4 100
```

Параметры:
1. `../sequential/data/jobs_10000.csv` — файл с данными
2. `10` — число процессоров (M)
3. `logarithmic` — закон охлаждения
4. `4` — число воркеров (Nproc)
5. `100` — начальная температура (опционально)

## 5. Исследование последовательного алгоритма

### 5.1. Поиск "тяжёлого" набора данных

```bash
cd АИО/sequential
# Тест с 10 млн работ
time ./build/sequential_app data/jobs_10000000.csv 40 logarithmic 100
```

Цель: найти N и M, где выполнение занимает > 1 минуты.

### 5.2. Сравнение законов охлаждения

На "тяжёлом" наборе запустить для каждого закона:
```bash
./build/sequential_app data/jobs_10000000.csv 40 boltzmann 100
./build/sequential_app data/jobs_10000000.csv 40 cauchy 100
./build/sequential_app data/jobs_10000000.csv 40 logarithmic 100
```

Сравнить: время выполнения и качество решения (K2).

### 5.3. Построение heat map

```bash
cd АИО/sequential/scripts
RUNS=5 \
JOB_VALUES_STR="2000000 4000000 6000000" \
PROC_VALUES_STR="20 30 40" \
COOLING=logarithmic \
RESULTS_CSV="../data/heatmap_metrics_logarithmic.csv" \
./run_heatmap.sh
```

Построение графиков:
```bash
cd /home/alex/Desktop/prac/prac2
python python/plot_heatmap.py \
  АИО/sequential/data/heatmap_metrics_logarithmic.csv \
  АИО/sequential/data/heatmap_time_logarithmic.png \
  --field avg_time_seconds \
  --title "Logarithmic: Average Runtime (s)"

python python/plot_heatmap.py \
  АИО/sequential/data/heatmap_metrics_logarithmic.csv \
  АИО/sequential/data/heatmap_cost_logarithmic.png \
  --field avg_cost \
  --title "Logarithmic: Average Final Cost"
```

## 6. Исследование параллельного алгоритма

### 6.1. Исследование Nproc

```bash
cd АИО/parallel/scripts
JOBS_FILE="../../sequential/data/jobs_10000.csv" \
NUM_PROCESSORS=10 \
COOLING=logarithmic \
WORKERS_STR="1 2 4 8" \
RUNS=3 \
./run_parallel_study.sh
```

### 6.2. Построение графика

```bash
cd /home/alex/Desktop/prac/prac2
python python/plot_parallel_study.py \
  АИО/parallel/data/parallel_study.csv \
  АИО/parallel/data/parallel_study_graph.png \
  --sequential-time <время_из_sequential>
```

## 7. Отладка алгоритма

### Проверка с отрицанием К2

Для отрицания К2 оптимальное решение:
- Все работы на одном процессоре
- Отсортированы по убыванию длительности

```bash
cd АИО/sequential
./build/sequential_app data/jobs_1000.csv 1 logarithmic 10000 --negate-cost
```

Ожидаемый результат:
- Все 1000 работ на процессоре 0
- Работы отсортированы по убыванию длительности

## 8. Интерпретация результатов

### Последовательный алгоритм

**Успешно, если**:
- Время выполнения растёт с увеличением N и M
- Разные законы охлаждения дают разные результаты
- Logarithmic обычно быстрее, Cauchy даёт лучшее качество

### Параллельный алгоритм

**На маленьких задачах (< 100K работ)**:
- Параллельная версия медленнее из-за overhead
- Но находит лучшие решения (меньшая стоимость K2)

**На больших задачах (> 1M работ)**:
- Ожидается ускорение при 2-4 воркерах
- Дальнейшее увеличение воркеров даёт < 10% прироста

## 9. Типичные проблемы

### Проблема: "Sequential average time: 0s"
**Причина**: Неправильный парсинг вывода  
**Решение**: Проверить формат вывода, использовать `sed` вместо `awk`

### Проблема: Параллельная версия очень медленная
**Причина**: Слишком много глобальных итераций  
**Решение**: Уменьшить `globalStagnationLimit` в `ProcessCoordinator.h`

### Проблема: Не помещаются числа в heat map
**Причина**: Большие значения K2  
**Решение**: Использовать экспоненциальную нотацию (уже реализовано в `plot_heatmap.py`)

## 10. Файлы результатов

После выполнения всех экспериментов:

```
АИО/sequential/data/
├── jobs_*.csv                          # Входные данные
├── heatmap_metrics_*.csv               # Метрики для heat map
├── heatmap_time_*.png                  # Heat map времени
└── heatmap_cost_*.png                  # Heat map стоимости

АИО/parallel/data/
├── parallel_study.csv                  # Результаты исследования Nproc
└── parallel_study_graph.png            # График зависимости

docs/
├── PARALLEL_STUDY_RESULTS.md           # Анализ параллельного алгоритма
└── TESTING_INSTRUCTIONS.md             # Эта инструкция
```

