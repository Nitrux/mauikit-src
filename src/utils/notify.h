/*
 * <one line to give the program's name and a brief idea of what it does.>
 * Copyright (C) 2021  <copyright holder> <email>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <QObject>
#include <QList>
#include <QQmlListProperty>
#include <QUrl>
#include <QQmlEngine>

class Notify;

/**
 * @brief An action associated with a system notification.
 *
 * NotifyAction stores the user-visible action label and emits triggered() when
 * the notification backend reports that the action was activated.
 *
 * @note Notification action forwarding is not currently enabled by the Linux
 * backend, so actions attached to Notify are not displayed or triggered there.
 */
class NotifyAction : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_DISABLE_COPY(NotifyAction)

    /**
     * The user-visible label for the notification action.
     */
    Q_PROPERTY(QString text READ text WRITE setText NOTIFY textChanged)
    
public:
    NotifyAction(QObject *parent = nullptr);
    void setText(const QString &text);
    QString text() const;
    
private:
    QString m_text;
    
Q_SIGNALS:
    /** Emitted when this action is activated for @p notify. */
    void triggered(Notify* notify);
    void textChanged();
};

class KNotification;

/**
 * @brief Describes and sends a system notification.
 *
 * Set the required eventId and any desired presentation properties, then call
 * send(). The event identifier and component name are interpreted by the
 * platform notification backend; on Linux this is KNotification.
 *
 * Notify instances describe notifications but do not retain or manage a sent
 * notification after send() returns.
 */
class Notify : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_DISABLE_COPY(Notify)

    /**
     * The notification component or application identifier understood by the
     * platform backend.
     */
    Q_PROPERTY(QString componentName READ componentName WRITE setComponentName NOTIFY componentNameChanged)

    /**
     * The required event identifier used to select the notification event.
     */
    Q_PROPERTY(QString eventId READ eventId WRITE setEventId REQUIRED)

    /**
     * The notification title.
     */
    Q_PROPERTY(QString title READ title WRITE setTitle NOTIFY titleChanged)

    /**
     * The notification body text.
     */
    Q_PROPERTY(QString message READ message WRITE setMessage NOTIFY messageChanged)

    /**
     * The name of the themed icon representing the notification.
     */
    Q_PROPERTY(QString iconName READ iconName WRITE setIconName NOTIFY iconNameChanged)

    /**
     * An optional image source to display with the notification.
     */
    Q_PROPERTY(QUrl imageSource READ imageSource WRITE setImageSource NOTIFY imageSourceChanged)

    /**
     * The additional actions associated with the notification.
     *
     * @note The current implementation does not expose a writable action list
     * to QML, and the Linux backend does not forward notification actions.
     */
    Q_PROPERTY(QQmlListProperty<NotifyAction> actions READ actions)

    /**
     * The action to trigger when the notification itself is activated.
     *
     * @note The current Linux backend does not forward or trigger the default
     * action.
     */
    Q_PROPERTY(NotifyAction * defaultAction READ defaultAction WRITE setDefaultAction NOTIFY defaulActionChanged)

    /**
     * URLs associated with the notification, such as files relevant to the
     * event.
     */
    Q_PROPERTY(QList<QUrl> urls READ urls WRITE setUrls NOTIFY urlsChanged)

public:
    Notify(QObject * parent = nullptr);

    QQmlListProperty<NotifyAction> actions();

    void appendAction(NotifyAction*);
    int actionsCount() const;
    NotifyAction *action(int) const;
    void clearActions();
    void replaceAction(int, NotifyAction*);
    void removeLastAction();

    const QString &componentName() const;
    void setComponentName(const QString &newComponentName);

    const QString &eventId() const;
    void setEventId(const QString &newEventId);

    const QString &title() const;
    void setTitle(const QString &newTitle);

    const QString &message() const;
    void setMessage(const QString &newMessage);

    const QString &iconName() const;
    void setIconName(const QString &newIconName);

    const QUrl &imageSource() const;
    void setImageSource(const QUrl &newImageSource);

    NotifyAction *defaultAction() const;
    void setDefaultAction(NotifyAction *newDefaultAction);

    const QList<QUrl> &urls() const;
    void setUrls(const QList<QUrl> &newUrls);

private Q_SLOTS:
    void actionActivated(int index);

public Q_SLOTS:
    /**
     * Sends a notification using the currently configured properties.
     * Each invocation creates and submits a new notification event.
     */
    void send();
//    void send(const QString &title, const QString &message, const QString &iconName);

Q_SIGNALS:
    void componentNameChanged(QString);

    void titleChanged(QString);

    void messageChanged(QString);

    void iconNameChanged(QString);

    void imageSourceChanged(QUrl);

    void defaulActionChanged();

    void urlsChanged(QList<QUrl>);

private:
    static void appendAction(QQmlListProperty<NotifyAction>*, NotifyAction*);
    static int actionsCount(QQmlListProperty<NotifyAction>*);
    static NotifyAction* action(QQmlListProperty<NotifyAction>*, int);
    static void clearActions(QQmlListProperty<NotifyAction>*);
    static void replaceAction(QQmlListProperty<NotifyAction>*, int, NotifyAction*);
    static void removeLastAction(QQmlListProperty<NotifyAction>*);

    QList<NotifyAction*> m_actions;

    NotifyAction * m_defaultAction;
    QString m_eventId;
    QString m_title;
    QString m_message;
    QString m_iconName;
    QString m_componentName;
    QUrl m_imageSource;
    QList<QUrl> m_urls;
};

