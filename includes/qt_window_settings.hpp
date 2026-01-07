#ifndef QT_WINDOW_SETTINGS_HPP
#define QT_WINDOW_SETTINGS_HPP

#include <QObject>
#include <QtQml>

namespace telemetry {
struct QtWindowSettings : public WindowSettings {
  Q_GADGET

public:
  enum class WindowType { DESKTOP, DOCK, NORMAL };
  Q_ENUM(WindowType)

  enum class WindowStacking {
    FG,    // Always on top
    BOTTOM // Desktop level / Always at bottom
  };
  Q_ENUM(WindowStacking)

  WindowType type = WindowType::DESKTOP;
  WindowStacking stacking = WindowStacking::BOTTOM;
  bool wmIgnore = true;

  // Position and size mirroring EWW geometry
  int x = 20;
  int y = 30;
  int width = 800;
  int height = 950;
};

}; // namespace telemetry
// Register the type so QML understands the enums
//
// qmlRegisterUncreatableType<WindowSettings>("Telemetry.Types", 1, 0,
// "WindowSettings", "Enums only");

#endif
