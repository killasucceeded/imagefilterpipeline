#pragma once

#include <cstddef>      // std::size_t — беззнаковый целый тип для размеров
#include <string>       // std::string
#include <vector>       // (не используется напрямую, но часто нужен в фильтрах)

#include "../Exceptions.hpp"
#include "../Filter.hpp"
#include "../Image.hpp"

namespace ifp {

/// Фильтр размытия — Box Blur (прямоугольное усреднение).
///
/// Алгоритм: для каждого пикселя берём квадратное окно соседних пикселей
/// («ядро» свёртки) и заменяем пиксель на среднее арифметическое всех пикселей в окне.
///
/// Размер окна задаётся радиусом:
///   radius=1 → окно 3×3 (каждый пиксель усредняется с 8 соседями)
///   radius=2 → окно 5×5
///   radius=N → окно (2N+1)×(2N+1)
///
/// На границах изображения окно обрезается (clamping) — не выходим за пределы.
class BlurFilter : public Filter {
 public:
  /// Конструктор с параметром радиуса.
  /// explicit — запрещает неявное преобразование числа в BlurFilter.
  /// Значение по умолчанию radius=1 — минимальное размытие (окно 3×3).
  explicit BlurFilter(std::size_t radius = 1) : radius_(radius) {
    if (radius == 0) {
      // Радиус 0 означает окно 1×1 — фильтр ничего не делает.
      // Считаем это ошибкой программиста.
      throw FilterException("Blur radius must be at least 1");
    }
  }

  /// Применяет размытие к изображению.
  void apply(Image& img) const override {
    const std::size_t w = img.width();
    const std::size_t h = img.height();

    // Создаём копию изображения — читаем из неё, пишем в оригинал.
    // ВАЖНО: без копии при обходе слева направо уже изменённые пиксели
    // попадали бы в окно следующих пикселей → неверный результат.
    Image copy = img;

    for (std::size_t y = 0; y < h; ++y) {
      for (std::size_t x = 0; x < w; ++x) {
        // Обрабатываем каждый канал независимо (R, G, B).
        for (std::size_t c = 0; c < Image::kChannels; ++c) {
          float sum  = 0.0f;
          int   count = 0;

          // Вычисляем границы окна с учётом краёв изображения (clamping).
          // Оператор ? : — тернарный оператор: (условие ? если_да : если_нет).
          // Защита от underflow: std::size_t беззнаковый, (0 - 1) было бы огромным числом.
          const auto yMin = static_cast<std::size_t>((y > radius_) ? y - radius_ : 0);
          const auto yMax = std::min(y + radius_, h - 1);
          const auto xMin = static_cast<std::size_t>((x > radius_) ? x - radius_ : 0);
          const auto xMax = std::min(x + radius_, w - 1);

          // Суммируем все пиксели в окне.
          for (std::size_t ky = yMin; ky <= yMax; ++ky) {
            for (std::size_t kx = xMin; kx <= xMax; ++kx) {
              sum += copy.at(kx, ky, c);  // читаем из копии, не из оригинала
              ++count;
            }
          }

          // Среднее арифметическое → записываем в оригинал.
          img.at(x, y, c) = clamp(sum / static_cast<float>(count));
        }
      }
    }
  }

  /// Имя фильтра включает радиус — удобно для логов.
  [[nodiscard]] std::string name() const override {
    return "Blur(r=" + std::to_string(radius_) + ")";
  }

 private:
  std::size_t radius_;  // радиус ядра размытия, задаётся при создании
};

}  // namespace ifp