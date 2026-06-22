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

#include <QPainterPath>
#include <QRectF>
#include <QScreen>

#include <private/qhighdpiscaling_p.h>
#include <private/qwaylanddisplay_p.h>
#include <private/qwaylandintegration_p.h>
#include <private/qwaylandwindow_p.h>
#include <qwayland-ext-background-effect-v1.h>
#include <qwaylandclientextension.h>

#include <KWindowEffects>
#include <KWindowSystem>

#include "style.h"

class BackgroundEffectSurface : public QtWayland::ext_background_effect_surface_v1
{
public:
    explicit BackgroundEffectSurface(::ext_background_effect_surface_v1 *surface)
        : QtWayland::ext_background_effect_surface_v1(surface)
    {
    }

    ~BackgroundEffectSurface() override
    {
        if (!isInitialized())
            return;

        destroy();
    }

    void setBlurRegion(const QRegion &region)
    {
        if (!isInitialized())
            return;

        if (region.isEmpty())
        {
            set_blur_region(nullptr);
            return;
        }

        static const auto *waylandIntegration = QtWaylandClient::QWaylandIntegration::instance();
        auto *display = waylandIntegration->display();
        auto *wlRegion = display->createRegion(region);
        set_blur_region(wlRegion);
        wl_region_destroy(wlRegion);
    }
};

namespace {

class BackgroundEffectManager
    : public QWaylandClientExtensionTemplate<BackgroundEffectManager>
    , public QtWayland::ext_background_effect_manager_v1
{
    Q_OBJECT

public:
    BackgroundEffectManager()
        : QWaylandClientExtensionTemplate<BackgroundEffectManager>(1)
    {
        initialize();
    }

    BackgroundEffectSurface *createEffectSurface(QtWaylandClient::QWaylandWindow *window)
    {
        return new BackgroundEffectSurface(get_background_effect(window->surface()));
    }

    bool blurAvailable() const
    {
        return isActive() && m_blurAvailable;
    }

    static BackgroundEffectManager *instance()
    {
        static auto *instance = new BackgroundEffectManager();
        return instance->isInitialized() ? instance : nullptr;
    }

signals:
    void blurAvailableChanged();

protected:
    void ext_background_effect_manager_v1_capabilities(uint32_t flags) override
    {
        const bool available = static_cast<bool>(flags & capability_blur);
        if (available == m_blurAvailable)
            return;

        m_blurAvailable = available;
        Q_EMIT blurAvailableChanged();
    }

private:
    bool m_blurAvailable = false;
};

} // namespace

WindowBlur::WindowBlur(QObject *parent) noexcept
    : QObject(parent)
{
}

WindowBlur::~WindowBlur()
{
    if (m_view)
        QObject::disconnect(m_view, nullptr, this, nullptr);
    if (m_waylandWindow)
        QObject::disconnect(static_cast<QObject *>(m_waylandWindow), nullptr, this, nullptr);
    m_backgroundEffectSurface.reset();
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
    if (m_waylandWindow)
        QObject::disconnect(static_cast<QObject *>(m_waylandWindow), nullptr, this, nullptr);

    m_backgroundEffectSurface.reset();
    m_waylandWindow = nullptr;
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

void WindowBlur::onWaylandSurfaceCreated()
{
    m_backgroundEffectSurface.reset();
    updateBlur();
}

void WindowBlur::onWaylandSurfaceDestroyed()
{
    m_backgroundEffectSurface.reset();
}

QRegion WindowBlur::roundedBlurRegion(const QRect &rect, qreal radius)
{
    if (rect.isEmpty())
        return QRegion();

    if (radius <= 0.0)
        return QRegion(rect);

    const qreal maxRadius = qMin(rect.width(), rect.height()) / 2.0;
    const qreal clampedRadius = qMax<qreal>(0.0, qMin(radius, maxRadius));
    if (clampedRadius <= 0.0)
        return QRegion(rect);

    QPainterPath path;
    path.addRoundedRect(QRectF(rect), clampedRadius, clampedRadius);
    return QRegion(path.toFillPolygon().toPolygon());
}

void WindowBlur::updateWaylandWindow()
{
    if (!m_view)
        return;

    if (m_view->isVisible() && !m_view->handle())
        m_view->create();

    auto *waylandWindow = dynamic_cast<QtWaylandClient::QWaylandWindow *>(m_view->handle());
    if (waylandWindow == m_waylandWindow)
        return;

    if (m_waylandWindow)
        QObject::disconnect(static_cast<QObject *>(m_waylandWindow), nullptr, this, nullptr);

    m_waylandWindow = waylandWindow;
    if (!m_waylandWindow)
        return;

    QObject::connect(static_cast<QObject *>(m_waylandWindow), &QObject::destroyed, this, [this]() {
        m_waylandWindow = nullptr;
        m_backgroundEffectSurface.reset();
    });
    QObject::connect(m_waylandWindow, &QtWaylandClient::QWaylandWindow::surfaceCreated, this, &WindowBlur::onWaylandSurfaceCreated);
    QObject::connect(m_waylandWindow, &QtWaylandClient::QWaylandWindow::surfaceDestroyed, this, &WindowBlur::onWaylandSurfaceDestroyed);
}

void WindowBlur::updateBlur()
{
    if (!m_view)
        return;

    if (!KWindowSystem::isPlatformWayland())
    {
        KWindowEffects::enableBlurBehind(m_view, m_enabled, m_rect);
        KWindowEffects::enableBackgroundContrast(m_view, m_enabled);
        return;
    }

    updateWaylandWindow();

    if (!m_enabled || !m_view->isVisible() || !m_waylandWindow || !m_waylandWindow->surface())
    {
        m_backgroundEffectSurface.reset();
        KWindowEffects::enableBlurBehind(m_view, false, m_rect);
        KWindowEffects::enableBackgroundContrast(m_view, false);
        return;
    }

    auto *manager = BackgroundEffectManager::instance();
    if (manager)
        QObject::connect(manager, &BackgroundEffectManager::blurAvailableChanged, this, &WindowBlur::updateBlur, Qt::UniqueConnection);

    if (!manager || !manager->blurAvailable())
    {
        m_backgroundEffectSurface.reset();
        KWindowEffects::enableBlurBehind(m_view, m_enabled, m_rect);
        KWindowEffects::enableBackgroundContrast(m_view, m_enabled);
        return;
    }

    KWindowEffects::enableBlurBehind(m_view, false, m_rect);
    KWindowEffects::enableBackgroundContrast(m_view, false);

    if (!m_backgroundEffectSurface)
        m_backgroundEffectSurface.reset(manager->createEffectSurface(m_waylandWindow));

    if (!m_backgroundEffectSurface)
        return;

    QRegion region = roundedBlurRegion(m_rect, m_windowRadius);
    if (!region.isEmpty())
    {
        const qreal scale = QHighDpiScaling::factor(m_view);
        if (!qFuzzyCompare(scale, 1.0))
            region = QHighDpi::scale(region, scale);

        const auto margins = m_waylandWindow->clientSideMargins();
        region.translate(margins.left(), margins.top());
    }

    m_backgroundEffectSurface->setBlurRegion(region);
}

#include "windowblur.moc"
