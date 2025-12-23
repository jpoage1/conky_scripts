// src/qt_main.cpp
#include <QApplication>
#include <QCoreApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QTimer>
#include <functional>
#include <iostream>
#include <mutex>
#include <thread>

#include "base_kernel.hpp"
#include "bot_kernel.hpp"
#include "cli_parser.hpp"
#include "filesystem.hpp"
#include "game_context.hpp"
#include "gamesession.hpp"
#include "kernel_io.hpp"
#include "player_kernel.hpp"
#include "shop_system.hpp"

// --- MODE B: GUI APPLICATION ---
int main(int argc, char* argv[]) {
  QApplication app(argc, argv);  // <--- LOADS GUI LIBS

  GameContext ctx;

  QQmlApplicationEngine engine;
  engine.rootContext()->setContextProperty("rwKernel", ctx.kernel);
  engine.rootContext()->setContextProperty("rwGame", ctx.session);
  engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
  if (engine.rootObjects().isEmpty()) return -1;

  // 4. State variables for Game Loop
  bool session_notified = false;  // Logic flag for gateway hack

  // GUI Game Loop
  QTimer timer;
  QObject::connect(&timer, &QTimer::timeout, [&]() {
    // std::lock_guard<std::mutex> lock(&ctx.kernel);
    if (!ctx.kernel->health->game_over) {
      ctx.bot->tick();
      ctx.kernel->tick();
      if (ctx.bot->is_dead()) {
        // Check if it's a Gateway (Ends in .1)
        if (ctx.bot->ip_address.length() > 2 &&
            ctx.bot->ip_address.back() == '1') {
          int sector = NetworkManager::get_sector_from_ip(ctx.bot->ip_address);
          ctx.session->register_hack(sector);
        }
      }
    }
    if (ctx.kernel->metabolism.tick_counter % 10 == 0)
      ctx.kernel->sync_vfs_to_disk();
    // --- GATEWAY HACK LOGIC (Inserted Here) ---
    if (ctx.bot->is_dead() && !session_notified) {
      // Check if the dead bot was a Gateway (IP ends in .1)
      if (ctx.bot->ip_address.back() == '1') {
        int sector = NetworkManager::get_sector_from_ip(ctx.bot->ip_address);
        ctx.session->register_hack(sector);
      }
      session_notified = true;
    }
  });
  timer.start(16);

  // CLI Thread
  std::thread cli_thread([&]() {
    std::string input;
    while (true) {
      kout << "root@rootwars [$" << ctx.kernel->credits << "]: ";
      if (!std::getline(std::cin, input)) break;
      if (input == "exit") {
        QMetaObject::invokeMethod(&app, "quit");
        break;
      }
      ctx.kernel->execute_command(input);
    }
  });
  cli_thread.detach();

  return app.exec();
}
