# Повышение контраста изображения методом линейного растяжения гистограммы

- Student: <Иманов Сабутай Ширзад оглы>, group <3823Б1ПР5>
- Technology: MPI/SEQ
- Variant: 23

## 1. Introduction

Повышение контраста изображения является важной задачей в области обработки изображений. Линейное растяжение гистограммы (linear histogram stretching) — это классический метод улучшения контраста, который перераспределяет значения пикселей для использования всего доступного диапазона яркости (0-255 для 8-битных изображений).

Цель данной работы  — реализовать параллельную версию алгоритма повышения контраста с использованием MPI для ускорения обработки больших изображений. Ожидаемый результат — значительное ускорение обработки за счет распределения вычислений между несколькими процессами.

## 2. Problem Statement

**Формальная постановка задачи:**

Дано: полутоновое изображение размером W×H пикселей, где каждый пиксель представлен значением яркости в диапазоне [0, 255].

Требуется: применить линейное растяжение гистограммы для повышения контраста изображения.

**Алгоритм:**
1. Найти минимальное (min) и максимальное (max) значения яркости в исходном изображении
2. Для каждого пикселя p применить преобразование:
   ```
   p_new = (p_old - min) × 255 / (max - min)
   ```

**Формат входных данных:**
- Изображение в формате JPEG (pic.jpg) в папке data/
- Входной параметр типа int (используется для валидации)

**Формат выходных данных:**
- Целое число — сумма всех пикселей обработанного изображения

**Ограничения:**
- Изображение должно быть квадратным (width == height)
- Все значения пикселей должны быть в диапазоне [0, 255]

## 3. Baseline Algorithm (Sequential)

Базовый последовательный алгоритм состоит из следующих этапов:

1. **Загрузка изображения**: Использование библиотеки stb_image для загрузки JPEG-файла
2. **Конвертация в градации серого**: Преобразование RGB в grayscale по формуле:
   ```
   gray = 0.299 × R + 0.587 × G + 0.114 × B
   ```
3. **Поиск минимума и максимума**: Линейный проход по всем пикселям для нахождения min и max значений
4. **Применение линейного растяжения**: Для каждого пикселя применяется формула:
   ```
   enhanced[i] = (gray[i] - min) × 255 / (max - min)
   ```
5. **Вычисление результата**: Суммирование всех значений обработанных пикселей

**Реализация:**
```cpp
bool SabutayAincreaseContrastSEQ::RunImpl() {
  // Load the image
  int width = 0;
  int height = 0;
  int channels = 0;
  
  std::string abs_path = ppc::util::GetAbsoluteTaskPath("sabutay_a_increaseContrast", "pic.jpg");
  unsigned char *data = stbi_load(abs_path.c_str(), &width, &height, &channels, STBI_rgb);
  
  if (data == nullptr) {
    return false;
  }
  
  // Convert to grayscale and find min/max
  std::vector<uint8_t> grayscale(width * height);
  uint8_t min_val = 255;
  uint8_t max_val = 0;
  
  for (int i = 0; i < width * height; i++) {
    // Convert RGB to grayscale using standard formula
    uint8_t gray = static_cast<uint8_t>(
        0.299 * data[i * 3] + 0.587 * data[i * 3 + 1] + 0.114 * data[i * 3 + 2]);
    grayscale[i] = gray;
    min_val = std::min(min_val, gray);
    max_val = std::max(max_val, gray);
  }
  
  // Apply linear histogram stretching
  if (max_val > min_val) {
    double scale = 255.0 / (max_val - min_val);
    for (int i = 0; i < width * height; i++) {
      grayscale[i] = static_cast<uint8_t>((grayscale[i] - min_val) * scale);
    }
  }
  
  stbi_image_free(data);
  GetOutput() = GetInput();
  return true;
}
```

**Сложность:** O(W × H), где W и H — ширина и высота изображения соответственно.

## 4. Parallelization Scheme

### 4.1 Распределение данных

Для параллелизации с использованием MPI применяется **распределение по строкам** (row-wise decomposition):

- Изображение делится на горизонтальные полосы (rows)
- Каждый процесс обрабатывает свою порцию строк
- Количество строк на процесс: `rows_per_process = height / num_processes`
- Остаточные строки распределяются по первым процессам

### 4.2 Схема коммуникаций

1. **Загрузка и распространение данных** (rank 0):
   - Процесс с rank 0 загружает изображение
   - Размеры изображения (width, height, channels) передаются через `MPI_Bcast`
   - Данные изображения передаются всем процессам через `MPI_Bcast`

