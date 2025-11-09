# Объяснение методов serialize и deserialize

## Зачем нужны эти методы?

Методы `serialize()` и `deserialize()` используются **только в параллельной версии** алгоритма для передачи решения между процессами через **UNIX сокеты**.

### Проблема
В параллельной версии:
- **Родительский процесс** (координатор) хранит глобально лучшее решение
- **Дочерние процессы** (воркеры) получают это решение, улучшают его локально и возвращают результат
- Процессы **не могут** просто передать указатель или ссылку на объект `SchedulingSolution` — у них разные адресные пространства!

### Решение
Решение нужно **сериализовать** (преобразовать в байты) → передать через сокет → **десериализовать** (восстановить объект).

## Формат данных

### Что возвращает `serialize()`?

```cpp
std::vector<int> serialize() const;
```

Возвращает **вектор целых чисел** (`std::vector<int>`), который кодирует **полное распределение работ по процессорам**.

### Структура вектора `data`

Вектор имеет следующую структуру:

```
data = [
    size_0, job_0_0, job_0_1, ..., job_0_{size_0-1},  // Процессор 0
    size_1, job_1_0, job_1_1, ..., job_1_{size_1-1},  // Процессор 1
    ...
    size_{M-1}, job_{M-1}_0, ..., job_{M-1}_{size_{M-1}-1}  // Процессор M-1
]
```

Где:
- `size_i` — количество работ на процессоре `i` (int)
- `job_i_j` — ID работы (int, от 0 до N-1)

### Пример

Допустим, у нас:
- 3 процессора (M=3)
- 5 работ (N=5)
- Распределение:
  - Процессор 0: работы [2, 4]
  - Процессор 1: работы [0, 1, 3]
  - Процессор 2: работы [] (пусто)

Тогда `serialize()` вернёт:
```cpp
std::vector<int> data = {
    2,      // размер очереди процессора 0
    2, 4,   // ID работ на процессоре 0
    3,      // размер очереди процессора 1
    0, 1, 3,// ID работ на процессоре 1
    0       // размер очереди процессора 2 (пусто)
};
```

## Как это работает в параллельной версии

### 1. Родительский процесс отправляет задачу воркеру

```cpp
// В ProcessCoordinator::run()
const auto serializedBest = toInt32Vector(globalBest->serialize());
sendTask(worker.socketFd, seed, serializedBest);
```

**Что происходит**:
1. `globalBest->serialize()` → `std::vector<int>` (распределение работ)
2. `toInt32Vector()` → `std::vector<int32_t>` (для передачи через сокет)
3. `sendTask()` → записывает в сокет:
   - Тип сообщения (1 = задача)
   - Seed для генератора случайных чисел
   - Размер вектора
   - Сами данные (байты вектора)

### 2. Воркер получает задачу

```cpp
// В workerLoop()
auto serializedSchedule = toIntVector(payload);
auto initialSolution = SchedulingSolution::deserialize(
    numProcessors, jobDurations, serializedSchedule, seed);
```

**Что происходит**:
1. `readAll()` → читает байты из сокета в `std::vector<int32_t>`
2. `toIntVector()` → преобразует в `std::vector<int>`
3. `deserialize()` → восстанавливает объект `SchedulingSolution`:
   - Создаёт новый объект (без инициализации)
   - Читает размеры очередей для каждого процессора
   - Заполняет очереди ID работ
   - Обновляет индексы `m_jobToProcessor` и `m_jobPosition`

### 3. Воркер возвращает результат

```cpp
// В workerLoop()
auto serializedBest = toInt32Vector(schedulingSolution->serialize());
writeAll(fd, &cost, sizeof(cost));
writeAll(fd, &responseSize, sizeof(responseSize));
writeAll(fd, serializedBest.data(), serializedBest.size() * sizeof(int32_t));
```

**Что происходит**:
1. Воркер сериализует своё лучшее решение
2. Отправляет стоимость решения (double)
3. Отправляет размер вектора (uint32_t)
4. Отправляет сам вектор (байты)

