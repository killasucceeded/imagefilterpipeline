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

    int w = 0, h = 0, channels = 0;
    // w, h, channels — выходные параметры stbi_load.
    // stbi_load сам определит размеры изображения и запишет их сюда.

    // stbi_load возвращает указатель на malloc-буфер с пикселями.
    // Оборачиваем в unique_ptr с кастомным deleter'ом (stbi_image_free вместо delete),
    // чтобы буфер гарантированно освободился даже при исключении (RAII).
    auto deleter = [](unsigned char* p) { stbi_image_free(p); };
    std::unique_ptr<unsigned char, decltype(deleter)> raw(
        stbi_load(
            path.c_str(),      // путь к файлу
            &w, &h,            // stbi запишет сюда ширину и высоту
            &channels,         // stbi запишет сюда число каналов в файле
            Image::kChannels   // просим stbi принудительно вернуть 3 канала (RGB)
        ),
        deleter  // кастомный освободитель памяти
    );

    if (!raw) {
      // stbi_load вернул nullptr — файл есть, но прочитать не смог.
      throw ImageFormatException(
          std::string("stbi_load failed: ") + stbi_failure_reason() + " — " + path);
    }

    // Создаём наш объект Image и копируем пиксели из stbi-буфера.
    // После этого stbi-буфер нам больше не нужен — unique_ptr его освободит.
    Image img(static_cast<std::size_t>(w), static_cast<std::size_t>(h));
    std::memcpy(
        img.rawData(),   // куда копировать — буфер нашего Image
        raw.get(),       // откуда копировать — буфер stbi
        static_cast<std::size_t>(w) * h * Image::kChannels  // сколько байт
    );

    return img;  // возвращаем Image, обёрнутый в optional
  }


};

}  // namespace ifp