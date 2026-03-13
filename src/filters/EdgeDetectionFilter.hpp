#pragma once

#include <algorithm>  // std::clamp — ограничение значения диапазоном
#include <array>      // std::array — массив фиксированного размера, известного на этапе компиляции
#include <cmath>      // std::sqrt — квадратный корень
#include <string>     // std::string

#include "../Filter.hpp"
#include "../Image.hpp"

namespace ifp {

/// Фильтр выделения границ — оператор Собеля (Sobel operator).
///
/// Алгоритм:
///   1. Для каждого пикселя вычисляем градиенты Gx и Gy — насколько сильно
///      меняется яркость по горизонтали и вертикали соответственно.
///   2. Величина градиента: G = sqrt(Gx² + Gy²)
///   3. Большой G → резкая граница (выделяем белым).
///      Малый G → однородная область (почти чёрная).
///
/// Градиенты вычисляются свёрткой с ядрами Собеля 3×3:
///
///   kSobelX (горизонтальные границы):   kSobelY (вертикальные границы):
///   -1  0  1                             -1 -2 -1
///   -2  0  2                              0  0  0
///   -1  0  1                              1  2  1
///
/// Результат — изображение в оттенках серого: белые линии на чёрном фоне.
class EdgeDetectionFilter : public Filter {
 public:
  void apply(Image& img) const override {
    const std::size_t w = img.width();
    const std::size_t h = img.height();

    // Копия нужна для чтения оригинала пока пишем в img.
    // Без копии соседние пиксели при вычислении градиента были бы уже изменены.
    Image copy = img;

    for (std::size_t y = 0; y < h; ++y) {
      for (std::size_t x = 0; x < w; ++x) {
        float gx = 0.0f;  // горизонтальный градиент
        float gy = 0.0f;  // вертикальный градиент

        // Обходим окрестность 3×3 вокруг пикселя (x, y).
        // dy и dx пробегают значения {-1, 0, 1} — смещения относительно центра.
        for (int dy = -1; dy <= 1; ++dy) {
          for (int dx = -1; dx <= 1; ++dx) {
            // Вычисляем координаты соседнего пикселя с clamping на границах.
            // std::clamp — ограничиваем диапазоном [0, w-1] и [0, h-1],
            // чтобы не выйти за пределы изображения.
            const auto nx = static_cast<std::size_t>(
                std::clamp(static_cast<int>(x) + dx, 0, static_cast<int>(w) - 1));
            const auto ny = static_cast<std::size_t>(
                std::clamp(static_cast<int>(y) + dy, 0, static_cast<int>(h) - 1));

            // Яркость соседнего пикселя — усредняем по BT.601.
            // Оператор Собеля работает с яркостью, а не с отдельными каналами.
            const float lum = luminance(copy, nx, ny);

            // Умножаем яркость на соответствующий коэффициент ядра Собеля.
            // dy+1 и dx+1 переводят {-1,0,1} в индексы {0,1,2} массива.
            gx += kSobelX[dy + 1][dx + 1] * lum;
            gy += kSobelY[dy + 1][dx + 1] * lum;
          }
        }

        // Величина градиента — длина вектора (gx, gy).
        // clamp() — ограничиваем результат диапазоном [0, 255].
        const uint8_t edge = clamp(std::sqrt(gx * gx + gy * gy));

        // Записываем одинаковое значение во все каналы → оттенок серого.
        img.at(x, y, 0) = edge;  // R
        img.at(x, y, 1) = edge;  // G
        img.at(x, y, 2) = edge;  // B
      }
    }
  }

  [[nodiscard]] std::string name() const override { return "EdgeDetection(Sobel)"; }
