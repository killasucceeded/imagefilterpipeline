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

// ── GrayscaleFilter tests ─────────────────────────────────────────────────────

TEST(GrayscaleFilterTest, PureRedBecomesGray) {
  Image img = makeColorImage(255, 0, 0);
  GrayscaleFilter f;
  f.apply(img);

  // All channels should be equal after grayscale
  EXPECT_EQ(img.at(0, 0, 0), img.at(0, 0, 1));
  EXPECT_EQ(img.at(0, 0, 1), img.at(0, 0, 2));
  // Luminance of pure red ≈ 0.299 × 255 ≈ 76
  EXPECT_NEAR(img.at(0, 0, 0), 76, 1);
}

TEST(GrayscaleFilterTest, WhiteStaysWhite) {
  Image img = makeColorImage(255, 255, 255);
  GrayscaleFilter f;
  f.apply(img);
  EXPECT_EQ(img.at(0, 0, 0), 255);
}

TEST(GrayscaleFilterTest, BlackStaysBlack) {
  Image img = makeColorImage(0, 0, 0);
  GrayscaleFilter f;
  f.apply(img);
  EXPECT_EQ(img.at(0, 0, 0), 0);
}

TEST(GrayscaleFilterTest, Name) {
  EXPECT_EQ(GrayscaleFilter{}.name(), "Grayscale");
}

// ── BrightnessFilter tests ────────────────────────────────────────────────────

TEST(BrightnessFilterTest, DoublingBrightness) {
  Image img = makeColorImage(100, 100, 100);
  BrightnessFilter f(2.0f);
  f.apply(img);
  EXPECT_EQ(img.at(0, 0, 0), 200);
}

TEST(BrightnessFilterTest, ClampsAt255) {
  Image img = makeColorImage(200, 200, 200);
  BrightnessFilter f(2.0f);
  f.apply(img);
  EXPECT_EQ(img.at(0, 0, 0), 255);
}

TEST(BrightnessFilterTest, ZeroFactorBlackens) {
  Image img = makeColorImage(200, 150, 100);
  BrightnessFilter f(0.0f);
  f.apply(img);
  EXPECT_EQ(img.at(0, 0, 0), 0);
  EXPECT_EQ(img.at(0, 0, 1), 0);
  EXPECT_EQ(img.at(0, 0, 2), 0);
}

TEST(BrightnessFilterTest, NegativeFactorThrows) {
  EXPECT_THROW(BrightnessFilter(-1.0f), FilterException);
}

TEST(BrightnessFilterTest, IdentityFactor) {
  Image img = makeColorImage(123, 45, 67);
  BrightnessFilter f(1.0f);
  f.apply(img);
  EXPECT_EQ(img.at(0, 0, 0), 123);
  EXPECT_EQ(img.at(0, 0, 1), 45);
  EXPECT_EQ(img.at(0, 0, 2), 67);
}

// ── BlurFilter tests ──────────────────────────────────────────────────────────

TEST(BlurFilterTest, ZeroRadiusThrows) {
  EXPECT_THROW(BlurFilter(0), FilterException);
}

TEST(BlurFilterTest, UniformImageUnchanged) {
  Image img = makeColorImage(128, 128, 128);
  BlurFilter f(1);
  f.apply(img);
  // Uniform image stays uniform after blur
  EXPECT_EQ(img.at(1, 1, 0), 128);
}

TEST(BlurFilterTest, CenterPixelAffected) {
  // Single bright pixel in center — after blur should be dimmer
  Image img(5, 5);
  img.at(2, 2, 0) = 255;  // one bright red pixel
  BlurFilter f(1);
  f.apply(img);
  // Center should now be less than 255 (averaged with neighbors)
  EXPECT_LT(img.at(2, 2, 0), 255);
}

TEST(BlurFilterTest, Name) {
  EXPECT_EQ(BlurFilter(2).name(), "Blur(r=2)");
}

// ── EdgeDetectionFilter tests ─────────────────────────────────────────────────

TEST(EdgeDetectionFilterTest, UniformImageProducesNoEdges) {
  Image img = makeColorImage(128, 128, 128, 8, 8);
  EdgeDetectionFilter f;
  f.apply(img);
  // Interior pixels of a uniform image should have near-zero gradient
  EXPECT_NEAR(img.at(4, 4, 0), 0, 5);
}

TEST(EdgeDetectionFilterTest, Name) {
  EXPECT_EQ(EdgeDetectionFilter{}.name(), "EdgeDetection(Sobel)");
}

// ── Pipeline tests ────────────────────────────────────────────────────────────

TEST(PipelineTest, EmptyPipelineDoesNothing) {
  Image img = makeColorImage(100, 100, 100);
  Pipeline p;
  p.run(img);
  EXPECT_EQ(img.at(0, 0, 0), 100);
}

TEST(PipelineTest, AddFilterIncreasesSize) {
  Pipeline p;
  EXPECT_EQ(p.size(), 0u);
  p.addFilter(std::make_unique<GrayscaleFilter>());
  EXPECT_EQ(p.size(), 1u);
  p.addFilter(std::make_unique<BrightnessFilter>(1.5f));
  EXPECT_EQ(p.size(), 2u);
}

TEST(PipelineTest, FiltersAppliedInOrder) {
  // Grayscale then brightness 0 → all black
  Image img = makeColorImage(200, 100, 50);
  Pipeline p;
  p.addFilter(std::make_unique<GrayscaleFilter>());
  p.addFilter(std::make_unique<BrightnessFilter>(0.0f));
  p.run(img);
  EXPECT_EQ(img.at(0, 0, 0), 0);
}

TEST(PipelineTest, NullFilterThrows) {
  Pipeline p;
  EXPECT_THROW(p.addFilter(nullptr), std::invalid_argument);
}

TEST(PipelineTest, FilterNameAccess) {
  Pipeline p;
  p.addFilter(std::make_unique<GrayscaleFilter>());
  EXPECT_EQ(p.filterName(0), "Grayscale");
}

// ── ImageLoader tests ─────────────────────────────────────────────────────────

TEST(ImageLoaderTest, LoadNonexistentReturnsNullopt) {
  auto result = ImageLoader::load("/nonexistent/path/image.ppm");
  EXPECT_FALSE(result.has_value());
}

TEST(ImageLoaderTest, SaveAndLoadRoundTrip) {
  Image original = makeColorImage(123, 45, 67, 6, 6);
  const std::string path = "/tmp/ifp_test_roundtrip.ppm";

  ImageLoader::save(original, path);

  auto loaded = ImageLoader::load(path);
  ASSERT_TRUE(loaded.has_value());

  EXPECT_EQ(loaded->width(),  original.width());
  EXPECT_EQ(loaded->height(), original.height());
  EXPECT_EQ(loaded->at(0, 0, 0), 123);
  EXPECT_EQ(loaded->at(0, 0, 1), 45);
  EXPECT_EQ(loaded->at(0, 0, 2), 67);

  std::remove(path.c_str());
}

TEST(ImageLoaderTest, SaveEmptyImageThrows) {
  Image empty;
  EXPECT_THROW(ImageLoader::save(empty, "/tmp/empty.ppm"), ImageException);
}
