#pragma once

#include <stdexcept> // std::runtime_error — стандартное исключение времени выполнения
#include <string>    // std::string

namespace ifp {

/// Базовое исключение проекта ImageFilterPipeline.
///
/// Все специфичные исключения наследуют от ImageException,
/// что позволяет ловить любую ошибку проекта одним catch:
///   catch (const ImageException& e) { ... }
///
/// Наследование от std::runtime_error даёт метод what(),
/// возвращающий строку с описанием ошибки.
class ImageException : public std::runtime_error {
 public:
  explicit ImageException(const std::string& msg)
      : std::runtime_error(msg) {}
  // explicit — запрещает неявное преобразование строки в исключение.
  // Конструктор базового класса получает сообщение об ошибке.
};

