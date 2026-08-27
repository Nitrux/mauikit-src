import QtQuick
import QtQuick.Shapes

Shape
{
    id: control

    layer.enabled: GraphicsInfo.api !== GraphicsInfo.Software && smooth
    layer.samples: 4
    smooth: true

    property Item item
    property color color: item ? item.color : "transparent"
    readonly property real arrowLeft: width / 8
    readonly property real arrowTop: height * 3 / 8
    readonly property real arrowWidth: width * 3 / 4
    readonly property real arrowHeight: height / 2

    x: item.mirrored ? item.leftPadding : item.width - width - item.rightPadding
    y: item.topPadding + (item.availableHeight - height) / 2
    visible: false
    height: 8
    width: 8

    ShapePath
    {
        fillColor: control.color
        strokeColor: "transparent"
        startX: control.arrowLeft
        startY: control.arrowTop
        PathLine { x: control.arrowLeft + control.arrowWidth; y: control.arrowTop }
        PathLine { x: control.arrowLeft + control.arrowWidth / 2; y: control.arrowTop + control.arrowHeight }
        PathLine { x: control.arrowLeft; y: control.arrowTop }
    }
}
