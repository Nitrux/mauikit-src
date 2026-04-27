/*
 * Copyright 2017 Marco Martin <mart@kde.org>
 * Copyright 2017 The Qt Company Ltd.
 *
 * GNU Lesser General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU Lesser
 * General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.LGPLv3 included in the
 * packaging of this file. Please review the following information to
 * ensure the GNU Lesser General Public License version 3 requirements
 * will be met: https://www.gnu.org/licenses/lgpl.html.
 *
 * GNU General Public License Usage
 * Alternatively, this file may be used under the terms of the GNU
 * General Public License version 2.0 or later as published by the Free
 * Software Foundation and appearing in the file LICENSE.GPL included in
 * the packaging of this file. Please review the following information to
 * ensure the GNU General Public License version 2.0 requirements will be
 * met: http://www.gnu.org/licenses/gpl-2.0.html.
 */


import QtQuick
import QtQuick.Templates as T
import org.mauikit.controls as Maui

T.CheckBox 
{
    id: control
    opacity: enabled ? 1 : 0.5
    focus: true
    focusPolicy: Qt.TabFocus
    readonly property bool hasText: control.text.length > 0
    readonly property bool hasIcon: !!(control.icon.name || control.icon.source)
    readonly property real indicatorImplicitWidth: indicator ? indicator.implicitWidth : 0
    readonly property real indicatorImplicitHeight: indicator ? indicator.implicitHeight : 0
    readonly property real iconImplicitWidth: hasIcon ? control.icon.width : 0
    readonly property real iconImplicitHeight: hasIcon ? control.icon.height : 0
    readonly property real contentSpacing: hasText && hasIcon ? control.spacing : 0
    readonly property real contentImplicitWidth: iconImplicitWidth + contentSpacing + (hasText ? _textMetrics.advanceWidth : 0)
    readonly property real contentImplicitHeight: Math.max(iconImplicitHeight, hasText ? _textMetrics.height : 0)
    implicitWidth: indicatorImplicitWidth + ((hasText || hasIcon) && indicator ? control.spacing : 0) + contentImplicitWidth + leftPadding + rightPadding
    implicitHeight: Math.max(contentImplicitHeight, indicatorImplicitHeight) + topPadding + bottomPadding
    
    padding: Maui.Style.defaultPadding
    spacing: Maui.Style.space.small
    
    hoverEnabled: true
    icon.color: control.down || control.pressed || control.checked ? Maui.Theme.highlightedTextColor : Maui.Theme.textColor
    icon.width: Maui.Style.iconSize
    icon.height: Maui.Style.iconSize

    TextMetrics
    {
        id: _textMetrics
        font: control.font
        text: control.text
    }
    
    indicator: CheckIndicator
    {
        x: control.text ? (control.mirrored ? control.width - width - control.rightPadding : control.leftPadding) : control.leftPadding + (control.availableWidth - width) / 2
        y: control.topPadding + (control.availableHeight - height) / 2
        control: control
    }
    
    contentItem: Maui.IconLabel
    {
        leftPadding: control.indicator && !control.mirrored ? control.indicator.implicitWidth + control.spacing : 0
        rightPadding: control.indicator && control.mirrored ? control.indicator.implicitWidth + control.spacing : 0
        text: control.text
        font: control.font
        // elide: Text.ElideRight
        //        visible: control.text
        // horizontalAlignment: Text.AlignLeft
        // verticalAlignment: Text.AlignVCenter
        color: control.icon.color
        icon: control.icon
    }
    
    background: Rectangle
    {
        visible: !control.flat
        
        color: control.pressed || control.down || control.checked ? control.Maui.Theme.highlightColor : (control.highlighted || control.hovered ? control.Maui.Theme.hoverColor : Maui.Theme.backgroundColor)        
        
        radius: Maui.Style.radiusV
        
        Behavior on color
        {
            Maui.ColorTransition{}
        }
        
        Behavior on border.color
        {
            Maui.ColorTransition{}
        }
        border.color: statusColor(control)
        
        function statusColor(control)
        {
            if(control.Maui.Controls.status)
            {
                switch(control.Maui.Controls.status)
                {
                    case Maui.Controls.Positive: return control.Maui.Theme.positiveBackgroundColor
                    case Maui.Controls.Negative: return control.Maui.Theme.negativeBackgroundColor
                    case Maui.Controls.Neutral: return control.Maui.Theme.neutralBackgroundColor
                    case Maui.Controls.Normal:
                    default:
                        return "transparent"
                }
            }
            
            return "transparent"
        }
    }
}
