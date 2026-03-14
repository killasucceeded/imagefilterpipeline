// Стандартные заголовки C++
#include <filesystem>  // std::filesystem::path — работа с путями к файлам
#include <iostream>    // std::cout, std::cin — ввод/вывод в консоль
#include <limits>      // std::numeric_limits
#include <memory>      // std::make_unique — создание объектов с умными указателями
#include <optional>    // std::optional — значение, которое может отсутствовать
#include <sstream>     // std::ostringstream — строковый поток (для форматирования)
#include <string>      // std::string
#include <vector>      // std::vector — динамический массив
#include <clocale>     // std::setlocale — настройка локали (для кириллицы)

// Заголовки нашего проекта
#include "Exceptions.hpp"               // ImageException и производные
#include "Image.hpp"                    // класс Image — хранит пиксели
#include "ImageLoader.hpp"              // загрузка/сохранение файлов
#include "Pipeline.hpp"                 // конвейер фильтров
#include "filters/BlurFilter.hpp"       // фильтр размытия
#include "filters/BrightnessFilter.hpp" // фильтр яркости
#include "filters/EdgeDetectionFilter.hpp" // выделение границ
#include "filters/GrayscaleFilter.hpp"  // перевод в оттенки серого

// Весь UI-код находится в пространстве имён ifp::ui.
// Это предотвращает конфликты имён с другими частями программы.
namespace ifp::ui {

// ── Вспомогательные функции ───────────────────────────────────────────────────

/// Очищает экран терминала.
/// Использует системную команду — платформозависимо:
///   Windows → cls, Unix/Linux/Mac → clear
void clearScreen() {
#ifdef _WIN32
  std::system("cls");   // препроцессор выбирает ветку для Windows
#else
  std::system("clear"); // для остальных ОС
#endif
}

/// Ждёт нажатия Enter — даёт пользователю время прочитать сообщение.
void pause() {
  std::cout << "\nНажмите Enter для продолжения...";
  std::string dummy;
  std::getline(std::cin, dummy);  // читаем строку и игнорируем содержимое
}

/// Выводит сообщение и читает строку от пользователя.
/// Возвращает введённую строку (может быть пустой если нажать Enter).
std::string prompt(const std::string& message) {
  std::cout << message;
  std::string line;
  std::getline(std::cin, line);  // getline читает всю строку включая пробелы
  return line;
}

/// Читает целое число в диапазоне [minVal, maxVal].
/// Повторяет запрос пока пользователь не введёт корректное значение.
int promptInt(const std::string& message, int minVal, int maxVal) {
  while (true) {
    std::cout << message;
    std::string line;
    std::getline(std::cin, line);
    try {
      int v = std::stoi(line);  // stoi — string to int, бросает при неверном вводе
      if (v >= minVal && v <= maxVal) return v;
    } catch (...) {
      // catch (...) — ловим любое исключение (std::invalid_argument от stoi и т.д.)
      // Просто игнорируем и просим ввести снова.
    }
    std::cout << "  [!] Введите число от " << minVal << " до " << maxVal << "\n";
  }
}

/// Читает число с плавающей точкой в диапазоне [minVal, maxVal].
float promptFloat(const std::string& message, float minVal, float maxVal) {
  while (true) {
    std::cout << message;
    std::string line;
    std::getline(std::cin, line);
    try {
      float v = std::stof(line);  // stof — string to float
      if (v >= minVal && v <= maxVal) return v;
    } catch (...) {}
    std::cout << "  [!] Введите число от " << minVal << " до " << maxVal << "\n";
  }
}

// ── Шапка программы ───────────────────────────────────────────────────────────

/// Выводит декоративную шапку программы.
void printBanner() {
  std::cout <<
    "╔══════════════════════════════════════════════╗\n"
    "║      ImageFilterPipeline  —  C++23           ║\n"
    "╚══════════════════════════════════════════════╝\n\n";
  // Несколько строковых литералов подряд — компилятор склеивает их в одну строку.
}

// ── Состояние приложения ──────────────────────────────────────────────────────

/// Структура AppState хранит всё текущее состояние приложения.
/// Передаётся по ссылке во все функции меню — единое место хранения данных.
struct AppState {
  std::optional<Image>     image;      // загруженное изображение (может отсутствовать)
  std::string              imagePath;  // путь к загруженному файлу (для отображения)
  Pipeline                 pipeline;   // конвейер с добавленными фильтрами
  std::vector<std::string> filterLog;  // список имён фильтров для отображения в меню

