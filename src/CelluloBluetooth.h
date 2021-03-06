/*
 * Copyright (C) 2015 EPFL
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/.
 */

/**
 * @file CelluloBluetooth.h
 * @brief Header for Bluetooth communication with Cellulo robots
 * @author Ayberk Özgür
 * @version 1.0
 * @date 2015-05-20
 */

#ifndef CELLULOBLUETOOTH_H
#define CELLULOBLUETOOTH_H

#include<QTimer>
#include<QQueue>
#include<QQuickItem>
#include<QBluetoothSocket>
#include<QColor>

/**
 * @brief Bluetooth communicator for a Cellulo robot
 */
class CelluloBluetooth : public QQuickItem {
Q_OBJECT
    Q_PROPERTY(QString macAddr WRITE setMacAddr READ getMacAddr)
    Q_PROPERTY(bool connected READ getConnected NOTIFY connectedChanged)
    Q_PROPERTY(bool connecting READ getConnecting NOTIFY connectingChanged)
    Q_PROPERTY(int batteryState READ getBatteryState NOTIFY batteryStateChanged)
    Q_PROPERTY(bool imageStreamingEnabled WRITE setImageStreamingEnabled READ getImageStreamingEnabled)
    Q_PROPERTY(bool timestampingEnabled WRITE setTimestampingEnabled READ getTimestampingEnabled)
    Q_PROPERTY(float x READ getX NOTIFY poseChanged)
    Q_PROPERTY(float y READ getY NOTIFY poseChanged)
    Q_PROPERTY(float theta READ getTheta NOTIFY poseChanged)
    Q_PROPERTY(int lastTimestamp READ getLastTimestamp NOTIFY timestampChanged)
    Q_PROPERTY(float framerate READ getFramerate NOTIFY timestampChanged)
    Q_PROPERTY(bool kidnapped READ getKidnapped NOTIFY kidnappedChanged)

public:

    enum COMMAND_TYPE{
        PING = 0,
        SET_BCAST_PERIOD,
        IMAGE_STREAM_ENABLE,
        TIMESTAMP_ENABLE,
        FRAME_REQUEST,
        BATTERY_STATE_REQUEST,
        SET_VISUAL_STATE,
        SET_VISUAL_EFFECT,
        SET_MOTOR_OUTPUT,
        SET_ALL_MOTOR_OUTPUTS,
        SET_GOAL_VELOCITY,
        SET_GOAL_VELOCITY_COMPACT,
        SET_GOAL_POSE,
        SET_GOAL_POSITION,
        RESET,
        SHUTDOWN
    };

    enum RECEIVE_MESSAGES{
        BOOT_COMPLETE = 0,
        WAKE_UP,
        SHUTTING_DOWN,
        LOW_BATTERY,
        BATTERY_STATE_CHANGED,
        TOUCH_BEGIN,
        TOUCH_LONG_PRESSED,
        TOUCH_RELEASED,
        POSE_CHANGED,
        KIDNAP,
        ACKNOWLEDGED,
        DEBUG,
        NUM_RECEIVE_MESSAGES,
        INVALID_MESSAGE = -1
    };

    static const int BT_CONNECT_TIMEOUT_MILLIS = 10000; ///< Will try to reconnect after this much time

    static const int FRAME_TIMEOUT_MILLIS = 10000;   ///< Will wait this many millis for a camera frame to complete

    static const int IMG_WIDTH = 752/4;              ///< Image width of the robot's camera
    static const int IMG_HEIGHT = 480/4;             ///< Image height of the robot's camera

    static constexpr float FRAMERATE_SMOOTH_FACTOR = 0.99f;  ///< Smoothing factor for framerate, closer to 1.0 means less update

    static constexpr float GOAL_POSE_FACTOR = 100.0f;       ///< Goal pose elements are multiplied by this before comm.
    static constexpr float GOAL_VELOCITY_FACTOR = 100.0f;   ///< Goal velocities are multiplied by this before comm.
    static const int GOAL_VELOCITY_COMPACT_DIVISOR = 2;     ///< Goal velocities are divided by this in the compact velocity command

