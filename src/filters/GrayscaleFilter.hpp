#pragma once

#include <string>       // std::string
#include "../Filter.hpp"
#include "../Image.hpp"

namespace ifp {

/// Фильтр перевода в оттенки серого (Grayscale).
///
/// Алгоритм: взвешенное среднее каналов R, G, B по стандарту ITU-R BT.601.
/// Коэффициенты учитывают восприятие яркости человеческим глазом:
///   - зелёный воспринимается ярче всего (0.587)
///   - красный — средне (0.299)
///   - синий — темнее всего (0.114)
///
/// Итого: gray = 0.299*R + 0.587*G + 0.114*B
/// Полученное значение записывается во все три канала → оттенок серого.
class GrayscaleFilter : public Filter {
  // public Filter — GrayscaleFilter является фильтром (наследование).
  // Благодаря этому объект GrayscaleFilter можно использовать везде,
  // где ожидается Filter* или Filter& (полиморфизм).

 public:
  /// Применяет фильтр — переводит каждый пиксель изображения в серый.
  /// override — явно указывает, что метод переопределяет виртуальный метод базового класса.
  void apply(Image& img) const override {
    // Перебираем все пиксели изображения по строкам (y) и столбцам (x).
    for (std::size_t y = 0; y < img.height(); ++y) {
      for (std::size_t x = 0; x < img.width(); ++x) {

        // Вычисляем яркость пикселя по формуле BT.601.
        // getLuminanceWeight вызывается с символом канала — компилятор вычислит
        // константы на этапе компиляции благодаря constexpr (нет затрат в рантайме).
        const float gray =
            getLuminanceWeight('R') * img.at(x, y, 0) +  // красный канал
            getLuminanceWeight('G') * img.at(x, y, 1) +  // зелёный канал
            getLuminanceWeight('B') * img.at(x, y, 2);   // синий канал

        // clamp() из базового класса Filter — ограничивает float до [0..255] → uint8_t.
        const uint8_t g = clamp(gray);

        // Записываем одинаковое значение во все три канала → серый цвет.
        img.at(x, y, 0) = g;  // R
        img.at(x, y, 1) = g;  // G
        img.at(x, y, 2) = g;  // B
      }
    }
  }

  /// Возвращает имя фильтра для отображения в меню и логах.
  [[nodiscard]] std::string name() const override { return "Grayscale"; }

private:
  /// Возвращает коэффициент яркости для заданного канала (BT.601).
  ///
  /// constexpr — вычисляется на этапе компиляции: если channel известен
  /// заранее (как 'R', 'G', 'B'), компилятор заменит вызов на число прямо в коде.
  /// static — метод принадлежит классу, не конкретному объекту.
  /// noexcept — не бросает исключений.
  [[nodiscard]] static constexpr float getLuminanceWeight(char channel) noexcept {
    switch (channel) {
      case 'R': return 0.299f;  // красный — 29.9% воспринимаемой яркости
      case 'G': return 0.587f;  // зелёный — 58.7%
      case 'B': return 0.114f;  // синий   — 11.4%
      default:  return 0.0f;    // неизвестный канал — не вносит вклад
    }
  }
};

}  // namespace ifp