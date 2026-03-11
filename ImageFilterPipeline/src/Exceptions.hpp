#pragma once

#include <stdexcept> // std::runtime_error — стандартное исключение времени выполнения
#include <string>    // std::string

namespace ifp {

/// Базовое исключение проекта ImageFilterPipeline.
///
/// Все специфичные исключения наследуют от ImageException,
/// что позволяет ловить любую ошибку проекта одним catch:
///   catch (const ImageException& e) { ... }
///
/// Наследование от std::runtime_error даёт метод what(),
/// возвращающий строку с описанием ошибки.
class ImageException : public std::runtime_error {
 public:
  explicit ImageException(const std::string& msg)
      : std::runtime_error(msg) {}
  // explicit — запрещает неявное преобразование строки в исключение.
  // Конструктор базового класса получает сообщение об ошибке.
};

    /// Исключение ввода-вывода — файл не найден или недоступен для записи.
    ///
    /// Хранит путь к файлу дополнительно, чтобы вызывающий код мог его получить
    /// через метод path() без разбора строки сообщения.
    class FileIOException : public ImageException {
    public:
        explicit FileIOException(const std::string& path)
            : ImageException("Cannot read/write file: " + path), path_(path) {}

        /// Возвращает путь к файлу, вызвавшему исключение.
        [[nodiscard]] const std::string& path() const noexcept { return path_; }

    private:
        std::string path_;  // сохраняем путь отдельно для удобного доступа
    };

    /// Исключение формата изображения — файл повреждён или формат не поддерживается.
    /// Например: попытка загрузить бинарный файл как PNG.
    class ImageFormatException : public ImageException {
    public:
        explicit ImageFormatException(const std::string& msg)
            : ImageException("Image format error: " + msg) {}
    };

    /// Исключение фильтра — неверные параметры при создании фильтра.
    /// Например: BlurFilter(0) — нулевой радиус не имеет смысла.
    class FilterException : public ImageException {
    public:
        explicit FilterException(const std::string& msg)
            : ImageException("Filter error: " + msg) {}
    };

}  // namespace ifp