    static QByteArray frameBuffer;                   ///< Container for the received camera frame data

    /**
     * @brief Creates a new Cellulo robot communicator
     *
     * @param parent Parent of this QML item
     */
    explicit CelluloBluetooth(QQuickItem* parent = 0);

    /**
     * @brief Destroys this Cellulo robot communicator
     */
    virtual ~CelluloBluetooth();

    /**
     * @brief Gets the latest camera frame
     *
     * @return The latest camera frame; IMG_WIDTH*IMG_HEIGHT many ints in grayscale, 0 to 255
     */
    QVariantList getFrame() const;

    /**
     * @brief Gets the current MAC address
     *
     * @return The current MAC address
     */
    QString getMacAddr(){ return macAddr; }

    /**
     * @brief Gets whether currently connected over Bluetooth
     *
     * @return Whether currently connected over Bluetooth
     */
    bool getConnected(){ return connected; }

    /**
     * @brief Gets whether currently trying to connect over Bluetooth
     *
     * @return Whether currently trying to connect over Bluetooth
     */
    bool getConnecting(){ return connecting; }

    /**
     * @brief Gets whether image streaming is currently enabled
     *
     * @return Whether image streaming is enabled or localization is enabled
     */
    bool getImageStreamingEnabled(){ return imageStreamingEnabled; }

    /**
     * @brief Gets whether timestamping along with pose is currently enabled
     *
     * @return Whether timestamping is enabled
     */
    bool getTimestampingEnabled(){ return timestampingEnabled; }

    /**
     * @brief Gets the latest battery state
     *
     * @return Battery state as described by the BATTERY_STATE enumeration
     */
    int getBatteryState(){ return batteryState; }

    /**
     * @brief Gets the latest x position
     *
     * @return Latest x position in grid coordinates
     */
    float getX(){ return x; }

    /**
     * @brief Gets the latest y position
     *
     * @return Latest y position in grid coordinates
     */
    float getY(){ return y; }

    /**
     * @brief Gets the latest orientation
     *
     * @return Latest orientation in degrees
     */
    float getTheta(){ return theta; }

    /**
     * @brief Gets the latest available timestamp
     *
     * @return The latest received timestamp in milliseconds
     */
    int getLastTimestamp(){ return lastTimestamp; }

    /**
     * @brief Gets the localization framerate
     *
     * @return Localization framerate in milliseconds
     */
    float getFramerate(){ return framerate; }

    /**
     * @brief Gets the latest kidnapped state
     *
     * @return Whether kidnapped or on encoded paper
     */
    bool getKidnapped(){ return kidnapped; }

private slots:

    /**
     * @brief Called when the server sends data
     */
    void socketDataArrived();

    /**
     * @brief Called when (re)connected to server
     */
    void socketConnected();

    /**
     * @brief Called when disconnected from server
     */
    void socketDisconnected();

    /**
     * @brief Disconnects first and then connects again if not connected yet
     */
    void refreshConnection();

    /**
     * @brief Called when the server did not complete the camera frame in time
     */
    void frameTimeout();

public slots:

    /**
     * @brief Sets the MAC address of the server and immediately connects to it
     *
     * @param macAddr Bluetooth MAC address of the server (robot)
     */
    void setMacAddr(QString macAddr);

    /**
     * @brief Creates socket and connects to the current target MAC address
     */
    void connectToServer();

    /**
     * @brief Disconnects from the currently connected device and destroys the socket if possible
     */
    void disconnectFromServer();

    /**
     * @brief Sets the pose broadcast period
     *
     * @param period Desired period in milliseconds
     */
    void setPoseBcastPeriod(unsigned int period);

    /**
     * @brief Enables image streaming + disables localization or vice versa
     *
     * @param enabled Whether to enable image streaming
     */
    void setImageStreamingEnabled(bool enabled);

