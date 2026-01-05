// src/system_metrics_proxy.hpp
#ifndef SYSTEM_METRICS_PROXY_HPP
#define SYSTEM_METRICS_PROXY_HPP

#include <QObject>
#include <QVariant>
#include <nlohmann/json.hpp>

class SystemMetricsProxy : public QObject {
    Q_OBJECT
    Q_PROPERTY(QVariant systemData READ systemData NOTIFY dataChanged)

public:
    explicit SystemMetricsProxy(QObject* parent = nullptr) : QObject(parent) {}

    QVariant systemData() const { return m_data; }

    void updateData(const nlohmann::json& j) {
        m_data = jsonToVariant(j);
        emit dataChanged();
    }

signals:
    void dataChanged();

private:
    QVariant m_data;

    // Direct conversion logic for QML
    QVariant jsonToVariant(const nlohmann::json& j) {
        if (j.is_null()) return QVariant();
        if (j.is_object()) {
            QVariantMap map;
            for (auto it = j.begin(); it != j.end(); ++it)
                map.insert(QString::fromStdString(it.key()), jsonToVariant(it.value()));
            return map;
        }
        if (j.is_array()) {
            QVariantList list;
            for (const auto& el : j) list.append(jsonToVariant(el));
            return list;
        }
        if (j.is_string()) return QString::fromStdString(j.get<std::string>());
        if (j.is_number_integer()) return QVariant::fromValue(j.get<long long>());
        if (j.is_number_float()) return j.get<double>();
        if (j.is_boolean()) return j.get<bool>();
        return QVariant();
    }
};

#endif