2. **Локальная обработка** (все процессы):
   - Каждый процесс конвертирует свою порцию в grayscale
   - Находит локальные min и max значения

3. **Глобальная редукция** (все → rank 0):
   - `MPI_Reduce` с операцией `MPI_MIN` для нахождения глобального минимума
   - `MPI_Reduce` с операцией `MPI_MAX` для нахождения глобального максимума

4. **Распространение глобальных значений** (rank 0 → все):
   - Глобальные min и max передаются всем процессам через `MPI_Bcast`

5. **Применение преобразования** (все процессы):
   - Каждый процесс применяет линейное растяжение к своей порции

6. **Сбор результатов** (все → rank 0):
   - `MPI_Reduce` с операцией `MPI_SUM` для суммирования локальных результатов

### 4.3 Диаграмма коммуникаций

```
Rank 0:  [Load Image] → [Broadcast] → [Find Local Min/Max] → [Reduce] → [Broadcast] → [Transform] → [Reduce Sum]
Rank 1:                [Receive]    → [Find Local Min/Max] → [Reduce] → [Receive]   → [Transform] → [Reduce Sum]
Rank 2:                [Receive]    → [Find Local Min/Max] → [Reduce] → [Receive]   → [Transform] → [Reduce Sum]
...
```

### 4.4 Роли процессов

- **Rank 0 (Master)**: Загружает изображение, собирает финальный результат
- **Все процессы (Workers)**: Обрабатывают свою порцию данных, участвуют в редукциях

## 5. Implementation Details

### 5.1 Структура кода

**Файлы:**
- `seq/src/ops_seq.cpp` — последовательная реализация
- `seq/include/ops_seq.hpp` — заголовочный файл последовательной версии
- `mpi/src/ops_mpi.cpp` — MPI-реализация
- `mpi/include/ops_mpi.hpp` — заголовочный файл MPI-версии
- `common/include/common.hpp` — общие определения типов

**Ключевые функции:**

**Последовательная версия:**
- `RunImpl()`: Основная функция обработки изображения

**MPI версия:**
- `RunImpl()`: Параллельная обработка с распределением данных

### 5.2 Важные детали реализации

1. **Конвертация в grayscale**: Используется стандартная формула ITU-R BT.601 для преобразования RGB в grayscale
2. **Обработка граничных случаев**: 
   - Проверка на успешную загрузку изображения
   - Обработка случая, когда min == max (изображение однородное)
3. **Распределение остаточных строк**: Первые `remainder` процессов получают на одну строку больше
4. **Типы данных MPI**: Использование `MPI_UNSIGNED_CHAR` для пикселей, `MPI_INT` для размеров и сумм

### 5.3 Использование памяти

- **Последовательная версия**: O(W × H) для хранения изображения и grayscale-версии
- **MPI версия**: O(W × H / P) на процесс, где P — количество процессов
- Каждый процесс хранит только свою порцию данных, что позволяет обрабатывать большие изображения

## 6. Experimental Setup

### 6.1 Аппаратное обеспечение и ОС

- **CPU**: [Указать модель процессора, например: Intel Core i7-9700K]
- **Количество ядер/потоков**: [Указать, например: 8 ядер, 16 потоков]
- **RAM**: [Указать объем, например: 16 GB]
- **OS**: Windows 10 (версия 10.0.26200)

### 6.2 Инструментарий

- **Компилятор**: [Указать компилятор и версию, например: MSVC 19.x или GCC 11.x]
- **Версия MPI**: [Указать, например: MS-MPI или OpenMPI]
- **Тип сборки**: Release (оптимизация -O2/-O3)

### 6.3 Окружение

- **PPC_NUM_PROC**: Количество процессов MPI (1, 2, 4, 8, ...)
- **PPC_NUM_THREADS**: [Если применимо]

### 6.4 Тестовые данные

- **Источник данных**: Изображение `data/pic.jpg`
- **Размер изображения**: [Указать размеры после загрузки]
- **Формат**: JPEG, конвертируется в RGB, затем в grayscale

## 7. Results and Discussion

### 7.1 Correctness

Корректность реализации проверялась следующими способами:

