#include <cstddef>
#include <cstdint>
#include <cstdio>

#include "console.hpp"
#include "font.hpp"
#include "frame_buffer_config.hpp"
#include "graphics.hpp"

void* operator new(size_t size, void* buf) { return buf; }

void operator delete(void* obj) noexcept {}

const PixelColor kDesktopBGColor{45, 118, 237};
const PixelColor kDesktopFGColor{255, 255, 255};

const int kMouseCursorWidth = 15;
const int kMouseCursorHeight = 24;
const char mouse_cursor_shape[kMouseCursorHeight][kMouseCursorWidth + 1] = {
    "@              ",  //
    "@@             ",  //
    "@.@            ",  //
    "@..@           ",  //
    "@...@          ",  //
    "@....@         ",  //
    "@.....@        ",  //
    "@......@       ",  //
    "@.......@      ",  //
    "@........@     ",  //
    "@.........@    ",  //
    "@..........@   ",  //
    "@...........@  ",  //
    "@............@ ",  //
    "@......@@@@@@@@",  //
    "@......@       ",  //
    "@....@@.@      ",  //
    "@...@ @.@      ",  //
    "@..@   @.@     ",  //
    "@.@    @.@     ",  //
    "@@      @.@    ",  //
    "@       @.@    ",  //
    "         @.@   ",  //
    "         @@@   ",  //
};

char pixel_writer_buf[sizeof(RGBResv8BitPerColorPixelWriter)];
PixelWriter* pixel_writer;

char console_buf[sizeof(Console)];
Console* console;

int printk(const char* format, ...) {
  va_list ap;
  int result;
  char s[1024];

  va_start(ap, format);
  result = vsprintf(s, format, ap);
  va_end(ap);

  console->PutString(s);
  return result;
}

extern "C" void KernelMain(const FrameBufferConfig& frame_buffer_config) {
  switch (frame_buffer_config.pixel_format) {
    case kPixelRGBResv8BitPerColor:
      pixel_writer = new (pixel_writer_buf)
          RGBResv8BitPerColorPixelWriter{frame_buffer_config};
      break;
    case kPixelBGRResv8BitPerColor:
      pixel_writer = new (pixel_writer_buf)
          BGRResv8BitPerColorPixelWriter{frame_buffer_config};
      break;
  }

  // for (int x = 0; x < frame_buffer_config.horizontal_resolution; ++x) {
  //   for (int y = 0; y < frame_buffer_config.vertical_resolution; ++y) {
  //     pixel_writer->Write(x, y, {255, 255, 255});
  //   }
  // }

  // console =
  //     new (console_buf) Console{*pixel_writer, {0, 0, 0}, {255, 255, 255}};

  const int kFrameWidth = frame_buffer_config.horizontal_resolution;
  const int kFrameHeight = frame_buffer_config.vertical_resolution;
  FillRectangle(*pixel_writer, {0, 0}, {kFrameWidth, kFrameHeight - 50},
                kDesktopBGColor);
  FillRectangle(*pixel_writer, {0, kFrameHeight - 50}, {kFrameWidth, 50},
                {1, 8, 17});
  FillRectangle(*pixel_writer, {0, kFrameHeight - 50},
                {kFrameWidth / 5, kFrameHeight - 50}, {80, 80, 80});
  Drawrectangle(*pixel_writer, {10, kFrameHeight - 40}, {30, 30},
                {160, 160, 160});

  console = new (console_buf)
      Console{*pixel_writer, kDesktopFGColor, kDesktopBGColor};
  printk("Welcome %s!!\n", "@atrn0");

  for (int dy = 0; dy < kMouseCursorHeight; ++dy) {
    for (int dx = 0; dx < kMouseCursorWidth; ++dx) {
      if (mouse_cursor_shape[dy][dx] == '@') {
        pixel_writer->Write(200 + dx, 100 + dy, {255, 255, 255});
      } else if (mouse_cursor_shape[dy][dx] == '.') {
        pixel_writer->Write(200 + dx, 100 + dy, {0, 0, 0});
      }
    }
  }

  while (1) __asm__("hlt");
}
