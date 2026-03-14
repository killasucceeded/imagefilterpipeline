#include <gtest/gtest.h>

#include <memory>
#include <optional>
#include <stdexcept>
#include <string>

#include "../src/Exceptions.hpp"
#include "../src/Filter.hpp"
#include "../src/Image.hpp"
#include "../src/ImageLoader.hpp"
#include "../src/Pipeline.hpp"
#include "../src/filters/BlurFilter.hpp"
#include "../src/filters/BrightnessFilter.hpp"
#include "../src/filters/EdgeDetectionFilter.hpp"
#include "../src/filters/GrayscaleFilter.hpp"

using namespace ifp;

// ── Helpers ───────────────────────────────────────────────────────────────────


Image makeColorImage(uint8_t r, uint8_t g, uint8_t b, std::size_t w = 4, std::size_t h = 4) {
  Image img(w, h);
  for (std::size_t y = 0; y < h; ++y) {
    for (std::size_t x = 0; x < w; ++x) {
      img.at(x, y, 0) = r;
      img.at(x, y, 1) = g;
      img.at(x, y, 2) = b;
    }
  }
  return img;
}

// ── Image tests ───────────────────────────────────────────────────────────────

TEST(ImageTest, ConstructAndAccess) {
  Image img(10, 5);
  EXPECT_EQ(img.width(), 10u);
  EXPECT_EQ(img.height(), 5u);
  EXPECT_FALSE(img.empty());
}

TEST(ImageTest, DefaultConstructedIsEmpty) {
  Image img;
  EXPECT_TRUE(img.empty());
}

TEST(ImageTest, ZeroDimensionsThrows) {
  EXPECT_THROW(Image(0, 5), std::invalid_argument);
  EXPECT_THROW(Image(5, 0), std::invalid_argument);
}

TEST(ImageTest, PixelReadWrite) {
  Image img(3, 3);
  img.at(1, 1, 0) = 128;
  img.at(1, 1, 1) = 64;
  img.at(1, 1, 2) = 32;
  EXPECT_EQ(img.at(1, 1, 0), 128);
  EXPECT_EQ(img.at(1, 1, 1), 64);
  EXPECT_EQ(img.at(1, 1, 2), 32);
}

TEST(ImageTest, OutOfBoundsThrows) {
  Image img(3, 3);
  EXPECT_THROW(img.at(3, 0, 0), std::out_of_range);
  EXPECT_THROW(img.at(0, 3, 0), std::out_of_range);
  EXPECT_THROW(img.at(0, 0, 3), std::out_of_range);
}

TEST(ImageTest, CopyConstructorDeepCopy) {
  Image original = makeColorImage(100, 150, 200);
  Image copy = original;

  // Mutate copy — original must be unchanged
  copy.at(0, 0, 0) = 0;
  EXPECT_EQ(original.at(0, 0, 0), 100);
}

TEST(ImageTest, MoveConstructor) {
  Image original = makeColorImage(10, 20, 30);
  Image moved = std::move(original);

  EXPECT_TRUE(original.empty());
  EXPECT_FALSE(moved.empty());
  EXPECT_EQ(moved.at(0, 0, 0), 10);
}

