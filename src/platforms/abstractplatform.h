#pragma once
#include <QList>
#include <QObject>
#include <QString>
#include <QUrl>

#include "mauikit_export.h"

/**
 * @brief Interface for services whose implementation depends on the platform.
 *
 * Platform backends implement sharing, input-device capability queries, and
 * appearance information. A service may be unavailable on a particular
 * platform; callers should not assume every backend provides an interactive
 * implementation.
 */
class MAUIKIT_EXPORT AbstractPlatform : public QObject
{
    Q_OBJECT

    /** Whether the platform currently reports a dark appearance. */
    Q_PROPERTY(bool darkModeEnabled READ darkModeEnabled NOTIFY darkModeEnabledChanged)

public:
    explicit AbstractPlatform(QObject *parent = nullptr);

public Q_SLOTS:

    /**
     * Requests that the platform share the files identified by @p urls.
     *
     * Backends without a native share UI may emit shareFilesRequest() so the
     * application can handle the request itself.
     */
    virtual void shareFiles(const QList<QUrl> &urls) = 0;

    /**
     * Requests that the platform share a text string.
     *
     * @param urls The text to share.
     *
     * This operation may be unavailable on some platform backends.
     */
    virtual void shareText(const QString &urls) = 0;

    /** Returns whether the platform reports an available physical keyboard. */
    virtual bool hasKeyboard() = 0;

    /** Returns whether the platform reports an available pointing device. */
    virtual bool hasMouse() = 0;

    /**
     * Platform hook for posting a notification.
     *
     * The base implementation does nothing. Backends that support this hook
     * may use @p icon as a themed icon name and @p imageUrl as an image source.
     */
    virtual void notify(const QString &title, const QString &message, const QString &icon, const QString &imageUrl);

    virtual bool darkModeEnabled() = 0;

Q_SIGNALS:
    void hasKeyboardChanged();
    void hasMouseChanged();
    /** Emitted when file sharing must be handled by the application. */
    void shareFilesRequest(QStringList urls);
    void darkModeEnabledChanged();
};