    /**
     * @brief Enables timestamping along with pose and disables pose idling or vice-versa
     *
     * @param enabled Whether to enable timestamping
     */
    void setTimestampingEnabled(bool enabled);

    /**
     * @brief Sets output of motor 1
     *
     * @param output Value between -0xFFF and 0xFFF
     */
    void setMotor1Output(int output);

    /**
     * @brief Sets output of motor 2
     *
     * @param output Value between -0xFFF and 0xFFF
     */
    void setMotor2Output(int output);

    /**
     * @brief Sets output of motor 3
     *
     * @param output Value between -0xFFF and 0xFFF
     */
    void setMotor3Output(int output);

    /**
     * @brief Sets outputs of all motors
     *
     * @param m1output Value between -0xFFF and 0xFFF
     * @param m2output Value between -0xFFF and 0xFFF
     * @param m3output Value between -0xFFF and 0xFFF
     */
    void setAllMotorOutputs(int m1output, int m2output, int m3output);

    /**
     * @brief Sets robot goal velocity in global world frame
     *
     * @param vx X velocity in mm/s
     * @param vy Y velocity in mm/s
     * @param w Angular velocity in rad/s
     */
    void setGoalVelocity(float vx, float vy, float w);

    /**
     * @brief Sets robot goal velocity in global world frame
     *
     * Since this is a compact message, there is no angular velocity and resolution is low (2 mm/s)
     *
     * @param vx X velocity in mm/s (between -256 and 254)
     * @param vy Y velocity in mm/s (between -256 and 254)
     */
    void setGoalVelocityCompact(int vx, int vy);

    /**
     * @brief Sets a pose goal to track
     *
     * @param x X goal in grid coordinates
     * @param y Y goal in grid coordinates
     * @param theta Theta goal in degrees
     * @param v Maximum linear speed to track pose in mm/s
     * @param w Maximum angular speed to track pose in rad/s
     */
    void setGoalPose(float x, float y, float theta, float v, float w);

    /**
     * @brief Sets a position goal to track
     *
     * @param x X goal in grid coordinates
     * @param y Y goal in grid coordinates
     * @param v Maximum linear speed to track pose in mm/s
     */
    void setGoalPosition(float x, float y, float v);

    /**
     * @brief Sends a ping, expecting an acknowledge
     */
    void ping();

    /**
     * @brief Requests a camera frame to be sent
     */
    void requestFrame();

    /**
     * @brief Sends a battery state query
     */
    void queryBatteryState();

    /**
     * @brief Sets the visual state, i.e the visual response of the robot to touches
     *
     * @param state The visual state
     */
    void setVisualState(int state);

    /**
     * @brief Sets the visual effect on the robot, changing LED illumination
     *
     * @param effect The effect ordinal
     * @param color Color
     * @param value A value possibly meaningful for the effect (between 0 and 255)
     */
    void setVisualEffect(int effect, QColor color, int value);

    /**
     * @brief Initiates a software reset on the robot
     */
    void reset();

    /**
     * @brief Initiates sleep on the robot
     */
    void shutdown();

signals:

    /**
     * @brief Emitted when Bluetooth connection state changes
     */
    void connectedChanged();

    /**
     * @brief Emitted when Bluetooth starts or stops trying to connect
     */
    void connectingChanged();

    /**
     * @brief Emitted when the robot is ready after a power up or a reset
     */
    void bootCompleted();

    /**
     * @brief Emitted when the robot wakes up from sleep (off-state)
     */
    void wokeUp();

    /**
     * @brief Emitted when the robot is about to sleep (shutdown) due to the user command via touch keys
     */
    void shuttingDown();

    /**
     * @brief Emitted when the robot is about to sleep (shutdown) due to low battery
     */
    void lowBattery();

    /**
     * @brief Emitted when the battery state changes
     */
    void batteryStateChanged();

