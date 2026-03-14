#pragma once

#include <iostream>  // std::ostream — для вывода лога выполнения
#include <memory>    // std::unique_ptr — умный указатель с эксклюзивным владением
#include <string>    // std::string
#include <vector>    // std::vector — динамический массив фильтров

#include "Filter.hpp"
#include "Image.hpp"

namespace ifp {

/// Класс Pipeline — «конвейер» обработки изображения.
///
/// Хранит список фильтров и последовательно применяет их к изображению.
/// Паттерн проектирования: «Цепочка обязанностей» (Chain of Responsibility) —
/// каждый фильтр обрабатывает изображение и передаёт результат следующему.
///
/// Фильтры хранятся как vector<unique_ptr<Filter>>:
/// - vector  — динамический массив, порядок применения соответствует порядку добавления
/// - unique_ptr — Pipeline является единственным владельцем каждого фильтра
/// - Filter* (полиморфизм) — вызываются виртуальные методы конкретных фильтров
class Pipeline {
 public:
  Pipeline() = default;

  // Pipeline нельзя копировать — unique_ptr не копируются.
  // Если бы это было нужно, пришлось бы реализовать глубокое копирование фильтров.
  Pipeline(const Pipeline&)            = delete;
  Pipeline& operator=(const Pipeline&) = delete;

  // Pipeline можно перемещать — передача владения вектором фильтров.
  // = default: компилятор генерирует перемещение автоматически.
  Pipeline(Pipeline&&)            = default;
  Pipeline& operator=(Pipeline&&) = default;

  /// Добавляет фильтр в конец конвейера.
  ///
  /// Принимает unique_ptr по значению — вызывающий код передаёт владение.
  /// Типичный вызов: pipeline.addFilter(std::make_unique<GrayscaleFilter>());
  /// После вызова переменная вызывающего кода становится nullptr.
  void addFilter(std::unique_ptr<Filter> filter) {
    if (!filter) {
      // Защита от добавления nullptr — явная ошибка программиста.
      throw std::invalid_argument("Cannot add null filter to pipeline");
    }
    // std::move необходим, так как unique_ptr нельзя скопировать.
    // После push_back локальная переменная filter становится nullptr.
    filters_.push_back(std::move(filter));
  }



 private:
  /// Вектор фильтров — порядок элементов определяет порядок применения.
  /// unique_ptr гарантирует, что каждый фильтр существует в единственном экземпляре
  /// и автоматически удаляется при уничтожении Pipeline (RAII).
  std::vector<std::unique_ptr<Filter>> filters_;
};

}  // namespace ifp