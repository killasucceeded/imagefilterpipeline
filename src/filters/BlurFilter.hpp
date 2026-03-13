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

  /// Применяет изменение яркости ко всем пикселям изображения.
  void apply(Image& img) const override {
    // Тройной вложенный цикл: по строкам, столбцам и каналам.
    // Все три канала обрабатываются одинаково — умножение на factor_.
    for (std::size_t y = 0; y < img.height(); ++y) {
      for (std::size_t x = 0; x < img.width(); ++x) {
        for (std::size_t c = 0; c < Image::kChannels; ++c) {
          // img.at() возвращает ссылку — записываем результат напрямую.
          // clamp() из базового Filter — защищает от выхода за [0, 255].
          img.at(x, y, c) = clamp(img.at(x, y, c) * factor_);
        }
      }
    }
  }

  /// Имя включает коэффициент — удобно видеть в логах Pipeline.
  [[nodiscard]] std::string name() const override {
    return "Brightness(x" + std::to_string(factor_) + ")";
  }

 private:
  float factor_;  // множитель яркости, задаётся при создании фильтра
};

}  // namespace ifp