#ifndef ZIGPASSBACKEND_H
#define ZIGPASSBACKEND_H

#include <mutex>
#include <QVector>
#include <QMap>
#include <QProcess>

#include "passwordbackends/passwordbackend.h"
#include "passwordmanager.h"

#include <zigpass.h>

typedef struct zigpass_entry_s zigpass_entry_t;
typedef struct zigpass_options_s zigpass_options_t;
typedef struct zigpass_store_s zigpass_store_t;
typedef struct zigpass_login_field_s zigpass_login_field_t;

class FALKON_EXPORT ZigPassBackend : public PasswordBackend
{
public:
    explicit ZigPassBackend();
    ~ZigPassBackend();

    QString name() const override;

    QVector<PasswordEntry> getEntries(const QUrl &url) override;
    QVector<PasswordEntry> getAllEntries() override;

    void addEntry(const PasswordEntry &entry) override;
    bool updateEntry(const PasswordEntry &entry) override;
    void updateLastUsed(PasswordEntry &entry) override;

    void removeEntry(const PasswordEntry &entry) override;
    void removeAll() override;

private:
    void initialize();

    QMap<QString, PasswordEntry> m_cache;
    bool m_is_initialized;

    zigpass_store_t m_store;
    std::mutex m_store_token;
    std::string m_password_store_path;
};

#endif // ZIGPASSBACKEND_H