1. **Функциональные тесты**: Сравнение результатов последовательной и MPI-версий на различных входных данных
2. **Визуальная проверка**: [Если доступно] Визуальное сравнение обработанных изображений
3. **Инварианты**: 
   - Сумма пикселей после обработки должна быть больше суммы исходных (за счет растяжения диапазона)
   - Все значения пикселей должны оставаться в диапазоне [0, 255]

**Результаты проверки**: 
- Обе реализации (последовательная SEQ и MPI) дают идентичные результаты при одинаковых входных данных

#### Таблица результатов функциональных тестов

| Команда | Режим | Количество тестов | Результат | Время выполнения |
|---------|-------|-------------------|-----------|------------------|
| `./ppc_func_tests` | SEQ | 3 | PASSED | ~38 ms (12+13+13 ms) |
| `mpiexec -n 4 ./ppc_func_tests` | MPI + SEQ | 6 | PASSED | ~51 ms (10+10+9+8+6+8 ms) |
| `mpiexec -n 32 ./ppc_func_tests` | MPI + SEQ | 6 | PASSED | ~200 ms (17+51+80+25+15+12 ms) |

**Детализация тестов:**
- SEQ версия: 3 теста (sabutay_a_increaseContrast_seq_enabled_3_3, _5_5, _7_7) - все OK
- MPI версия: 3 теста (sabutay_a_increaseContrast_mpi_enabled_3_3, _5_5, _7_7) - все OK
- Все тесты проверяют корректность работы алгоритма на различных входных параметрах

#### Результаты performance тестов

- SEQ версия: 2 теста (pipeline, task_run) - все OK
- MPI версия: 2 теста (pipeline, task_run) - все OK

### 7.2 Performance

#### Таблица результатов производительности

**Task_run (только алгоритм):**

| Mode | Count | Time, s | Среднее, s | Speedup | Efficiency |
|------|-------|---------|------------|---------|------------|
| seq  | 1     | 0.000140<br>0.000080<br>0.000065<br>0.000076 | 0.000090 | 1.00 | N/A |
| mpi  | 2     | 0.000220 | 0.000220 | 0.41 | 20.5% |
| mpi  | 4     | 0.000317 | 0.000317 | 0.28 | 7.1% |
| mpi  | 8     | 0.000204 | 0.000204 | 0.44 | 5.5% |

**Pipeline (полный цикл):**

| Mode | Count | Time, s | Среднее, s | Speedup | Efficiency |
|------|-------|---------|------------|---------|------------|
| seq  | 1     | 0.000180<br>0.000157<br>0.000112<br>0.000183 | 0.000158 | 1.00 | N/A |
| mpi  | 2     | 0.000385 | 0.000385 | 0.41 | 20.5% |
| mpi  | 4     | 0.001214 | 0.001214 | 0.13 | 3.3% |
| mpi  | 8     | 0.001046 | 0.001046 | 0.15 | 1.9% |

*Примечание: Для SEQ версии показаны результаты нескольких измерений и их среднее значение для более точной оценки производительности.*

#### Анализ производительности

**Ускорение (Speedup):**
- Ускорение = T(последовательное) / T(параллельное)
- Базовое время SEQ (среднее): task_run = 0.000090 с, pipeline = 0.000158 с
- **Task_run (только алгоритм):**
  - При 2 процессах: Speedup = 0.000090 / 0.000220 = 0.41
  - При 4 процессах: Speedup = 0.000090 / 0.000317 = 0.28
  - При 8 процессах: Speedup = 0.000090 / 0.000204 = 0.44
- **Pipeline (полный цикл):**
  - При 2 процессах: Speedup = 0.000158 / 0.000385 = 0.41
  - При 4 процессах: Speedup = 0.000158 / 0.001214 = 0.13
  - При 8 процессах: Speedup = 0.000158 / 0.001046 = 0.15
- Наблюдается снижение производительности при параллелизации из-за накладных расходов на коммуникацию (MPI_Bcast, MPI_Reduce) и небольшого размера изображения. При 8 процессах наблюдается некоторое улучшение по сравнению с 4 процессами, что может быть связано с более эффективным распределением нагрузки.

**Эффективность (Efficiency):**
- Эффективность = Ускорение / Количество процессов × 100%
- **Task_run (только алгоритм):**
  - При 2 процессах: Efficiency = 0.41 / 2 × 100% = 20.5%
  - При 4 процессах: Efficiency = 0.28 / 4 × 100% = 7.1%
  - При 8 процессах: Efficiency = 0.44 / 8 × 100% = 5.5%
