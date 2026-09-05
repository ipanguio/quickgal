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
                    anchors.verticalCenter:
                    parent.verticalCenter
                }

                Switch {
                    id: hiddenSwitch

                    anchors.verticalCenter: parent.verticalCenter

                    checked: appController.showHiddenAlbums

                    onCheckedChanged: {
                        console.log(
                            "Swith ocultos:",
                            checked
                        )

                        appController.showHiddenAlbums =
                            checked
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

                                    onStatusChanged: {
                                        if (status === Image.Error) {
                                            console.log(
                                                "Error cargando portada:",
                                                coverPath
                                            )
                                        }
                                    }
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
                    }
                }

                Component.onCompleted: {
                    console.log(
                        "AlbumGrid creado. Elementos:",
                        count
                    )
                }
            }
        }
    }
}