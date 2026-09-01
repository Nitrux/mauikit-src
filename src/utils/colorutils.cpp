/*
 *  SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "colorutils.h"
#include "imagecolors.h"

#include <QIcon>
#include <QtMath>
#include <cmath>
#include <map>

ColorUtils::ColorUtils(QObject *parent)
    : QObject(parent)
{
}

ColorUtils::Brightness ColorUtils::brightnessForColor(const QColor &color)
{
    auto luma = [](const QColor &color) {
        return (0.299 * color.red() + 0.587 * color.green() + 0.114 * color.blue()) / 255;
    };

    return luma(color) > 0.5 ? ColorUtils::Brightness::Light : ColorUtils::Brightness::Dark;
}

qreal ColorUtils::grayForColor(const QColor &color)
{
    return (0.299 * color.red() + 0.587 * color.green() + 0.114 * color.blue()) / 255;
}

QColor ColorUtils::alphaBlend(const QColor &foreground, const QColor &background)
{
    const auto foregroundAlpha = foreground.alpha();
    const auto inverseForegroundAlpha = 0xff - foregroundAlpha;
    const auto backgroundAlpha = background.alpha();

    if (foregroundAlpha == 0x00) {
        return background;
    }

    if (backgroundAlpha == 0xff) {
        return QColor::fromRgb((foregroundAlpha * foreground.red()) + (inverseForegroundAlpha * background.red()),
                               (foregroundAlpha * foreground.green()) + (inverseForegroundAlpha * background.green()),
                               (foregroundAlpha * foreground.blue()) + (inverseForegroundAlpha * background.blue()),
                               0xff);
    } else {
        const auto inverseBackgroundAlpha = (backgroundAlpha * inverseForegroundAlpha) / 255;
        const auto finalAlpha = foregroundAlpha + inverseBackgroundAlpha;
        Q_ASSERT(finalAlpha != 0x00);
        return QColor::fromRgb((foregroundAlpha * foreground.red()) + (inverseBackgroundAlpha * background.red()),
                               (foregroundAlpha * foreground.green()) + (inverseBackgroundAlpha * background.green()),
                               (foregroundAlpha * foreground.blue()) + (inverseBackgroundAlpha * background.blue()),
                               finalAlpha);
    }
}

QColor ColorUtils::linearInterpolation(const QColor &one, const QColor &two, double balance)
{
   auto scaleAlpha = [](const QColor &color, double factor) {
        return QColor::fromRgb(color.red(), color.green(), color.blue(), color.alpha() * factor);
    };
    auto linearlyInterpolateDouble = [](double one, double two, double factor) {
        return one + (two - one) * factor;
    };

    if (one == Qt::transparent) {
        return scaleAlpha(two, balance);
    }
    if (two == Qt::transparent) {
        return scaleAlpha(one, 1 - balance);
    }

    return QColor::fromHsv(std::fmod(linearlyInterpolateDouble(one.hue(), two.hue(), balance), 360.0),
                           qBound(0.0, linearlyInterpolateDouble(one.saturation(), two.saturation(), balance), 255.0),
                           qBound(0.0, linearlyInterpolateDouble(one.value(), two.value(), balance), 255.0),
                           qBound(0.0, linearlyInterpolateDouble(one.alpha(), two.alpha(), balance), 255.0));

}

// Some private things for the adjust, change, and scale properties
struct ParsedAdjustments {
    double red = 0.0;
    double green = 0.0;
    double blue = 0.0;

    double hue = 0.0;
    double saturation = 0.0;
    double value = 0.0;

    double alpha = 0.0;
};

ParsedAdjustments parseAdjustments(const QJSValue &value)
{
    ParsedAdjustments parsed;

    auto checkProperty = [](const QJSValue &value, const QString &property) {
        if (value.hasProperty(property)) {
            auto val = value.property(property);
            if (val.isNumber()) {
                return QVariant::fromValue(val.toNumber());
            }
        }
        return QVariant();
    };

    std::vector<std::pair<QString, double &>> items{{QStringLiteral("red"), parsed.red},
                                                    {QStringLiteral("green"), parsed.green},
                                                    {QStringLiteral("blue"), parsed.blue},
                                                    //
                                                    {QStringLiteral("hue"), parsed.hue},
                                                    {QStringLiteral("saturation"), parsed.saturation},
                                                    {QStringLiteral("value"), parsed.value},
                                                    {QStringLiteral("lightness"), parsed.value},
                                                    //
                                                    {QStringLiteral("alpha"), parsed.alpha}};

    for (const auto &item : items) {
        auto val = checkProperty(value, item.first);
        if (val.isValid()) {
            item.second = val.toDouble();
        }
    }

    if ((parsed.red || parsed.green || parsed.blue) && (parsed.hue || parsed.saturation || parsed.value)) {
        qCritical()  << "It is an error to have both RGB and HSL values in an adjustment.";
    }

    return parsed;
}

QColor ColorUtils::adjustColor(const QColor &color, const QJSValue &adjustments)
{
    auto adjusts = parseAdjustments(adjustments);

    if (qBound(-360.0, adjusts.hue, 360.0) != adjusts.hue) {
        qCritical() << "Hue is out of bounds";
    }
    if (qBound(-255.0, adjusts.red, 255.0) != adjusts.red) {
        qCritical()  << "Red is out of bounds";
    }
    if (qBound(-255.0, adjusts.green, 255.0) != adjusts.green) {
        qCritical()  << "Green is out of bounds";
    }
    if (qBound(-255.0, adjusts.blue, 255.0) != adjusts.blue) {
        qCritical()  << "Green is out of bounds";
    }
    if (qBound(-255.0, adjusts.saturation, 255.0) != adjusts.saturation) {
        qCritical()  << "Saturation is out of bounds";
    }
    if (qBound(-255.0, adjusts.value, 255.0) != adjusts.value) {
        qCritical()  << "Value is out of bounds";
    }
    if (qBound(-255.0, adjusts.alpha, 255.0) != adjusts.alpha) {
        qCritical()  << "Alpha is out of bounds";
    }

    auto copy = color;

    if (adjusts.alpha) {
        copy.setAlpha(adjusts.alpha);
    }

    if (adjusts.red || adjusts.green || adjusts.blue) {
        copy.setRed(copy.red() + adjusts.red);
        copy.setGreen(copy.green() + adjusts.green);
        copy.setBlue(copy.blue() + adjusts.blue);
    } else if (adjusts.hue || adjusts.saturation || adjusts.value) {
        copy.setHsl(std::fmod(copy.hue() + adjusts.hue, 360.0), //
                    copy.saturation() + adjusts.saturation, //
                    copy.value() + adjusts.value,
                    copy.alpha());
    }

    return copy;
}

QColor ColorUtils::scaleColor(const QColor &color, const QJSValue &adjustments)
{
    auto adjusts = parseAdjustments(adjustments);
    auto copy = color;

    if (qBound(-100.0, adjusts.red, 100.00) != adjusts.red) {
        qCritical()  << "Red is out of bounds";
    }
    if (qBound(-100.0, adjusts.green, 100.00) != adjusts.green) {
        qCritical()  << "Green is out of bounds";
    }
    if (qBound(-100.0, adjusts.blue, 100.00) != adjusts.blue) {
        qCritical()  << "Blue is out of bounds";
    }
    if (qBound(-100.0, adjusts.saturation, 100.00) != adjusts.saturation) {
        qCritical()  << "Saturation is out of bounds";
    }
    if (qBound(-100.0, adjusts.value, 100.00) != adjusts.value) {
        qCritical()  << "Value is out of bounds";
    }
    if (qBound(-100.0, adjusts.alpha, 100.00) != adjusts.alpha) {
        qCritical()  << "Alpha is out of bounds";
    }

    if (adjusts.hue != 0) {
        qCritical()  << "Hue cannot be scaled";
    }

    auto shiftToAverage = [](double current, double factor) {
        auto scale = qBound(-100.0, factor, 100.0) / 100;
        return current + (scale > 0 ? 255 - current : current) * scale;
    };

    if (adjusts.red || adjusts.green || adjusts.blue) {
        copy.setRed(qBound(0.0, shiftToAverage(copy.red(), adjusts.red), 255.0));
        copy.setGreen(qBound(0.0, shiftToAverage(copy.green(), adjusts.green), 255.0));
        copy.setBlue(qBound(0.0, shiftToAverage(copy.blue(), adjusts.blue), 255.0));
    } else {
        copy.setHsl(copy.hue(),
                    qBound(0.0, shiftToAverage(copy.saturation(), adjusts.saturation), 255.0),
                    qBound(0.0, shiftToAverage(copy.value(), adjusts.value), 255.0),
                    qBound(0.0, shiftToAverage(copy.alpha(), adjusts.alpha), 255.0));
    }

    return copy;
}

QColor ColorUtils::tintWithAlpha(const QColor &targetColor, const QColor &tintColor, double alpha)
{
    qreal tintAlpha = tintColor.alphaF() * alpha;
    qreal inverseAlpha = 1.0 - tintAlpha;

    if (qFuzzyCompare(tintAlpha, 1.0)) {
        return tintColor;
    } else if (qFuzzyIsNull(tintAlpha)) {
        return targetColor;
    }

    return QColor::fromRgbF(tintColor.redF() * tintAlpha + targetColor.redF() * inverseAlpha,
                            tintColor.greenF() * tintAlpha + targetColor.greenF() * inverseAlpha,
                            tintColor.blueF() * tintAlpha + targetColor.blueF() * inverseAlpha,
                            tintAlpha + inverseAlpha * targetColor.alphaF());
}

ColorUtils::XYZColor ColorUtils::colorToXYZ(const QColor &color)
{
    // http://wiki.nuaj.net/index.php/Color_Transforms#RGB_.E2.86.92_XYZ
    qreal r = color.redF();
    qreal g = color.greenF();
    qreal b = color.blueF();
    // Apply gamma correction (i.e. conversion to linear-space)
    auto correct = [](qreal &v) {
        if (v > 0.04045) {
            v = std::pow((v + 0.055) / 1.055, 2.4);
        } else {
            v = v / 12.92;
        }
    };

    correct(r);
    correct(g);
    correct(b);

    // Observer. = 2°, Illuminant = D65
    const qreal x = r * 0.4124 + g * 0.3576 + b * 0.1805;
    const qreal y = r * 0.2126 + g * 0.7152 + b * 0.0722;
    const qreal z = r * 0.0193 + g * 0.1192 + b * 0.9505;

    return XYZColor{x, y, z};
}

ColorUtils::LabColor ColorUtils::colorToLab(const QColor &color)
{
    // First: convert to XYZ
    const auto xyz = colorToXYZ(color);

    // Second: convert from XYZ to L*a*b
    qreal x = xyz.x / 0.95047; // Observer= 2°, Illuminant= D65
    qreal y = xyz.y / 1.0;
    qreal z = xyz.z / 1.08883;

    auto pivot = [](qreal &v) {
        if (v > 0.008856) {
            v = std::pow(v, 1.0 / 3.0);
        } else {
            v = (7.787 * v) + (16.0 / 116.0);
        }
    };

    pivot(x);
    pivot(y);
    pivot(z);

    LabColor labColor;
    labColor.l = std::max(0.0, (116 * y) - 16);
    labColor.a = 500 * (x - y);
    labColor.b = 200 * (y - z);

    return labColor;
}

qreal ColorUtils::chroma(const QColor &color)
{
    LabColor labColor = colorToLab(color);

    // Chroma is hypotenuse of a and b
    return sqrt(pow(labColor.a, 2) + pow(labColor.b, 2));
}

qreal ColorUtils::luminance(const QColor &color)
{
    const auto &xyz = colorToXYZ(color);
    // Luminance is equal to Y
    return xyz.y;
}

MauiKit::AdaptivePalette MauiKit::AdaptivePalette::fromImage(const QImage &image)
{
    if (image.isNull())
        return {};

    const QImage sample = image.width() > 128 || image.height() > 128
        ? image.scaled(128, 128, Qt::KeepAspectRatio, Qt::SmoothTransformation)
        : image;
    return fromImageData(ImageColors::generatePalette(sample));
}

MauiKit::AdaptivePalette MauiKit::AdaptivePalette::fromImageData(const ImageData &imageData)
{
    MauiKit::AdaptivePalette result;
    if (imageData.m_samples.isEmpty() || imageData.m_dominant.isValid() == false || imageData.m_highlight.isValid() == false)
        return result;

    ColorUtils colorUtils;
    const bool isDark = qGray(imageData.m_dominant.rgb()) < 128;
    const auto closestToWhite = [&imageData]() {
        if (qGray(imageData.m_closestToWhite.rgb()) < 200)
            return QColor(230, 230, 230);
        return imageData.m_closestToWhite;
    };
    const auto closestToBlack = [&imageData]() {
        if (qGray(imageData.m_closestToBlack.rgb()) > 80)
            return QColor(20, 20, 20);
        return imageData.m_closestToBlack;
    };
    const QColor foreground = isDark ? closestToWhite() : closestToBlack();
    const QColor imageBackground = isDark ? closestToBlack() : closestToWhite();
    const QColor highlight = imageData.m_highlight;
    const QColor backgroundBase = isDark ? QColor("#27292a") : QColor("#e8e8e8");
    const QColor buttonBase = isDark ? QColor("#4c5052") : QColor("#ffffff");
    const QColor viewBase = isDark ? QColor("#0a0b0b") : QColor("#fafafa");
    const QColor viewAlternateBase = isDark ? QColor("#1a1e1e") : QColor("#f0f0f0");
    const QColor viewHoverBase = isDark ? QColor("#1f1f1f") : QColor("#e5e5e5");
    const QColor hoverBase = isDark ? QColor("#202727") : QColor("#dbdbdb");
    const QColor buttonHoverBase = isDark ? QColor("#7d8487") : QColor("#f2f2f2");

    result.valid = true;
    result.textColor = foreground;
    result.disabledTextColor = foreground.lighter(120);
    result.highlightColor = highlight;
    result.highlightedTextColor = colorUtils.brightnessForColor(highlight) == ColorUtils::Dark ? closestToWhite() : closestToBlack();
    const QColor background = colorUtils.tintWithAlpha(backgroundBase, imageBackground, 0.1);
    result.backgroundColor = colorUtils.tintWithAlpha(background, highlight, 0.03);
    result.activeBackgroundColor = highlight;
    result.alternateBackgroundColor = colorUtils.tintWithAlpha(result.backgroundColor, highlight, 0.02);
    result.hoverColor = colorUtils.tintWithAlpha(hoverBase, highlight, 0.02);
    result.focusColor = highlight;
    result.activeTextColor = highlight;

    result.buttonTextColor = foreground;
    result.buttonBackgroundColor = colorUtils.tintWithAlpha(buttonBase, highlight, 0.06);
    result.buttonAlternateBackgroundColor = colorUtils.tintWithAlpha(buttonBase, highlight, 0.03);
    result.buttonHoverColor = colorUtils.tintWithAlpha(buttonHoverBase, highlight, 0.03);
    result.buttonFocusColor = highlight;

    result.viewTextColor = foreground;
    result.viewBackgroundColor = colorUtils.tintWithAlpha(viewBase, highlight, 0.07);
    result.viewAlternateBackgroundColor = colorUtils.tintWithAlpha(viewAlternateBase, highlight, 0.03);
    result.viewHoverColor = colorUtils.tintWithAlpha(viewHoverBase, highlight, 0.03);
    result.viewFocusColor = highlight;

    result.selectionTextColor = QColor("#fcfcfc");
    result.selectionBackgroundColor = highlight;
    result.selectionAlternateBackgroundColor = highlight.darker();
    result.selectionHoverColor = highlight.lighter();
    result.selectionFocusColor = highlight;

    result.complementaryTextColor = QColor("#eff0f1");
    result.complementaryBackgroundColor = colorUtils.tintWithAlpha(QColor("#31363b"), highlight, 0.03);
    result.complementaryAlternateBackgroundColor = result.complementaryBackgroundColor.darker();
    result.complementaryHoverColor = result.complementaryBackgroundColor.lighter();
    result.complementaryFocusColor = highlight;

    result.headerTextColor = foreground;
    result.headerBackgroundColor = colorUtils.tintWithAlpha(background, highlight, 0.05);
    result.headerAlternateBackgroundColor = colorUtils.tintWithAlpha(background, highlight, 0.02);
    result.headerHoverColor = result.headerBackgroundColor.lighter();
    result.headerFocusColor = highlight;

    result.linkColor = QColor("#2980B9");
    result.linkBackgroundColor = QColor("#2980B9");
    result.visitedLinkColor = QColor("#7F8C8D");
    result.visitedLinkBackgroundColor = QColor("#2196F3");
    result.negativeTextColor = QColor("#dac7cb");
    result.negativeBackgroundColor = QColor("#DA4453");
    result.neutralTextColor = QColor("#fafafa");
    result.neutralBackgroundColor = QColor("#F67400");
    result.positiveTextColor = QColor("#fafafa");
    result.positiveBackgroundColor = QColor("#27AE60");

    result.tooltipTextColor = QColor("#fafafa");
    result.tooltipBackgroundColor = QColor("#333");
    result.tooltipAlternateBackgroundColor = result.tooltipBackgroundColor.darker();
    result.tooltipHoverColor = QColor("#000");
    result.tooltipFocusColor = QColor("#000");

    return result;
}

#include "moc_colorutils.cpp"
