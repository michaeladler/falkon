#include "zigpass_backend.h"
#include "zigpass_plugin.h"
#include "passwordmanager.h"

#include <iostream>

#include <QCryptographicHash>
#include <QDateTime>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QString>
#include <QUrlQuery>
#include <QVariant>
#include <QVector>
#include <QDir>
#include <mutex>

using std::cerr;

static const QString template_query_host =
    "{\"type\":\"queryHost\",\"host\":\"%1\"}";
static const QString template_get_login =
    "{\"type\":\"getLogin\",\"entry\":\"%1\"}";

static const QString placeholder_username = "___USERNAME-VALUE___";
static const QString placeholder_password = "___PASSWORD-VALUE___";

ZigPassBackend::ZigPassBackend()
    : PasswordBackend()
    , m_is_initialized(false)
{
}

ZigPassBackend::~ZigPassBackend() {
    zigpass_store_deinit(&m_store);
}

QString ZigPassBackend::name() const
{
    return QSL("ZigPass");
}

QVector<PasswordEntry> ZigPassBackend::getEntries(const QUrl &url)
{
    initialize();

    QVector<PasswordEntry> result;
    QString host_s = PasswordManager::createHost(url);

    if (m_cache.contains(host_s)) {
        cerr << "[ZIGPASS] Returning cached entry\n";
        result.append(m_cache[host_s]);
        return result;
    } else {
        cerr << "[ZIGPASS] Cache miss\n";
    }

    const std::lock_guard<std::mutex> lock(m_store_token);

    if (m_cache.contains(host_s)) {
        cerr << "[ZIGPASS] Second try successful: Returning cached entry\n";
        result.append(m_cache[host_s]);
        return result;
    }  else {
        cerr << "[ZIGPASS] Cache miss\n";
    }

    const std::string host = host_s.toStdString();
    cerr << "[ZIGPASS] Looking up host: " << host.c_str() << "\n";

    zigpass_entry_t entry;
    zigpass_entry_init(&entry);
    cerr << "[ZIGPASS] Querying store\n";
    if (zigpass_find_host(&m_store, reinterpret_cast<const uint8_t *>(host.c_str()), host.length(), &entry)) {
        cerr << "[ZIGPASS] Found result\n";

        PasswordEntry password_entry;
        password_entry.host = host_s;

        auto password_byte_array = QByteArray::fromRawData(reinterpret_cast<const char *>(entry.password_ptr), entry.password_len);
        password_entry.password = QString(password_byte_array);

        auto username_byte_array = QByteArray::fromRawData(reinterpret_cast<const char *>(entry.username_ptr), entry.username_len);
        password_entry.username = QString(username_byte_array);

        QCryptographicHash hasher(QCryptographicHash::Algorithm::Sha256);
        hasher.addData(password_entry.host.toUtf8());
        hasher.addData(password_entry.username.toUtf8());
        hasher.addData(password_entry.password.toUtf8());
        password_entry.id = hasher.result().toHex();

        auto login_field_count = zigpass_entry_login_field_count(&entry);
        if (login_field_count > 0) {
            QUrlQuery query;

            zigpass_login_field_t login_field;
            for (uintptr_t i = 0; i < login_field_count; ++i) {
                bool found = zigpass_entry_get_login_field(&entry, i, &login_field);
                if (found) {
                    auto key_array = QByteArray::fromRawData(reinterpret_cast<const char *>(login_field.key_ptr), login_field.key_len);
                    auto key = QString(key_array);
                    auto value_array = QByteArray::fromRawData(reinterpret_cast<const char *>(login_field.value_ptr), login_field.value_len);
                    auto value = QString(value_array);
                    if (value == placeholder_username) {
                        value = password_entry.username;
                    } else if (value == placeholder_password) {
                        value = PasswordManager::urlEncodePassword(password_entry.password);
                    }
                    query.addQueryItem(key, value);
                }
            }

            if (!query.isEmpty()) {
                password_entry.data = query.query().toUtf8();
            }
        }

        cerr << "[ZIGPASS] username: " << password_entry.username.toStdString() << "\n";
        result.append(password_entry);
        m_cache[host_s] = password_entry;
    } else {
        cerr << "[ZIGPASS] No entry found for host\n";
    }
    zigpass_entry_deinit(&entry);

    // Sort to prefer last updated entries
    std::sort(result.begin(), result.end());
    return result;
}

QVector<PasswordEntry> ZigPassBackend::getAllEntries()
{
    initialize();
    cerr << "[ZIGPASS] getAllEntries was called\n";
    return QVector<PasswordEntry>::fromList(m_cache.values());
}

void ZigPassBackend::addEntry(const PasswordEntry &entry)
{
    initialize();
    cerr << "[ZIGPASS] addEntry was called\n";

    PasswordEntry stored = entry;
    stored.updated = QDateTime::currentDateTime().toTime_t();

    // TODO: Persist in zigpass

    m_cache[entry.host] = entry;
}

bool ZigPassBackend::updateEntry(const PasswordEntry &entry)
{
    initialize();
    cerr << "[ZIGPASS] updateEntry was called\n";

    // TODO: Persist in zigpass

    if (m_cache.contains(entry.host)) {
        m_cache[entry.host] = entry;
        return true;
    }
    return false;
}

void ZigPassBackend::updateLastUsed(PasswordEntry &entry)
{
    initialize();
    cerr << "[ZIGPASS] updateLastUsed was called\n";

    entry.updated = QDateTime::currentDateTime().toTime_t();
    m_cache[entry.host] = entry;
}

void ZigPassBackend::removeEntry(const PasswordEntry &entry)
{
    initialize();
    cerr << "[ZIGPASS] removeEntry was called\n";

    m_cache.remove(entry.host);
}

void ZigPassBackend::removeAll()
{
    initialize();
    cerr << "[ZIGPASS] removeAll was called\n";
    m_cache.clear();
}

void ZigPassBackend::initialize()
{

    if (!m_is_initialized) {
        m_is_initialized = true;

        auto store_path = QDir::homePath();
        store_path.append("/.password-store");
        m_password_store_path = store_path.toStdString();

        std::cerr << "[ZIGPASS] Using password_store " << m_password_store_path << "\n";

        zigpass_options_t options = {
            .password_store_ptr = reinterpret_cast<const uint8_t *>(m_password_store_path.c_str()),
            .password_store_len = m_password_store_path.length(),
        };

        if (!zigpass_store_init(&m_store, &options)) {
            std::cerr << "[ZIGPASS] Error initializing store\n";
        }
    }
}

