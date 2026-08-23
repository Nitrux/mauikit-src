#pragma once
#include <QObject>
#include <QSettings>
#include <QString>
#include <QUrl>
#include <QVariant>

#include "mauikit_export.h"
#include <QCoreApplication>

/**
 * @brief Identifies one value in the local application settings.
 *
 * A setting is addressed by a key within a group. value() reads the persisted
 * value and falls back to defaultValue when no value has been stored;
 * setValue() persists a replacement through AppSettings::local().
 */
class SettingSection : public QObject
{
    Q_OBJECT

    /** The key of the setting within group. */
    Q_PROPERTY(QString key READ key WRITE setKey NOTIFY keyChanged)

    /** The settings group containing key. */
    Q_PROPERTY(QString group READ group WRITE setGroup NOTIFY groupChanged)

    /** The value returned when the setting has not been stored. */
    Q_PROPERTY(QVariant defaultValue READ defaultValue WRITE setDefaultValue NOTIFY defaultValueChanged)

private:
    QString m_key;
    QString m_group;
    QVariant m_defaultValue;

public:
    explicit SettingSection(QObject *parent = nullptr);
    QString key() const;
    QString group() const;
    QVariant defaultValue() const;
    QVariant value() const;

public Q_SLOTS:
    void setKey(QString key);
    void setGroup(QString group);
    void setValue(QVariant value);
    void setDefaultValue(QVariant defaultValue);

Q_SIGNALS:
    void keyChanged(QString key);
    void groupChanged(QString group);
    void defaultValueChanged(QVariant defaultValue);
};

/**
 * @brief Provides structured access to persistent application settings.
 *
 * AppSettings stores values with QSettings. Values are addressed by a key and
 * group, and save() synchronizes each change before emitting settingChanged().
 * Use local() for the current application namespace or global() for settings
 * shared under the Maui Project namespace.
 */
class MAUIKIT_EXPORT AppSettings : public QObject
{
    Q_OBJECT
public:
    /**
     * Returns the settings store for the current application and organization.
     */
    static AppSettings &local()
    {
        static AppSettings settings;
        return settings;
    }

    /**
     * Returns the settings store shared under the Maui Project application
     * namespace.
     */
    static AppSettings &global()
    {
        static AppSettings settings(QStringLiteral("mauiproject"));
        return settings;
    }

    AppSettings(const AppSettings &) = delete;
    AppSettings &operator=(const AppSettings &) = delete;
    AppSettings(AppSettings &&) = delete;
    AppSettings &operator=(AppSettings &&) = delete;

    /**
     * Returns the local URL of the file used by the underlying settings store.
     */
    QUrl url() const;

    /**
     * Returns the value stored for @p key in @p group.
     *
     * @param key The setting name.
     * @param group The group containing the setting.
     * @param defaultValue The value returned when the key does not exist.
     */
    QVariant load(const QString &key, const QString &group, const QVariant &defaultValue) const;

    /**
     * Stores @p value for @p key in @p group and synchronizes the settings file.
     */
    void save(const QString &key, const QVariant &value, const QString &group);

private:
    explicit AppSettings(QString app = qApp->applicationName(), QString org = qApp->organizationName().isEmpty() ? QStringLiteral("org.kde.maui") : qApp->organizationName());

    QString m_app;
    QString m_org;
    QSettings *m_settings;

Q_SIGNALS:
    /**
     * Emitted after a value has been saved and synchronized.
     *
     * @param url The settings file that changed.
     * @param key The changed setting name.
     * @param value The newly stored value.
     * @param group The group containing the setting.
     */
    void settingChanged(QUrl url, QString key, QVariant value, QString group);
};