### 4. Родительский процесс получает результат

```cpp
// В ProcessCoordinator::run()
const WorkerResult result = receiveResult(worker.socketFd);
if (!result.serializedSolution.empty() && result.cost < globalBestCost) {
    auto updatedSolution = SchedulingSolution::deserialize(
        m_numProcessors, m_jobDurations, result.serializedSolution, newSeed);
    globalBest = std::move(updatedSolution);
}
```

**Что происходит**:
1. Читает стоимость и размер вектора
2. Читает вектор данных
3. Если решение лучше → десериализует и обновляет глобальное лучшее решение

## Реализация serialize()

```cpp
std::vector<int> SchedulingSolution::serialize() const {
    std::vector<int> data;
    data.reserve(numJobs() + m_numProcessors);
    
    // Для каждого процессора:
    for (const auto& queue : m_processorJobs) {
        // 1. Записываем размер очереди
        data.push_back(static_cast<int>(queue.size()));
        
        // 2. Записываем ID всех работ в очереди
        data.insert(data.end(), queue.begin(), queue.end());
    }
    
    return data;
}
```

**Размер вектора**: `N + M`, где N — число работ, M — число процессоров.

## Реализация deserialize()

```cpp
std::unique_ptr<SchedulingSolution> SchedulingSolution::deserialize(
    int numProcessors,
    const std::vector<uint32_t>& jobDurations,
    const std::vector<int>& data,
    unsigned int seed) {
    
    // Создаём пустое решение (без инициализации)
    auto solution = std::make_unique<SchedulingSolution>(
        numProcessors, jobDurations, seed, false);
    
    std::size_t index = 0;
    
    // Для каждого процессора:
    for (int processor = 0; processor < numProcessors; ++processor) {
        // 1. Читаем размер очереди
        int count = data[index++];
        
        // 2. Читаем ID работ и добавляем в очередь
        for (int i = 0; i < count; ++i) {
            int jobId = data[index++];
            queue.push_back(jobId);
            solution->m_jobToProcessor[jobId] = processor;
            solution->m_jobPosition[jobId] = queue.size() - 1;
        }
    }
    
    return solution;
}
```

**Проверки**:
- Все работы должны быть распределены (ровно N работ)
- Каждая работа должна быть назначена ровно один раз
- ID работ должны быть валидными (0..N-1)
- Размер вектора должен точно соответствовать структуре

## Почему int32_t, а не int?

В функции `sendTask()` используется `int32_t` (32-битное целое), а не `int` (который может быть 64-битным на некоторых системах), чтобы:
1. **Гарантировать размер** — всегда 4 байта
2. **Совместимость** между разными системами
3. **Эффективность** — меньше данных для передачи

## Визуализация процесса

```
┌─────────────────┐
│ Родительский    │
│ процесс         │
│                 │
│ globalBest      │
│ (SchedulingSol) │
└────────┬────────┘
         │ serialize()
         │ → [2, 0, 1, 3, 2, 4]
         │
         ▼
    ┌─────────┐
    │ Сокет   │
    │ (байты) │
    └────┬────┘
         │
         │ readAll()
         │
         ▼
┌─────────────────┐
│ Дочерний        │
│ процесс         │
│                 │
│ deserialize()   │
│ → SchedulingSol │
└─────────────────┘
```

## Итого

- **`serialize()`**: Преобразует объект `SchedulingSolution` в `std::vector<int>` для передачи через сокет
- **`deserialize()`**: Восстанавливает объект `SchedulingSolution` из `std::vector<int>`
- **Вектор `data`**: Кодирует распределение работ по процессорам в формате `[размер_0, работы_0, размер_1, работы_1, ...]`
- **Использование**: Только в параллельной версии для межпроцессного обмена данными

Эти методы **не используются** в последовательной версии — там нет необходимости передавать решение между процессами.

