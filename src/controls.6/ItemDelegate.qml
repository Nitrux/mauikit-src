/*
 *   Copyright 2018 Camilo Higuita <milo.h@aol.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick
import QtQuick.Controls

import org.mauikit.controls as Maui

/**
 * @inherit QtQuick.Controls.Control
 * @brief ItemDelegate is the base for the MauiKit delegate controls.
 * It is radically different from QQC2 ItemDelegate.
 *
 * <a href="https://doc.qt.io/qt-6/qml-qtquick-controls-control.html">This controls inherits from QQC2 Control, to checkout its inherited properties refer to the Qt Docs.</a>
 *
 * This is just a container with some predefined features to allow items using it to be drag and drop, and allow long-press selection to trigger contextual actions.
 *
 * Some of the controls that inherit from this are the GridBrowserDelegate adn ListBrowserDelegate.
 *
 * @section features Features
 *
 * Setting up the drag and drop feature requires a few lines. To start you need to set `draggable: true`, and after that set up what the drag data will contain.:
 *
 * @code
 * ItemDelegate
 * {
 *    draggable: true
 *
 *    Drag.keys: ["text/uri-list"]
 *    Drag.mimeData: { "text/uri-list": "file:/path/one.txt,file:/path/two.txt,file:/path/three.txt" } //a dummy example of three paths in a single string separated by comma.
 * }
 * @endcode
 *
 * Another feature is to react to a long-press - to emulate a "right-click" - sent by a touch gesture in a mobile device, where it could mean a request to launch some contextual action menu for the item.
 *
 * @attention The long-press can either launch the drag-and-drop (DnD) or the contextual request via the `pressAndHold` signal. If after a long press the item is dragged while maintaining the pressed it will launch the DnD action, but if the long-press is released then it will launch the signal `pressAndHold`.
 * @see pressAndHold
 */