    /**
     * @brief Emitted when a key is touched
     *
     * @param key The key that is touched
     */
    void touchBegan(int key);

    /**
     * @brief Emitted when a key is touched for more than the long touch time
     *
     * @param key The key that is touched
     */
    void longTouch(int key);

    /**
     * @brief Emitted when a key is released
     *
     * @param key The key that is released
     */
    void touchReleased(int key);

    /**
     * @brief Emitted when the pose of the robot changes
     */
    void poseChanged();

    /**
     * @brief A new onboard localization timestamp has been received
     */
    void timestampChanged();

    /**
     * @brief Emitted when the kidnap state of the robot changes
     */
    void kidnappedChanged();

    /**
     * @brief Emitted when a camera frame from the robot is ready to read
     */
    void frameReady();

private:

    static const char* commandStrings[];    ///< Strings sent over Bluetooth to give commands
    static const char* receiveStrings[];    ///< Strings received over Bluetooth as response or event

    QTimer btConnectTimeoutTimer;           ///< Timeout timer to reconnect if connection fails
    QBluetoothSocket* socket;               ///< Bluetooth socket connected to the server
    QString macAddr;                        ///< Bluetooth MAC address of the server
    bool imageStreamingEnabled;             ///< Whether image streaming is enabled or localization is enabled
    bool timestampingEnabled;               ///< Whether timestamping along with pose is enabled and idling disabled
    int lastTimestamp;                      ///< Latest received onboard timestamp (in milliseconds)
    float framerate;                        ///< Framerate calculated over time
    QTimer frameTimeoutTimer;               ///< When this timer runs out, frame is completed even if it is not complete
    QByteArray receiveBuffer;               ///< Receive buffer until the current response/event message is complete
    bool expectingFrame;                    ///< True after sending a camera frame request until the camera frame arrives completely
    unsigned int currentPixel;              ///< Current pixel in the camera frame being received

    bool connected;                         ///< Whether Bluetooth is connected now
    bool connecting;                        ///< Whether Bluetooth is trying to connect
    int batteryState;                       ///< Current battery state
    float x;                                ///< Current x position in grid coordinates
    float y;                                ///< Current y position in grid coordinates
    float theta;                            ///< Current orientation in degrees
    bool kidnapped;                         ///< Whether currently kidnapped

    /**
     * @brief Resets properties of the robot to default
     */
    void resetProperties();

    /**
     * @brief Connects or reconnects to the service on the server if not already connected
     */
    void openSocket();

    /**
     * @brief Sets the motor output
     *
     * @param motor 1, 2 or 3
     * @param output Value between -FFF and FFF
     */
    void setMotorOutput(int motor, int output);

    /**
     * @brief Sends command over Bluetooth
     *
     * @param type Type of the command
     * @param message Message body itself
     */
    void sendCommand(COMMAND_TYPE type, QByteArray& message);

    /**
     * @brief Processes the response in the receive buffer if possible
     */
    void processResponse();

    /**
     * @brief Gets the ordinal of the received message in the receive buffer
     *
     * @return Received message ordinal in the RECEIVE_MESSAGES enum
     */
    RECEIVE_MESSAGES getReceivedMessage();

    /**
     * @brief Calculates the integer value from an uppercase hex string
     *
     * @param array Array containing the hex string
     * @param begin Index of most significant digit
     * @param end Index of least significant digit
     *
     * @return The value
     */
    int hexToInt(QByteArray const& array, int begin, int end);

    /**
     * @brief Converts single digit integer to hexadecimal character
     *
     * @param value Between 0x0 and 0xF
     *
     * @return Hexadecimal representation of value
     */
    char getHexChar(unsigned int value);

    /**
     * @brief Calculates the number of 1 bits in the given value
     *
     * @param value Given value
     *
     * @return Number of 1 bits in given value
     */
    char getNumberOfOnes(unsigned int value);
};

#endif // CELLULOBLUETOOTH_H
