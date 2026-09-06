import QtQuick 2.7
import QtQuick.Window 2.2
import Lomiri.Components 1.3

Window {
    id: window

    visible: true

    width: units.gu(45)
    height: units.gu(75)

    title: "QuickGal"

    MainView {
        id: mainView

        anchors.fill: parent
        applicationName: "quickgal.nacho"

        PageStack {
            id: pageStack

            anchors.fill: parent

            Component.onCompleted: {
                push(albumsPage)
            }
        }

        Component {
            id: albumsPage

            Page {
                id: page

                header: PageHeader {
                    id: pageHeader
                    title: "QuickGal"
                }

                Row {
                    id: optionsRow

                    anchors {
                        top: pageHeader.bottom
                        left: parent.left
                        right: parent.right
                        leftMargin: units.gu(2)
                        rightMargin: units.gu(2)
                    }

                    height: units.gu(7)
                    spacing: units.gu(2)

                    Label {
                        text: "Mostrar álbumes ocultos"
                        anchors.verticalCenter: parent.verticalCenter
                    }

                    Switch {
                        id: hiddenSwitch

                        anchors.verticalCenter: parent.verticalCenter

                        checked: appController.showHiddenAlbums

                        onCheckedChanged: {
                            console.log(
                                "Switch ocultos:",
                                checked
                            )

                            appController.showHiddenAlbums = checked
                        }
                    }
                }

                GridView {
                    id: albumGrid

                    anchors {
                        top: optionsRow.bottom
                        left: parent.left
                        right: parent.right
                        bottom: parent.bottom
                    }

                    clip: true

                    cellWidth: width / 2
                    cellHeight: units.gu(26)

                    model: albumModel

                    delegate: Item {
                        width: albumGrid.cellWidth
                        height: albumGrid.cellHeight

                        Rectangle {
                            anchors {
                                fill: parent
                                margins: units.gu(1)
                            }

                            color: theme.palette.normal.background

                            Column {
                                anchors {
                                    fill: parent
                                    margins: units.gu(1)
                                }

                                spacing: units.gu(0.5)

                                Rectangle {
                                    width: parent.width
                                    height: units.gu(18)

                                    color: "#cccccc"

                                    Image {
                                        anchors.fill: parent

                                        source: "file://" + coverPath

                                        fillMode: Image.PreserveAspectCrop

                                        asynchronous: true
                                        cache: true
                                    }
                                }

                                Label {
                                    width: parent.width

                                    text: name

                                    font.bold: true
                                    elide: Text.ElideRight
                                }

                                Label {
                                    text: count + " elementos"
                                }
                            }

                            MouseArea {
                                anchors.fill: parent

                                onClicked: {
                                    console.log(
                                        "Abriendo el album:",
                                        name,
                                        path
                                    )

                                    appController.openAlbum(path)

                                    pageStack.push(
                                        albumPageComponent,
                                        {
                                            albumName: name
                                        }
                                    )
                                }
                            }
                        }
                    }
                }
            }
        }

        Component {
            id: albumPageComponent

            Page {
                id: albumPage

                property string albumName: ""

                header: PageHeader {
                    id: albumHeader
                    title: albumPage.albumName
                }

                GridView {
                    id: mediaGrid

                    anchors {
                        top: albumHeader.bottom
                        left: parent.left
                        right: parent.right
                        bottom: parent.bottom
                    }

                    model: mediaModel

                    cellWidth: width / 3
                    cellHeight: cellWidth

                    delegate: Item {
                        width: mediaGrid.cellWidth
                        height: mediaGrid.cellHeight

                        Image {
                            anchors {
                                fill: parent
                                margins: units.gu(0.25)
                            }

                            source: "file://" + path

                            fillMode: Image.PreserveAspectCrop

                            asynchronous: true
                            cache: true
                        }

                        MouseArea {
                            anchors.fill: parent

                            onClicked: {
                                console.log("Abriendo imagen:", path)

                                pageStack.push(
                                    imagePageComponent,
                                    {
                                        imagePath: path,
                                        imageName: fileName
                                    }
                                )
                            }
                        }
                    }
                }
            }
        }

        Component {
            id: imagePageComponent

            Page {
                id: imagePage

                property string imagePath: ""
                property string imageName: ""

                header: PageHeader {
                    id: imageHeader
                    title: imagePage.imageName
                }

                Rectangle {
                    anchors {
                        top: imageHeader.bottom
                        left: parent.left
                        right: parent.right
                        bottom: parent.bottom
                    }

                    color: "black"

                    Image {
                        anchors.fill: parent

                        source: "file://" + imagePage.imagePath

                        fillMode: Image.PreserveAspectFit

                        asynchronous: true
                        cache: true
                    }
                }
            }
        }
    }
}