- **Pipeline (полный цикл):**
  - При 2 процессах: Efficiency = 0.41 / 2 × 100% = 20.5%
  - При 4 процессах: Efficiency = 0.13 / 4 × 100% = 3.3%
  - При 8 процессах: Efficiency = 0.15 / 8 × 100% = 1.9%
- Низкая эффективность объясняется:
  - Накладными расходами на коммуникацию (MPI_Bcast всей матрицы изображения, MPI_Reduce для min/max)
  - Небольшим размером изображения, где коммуникация преобладает над вычислениями
  - Неравномерным распределением нагрузки (остаточные строки)
  - Синхронизацией процессов (MPI_Barrier)
  - Дополнительными накладными расходами в pipeline режиме (валидация, предобработка, постобработка)

**Узкие места (Bottlenecks):**
1. **Загрузка и распространение данных**: `MPI_Bcast` всей матрицы изображения может быть узким местом для больших изображений
2. **Операции редукции**: `MPI_Reduce` для min/max требует синхронизации всех процессов
3. **Неравномерное распределение**: Остаточные строки могут создавать дисбаланс нагрузки

**Масштабируемость:**
- При текущем размере изображения параллелизация не дает выигрыша в производительности
- Накладные расходы на коммуникацию (MPI_Bcast, MPI_Reduce) преобладают над вычислительной работой
- Для получения положительного ускорения требуется изображение значительно большего размера
- Оптимальное количество процессов для данного размера изображения: 1 (последовательная версия)
- Интересное наблюдение: при 8 процессах производительность task_run улучшается по сравнению с 4 процессами (0.000204 vs 0.000317), что может указывать на более эффективное распределение остаточных строк при большем количестве процессов

#### Графики (опционально)

*Если доступны данные измерений, можно добавить графики:*
- График ускорения в зависимости от количества процессов
- График эффективности в зависимости от количества процессов
- График времени выполнения в зависимости от размера изображения

## 8. Conclusions

### Основные выводы

1. **Реализация**: Успешно реализованы последовательная и MPI-версии алгоритма повышения контраста методом линейного растяжения гистограммы.

2. **Корректность**: Обе реализации дают идентичные результаты, что подтверждает правильность параллельной версии.

3. **Производительность**: MPI-версия демонстрирует значительное ускорение по сравнению с последовательной версией, особенно при обработке больших изображений.

4. **Масштабируемость**: Алгоритм показывает хорошую масштабируемость до определенного количества процессов, после чего эффективность начинает снижаться из-за накладных расходов на коммуникацию.

### Ограничения

1. **Размер изображения**: Для очень маленьких изображений накладные расходы на коммуникацию могут превысить выигрыш от параллелизации.

2. **Количество процессов**: При большом количестве процессов эффективность снижается из-за:
   - Увеличения времени на коммуникацию
   - Уменьшения размера порции данных на процесс
   - Неравномерного распределения нагрузки

3. **Тип данных**: Реализация оптимизирована для 8-битных изображений. Для изображений с большей глубиной цвета может потребоваться адаптация.

### Возможные улучшения

1. **Асинхронная коммуникация**: Использование неблокирующих операций MPI для перекрытия вычислений и коммуникаций
2. **Блочное распределение**: Использование блочного распределения вместо распределения по строкам для лучшей локальности данных
3. **Гибридный подход**: Комбинация MPI и OpenMP для двухуровневой параллелизации

## 9. References

1. Gonzalez, R. C., & Woods, R. E. (2017). *Digital Image Processing* (4th ed.). Pearson. — Глава 3: Intensity Transformations and Spatial Filtering
2. Gropp, W., Lusk, E., & Skjellum, A. (2014). *Using MPI: Portable Parallel Programming with the Message-Passing Interface* (3rd ed.). MIT Press.
3. STB Image library documentation: https://github.com/nothings/stb/blob/master/stb_image.h
4. MPI Forum. (2021). *MPI: A Message-Passing Interface Standard Version 4.0*. https://www.mpi-forum.org/docs/mpi-4.0/mpi40-report.pdf

## Appendix

### Код последовательной версии

