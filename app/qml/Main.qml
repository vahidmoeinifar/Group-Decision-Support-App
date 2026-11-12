//////////////////////////////////////////////////
//             BY: Vahid Moeinifar              //
//   PhD student of AGH university of Krakow    //
//                                              //
//////////////////////////////////////////////////

import QtQuick
import QtQuick.Controls
import GroupDecision

// The window. Shows the sign-in page or the workspace, plus messages on top.
ApplicationWindow {
    id: window

    // Set from main.cpp
    required property AppController app

    width: 1320
    height: 900
    minimumWidth: 900
    minimumHeight: 640
    visible: true
    title: app.session.signedIn ? "Group Decision – " + app.session.displayName : "Group Decision"
    color: Theme.mist
    font.pixelSize: Theme.bodySize

    Loader {
        anchors.fill: parent
        sourceComponent: window.app.session.signedIn ? workspace : login
    }

    Component {
        id: login
        LoginPage { session: window.app.session }
    }

    Component {
        id: workspace
        Workspace { app: window.app }
    }

    Toast {
        id: toast
        anchors.fill: parent
    }

    Connections {
        target: window.app
        function onNotify(message, kind) { toast.show(message, kind) }
    }
}
