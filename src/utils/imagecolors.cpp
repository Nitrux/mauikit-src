/*
 *  Copyright 2020 Marco Martin <mart@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

#include "imagecolors.h"
#include "platformtheme.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QUrl>
#include <QTimer>
#include <QtConcurrent>

#include <cmath>

#define return_fallback(value)                                                                                                                                 \
    if (m_imageData.m_samples.size() == 0) {                                                                                                                   \
        return value;                                                                                                                                          \
    }

#define return_fallback_finally(value, finally)                                                                                                                \
    if (m_imageData.m_samples.size() == 0) {                                                                                                                   \
return value.isValid() ? value : static_cast<MauiKit::Platform::PlatformTheme *>(qmlAttachedPropertiesObject<MauiKit::Platform::PlatformTheme>(this, true))->finally();  \
    }

ImageColors::ImageColors(QObject *parent)
    : QObject(parent)
{
    m_imageSyncTimer = new QTimer(this);
    m_imageSyncTimer->setSingleShot(true);
    m_imageSyncTimer->setInterval(100);
    m_sourceWatcher = new QFileSystemWatcher(this);
    connect(m_sourceWatcher, &QFileSystemWatcher::fileChanged, this, [this]() {
        m_imageSyncTimer->start();
    });
    connect(m_sourceWatcher, &QFileSystemWatcher::directoryChanged, this, [this]() {
        m_imageSyncTimer->start();
    });
    connect(m_imageSyncTimer, &QTimer::timeout, this, &ImageColors::reloadSourceFile);
}

ImageColors::~ImageColors()
{
}

void ImageColors::setSource(const QVariant &source)
{
    clearSourceWatcher();
    m_sourceFilePath.clear();

    if (!source.isValid() || source.isNull())
    {
        setSourceImage(QImage());
        m_source = source;
        Q_EMIT sourceChanged();
        return;
    }

    if (source.canConvert<QQuickItem *>()) {
        qDebug() << "can convert to item";
        setSourceItem(source.value<QQuickItem *>());
    } else if (source.canConvert<QImage>()) {
        qDebug() << "can convert to image";
        setSourceImage(source.value<QImage>());
    } else if (source.canConvert<QIcon>()) {
        qDebug() << "can convert to icon";
        setSourceImage(source.value<QIcon>().pixmap(128, 128).toImage());
    } else if (source.canConvert<QString>() || source.canConvert<QUrl>()) {
        qDebug() << "can convert to string";
        const QString sourceString = source.canConvert<QUrl>() ? source.value<QUrl>().toString() : source.toString();
        if (sourceString.isEmpty())
        {
            setSourceImage(QImage());
            m_source = source;
            Q_EMIT sourceChanged();
            return;
        }

        if (sourceString.startsWith(QStringLiteral("qrc:")))
        {
            qDebug() << "SET IMAGE FROM QRC IMAGE COLORS" << sourceString;
            setSourceImage(QImage(QStringLiteral(":") + sourceString.mid(4)));
        }
        else
        {
            const QUrl sourceUrl(sourceString);
            const QString localPath = sourceUrl.isLocalFile() ? sourceUrl.toLocalFile() : sourceString;
            const QFileInfo imageInfo(localPath);
            const bool localFileSource = sourceUrl.isLocalFile() || localPath.startsWith(QLatin1Char(47));
            if (imageInfo.isFile())
            {
                const QString canonicalPath = imageInfo.canonicalFilePath();
                setSourceImage(QImage(canonicalPath));
                m_sourceFilePath = canonicalPath;
                watchSourceFile(m_sourceFilePath);
            }
            else if (localFileSource)
            {
                setSourceImage(QImage());
                m_sourceFilePath = imageInfo.absoluteFilePath();
                watchSourceFile(m_sourceFilePath);
            }
            else
            {
                setSourceImage(QIcon::fromTheme(sourceString).pixmap(128, 128).toImage());
            }
        }
    } else {
        setSourceImage(QImage());
        m_source = source;
        Q_EMIT sourceChanged();
        return;
    }

    m_source = source;
    Q_EMIT sourceChanged();
}

QVariant ImageColors::source() const
{
    return m_source;
}

void ImageColors::setSourceImage(const QImage &image)
{
    if (m_window) {
        disconnect(m_window.data(), nullptr, this, nullptr);
    }
    if (m_sourceItem) {
        disconnect(m_sourceItem.data(), nullptr, this, nullptr);
    }
    if (m_grabResult) {
        disconnect(m_grabResult.data(), nullptr, this, nullptr);
        m_grabResult.clear();
    }

    m_sourceItem.clear();
    m_window.clear();

    m_sourceImage = image;
    if (m_sourceImage.width() > 128 || m_sourceImage.height() > 128)
        m_sourceImage = m_sourceImage.scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    m_imageData = {};
    update();
    if (m_sourceImage.isNull())
        Q_EMIT paletteChanged();
}

QImage ImageColors::sourceImage() const
{
    return m_sourceImage;
}

void ImageColors::setSourceItem(QQuickItem *source)
{
    clearSourceWatcher();
    m_sourceFilePath.clear();

    if (m_sourceItem == source) {
        return;
    }

    if (m_window) {
        disconnect(m_window.data(), nullptr, this, nullptr);
    }
    if (m_sourceItem) {
        disconnect(m_sourceItem, nullptr, this, nullptr);
    }
    m_sourceItem = source;
    m_sourceImage = QImage();
    m_imageData = {};
    m_window.clear();
    update();
    Q_EMIT paletteChanged();

    if (m_sourceItem) {
        auto syncWindow = [this]() {
            if (m_window) {
                disconnect(m_window.data(), nullptr, this, nullptr);
            }
            if (!m_sourceItem)
            {
                m_window.clear();
                return;
            }
            m_window = m_sourceItem->window();
            if (m_window) {
                connect(m_window, &QWindow::visibleChanged, this, &ImageColors::update);
                update();
            }
        };

        connect(m_sourceItem, &QQuickItem::windowChanged, this, syncWindow);
        syncWindow();
    }
}

void ImageColors::watchSourceFile(const QString &path)
{
    clearSourceWatcher();
    if (m_sourceWatcher == nullptr)
        return;

    const QFileInfo info(path);
    if (info.isFile())
        m_sourceWatcher->addPath(path);
    if (QDir(info.absolutePath()).exists())
        m_sourceWatcher->addPath(info.absolutePath());
}

void ImageColors::reloadSourceFile()
{
    if (m_sourceFilePath.isEmpty())
        return;

    const QString path = m_sourceFilePath;
    if (QFileInfo(path).isFile())
    {
        setSourceImage(QImage(path));
        watchSourceFile(path);
    }
    else
    {
        setSourceImage(QImage());
        watchSourceFile(path);
    }
}

void ImageColors::clearSourceWatcher()
{
    if (m_sourceWatcher == nullptr)
        return;

    if (m_sourceWatcher->files().isEmpty() == false)
        m_sourceWatcher->removePaths(m_sourceWatcher->files());
    if (m_sourceWatcher->directories().isEmpty() == false)
        m_sourceWatcher->removePaths(m_sourceWatcher->directories());
}

QQuickItem *ImageColors::sourceItem() const
{
    return m_sourceItem;
}

void ImageColors::update()
{
    if (m_futureImageData)
    {
        auto *previousWatcher = m_futureImageData;
        m_futureImageData = nullptr;
        previousWatcher->cancel();
        previousWatcher->deleteLater();
    }
    auto runUpdate = [this]() {
        const QImage image = m_sourceImage;
        QFuture<ImageData> future = QtConcurrent::run([image]() {
            return generatePalette(image);
        });
        auto *watcher = new QFutureWatcher<ImageData>(this);
        m_futureImageData = watcher;
        connect(watcher, &QFutureWatcher<ImageData>::finished, this, [this, watcher]() {
            if (m_futureImageData != watcher)
            {
                watcher->deleteLater();
                return;
            }

            m_imageData = watcher->future().result();
            watcher->deleteLater();
            m_futureImageData = nullptr;
            Q_EMIT paletteChanged();
        });
        watcher->setFuture(future);
    };

    if (!m_sourceItem || !m_window) {
        if (!m_sourceImage.isNull()) {
            runUpdate();
        }
        return;
    }

    if (m_grabResult) {
        disconnect(m_grabResult.data(), nullptr, this, nullptr);
        m_grabResult.clear();
    }

    m_grabResult = m_sourceItem->grabToImage(QSize(128, 128));

    if (m_grabResult) {
        connect(m_grabResult.data(), &QQuickItemGrabResult::ready, this, [this, runUpdate]() {
            m_sourceImage = m_grabResult->image();
            m_grabResult.clear();
            runUpdate();
        });
    }
}

inline int squareDistance(QRgb color1, QRgb color2)
{
    // https://en.wikipedia.org/wiki/Color_difference
    // Using RGB distance for performance, as CIEDE2000 istoo complicated
    if (qRed(color1) - qRed(color2) < 128) {
        return 2 * pow(qRed(color1) - qRed(color2), 2) //
            + 4 * pow(qGreen(color1) - qGreen(color2), 2) //
            + 3 * pow(qBlue(color1) - qBlue(color2), 2);
    } else {
        return 3 * pow(qRed(color1) - qRed(color2), 2) //
            + 4 * pow(qGreen(color1) - qGreen(color2), 2) //
            + 2 * pow(qBlue(color1) - qBlue(color2), 2);
    }
}

void ImageColors::positionColor(QRgb rgb, QList<ImageData::colorStat> &clusters)
{
    for (auto &stat : clusters) {
        if (squareDistance(rgb, stat.centroid) < s_minimumSquareDistance) {
            stat.colors.append(rgb);
            return;
        }
    }

    ImageData::colorStat stat;
    stat.colors.append(rgb);
    stat.centroid = rgb;
    clusters << stat;
}

ImageData ImageColors::generatePalette(const QImage &sourceImage)
{
    ImageData imageData;

    if (sourceImage.isNull() || sourceImage.width() == 0) {
        return imageData;
    }

    imageData.m_clusters.clear();
    imageData.m_samples.clear();

    QColor sampleColor;
    int r = 0;
    int g = 0;
    int b = 0;
    int c = 0;
    for (int x = 0; x < sourceImage.width(); ++x) {
        for (int y = 0; y < sourceImage.height(); ++y) {
            sampleColor = sourceImage.pixelColor(x, y);
            if (sampleColor.alpha() == 0) {
                continue;
            }
            QRgb rgb = sampleColor.rgb();
            c++;
            r += qRed(rgb);
            g += qGreen(rgb);
            b += qBlue(rgb);
            imageData.m_samples << rgb;
            positionColor(rgb, imageData.m_clusters);
        }
    }

    if (imageData.m_samples.isEmpty()) {
        return imageData;
    }

    imageData.m_average = QColor(r / c, g / c, b / c, 255);

    for (int iteration = 0; iteration < 5; ++iteration) {
        for (auto &stat : imageData.m_clusters) {
            r = 0;
            g = 0;
            b = 0;
            c = 0;

            for (auto color : std::as_const(stat.colors)) {
                c++;
                r += qRed(color);
                g += qGreen(color);
                b += qBlue(color);
            }
            r = r / c;
            g = g / c;
            b = b / c;
            stat.centroid = qRgb(r, g, b);
            stat.ratio = qreal(stat.colors.count()) / qreal(imageData.m_samples.count());
            stat.colors = QList<QRgb>({stat.centroid});
        }

        for (auto color : std::as_const(imageData.m_samples)) {
            positionColor(color, imageData.m_clusters);
        }
    }

    std::sort(imageData.m_clusters.begin(), imageData.m_clusters.end(), [](const ImageData::colorStat &a, const ImageData::colorStat &b) {
        return a.colors.size() > b.colors.size();
    });

    // compress blocks that became too similar
    auto sourceIt = imageData.m_clusters.end();
    QList<QList<ImageData::colorStat>::iterator> itemsToDelete;
    while (sourceIt != imageData.m_clusters.begin()) {
        sourceIt--;
        for (auto destIt = imageData.m_clusters.begin(); destIt != imageData.m_clusters.end() && destIt != sourceIt; destIt++) {
            if (squareDistance((*sourceIt).centroid, (*destIt).centroid) < s_minimumSquareDistance) {
                const qreal ratio = (*sourceIt).ratio / (*destIt).ratio;
                const int r = ratio * qreal(qRed((*sourceIt).centroid)) + (1 - ratio) * qreal(qRed((*destIt).centroid));
                const int g = ratio * qreal(qGreen((*sourceIt).centroid)) + (1 - ratio) * qreal(qGreen((*destIt).centroid));
                const int b = ratio * qreal(qBlue((*sourceIt).centroid)) + (1 - ratio) * qreal(qBlue((*destIt).centroid));
                (*destIt).ratio += (*sourceIt).ratio;
                (*destIt).centroid = qRgb(r, g, b);
                itemsToDelete << sourceIt;
                break;
            }
        }
    }
    for (const auto &i : std::as_const(itemsToDelete)) {
        imageData.m_clusters.erase(i);
    }

    imageData.m_highlight = QColor();
    imageData.m_dominant = QColor(imageData.m_clusters.first().centroid);
    imageData.m_closestToBlack = Qt::white;
    imageData.m_closestToWhite = Qt::black;

    imageData.m_palette.clear();

    bool first = true;

    for (const auto &stat : std::as_const(imageData.m_clusters)) {
        QVariantMap entry;
        const QColor color(stat.centroid);
        entry[QStringLiteral("color")] = color;
        entry[QStringLiteral("ratio")] = stat.ratio;

        QColor contrast = QColor(255 - color.red(), 255 - color.green(), 255 - color.blue());
        contrast.setHsl(contrast.hslHue(), //
                        contrast.hslSaturation(), //
                        128 + (128 - contrast.lightness()));
        QColor tempContrast;
        int minimumDistance = 4681800; // max distance: 4*3*2*3*255*255
        for (const auto &stat : std::as_const(imageData.m_clusters)) {
            const int distance = squareDistance(contrast.rgb(), stat.centroid);

            if (distance < minimumDistance) {
                tempContrast = QColor(stat.centroid);
                minimumDistance = distance;
            }
        }

        if (imageData.m_clusters.size() <= 3) {
            if (qGray(imageData.m_dominant.rgb()) < 120) {
                contrast = QColor(230, 230, 230);
            } else {
                contrast = QColor(20, 20, 20);
            }
            // TODO: replace m_clusters.size() > 3 with entropy calculation
        } else if (squareDistance(contrast.rgb(), tempContrast.rgb()) < s_minimumSquareDistance * 1.5) {
            contrast = tempContrast;
        } else {
            contrast = tempContrast;
            contrast.setHsl(contrast.hslHue(),
                            contrast.hslSaturation(),
                            contrast.lightness() > 128 ? qMin(contrast.lightness() + 20, 255) : qMax(0, contrast.lightness() - 20));
        }

        entry[QStringLiteral("contrastColor")] = contrast;

        if (first) {
            imageData.m_dominantContrast = contrast;
            imageData.m_dominant = color;
        }
        first = false;

        if (!imageData.m_highlight.isValid() || ColorUtils::chroma(color) > ColorUtils::chroma(imageData.m_highlight)) {
            imageData.m_highlight = color;
        }

        if (qGray(color.rgb()) > qGray(imageData.m_closestToWhite.rgb())) {
            imageData.m_closestToWhite = color;
        }
        if (qGray(color.rgb()) < qGray(imageData.m_closestToBlack.rgb())) {
            imageData.m_closestToBlack = color;
        }
        imageData.m_palette << entry;
    }

    return imageData;
}

QVariantList ImageColors::palette() const
{
    if (m_futureImageData) {
        qWarning() << m_futureImageData->future().isFinished();
    }
    return_fallback(m_fallbackPalette) return m_imageData.m_palette;
}

ColorUtils::Brightness ImageColors::paletteBrightness() const
{
    /* clang-format off */
    return_fallback(m_fallbackPaletteBrightness)

    return qGray(m_imageData.m_dominant.rgb()) < 128 ? ColorUtils::Dark : ColorUtils::Light;
    /* clang-format on */
}

