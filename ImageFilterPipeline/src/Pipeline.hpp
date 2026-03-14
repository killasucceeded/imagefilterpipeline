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

  /// Применяет все фильтры к изображению по порядку.
  ///
  /// log — поток для вывода прогресса (по умолчанию std::cout).
  ///       Можно передать std::ostringstream для тестов без вывода в консоль.
  void run(Image& img, std::ostream& log = std::cout) const {
    if (filters_.empty()) {
      log << "[Pipeline] No filters — image unchanged.\n";
      return;
    }

    log << "[Pipeline] Starting — " << filters_.size() << " filter(s)\n";

    // Range-based for loop + const auto& — перебираем фильтры без копирования.
    // filter имеет тип const unique_ptr<Filter>&
    for (const auto& filter : filters_) {
      log << "  -> Applying: " << filter->name() << " ... ";
      filter->apply(img);  // виртуальный вызов — выполняется код конкретного фильтра
      log << "done\n";
    }

    log << "[Pipeline] Complete.\n";
  }

  /// Количество фильтров в конвейере.
  [[nodiscard]] std::size_t size()  const noexcept { return filters_.size(); }

  /// Возвращает true, если фильтров нет.
  [[nodiscard]] bool        empty() const noexcept { return filters_.empty(); }

  /// Возвращает имя фильтра по индексу — используется в тестах.
  /// vector::at() бросает std::out_of_range при неверном индексе.
  [[nodiscard]] const std::string filterName(std::size_t i) const {
    return filters_.at(i)->name();
  }

 private:
  /// Вектор фильтров — порядок элементов определяет порядок применения.
  /// unique_ptr гарантирует, что каждый фильтр существует в единственном экземпляре
  /// и автоматически удаляется при уничтожении Pipeline (RAII).
  std::vector<std::unique_ptr<Filter>> filters_;
};

}  // namespace ifp