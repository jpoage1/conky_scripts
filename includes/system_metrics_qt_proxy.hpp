#ifndef SYSTEM_METRICS_QT_PROXY_HPP
#define SYSTEM_METRICS_QT_PROXY_HPP

#include <QObject>
#include <QVariant>
#include <nlohmann/json.hpp>
#include "log.hpp"
#include "system_metrics_proxy.hpp"

class SystemMetricsQtProxy : public QObject, public  SystemMetricsProxy {
    Q_OBJECT
    Q_PROPERTY(QVariantList data READ data NOTIFY dataChanged)

public:
    explicit SystemMetricsQtProxy(QObject* parent = nullptr) : QObject(parent) {}

    QVariantList data() const { return m_data; }

    void updateData(const nlohmann::json& j) {
        // DEBUG: Log the incoming JSON structure
        SPDLOG_DEBUG("Proxy: Received JSON of type: {}", 
                     j.is_array() ? "array" : j.is_object() ? "object" : "other");
        SPDLOG_DEBUG("Proxy: JSON dump (first 200 chars): {}", 
                     j.dump().substr(0, 200));

        QVariant var = jsonToVariant(j);

        if (var.isNull()) {
            SPDLOG_ERROR("Proxy: jsonToVariant returned Null");
            return;
        }

        // DEBUG: Log what type we actually got
        SPDLOG_DEBUG("Proxy: Converted to QVariant type: {}", var.typeName());

        if (var.canConvert<QVariantList>()) {
            QVariantList newData = var.toList();
            
            // DEBUG: Log the size and structure
            SPDLOG_DEBUG("Proxy: Array contains {} elements", newData.size());
            if (!newData.isEmpty() && newData[0].canConvert<QVariantMap>()) {
                QVariantMap firstMap = newData[0].toMap();
                SPDLOG_DEBUG("Proxy: First element has {} keys", firstMap.size());
                SPDLOG_DEBUG("Proxy: Sample keys: {}", 
                             firstMap.keys().mid(0, 5).join(", ").toStdString());
            }
            
            m_data = newData;
            emit dataChanged();  // This MUST be called for QML to update
            SPDLOG_DEBUG("Proxy: dataChanged signal emitted");
        } else {
            SPDLOG_ERROR("Proxy: Cannot convert to QVariantList, got: {}", 
                         var.typeName());
        }
    }

signals:
    void dataChanged();

private:
    QVariantList m_data;

    QVariant jsonToVariant(const nlohmann::json& j) {
        if (j.is_null()) {
            SPDLOG_DEBUG("jsonToVariant: null value");
            return QVariant();
        }
        
        if (j.is_object()) {
            QVariantMap map;
            for (auto it = j.begin(); it != j.end(); ++it) {
                map.insert(QString::fromStdString(it.key()), 
                          jsonToVariant(it.value()));
            }
            return map;
        }
        
        if (j.is_array()) {
            QVariantList list;
            for (const auto& el : j) {
                list.append(jsonToVariant(el));
            }
            return list;
        }
        
        if (j.is_string()) return QString::fromStdString(j.get<std::string>());
        if (j.is_number_integer()) return QVariant::fromValue(j.get<long long>());
        if (j.is_number_float()) return j.get<double>();
        if (j.is_boolean()) return j.get<bool>();
        
        SPDLOG_WARN("jsonToVariant: Unknown JSON type");
        return QVariant();
    }
};

#endif
