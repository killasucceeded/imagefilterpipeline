#pragma once

#include <string>       // std::string
#include "../Exceptions.hpp"
#include "../Filter.hpp"
#include "../Image.hpp"

namespace ifp {

/// Фильтр регулировки яркости (Brightness).
///
/// Алгоритм: умножаем значение каждого канала на коэффициент factor.
///   factor > 1.0 — осветление (например, 2.0 удвоит яркость)
///   factor = 1.0 — изображение не изменяется
///   factor < 1.0 — затемнение (например, 0.5 вдвое уменьшит яркость)
///   factor = 0.0 — полностью чёрное изображение
///
/// Результат ограничивается диапазоном [0, 255] через clamp(),
/// поэтому пересвет (overexposure) не приводит к «обёртыванию» значений.
class BrightnessFilter : public Filter {
 public:
  /// Конструктор с коэффициентом яркости.
  /// explicit — запрещает неявное преобразование float в BrightnessFilter.
  /// Бросает FilterException при отрицательном factor (физического смысла нет).
  explicit BrightnessFilter(float factor) : factor_(factor) {
    if (factor < 0.0f) {
      throw FilterException(
          "Brightness factor must be non-negative, got: " + std::to_string(factor));
    }
  }

