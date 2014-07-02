import QtQuick 2.2
import QtQuick.Window 2.1
import QtQuick.Dialogs 1.0
import QtQuick.Controls 1.0
import QtQuick.Controls.Styles 1.0
import QtQuick.Layouts 1.0

Item {
    id: root

    property string software: "Feather"
    property real version: 0.1
    property string title: software + " v" + version

    ApplicationWindow {
        id: main_window
        visibility: Window.Maximized
        visible: true
        title: "Feather"

        // splash screen
        //Window { id: splash; width: 600; height: 347; visible: true; flags: Qt.SplashScreen; Image { anchors.fill: parent; source: "./icons/splash.jpg" } }

        menuBar: MainMenu {}

        toolBar: ToolBar { id: toolbar }

        statusBar: StatusBar { id: status_bar; Label { text: "Welcome to " + root.software + " v" + root.version } }

        Viewport3D {
            anchors.fill: parent
        }

        Component.onCompleted: { }


    }

}
