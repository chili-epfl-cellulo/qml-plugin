import QtQuick 2.2
import QtQuick.Window 2.1
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles 1.3
import QtBluetooth 5.2
import Cellulo 1.0

ApplicationWindow {
    id: window
    visible: true
    minimumHeight: height
    minimumWidth: width

    property real numSamples: 20
    property variant coords: []
    property bool coordsReady: false
    property bool collecting: false
    property bool zeroWhenFinished: false
    property real xMean: 0
    property real yMean: 0
    property real xStdev: 1
    property real yStdev: 1
    property real xRobotZero: 0
    property real yRobotZero: 0

    Component.onCompleted: {
        var temp = new Array(0);
        for(var i=0;i<numSamples;i++)
            temp.push(Qt.vector2d(0,0));
        coords = temp;
    }

    Column{
        spacing: 5

        Text{
            text: "Connected?"
            color: robotComm.connected ? "green" : "red"
        }

        Row{
            spacing: 5

            Button {
                text: "Zero robot coords"
                onClicked: {
                    if(coordsReady){
                        xRobotZero = xMean;
                        yRobotZero = yMean;
                    }
                }
            }

            TextField{
                id: xRobotZeroField
                text: xRobotZero
                readOnly: true
                anchors.verticalCenter: parent.verticalCenter
            }

            TextField{
                id: yRobotZeroField
                text: yRobotZero
                readOnly: true
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        Row{
            spacing: 5

            Button {
                text: "Measure"
                anchors.verticalCenter: parent.verticalCenter
                onClicked: {
                    ready.color = "red";
                    coordsReady = false;
                    collecting = true;
                }
            }

            Button {
                text: "Measure and Zero"
                anchors.verticalCenter: parent.verticalCenter
                onClicked: {
                    ready.color = "red";
                    coordsReady = false;
                    collecting = true;
                    zeroWhenFinished = true;
                }
            }

            CheckBox{
                id: logEverything
                checked: false
                text: "Log every pose"
                anchors.verticalCenter: parent.verticalCenter
            }
        }

        Text{
            id: ready
            text: "Ready?"
            color: "red"
        }

        Text{
            text: robotComm.x*robotComm.gridSpacing + " " + robotComm.y*robotComm.gridSpacing
        }
    }

    CelluloBluetooth{
        property real gridSpacing: 0.508
        property real currentIndex: 0
        property real collectStartIndex: -1

        id: robotComm
        macAddr: "00:06:66:74:48:A7"

        onPoseChanged: {
            coords[currentIndex] = Qt.vector2d(x*gridSpacing, y*gridSpacing);

            if(logEverything.checked)
                console.log((coords[currentIndex].x - xRobotZero) + " " + (coords[currentIndex].y - yRobotZero));

            if(collecting){
                if(collectStartIndex < 0)
                    collectStartIndex = currentIndex;

                if((currentIndex + 1) % numSamples == collectStartIndex){
                    collectStartIndex = -1;

                    //Means
                    xMean = 0;
                    yMean = 0;
                    for(var i=0;i<numSamples;i++){
                        xMean += coords[i].x;
                        yMean += coords[i].y;
                    }
                    xMean /= numSamples;
                    yMean /= numSamples;

                    //Stdevs
                    xStdev = 0;
                    yStdev = 0;
                    for(var i=0;i<numSamples;i++){
                        xStdev += Math.pow(xMean - coords[i].x, 2);
                        yStdev += Math.pow(yMean - coords[i].y, 2);
                    }
                    xStdev = Math.sqrt(xStdev/(numSamples - 1));
                    yStdev = Math.sqrt(yStdev/(numSamples - 1));

                    //Record data
                    console.log((xMean - xRobotZero) + " " + xStdev + " " + (yMean - yRobotZero) + " " + yStdev);

                    if(zeroWhenFinished){
                        xRobotZero = xMean;
                        yRobotZero = yMean;
                    }

                    collecting = false;
                    zeroWhenFinished = false;
                    ready.color = "green";
                    coordsReady = true;
                }
            }

            currentIndex = (currentIndex + 1) % numSamples;
        }
    }
}
