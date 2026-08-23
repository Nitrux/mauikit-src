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

import QtQuick as Q
import QtQml
import QtQuick.Window
import QtQuick.Controls as QQC

import org.mauikit.controls as Maui
import QtQuick.Effects

/**
 * @inherit QtQuick.Controls.Popup
 * @brief A centered popup with responsive sizing and MauiKit styling.
 *
 * By default, Popup sizes itself from widthHint and heightHint while respecting
 * maxWidth and maxHeight. Set filling to make it occupy its parent and remove
 * the surrounding margins. Focus is restored to the previously focused item
 * after the popup closes, provided that item is still visible and enabled.
 */
QQC.Popup
{
    id: control

    objectName: "MauiKit Popup"

    property Q.Item _focusItemBeforeOpen: null
    property QtObject _focusRestoration: Connections
    {
        target: control

        function onAboutToShow()
        {
            const window = control.parent ? control.parent.Window.window : null
            control._focusItemBeforeOpen = window ? window.activeFocusItem : null
        }

        function onClosed()
        {
            const item = control._focusItemBeforeOpen
            control._focusItemBeforeOpen = null

            if (!item)
                return

            Qt.callLater(() => {
                if (!item)
                    return

                var ancestor = item
                while (ancestor)
                {
                    if (ancestor.visible === false || ancestor.enabled === false)
                        return

                    ancestor = ancestor.parent
                }

                item.forceActiveFocus()
            })
        }
    }

    width: (filling ? parent.width  : mWidth)
    height: (filling ? parent.height : mHeight)

    anchors.centerIn: parent
    
    Q.Behavior on width
    {
        enabled: control.hint === 1
        
        Q.NumberAnimation
        {
            duration: Maui.Style.units.shortDuration
            easing.type: Q.Easing.InOutQuad
        }
    }
    
    Q.Behavior on height
    {
        enabled: control.hint === 1
        
        Q.NumberAnimation
        {
            duration: Maui.Style.units.shortDuration
            easing.type: Q.Easing.InOutQuad
        }
    }
    
    readonly property int mWidth: Math.round(Math.min(control.parent.width * widthHint, maxWidth))
    readonly property int mHeight: Math.round(Math.min(control.parent.height * heightHint, maxHeight))
    
    margins: filling ? 0 : Maui.Style.space.medium
        
    /**
     * @property bool Popup::filling
     * Whether the popup fills its parent. When true, width and height hints,
     * maximum dimensions, margins, and corner rounding are not applied.
     * The default is false.
     */
    property bool filling : false

    /**
     * @property list<QtObject> Popup::content
     * The visual items displayed inside the popup.
     */
    default property alias content : _content.data

    /**
     * @property int Popup::maxWidth
     * The maximum width used when filling is false. The default is 700 pixels.
     */
    property int maxWidth : 700

    /**
     * @property int Popup::maxHeight
     * The maximum height used when filling is false. The default is 400 pixels.
     */
    property int maxHeight : 400

    /**
     * @property real Popup::hint
     * The default parent-size proportion used by widthHint and heightHint.
     * The default is 0.9.
     */
    property double hint : 0.9

    /**
     * @property real Popup::heightHint
     * The proportion of the parent height requested before maxHeight is
     * applied. The default follows hint.
     */
    property double heightHint: hint

    /**
     * @property real Popup::widthHint
     * The proportion of the parent width requested before maxWidth is applied.
     * The default follows hint.
     */
    property double widthHint: hint

    contentItem: Q.Item
    {
        id: _content
        objectName: "Popup Container"

        Maui.Theme.colorSet: control.Maui.Theme.colorSet
        Maui.Theme.inherit: control.Maui.Theme.inherit

        layer.enabled: Q.GraphicsInfo.api !== Q.GraphicsInfo.Software
        layer.effect: MultiEffect
        {
            maskEnabled: true
            maskThresholdMin: 0.5
            maskSpreadAtMin: 1.0
            maskSpreadAtMax: 0.0
            maskThresholdMax: 1.0
            maskSource: Q.ShaderEffectSource
            {
                sourceItem: Q.Rectangle
                {
                    width: _content.width
                    height: _content.height
                    radius:  control.filling ? 0 : Maui.Style.radiusV
                }
            }
        }
    }

    Maui.Controls.flat: control.filling
}
