/**
 * @file main.cpp
 *
 * カーネル本体のプログラムを書いたファイル．
 */

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <deque>
#include <limits>
#include <numeric>
#include <vector>

#include "acpi.hpp"
#include "asmfunc.h"
#include "console.hpp"
#include "font.hpp"
#include "frame_buffer_config.hpp"
#include "graphics.hpp"
#include "interrupt.hpp"
#include "layer.hpp"
#include "logger.hpp"
#include "memory_manager.hpp"
#include "memory_map.hpp"
#include "message.hpp"
#include "mouse.hpp"
#include "paging.hpp"
#include "pci.hpp"
#include "segment.hpp"
#include "timer.hpp"
#include "usb/xhci/xhci.hpp"
#include "window.hpp"

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

std::shared_ptr<Window> main_window;
unsigned int main_window_layer_id;
void InitializeMainWindow() {
  main_window = std::make_shared<Window>(160, 52, screen_config.pixel_format);
  DrawWindow(*main_window->Writer(), "Hello Window");
  main_window_layer_id = layer_manager->NewLayer()
                             .SetWindow(main_window)
                             .SetDraggable(true)
                             .Move({300, 100})
                             .ID();
  layer_manager->UpDown(main_window_layer_id, std::numeric_limits<int>::max());
}

std::deque<Message>* main_queue;

alignas(16) uint8_t kernel_main_stack[1024 * 1024];

extern "C" void KernelMainNewStack(
    const FrameBufferConfig& frame_buffer_config_ref,
    const MemoryMap& memory_map_ref, const acpi::RSDP& acpi_table) {
  MemoryMap memory_map{memory_map_ref};

  InitializeGraphics(frame_buffer_config_ref);
  InitializeConsole();

  printk("Welcome %s!!\n", "@atrn0");
  printk("day11e\n");
  SetLogLevel(kWarn);

  InitializeSegmentation();
  InitializePaging();
  InitializeMemoryManager(memory_map);
  ::main_queue = new std::deque<Message>(32);
  InitializeInterrupt(main_queue);
  InitializePCI();
  usb::xhci::Initialize();

  InitializeLayer();
  InitializeMainWindow();
  InitializeMouse();
  layer_manager->Draw({{0, 0}, ScreenSize()});

  acpi::Initialize(acpi_table);
  InitializeLAPICTimer(*main_queue);

  timer_manager->AddTimer(Timer(200, 2));
  timer_manager->AddTimer(Timer(600, -1));

  char str[128];

  while (true) /* event loop */ {
    __asm__("cli");
    const auto tick = timer_manager->CurrentTick();
    __asm__("sti");

    sprintf(str, "%010lu", tick);
    FillRectangle(*main_window->Writer(), {24, 28}, {8 * 10, 16},
                  {0xc6, 0xc6, 0xc6});
    WriteString(*main_window->Writer(), {24, 28}, str, {0, 0, 0});
    layer_manager->Draw(main_window_layer_id);

    // main_queueの排他制御のため、割り込みの受付を停止する。
    __asm__("cli");
    if (main_queue->size() == 0) {
      // 割り込みがキューに入っていない場合、
      // 割り込みを受け付けるようにしてからCPUをスリープさせる
      __asm__("sti\n\thlt");
      // 割り込みが発生した場合CPUは割り込みを処理した後、
      // ここから処理を再開する。
      continue;
    }

    Message msg = main_queue->front();
    main_queue->pop_front();
    __asm__("sti");  // 割り込みを受け付ける

    switch (msg.type) {
      case Message::kInterruptXHCI:
        usb::xhci::ProcessEvents();
        break;
      case Message::kTimerTimeout:
        printk("Timer: timeout = %lu, value = %d\n", msg.arg.timer.timeout,
               msg.arg.timer.value);
        if (msg.arg.timer.value > 0) {
          timer_manager->AddTimer(
              Timer(msg.arg.timer.timeout + 100, msg.arg.timer.value + 1));
        }
        break;
      default:
        Log(kError, "Unknown message type: %d\n", msg.type);
    }
  }

  while (1) __asm__("hlt");
}

extern "C" void __cxa_pure_virtual() {
  while (1) __asm__("hlt");
}
