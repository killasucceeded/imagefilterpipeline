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

};

}  // namespace ifp