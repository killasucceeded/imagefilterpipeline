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



}  // namespace ifp