QColor ImageColors::average() const
{
    /* clang-format off */
    return_fallback_finally(m_fallbackAverage, linkBackgroundColor)

    return m_imageData.m_average;
    /* clang-format on */
}

QColor ImageColors::dominant() const
{
    /* clang-format off */
    return_fallback_finally(m_fallbackDominant, linkBackgroundColor)

    return m_imageData.m_dominant;
    /* clang-format on */
}

QColor ImageColors::dominantContrast() const
{
    /* clang-format off */
    return_fallback_finally(m_fallbackDominantContrasting, linkBackgroundColor)

    return m_imageData.m_dominantContrast;
    /* clang-format on */
}

QColor ImageColors::foreground() const
{
    /* clang-format off */
    return_fallback_finally(m_fallbackForeground, textColor)

    if (paletteBrightness() == ColorUtils::Dark)
    {
        if (qGray(m_imageData.m_closestToWhite.rgb()) < 200) {
            return QColor(230, 230, 230);
        }
        return m_imageData.m_closestToWhite;
    } else {
        if (qGray(m_imageData.m_closestToBlack.rgb()) > 80) {
            return QColor(20, 20, 20);
        }
        return m_imageData.m_closestToBlack;
    }
    /* clang-format on */
}

QColor ImageColors::background() const
{
    /* clang-format off */
    return_fallback_finally(m_fallbackBackground, backgroundColor)

    if (paletteBrightness() == ColorUtils::Dark) {
        if (qGray(m_imageData.m_closestToBlack.rgb()) > 80) {
            return QColor(20, 20, 20);
        }
        return m_imageData.m_closestToBlack;
    } else {
        if (qGray(m_imageData.m_closestToWhite.rgb()) < 200) {
            return QColor(230, 230, 230);
        }
        return m_imageData.m_closestToWhite;
    }
    /* clang-format on */
}

QColor ImageColors::highlight() const
{
    /* clang-format off */
    return_fallback_finally(m_fallbackHighlight, linkColor)

    return m_imageData.m_highlight;
    /* clang-format on */
}

QColor ImageColors::closestToWhite() const
{
    /* clang-format off */
    return_fallback(Qt::white)
    if (qGray(m_imageData.m_closestToWhite.rgb()) < 200) {
        return QColor(230, 230, 230);
    }
    /* clang-format on */

    return m_imageData.m_closestToWhite;
}

QColor ImageColors::closestToBlack() const
{
    /* clang-format off */
    return_fallback(Qt::black)
    if (qGray(m_imageData.m_closestToBlack.rgb()) > 80) {
        return QColor(20, 20, 20);
    }
    /* clang-format on */
    return m_imageData.m_closestToBlack;
}

#include "moc_imagecolors.cpp"
