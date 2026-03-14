#pragma once

#include <cstring>     // std::memcpy — копирование блока памяти
#include <filesystem>  // std::filesystem::exists — проверка существования файла
#include <memory>      // std::unique_ptr — RAII для буфера stbi
#include <optional>    // std::optional — «возможно отсутствующее» значение
#include <string>      // std::string

// stb_image и stb_image_write — header-only библиотеки для загрузки/сохранения изображений.
// Они не требуют компиляции отдельной библиотеки — весь код находится в .h файлах.
// ВАЖНО: #define STB_IMAGE_IMPLEMENTATION должен быть ровно в ОДНОМ .cpp файле
// (у нас — stb_impl.cpp). Здесь только объявления функций без реализации.
#include "stb_image.h"
#include "stb_image_write.h"

#include "Exceptions.hpp"
#include "Image.hpp"

namespace ifp {

/// Статический класс для загрузки и сохранения изображений.
///
/// Все методы статические — не нужно создавать объект ImageLoader,
/// просто вызываем ImageLoader::load(...) и ImageLoader::save(...).
///
/// Поддерживаемые форматы: PNG, JPEG, BMP, PPM (через stb_image).
/// Сохранение всегда в формате PNG (через stb_image_write).
class ImageLoader {
 public:
  /// Загружает изображение из файла.
  ///
  /// Возвращает std::optional<Image> — тип, который либо содержит значение,
  /// либо «пуст» (std::nullopt). Это элегантнее, чем возвращать nullptr или
  /// специальное значение для обозначения «файл не найден».
  ///
  /// Поведение:
  ///   - файл не существует → возвращает std::nullopt (не исключение!)
  ///   - файл повреждён     → бросает ImageFormatException
  ///   - успех              → возвращает Image с пикселями
  ///
  /// [[nodiscard]] — компилятор предупредит, если результат не используется.
  [[nodiscard]] static std::optional<Image> load(const std::string& path) {
    // std::filesystem::exists — проверяет наличие файла без попытки открыть его.
    // Возвращаем nullopt вместо исключения — «файл не найден» это ожидаемая ситуация,
    // а не ошибка программы.
    if (!std::filesystem::exists(path)) {
      return std::nullopt;
    }


};

}  // namespace ifp