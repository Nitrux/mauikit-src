import QtQuick
import QtQuick.Controls

import org.mauikit.controls as Maui

Maui.ShadowedRectangle
{
    id: control

    property string title

    signal closeRequested()

    implicitHeight: 28
    height: 28
    color: Maui.Theme.alternateBackgroundColor
    clip: true

    corners
    {
        topLeftRadius: Maui.Style.radiusV
        topRightRadius: Maui.Style.radiusV
        bottomLeftRadius: 0
        bottomRightRadius: 0
    }

    Label
    {
        anchors.centerIn: parent
        text: control.title
        font.bold: true
        horizontalAlignment: Text.AlignHCenter
    }

    Maui.CloseButton
    {
        anchors.right: parent.right
        anchors.rightMargin: Maui.Style.space.small
        anchors.verticalCenter: parent.verticalCenter
        implicitWidth: 18
        implicitHeight: 18
        width: 18
        height: 18
        padding: 0
        icon.width: 18
        icon.height: 18
        onClicked: control.closeRequested()
    }

    Maui.Separator
    {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        weight: Maui.Separator.Weight.Light
    }
}