  /// Проверяет, загружено ли изображение.
  /// has_value() — метод std::optional, возвращает true если значение есть.
  bool hasImage() const noexcept { return image.has_value(); }
};

// ── Функции подменю ───────────────────────────────────────────────────────────

/// Меню загрузки изображения.
/// state передаётся по ссылке — функция модифицирует состояние приложения.
void menuLoadImage(AppState& state) {
  std::cout << "── Загрузка изображения ─────────────────────\n";
  std::string path = prompt("Путь к файлу (PPM/PNG/JPEG): ");

  if (path.empty()) {
    std::cout << "  Отменено.\n";
    pause();
    return;
  }

  try {
    // ImageLoader::load возвращает std::optional<Image>:
    // nullopt — файл не найден, Image — успех, исключение — файл повреждён.
    auto loaded = ImageLoader::load(path);

    if (!loaded) {
      // Оператор ! для optional — true если значения нет (nullopt).
      std::cout << "  [!] Файл не найден: " << path << "\n";
    } else {
      // *loaded — разыменование optional, получаем Image.
      // std::move — передаём владение данными, избегая копирования буфера пикселей.
      state.image     = std::move(*loaded);
      state.imagePath = path;
      state.pipeline  = Pipeline{};   // сбрасываем старый конвейер
      state.filterLog.clear();        // очищаем список фильтров
      std::cout << "  ✓ Загружено: " << state.image->width()
                << " × " << state.image->height() << " пикселей\n";
    }
  } catch (const ImageException& e) {
    // Ловим только исключения нашего проекта — файл есть, но не читается.
    std::cout << "  [!] Ошибка: " << e.what() << "\n";
  }
  pause();
}

/// Меню добавления фильтра в конвейер.
void menuAddFilter(AppState& state) {
  if (!state.hasImage()) {
    std::cout << "  [!] Сначала загрузите изображение.\n";
    pause();
    return;
  }

  std::cout << "── Добавить фильтр ──────────────────────────\n"
               "  1. Grayscale     — оттенки серого\n"
               "  2. Blur          — размытие (box blur)\n"
               "  3. Brightness    — яркость\n"
               "  4. EdgeDetection — выделение границ (Собель)\n"
               "  0. Назад\n";

  int choice = promptInt("Выбор: ", 0, 4);

  switch (choice) {
    case 1:
      // std::make_unique<T>(...) — создаёт объект T на куче и возвращает unique_ptr.
      // addFilter принимает unique_ptr по значению — Pipeline становится владельцем.
      state.pipeline.addFilter(std::make_unique<GrayscaleFilter>());
      state.filterLog.push_back("Grayscale");
      std::cout << "  ✓ Добавлен: Grayscale\n";
      break;

    case 2: {
      // Фигурные скобки создают новую область видимости — переменная r не видна
      // за пределами этого case (иначе компилятор мог бы жаловаться на jump).
      int r = promptInt("  Радиус (1–10): ", 1, 10);
      state.pipeline.addFilter(std::make_unique<BlurFilter>(r));
      state.filterLog.push_back("Blur(r=" + std::to_string(r) + ")");
      std::cout << "  ✓ Добавлен: Blur(r=" << r << ")\n";
      break;
    }

    case 3: {
      float f = promptFloat("  Фактор (0.1–5.0): ", 0.1f, 5.0f);
      state.pipeline.addFilter(std::make_unique<BrightnessFilter>(f));
      state.filterLog.push_back("Brightness(x" + std::to_string(f) + ")");
      std::cout << "  ✓ Добавлен: Brightness(x" << f << ")\n";
      break;
    }

    case 4:
      state.pipeline.addFilter(std::make_unique<EdgeDetectionFilter>());
      state.filterLog.push_back("EdgeDetection(Sobel)");
      std::cout << "  ✓ Добавлен: EdgeDetection(Sobel)\n";
      break;

    default:
      break;  // case 0: просто выходим из switch
  }

  if (choice != 0) pause();
}

/// Меню запуска конвейера — применяет все добавленные фильтры к изображению.
void menuRunPipeline(AppState& state) {
  if (!state.hasImage()) {
    std::cout << "  [!] Сначала загрузите изображение.\n";
    pause();
    return;
  }
  if (state.pipeline.empty()) {
    std::cout << "  [!] Нет фильтров — сначала добавьте хотя бы один.\n";
    pause();
    return;
  }

  std::cout << "── Запуск конвейера ──────────────────────────\n";
  // *state.image — разыменование optional<Image>, получаем Image&
  // Pipeline::run изменяет изображение и логирует прогресс в std::cout
  state.pipeline.run(*state.image);
  std::cout << "  ✓ Все фильтры применены.\n";
  pause();
}

/// Меню сохранения изображения в файл.
void menuSaveImage(AppState& state) {
  if (!state.hasImage()) {
    std::cout << "  [!] Нет изображения для сохранения.\n";
    pause();
    return;
  }

  std::cout << "── Сохранение изображения ───────────────────\n";

  // std::filesystem::path — объект для работы с путями.
  // stem() — имя файла без расширения, extension() — расширение с точкой.
  // Пример: "photo.png" → stem="photo", extension=".png" → defName="photo_out.png"
  const std::filesystem::path src(state.imagePath);
  const std::string defName = src.stem().string() + "_out" + src.extension().string();

  std::cout << "  Имя по умолчанию: " << defName << "\n";
  std::string path = prompt("Путь для сохранения (Enter = по умолчанию): ");
  if (path.empty()) path = defName;  // если Enter — используем имя по умолчанию

  try {
    ImageLoader::save(*state.image, path);
    std::cout << "  ✓ Сохранено: " << path << "\n";
  } catch (const ImageException& e) {
    std::cout << "  [!] Ошибка: " << e.what() << "\n";
  }
  pause();
}

/// Меню просмотра текущего состояния приложения.
/// Принимает const AppState& — не изменяет состояние, только читает.
void menuStatus(const AppState& state) {
  std::cout << "── Текущее состояние ────────────────────────\n";

  if (state.hasImage()) {
    std::cout << "  Изображение : " << state.imagePath
              << " (" << state.image->width() << "×"
              << state.image->height() << ")\n";
  } else {
    std::cout << "  Изображение : не загружено\n";
  }

  if (state.filterLog.empty()) {
    std::cout << "  Фильтры     : (нет)\n";
  } else {
    std::cout << "  Фильтры     :\n";
    // std::size_t i — беззнаковый тип для индекса (как size() возвращает)
    for (std::size_t i = 0; i < state.filterLog.size(); ++i) {
      std::cout << "    " << (i + 1) << ". " << state.filterLog[i] << "\n";
    }
  }
  pause();
}

// ── Главное меню ──────────────────────────────────────────────────────────────

/// Выводит главное меню с текущим статусом изображения.
void printMainMenu(const AppState& state) {
  printBanner();

  // Строка статуса — показывает что загружено и сколько фильтров добавлено.
  if (state.hasImage()) {
    std::cout << "  Изображение : " << state.imagePath
              << " (" << state.image->width() << "×"
              << state.image->height() << ")";
    if (!state.filterLog.empty()) {
      std::cout << "  |  Фильтров: " << state.filterLog.size();
    }
    std::cout << "\n\n";
  } else {
    std::cout << "  Изображение не загружено\n\n";
  }

  std::cout <<
    "  1. Загрузить изображение\n"
    "  2. Добавить фильтр\n"
    "  3. Запустить конвейер (применить все фильтры)\n"
    "  4. Сохранить результат\n"
    "  5. Состояние / список фильтров\n"
    "  0. Выход\n\n";
}

/// Главный цикл приложения.
void run() {
  AppState state;  // создаём начальное (пустое) состояние

  while (true) {  // бесконечный цикл — выход через case 0 (return)
    clearScreen();
    printMainMenu(state);

    int choice = promptInt("Выбор: ", 0, 5);
    clearScreen();

    // switch/case — диспетчеризация по выбору пользователя.
    switch (choice) {
      case 1: menuLoadImage(state);   break;
      case 2: menuAddFilter(state);   break;
      case 3: menuRunPipeline(state); break;
      case 4: menuSaveImage(state);   break;
      case 5: menuStatus(state);      break;
      case 0:
        std::cout << "До свидания!\n";
        return;  // выходим из run() → программа завершается
    }
  }
}

}  // namespace ifp::ui

// ── Точка входа ───────────────────────────────────────────────────────────────

/// Функция main — точка входа программы. С неё начинается выполнение.
/// Возвращает 0 при успехе, ненулевое значение при ошибке (конвенция POSIX).
int main() {
  // Настройка кодировки для корректного отображения кириллицы в Windows.
  std::setlocale(LC_ALL, "ru-RU.UTF-8");
  system("chcp 65001 > nul");  // переключаем консоль Windows на UTF-8

  try {
    // Запускаем основной цикл приложения.
    ifp::ui::run();
  } catch (const std::exception& e) {
    // Ловим любое необработанное исключение — последний рубеж защиты.
    // std::exception — базовый класс всех стандартных исключений C++.
    std::cerr << "[FATAL] " << e.what() << "\n";
    return 1;  // ненулевой код возврата сигнализирует об ошибке
  }

  return 0;  // успешное завершение
}