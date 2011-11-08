/*
*   Copyright (C) 2011 by Daker Fernandes Pinheiro <dakerfp@gmail.com>
*   Copyright (C) 2011 by Marco Martin <mart@kde.org>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License as
*   published by the Free Software Foundation; either version 2, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU Library General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore

Item {
    id: button

    // Commmon API
    property bool flat: true
    property bool checked: false
    property bool checkable: false
    property alias pressed: mouse.pressed
    property alias text: label.text
    property alias iconSource: icon.source
    property alias font: label.font

    signal clicked()


    onFlatChanged: {
        surface.opacity = 1
    }


    implicitWidth: Math.max(50, icon.width + label.paintedWidth + surface.margins.left + surface.margins.right)
    implicitHeight: Math.max(20, Math.max(icon.height, label.paintedHeight) + surface.margins.top + surface.margins.bottom)

    // TODO: needs to define if there will be specific graphics for
    //     disabled buttons
    opacity: enabled ? 1.0 : 0.5

    Keys.onSpacePressed: internal.pressButton()
    Keys.onReturnPressed: internal.pressButton()
    Keys.onReleased: {
        if (event.key == Qt.Key_Space ||
            event.key == Qt.Key_Return)
            internal.releaseButton()
    }

    onActiveFocusChanged: {
        if (activeFocus) {
            shadow.state = "focus"
        } else if (checked) {
            shadow.state = "hidden"
        } else {
            shadow.state = "shadow"
        }
    }

    QtObject {
        id: internal
        property bool userPressed: false

        function pressButton()
        {
            userPressed = true
        }

        function releaseButton()
        {
            userPressed = false
            if (!button.enabled) {
                return
            }

            if (button.checkable) {
                button.checked = !button.checked
            }

            button.clicked()
            button.forceActiveFocus()
        }
    }

    ButtonShadow {
        id: shadow
        anchors.fill: parent
        visible: !flat
    }

    PlasmaCore.FrameSvgItem {
        id: surface

        anchors.fill: parent
        imagePath: "widgets/button"
        prefix: (internal.userPressed || checked) ? "pressed" : "normal"
        //internal: if there is no hover status, don't paint on mouse over in touchscreens
        opacity: (internal.userPressed || checked || !flat || (shadow.hasOverState && mouse.containsMouse)) ? 1 : 0
        Behavior on opacity {
            PropertyAnimation { duration: 250 }
        }
    }

    Item {
        anchors {
            fill: parent
            leftMargin: surface.margins.left
            topMargin: surface.margins.top
            rightMargin: surface.margins.right
            bottomMargin: surface.margins.bottom
        }

        IconLoader {
            id: icon

            anchors {
                verticalCenter: parent.verticalCenter
                left: label.text ? parent.left : undefined
                horizontalCenter: label.text ? undefined : parent.horizontalCenter
            }
        }

        Text {
            id: label

            anchors {
                top: parent.top
                bottom: parent.bottom
                left: icon.valid ? icon.right : parent.left
                right: parent.right
            }
            font.capitalization: theme.defaultFont.capitalization
            font.family: theme.defaultFont.family
            font.italic: theme.defaultFont.italic
            font.letterSpacing: theme.defaultFont.letterSpacing
            font.pointSize: theme.defaultFont.pointSize
            font.strikeout: theme.defaultFont.strikeout
            font.underline: theme.defaultFont.underline
            font.weight: theme.defaultFont.weight
            font.wordSpacing: theme.defaultFont.wordSpacing
            color: theme.buttonTextColor
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    MouseArea {
        id: mouse

        anchors.fill: parent
        hoverEnabled: true

        onPressed: internal.pressButton();

        onReleased: internal.releaseButton();

        onEntered: {
            if (!flat) {
                shadow.state = "hover"
            }
        }
        onExited: {
            if (!flat) {
                if (button.activeFocus) {
                    shadow.state = "focus"
                } else if (checked) {
                    shadow.state = "hidden"
                } else {
                    shadow.state = "shadow"
                }
            }
        }
    }
}
