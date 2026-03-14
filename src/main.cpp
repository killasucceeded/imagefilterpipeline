// Стандартные заголовки C++
#include <iostream>    // std::cout, std::cin — ввод/вывод в консоль
#include <string>      // std::string
#include <vector>      // std::vector — динамический массив

// Заголовки нашего проекта
#include "ImageLoader.hpp"              // загрузка/сохранение файлов

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

}  // namespace  ifp::ui