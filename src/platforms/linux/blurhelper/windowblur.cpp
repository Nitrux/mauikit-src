/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     cutefish <cutefishos@foxmail.com>
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

#include "windowblur.h"

#include "style.h"

WindowBlur::WindowBlur(QObject *parent) noexcept
    : QObject(parent)
{
}

WindowBlur::~WindowBlur()
{
    if (m_view)
        QObject::disconnect(m_view, nullptr, this, nullptr);
}

void WindowBlur::classBegin()
{
}

void WindowBlur::componentComplete()
{
    Style::instance()->setTranslucencyAvailable(m_enabled);
    updateBlur();
}

void WindowBlur::setView(QWindow *view)
{
    if (view == m_view)
        return;

    if (m_view)
        QObject::disconnect(m_view, nullptr, this, nullptr);
    m_view = view;

    if (m_view)
    {
        connect(m_view, &QWindow::visibleChanged, this, &WindowBlur::onViewVisibleChanged);
        onViewVisibleChanged(m_view->isVisible());
    }

    Q_EMIT viewChanged();
}

QWindow *WindowBlur::view() const
{
    return m_view;
}

void WindowBlur::setGeometry(const QRect &rect)
{
    if (rect == m_rect)
        return;

    m_rect = rect;
    updateBlur();
    Q_EMIT geometryChanged();
}

QRect WindowBlur::geometry() const
{
    return m_rect;
}

void WindowBlur::setEnabled(bool enabled)
{
    if (enabled == m_enabled)
        return;

    m_enabled = enabled;
    updateBlur();
    Q_EMIT enabledChanged();
}

bool WindowBlur::enabled() const
{
    return m_enabled;
}

void WindowBlur::setWindowRadius(qreal radius)
{
    if (radius == m_windowRadius)
        return;

    m_windowRadius = radius;
    updateBlur();
    Q_EMIT windowRadiusChanged();
}

qreal WindowBlur::windowRadius() const
{
    return m_windowRadius;
}

void WindowBlur::onViewVisibleChanged(bool)
{
    updateBlur();
}

void WindowBlur::updateBlur()
{
    // MauiKit leaves blur to the Wayland compositor.
}

#include "windowblur.moc"
