import QtQuick
import org.mauikit.controls as Maui

Item
{
    id: indicator

    implicitWidth: Maui.Style.iconSize + 2
    implicitHeight: Maui.Style.iconSize + 2

    property Item targetControl

    Rectangle
    {
        id: _outerCircle
        anchors.fill: parent
        radius: width / 2
        color: targetControl && targetControl.checked ? Maui.Theme.highlightColor : Maui.Theme.backgroundColor
        border.width: 2
        border.color: targetControl && targetControl.checked ? Maui.Theme.highlightedTextColor : Maui.ColorUtils.linearInterpolation(Maui.Theme.alternateBackgroundColor, Maui.Theme.textColor, 0.2)

        Rectangle
        {
            anchors.fill: parent
            anchors.margins: _outerCircle.border.width + 2
            radius: width / 2
            visible: targetControl && targetControl.checked
            color: _outerCircle.border.color
        }
    }
}