#pragma once
// pragma once — директива препроцессора, которая гарантирует, что этот заголовочный файл
// будет включён в единицу трансляции только один раз, даже если #include встречается
// несколько раз. Альтернатива классическим include guards (#ifndef / #define / #endif).

#include <cstdint>   // uint8_t — целое беззнаковое 8-битное (0..255), точный размер
#include <cstring>   // std::memcpy, std::memset — быстрое копирование/обнуление памяти
#include <memory>    // std::unique_ptr — умный указатель с эксклюзивным владением
#include <stdexcept> // std::invalid_argument, std::out_of_range — стандартные исключения
#include <string>    // std::string

namespace ifp {
// Пространство имён ifp (ImageFilterPipeline) — изолирует наши имена классов
// от имён в других библиотеках. Без namespace был бы риск конфликта имён
// (например, если другая библиотека тоже определяет класс Image).

/// Класс Image — представляет RGB-изображение в памяти.
///
/// Данные пикселей хранятся в «строчном» (row-major) порядке:
///   пиксель (x, y) → индекс (y * width + x) * 3  в массиве data_
///   каналы: 0=R, 1=G, 2=B
///
/// Демонстрирует RAII: память выделяется в конструкторе через unique_ptr
/// и автоматически освобождается при уничтожении объекта.
/// Реализовано Правило Пяти (Rule of Five), так как класс управляет ресурсом.
class Image {
 public:
  // ── Конструкторы ──────────────────────────────────────────────────────────

  /// Конструктор по умолчанию — создаёт «пустое» изображение (empty() == true).
  /// Нужен, например, для std::optional<Image> без значения.
  Image() = default;

  /// Основной конструктор — создаёт изображение заданного размера, заполненное нулями.
  /// Бросает std::invalid_argument, если width или height равны нулю.
  Image(std::size_t width, std::size_t height)
      : width_(width),
        height_(height),
        // make_unique<uint8_t[]> выделяет массив байт нужного размера на куче.
        // unique_ptr гарантирует освобождение памяти при уничтожении Image (RAII).
        data_(std::make_unique<uint8_t[]>(width * height * kChannels)) {
    if (width == 0 || height == 0) {
      // Бросаем исключение — нулевой размер не имеет смысла.
      throw std::invalid_argument("Image dimensions must be positive");
    }
    // Обнуляем все байты пикселей (чёрное изображение).
    std::memset(data_.get(), 0, width * height * kChannels);
  }

  // ── Правило Пяти (Rule of Five) ───────────────────────────────────────────
  // Если класс управляет ресурсом вручную (здесь — через unique_ptr),
  // нужно явно определить все пять специальных методов:
  // копирующий конструктор, копирующий оператор =,
  // перемещающий конструктор, перемещающий оператор =, деструктор.

  /// Копирующий конструктор — создаёт НЕЗАВИСИМУЮ копию изображения.
  /// Без него копия и оригинал делили бы один и тот же буфер данных (shallow copy),
  /// что приводило бы к double-free и повреждению памяти.
  Image(const Image& other)
      : width_(other.width_),
        height_(other.height_),
        data_(std::make_unique<uint8_t[]>(other.width_ * other.height_ * kChannels)) {
    // memcpy — побайтовое копирование всего буфера пикселей.
    std::memcpy(data_.get(), other.data_.get(), width_ * height_ * kChannels);
  }

  /// Копирующий оператор присваивания — аналог копирующего конструктора,
  /// но для уже существующего объекта. Проверка self-assignment (this != &other)
  /// защищает от случая img = img.
  Image& operator=(const Image& other) {
    if (this != &other) {
      width_  = other.width_;
      height_ = other.height_;
      data_   = std::make_unique<uint8_t[]>(width_ * height_ * kChannels);
      std::memcpy(data_.get(), other.data_.get(), width_ * height_ * kChannels);
    }
    return *this;
  }

  /// Перемещающий конструктор — «крадёт» ресурс у временного объекта (rvalue).
  /// Вместо копирования буфера просто передаёт владение unique_ptr.
  /// После перемещения исходный объект становится пустым (empty() == true).
  /// noexcept — гарантия, что не бросает исключений (важно для STL-контейнеров).
  Image(Image&& other) noexcept
      : width_(other.width_),
        height_(other.height_),
        data_(std::move(other.data_)) {  // std::move передаёт владение указателем
    other.width_  = 0;
    other.height_ = 0;
    // other.data_ теперь nullptr — деструктор other ничего не освободит
  }

  /// Перемещающий оператор присваивания.
  Image& operator=(Image&& other) noexcept {
    if (this != &other) {
      width_  = other.width_;
      height_ = other.height_;
      data_   = std::move(other.data_);
      other.width_  = 0;
      other.height_ = 0;
    }
    return *this;
  }
