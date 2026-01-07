// systemdata_provider.hpp
namespace telemetry {
class SystemDataProvider : public QObject {
  Q_OBJECT
  Q_PROPERTY(QVariantMap data READ data NOTIFY dataChanged)
public:
  void start() {
    process.start("./run-conkyd.sh");
    connect(&process, &QProcess::readyReadStandardOutput, [this]() {
      auto raw = process.readAllStandardOutput();
      auto doc = QJsonDocument::fromJson(raw);
      m_data = doc.array().first().toObject().toVariantMap();
      emit dataChanged();
    });
  }

private:
  QProcess process;
  QVariantMap m_data;
};
}; // namespace telemetry