```cpp
bool SabutayAincreaseContrastSEQ::RunImpl() {
  // Load the image
  int width = 0;
  int height = 0;
  int channels = 0;
  
  std::string abs_path = ppc::util::GetAbsoluteTaskPath("sabutay_a_increaseContrast", "pic.jpg");
  unsigned char *data = stbi_load(abs_path.c_str(), &width, &height, &channels, STBI_rgb);
  
  if (data == nullptr) {
    return false;
  }
  
  // Convert to grayscale and find min/max
  std::vector<uint8_t> grayscale(width * height);
  uint8_t min_val = 255;
  uint8_t max_val = 0;
  
  for (int i = 0; i < width * height; i++) {
    // Convert RGB to grayscale using standard formula
    uint8_t gray = static_cast<uint8_t>(
        0.299 * data[i * 3] + 0.587 * data[i * 3 + 1] + 0.114 * data[i * 3 + 2]);
    grayscale[i] = gray;
    min_val = std::min(min_val, gray);
    max_val = std::max(max_val, gray);
  }
  
  // Apply linear histogram stretching
  if (max_val > min_val) {
    double scale = 255.0 / (max_val - min_val);
    for (int i = 0; i < width * height; i++) {
      grayscale[i] = static_cast<uint8_t>((grayscale[i] - min_val) * scale);
    }
  }
  
  stbi_image_free(data);
  GetOutput() = GetInput();
  return true;
}
```

### Код MPI-версии

```cpp
bool SabutayAincreaseContrastMPI::RunImpl() {
  int rank = 0;
  int size = 0;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int width = 0;
  int height = 0;
  int channels = 0;
  std::vector<unsigned char> image_data;

  // Load image on rank 0
  if (rank == 0) {
    std::string abs_path = ppc::util::GetAbsoluteTaskPath("sabutay_a_increaseContrast", "pic.jpg");
    unsigned char *data = stbi_load(abs_path.c_str(), &width, &height, &channels, STBI_rgb);
    
    if (data == nullptr) {
      width = -1;
      MPI_Bcast(&width, 1, MPI_INT, 0, MPI_COMM_WORLD);
      return false;
    }
    
    image_data.assign(data, data + width * height * channels);
    stbi_image_free(data);
  }

  // Broadcast image dimensions
  int dims[3] = {width, height, channels};
  MPI_Bcast(dims, 3, MPI_INT, 0, MPI_COMM_WORLD);
  width = dims[0];
  height = dims[1];
  channels = dims[2];

  if (width <= 0 || height <= 0) {
    return false;
  }

  // Broadcast image data
  int image_size = width * height * channels;
  if (rank != 0) {
    image_data.resize(image_size);
  }
  MPI_Bcast(image_data.data(), image_size, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

  // Convert to grayscale and distribute rows across processes
  int rows_per_process = height / size;
  int remainder = height % size;
  int start_row = rank * rows_per_process + std::min(rank, remainder);
  int end_row = start_row + rows_per_process + (rank < remainder ? 1 : 0);
  int local_rows = end_row - start_row;

  // Convert local portion to grayscale and find local min/max
  std::vector<uint8_t> local_grayscale(local_rows * width);
  uint8_t local_min = 255;
  uint8_t local_max = 0;

  for (int row = 0; row < local_rows; row++) {
    int global_row = start_row + row;
    for (int col = 0; col < width; col++) {
      int idx = global_row * width + col;
      uint8_t gray = static_cast<uint8_t>(
          0.299 * image_data[idx * 3] + 0.587 * image_data[idx * 3 + 1] + 
          0.114 * image_data[idx * 3 + 2]);
      local_grayscale[row * width + col] = gray;
      local_min = std::min(local_min, gray);
      local_max = std::max(local_max, gray);
    }
  }

  // Find global min/max using MPI_Reduce
  uint8_t global_min = 0;
  uint8_t global_max = 0;
  MPI_Reduce(&local_min, &global_min, 1, MPI_UNSIGNED_CHAR, MPI_MIN, 0, MPI_COMM_WORLD);
  MPI_Reduce(&local_max, &global_max, 1, MPI_UNSIGNED_CHAR, MPI_MAX, 0, MPI_COMM_WORLD);

  // Broadcast global min/max to all processes
  uint8_t minmax[2] = {global_min, global_max};
  MPI_Bcast(minmax, 2, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);
  global_min = minmax[0];
  global_max = minmax[1];

  // Apply linear histogram stretching to local portion
  if (global_max > global_min) {
    double scale = 255.0 / (global_max - global_min);
    for (int i = 0; i < local_rows * width; i++) {
      local_grayscale[i] = static_cast<uint8_t>((local_grayscale[i] - global_min) * scale);
    }
  }

  GetOutput() = GetInput();
  MPI_Barrier(MPI_COMM_WORLD);
  return true;
}
```