Control
{
    id: control
    
    hoverEnabled: !Maui.Handy.isMobile
    
    padding: 0
    
    focus: true
    focusPolicy: Qt.TabFocus
    
    ToolTip.delay: 1000
    ToolTip.timeout: 5000
    ToolTip.visible: control.hovered && control.tooltipText
    ToolTip.text: control._sanitizeTooltip(control.tooltipText)
    
    /**
     * @brief The text used for the tool-tip, revealed when the item is hovered with the mouse cursor.
     * This type of text usually presents to the user with some extra information about the item.
     */
    property string tooltipText

    function _sanitizeTooltip(text)
    {
        const value = text ? text.toString() : ""
        if (value.startsWith("file://"))
            return decodeURIComponent(value.substring(7))

        return value
    }
    
    
    /**
     * @brief The children items of this item will be place by default inside an Item.
     * Ideally there is only one single child element. The children elements need to be positioned manually, using either anchors or coordinates and explicit sizes.
     * @code
     * Maui.ItemDelegate
     * {
    *   Rectangle //the single child element
     *  {
     *      anchors.fill: parent
     *      color: "pink"
     *  }
     * }
     * @endcode
     *
     * @property list<QtObject> ItemDelegate::content
     */
    default property alias content : _content.data

    /**
         * @brief An alias to the MouseArea handling the press events.
         * @note See Qt documentation on the MouseArea for more information.
         * @property MouseArea ItemDelegate::mouseArea
         */
    readonly property alias mouseArea : _mouseArea

    /**
         * @brief Whether the item should respond to a drag event after have been pressed for a long time.
         * If this is set to `true`, after the long press and a drag movement, the item contain will be captured as the Drag image source. And the drag target will be set to enable dropping the element.
         * By default this is set to `false`.
         */
    property bool draggable: false

    property bool _creatingDragPreview: false

    /**
     * @brief An optional image to use instead of generating a drag preview.
     * When empty, the preview is rendered from the delegate content without
     * modifying the live delegate.
     */
    property url dragPreviewSource

    /**
     * @brief The background color used by the generated drag preview.
     */
    property color dragPreviewBackgroundColor: Qt.rgba(control.selectedBackgroundColor.r,
                                                       control.selectedBackgroundColor.g,
                                                       control.selectedBackgroundColor.b,
                                                       1)

    /**
     * @brief The border color used by the generated drag preview.
     */
    property color dragPreviewBorderColor: Qt.darker(Maui.Theme.highlightColor, 1.35)

    /**
     * @brief The border width used by the generated drag preview.
     */
    property int dragPreviewBorderWidth: 1

    /**
         * @brief Whether the item should be styled in a "selected/checked" state.
     * This is kept as a compatibility alias to the canonical `selected` state.
     * @property bool ItemDelegate::isCurrentItem
     */
    property alias isCurrentItem: control.selected

    /**
         * @brief Whether the item is currently being pressed.
         * This is an alias to `mouseArea.containsPress` property.
         * @property bool ItemDelegate::containsPress
         */
    property alias containsPress: _mouseArea.containsPress

    /**
     * @brief Whether the delegate is selected.
     */
    property bool selected: false

    /**
     * @brief Compatibility alias for the selected state.
     */
    property alias highlighted: control.selected

    /**
     * @brief Whether the delegate currently uses its active visual state.
     */
    readonly property bool visuallyActive: control.selected || control.containsPress

    property color normalBackgroundColor: control.flat ? "transparent" : Maui.Theme.alternateBackgroundColor
    property color hoverBackgroundColor: Maui.Theme.hoverColor
    property color selectedBackgroundColor: Maui.Theme.highlightColor
    property color pressedBackgroundColor: control.selectedBackgroundColor

    property color normalForegroundColor: Maui.Theme.textColor
    property color selectedForegroundColor: Maui.ColorUtils.brightnessForColor(control.selectedBackgroundColor) === Maui.ColorUtils.Light
                                            ? "#333333"
                                            : "#fafafa"
    property color pressedForegroundColor: Maui.ColorUtils.brightnessForColor(control.pressedBackgroundColor) === Maui.ColorUtils.Light
                                           ? "#333333"
                                           : "#fafafa"

    readonly property color effectiveBackgroundColor: control.containsPress
                                                       ? control.pressedBackgroundColor
                                                       : (control.selected
                                                          ? control.selectedBackgroundColor
                                                          : (control.hovered
                                                             ? control.hoverBackgroundColor
                                                             : control.normalBackgroundColor))
    readonly property color effectiveForegroundColor: control.containsPress
                                                       ? control.pressedForegroundColor
                                                       : (control.selected
                                                          ? control.selectedForegroundColor
                                                          : control.normalForegroundColor)

    /**
         * @brief The border radius of the background.
         * @By default this is set to `Style.radiusV`, which picks the system preference for the radius of rounded elements corners.
         */
    property int radius:  Maui.Style.radiusV

    /**
         * @brief Whether the item should be styled as a flat element. A flat element usually does not have any selected state or background.
         * By default this property is set to `!Handy.isMobile"
         * @see Handy::isMobile
         */
    property bool flat : !Maui.Handy.isMobile

    /**
         * @brief Emitted when the item has been pressed.
         * @param mouse The object with the event information.
         */
    signal pressed(var mouse)

    /**
         * @brief Emitted when the item has been pressed and hold for a few seconds.
         * @param mouse The object with the event information.
         */
    signal pressAndHold(var mouse)

    /**
         * @brief Emitted when the item has been clicked - this means that the item has been pressed and then released.
         * @param mouse The object with the event information.
         */
    signal clicked(var mouse)

    /**
         * @brief Emitted when the item has been right clicked. Usually with a mouse device.
         * @param mouse The object with the event information.
         */
    signal rightClicked(var mouse)

    /**
         * @brief Emitted when the item has been double clicked in a short period of time.
         * @param mouse The object with the event information.
         */
    signal doubleClicked(var mouse)

    Drag.active: false
    Drag.dragType: Drag.Automatic
    //     Drag.supportedActions: Qt.MoveAction
    Drag.hotSpot.x: control.width / 2
    Drag.hotSpot.y: control.height / 2

    Connections
    {
        target: mouseArea.drag

        function onActiveChanged()
        {
            if (control.draggable && mouseArea.drag.active)
                control._captureDragPreview()

            control.Drag.active = control.draggable && mouseArea.drag.active
        }
    }

    onDraggableChanged:
    {
        if (!control.draggable)
        {
            control.Drag.active = false
        }
    }

    function _captureDragPreview()
    {
        if (!control.draggable)
            return

        if (control.dragPreviewSource.toString().length > 0)
        {
            control.Drag.imageSource = control.dragPreviewSource
            return
        }

        control._creatingDragPreview = true
        const preview = _dragPreviewLoader.item
        if (!preview)
        {
            control._creatingDragPreview = false
            return
        }

        const started = preview.grabToImage(function(result)
        {
            control.Drag.imageSource = result.url
            control._creatingDragPreview = false
        }, Qt.size(Math.ceil(control.width), Math.ceil(control.height)))

        if (!started)
            control._creatingDragPreview = false
    }

    readonly property alias _dragPreview: _dragPreviewLoader.item

    Loader
    {
        id: _dragPreviewLoader
        active: control.draggable && control._creatingDragPreview
        sourceComponent: Rectangle
        {
            x: -width - 1
            y: -height - 1
            width: control.width
            height: control.height

            color: control.dragPreviewBackgroundColor
            radius: control.radius
            clip: true

            ShaderEffectSource
            {
                x: control.contentItem.x
                y: control.contentItem.y
                width: control.contentItem.width
                height: control.contentItem.height

                sourceItem: control.contentItem
                sourceRect: Qt.rect(0, 0, control.contentItem.width, control.contentItem.height)
                live: control._creatingDragPreview
                recursive: true
                hideSource: false
            }

            Rectangle
            {
                anchors.fill: parent

                color: "transparent"
                radius: parent.radius
                border.width: control.dragPreviewBorderWidth
                border.color: control.dragPreviewBorderColor
                antialiasing: true
            }
        }
    }


    contentItem: Item
    {
        id: _content

        SequentialAnimation on scale
        {
            id: xAnim
            // Animations on properties start running by default
            running: false
            loops: 3
            NumberAnimation { from: 1; to: 0.97; duration: 200; easing.type: Easing.InBack }
            NumberAnimation { from: 0.97; to: 1; duration: 200; easing.type: Easing.InBack }
            PauseAnimation { duration: 50 } // This puts a bit of time between the loop
        }

        MouseArea
        {
            id: _mouseArea
            anchors.fill: parent
            
            propagateComposedEvents: false
            acceptedButtons: Qt.RightButton | Qt.LeftButton
            drag.threshold: 100
            drag.target: null
            property bool deferredPressAndHold: false
            
            onClicked: (mouse) =>
            {
                if (mouse.button === Qt.RightButton)
                    control.rightClicked(mouse)
                else
                    control.clicked(mouse)
            }
            
            onDoubleClicked: (mouse) => control.doubleClicked(mouse)
            
            onPressed: (mouse) =>
            {
                if (control.draggable && mouse.source !== Qt.MouseEventSynthesizedByQt)
                {
                    drag.target = _mouseArea
                    control.Drag.imageSource = ""
                } else {
                    drag.target = null
                }

                deferredPressAndHold = false
                control.pressed(mouse)
            }
            
            onReleased: (mouse) =>
            {
                if (deferredPressAndHold)
                {
                    control.pressAndHold(mouse)
                    deferredPressAndHold = false
                }
            }
            
            onPressAndHold: (mouse) =>
            {
                xAnim.running = control.draggable || mouse.source === Qt.MouseEventSynthesizedByQt

                if (control.draggable && mouse.source === Qt.MouseEventSynthesizedByQt)
                {
                    deferredPressAndHold = true
                    drag.target = _mouseArea
                    control.Drag.imageSource = ""
                } else {
                    deferredPressAndHold = false
                    drag.target = null
                    control.pressAndHold(mouse)
                }
            }
        }
    }

    background: Rectangle
    {
        color: control.effectiveBackgroundColor
        radius: control.radius
    }
}
