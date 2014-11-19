/* Copyright (C) 2012 Kristian Lauszus, TKJ Electronics. All rights reserved.
 This software may be distributed and modified under the terms of the GNU
 General Public License version 2 (GPL2) as published by the Free Software
 Foundation and appearing in the file GPL2.TXT included in the packaging of
 this file. Please note that GPL2 Section 2[b] requires that all works based
 on this software must also be made publicly available under the terms of
 the GPL2 ("Copyleft").
 Contact information
 -------------------
 Kristian Lauszus, TKJ Electronics
 Web      :  http://www.tkjelectronics.com
 e-mail   :  kristianl@tkjelectronics.com
 */

#ifndef _ps3enums_h
#define _ps3enums_h


//Start controllerEnums.h

/** Enum used to turn on the LEDs on the different controllers. */
enum LEDEnum {
        OFF = 0,
        LED1 = 1,
        LED2 = 2,
        LED3 = 3,
        LED4 = 4,

        LED5 = 5,
        LED6 = 6,
        LED7 = 7,
        LED8 = 8,
        LED9 = 9,
        LED10 = 10,
        /** Used to blink all LEDs on the Xbox controller */
        ALL = 5,
};

/** Used to set the colors of the Move and PS4 controller. */
enum ColorsEnum {
        /** r = 255, g = 0, b = 0 */
        Red = 0xFF0000,
        /** r = 0, g = 255, b = 0 */
        Green = 0xFF00,
        /** r = 0, g = 0, b = 255 */
        Blue = 0xFF,

        /** r = 255, g = 235, b = 4 */
        Yellow = 0xFFEB04,
        /** r = 0, g = 255, b = 255 */
        Lightblue = 0xFFFF,
        /** r = 255, g = 0, b = 255 */
        Purble = 0xFF00FF,

        /** r = 255, g = 255, b = 255 */
        White = 0xFFFFFF,
        /** r = 0, g = 0, b = 0 */
        Off = 0x00,
};

enum RumbleEnum {
        RumbleHigh = 0x10,
        RumbleLow = 0x20,
};

/** This enum is used to read all the different buttons on the different controllers */
enum ButtonEnum {
        /**@{*/
        /** These buttons are available on all the the controllers */
        UP = 0,
        RIGHT = 1,
        DOWN = 2,
        LEFT = 3,
        /**@}*/

        /**@{*/
        /** Wii buttons */
        PLUS = 5,
        TWO = 6,
        ONE = 7,
        MINUS = 8,
        HOME = 9,
        Z = 10,
        C = 11,
        B = 12,
        A = 13,
        /**@}*/

        /**@{*/
        /** These are only available on the Wii U Pro Controller */
        L = 16,
        R = 17,
        ZL = 18,
        ZR = 19,
        /**@}*/

        /**@{*/
        /** PS3 controllers buttons */
        SELECT = 4,
        START = 5,
        L3 = 6,
        R3 = 7,

        L2 = 8,
        R2 = 9,
        L1 = 10,
        R1 = 11,
        TRIANGLE = 12,
        CIRCLE = 13,
        CROSS = 14,
        SQUARE = 15,

        PS = 16,

        MOVE = 17, // Covers 12 bits - we only need to read the top 8
        T = 18, // Covers 12 bits - we only need to read the top 8
        /**@}*/

        /** PS4 controllers buttons - SHARE and OPTIONS are present instead of SELECT and START */
        SHARE = 4,
        OPTIONS = 5,
        TOUCHPAD = 17,
        /**@}*/

        /**@{*/
        /** Xbox buttons */
        BACK = 4,
        X = 14,
        Y = 15,
        XBOX = 16,
        SYNC = 17,
        BLACK = 8, // Available on the original Xbox controller
        WHITE = 9, // Available on the original Xbox controller
        /**@}*/

        /** PS Buzz controllers */
        RED = 0,
        YELLOW = 1,
        GREEN = 2,
        ORANGE = 3,
        BLUE = 4,
        /**@}*/
};

/** Joysticks on the PS3 and Xbox controllers. */
enum AnalogHatEnum {
        /** Left joystick x-axis */
        LeftHatX = 0,
        /** Left joystick y-axis */
        LeftHatY = 1,
        /** Right joystick x-axis */
        RightHatX = 2,
        /** Right joystick y-axis */
        RightHatY = 3,
};

/**
 * Sensors inside the Sixaxis Dualshock 3, Move controller and PS4 controller.
 * <B>Note:</B> that the location is shifted 9 when it's connected via USB on the PS3 controller.
 */
enum SensorEnum {
        /** Accelerometer values */
        aX = 50, aY = 52, aZ = 54,
        /** Gyro z-axis */
        gZ = 56,
        gX, gY, // These are not available on the PS3 controller

        /** Accelerometer x-axis */
        aXmove = 28,
        /** Accelerometer z-axis */
        aZmove = 30,
        /** Accelerometer y-axis */
        aYmove = 32,

        /** Gyro x-axis */
        gXmove = 40,
        /** Gyro z-axis */
        gZmove = 42,
        /** Gyro y-axis */
        gYmove = 44,

        /** Temperature sensor */
        tempMove = 46,

        /** Magnetometer x-axis */
        mXmove = 47,
        /** Magnetometer z-axis */
        mZmove = 49,
        /** Magnetometer y-axis */
        mYmove = 50,
};

/** Used to get the angle calculated using the PS3 controller and PS4 controller. */
enum AngleEnum {
        Pitch = 0x01,
        Roll = 0x02,
};


// START PS3Enums.h

/** Size of the output report buffer for the Dualshock and Navigation controllers */
#define PS3_REPORT_BUFFER_SIZE  48

/** Report buffer for all PS3 commands */
const uint8_t PS3_REPORT_BUFFER[PS3_REPORT_BUFFER_SIZE] = {
        0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00,
        0xff, 0x27, 0x10, 0x00, 0x32,
        0xff, 0x27, 0x10, 0x00, 0x32,
        0xff, 0x27, 0x10, 0x00, 0x32,
        0xff, 0x27, 0x10, 0x00, 0x32,
        0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

/** Size of the output report buffer for the Move Controller */
#define MOVE_REPORT_BUFFER_SIZE 7

/** Used to set the LEDs on the controllers */
const uint8_t PS3_LEDS[] = {
        0x00, // OFF
        0x01, // LED1
        0x02, // LED2
        0x04, // LED3
        0x08, // LED4

        0x09, // LED5
        0x0A, // LED6
        0x0C, // LED7
        0x0D, // LED8
        0x0E, // LED9
        0x0F, // LED10
};

/**
 * Buttons on the controllers.
 * <B>Note:</B> that the location is shifted 9 when it's connected via USB.
 */
const uint32_t PS3_BUTTONS[] = {
        0x10, // UP
        0x20, // RIGHT
        0x40, // DOWN
        0x80, // LEFT

        0x01, // SELECT
        0x08, // START
        0x02, // L3
        0x04, // R3

        0x0100, // L2
        0x0200, // R2
        0x0400, // L1
        0x0800, // R1

        0x1000, // TRIANGLE
        0x2000, // CIRCLE
        0x4000, // CROSS
        0x8000, // SQUARE

        0x010000, // PS
        0x080000, // MOVE - covers 12 bits - we only need to read the top 8
        0x100000, // T - covers 12 bits - we only need to read the top 8
};

/**
 * Analog buttons on the controllers.
 * <B>Note:</B> that the location is shifted 9 when it's connected via USB.
 */
const uint8_t PS3_ANALOG_BUTTONS[] = {
        23, // UP_ANALOG
        24, // RIGHT_ANALOG
        25, // DOWN_ANALOG
        26, // LEFT_ANALOG
        0, 0, 0, 0, // Skip SELECT, L3, R3 and START

        27, // L2_ANALOG
        28, // R2_ANALOG
        29, // L1_ANALOG
        30, // R1_ANALOG
        31, // TRIANGLE_ANALOG
        32, // CIRCLE_ANALOG
        33, // CROSS_ANALOG
        34, // SQUARE_ANALOG
        0, 0, // Skip PS and MOVE

        // Playstation Move Controller
        15, // T_ANALOG - Both at byte 14 (last reading) and byte 15 (current reading)
};

enum StatusEnum {
        // Note that the location is shifted 9 when it's connected via USB
        // Byte location | bit location
        Plugged = (38 << 8) | 0x02,
        Unplugged = (38 << 8) | 0x03,

        Charging = (39 << 8) | 0xEE,
        NotCharging = (39 << 8) | 0xF1,
        Shutdown = (39 << 8) | 0x01,
        Dying = (39 << 8) | 0x02,
        Low = (39 << 8) | 0x03,
        High = (39 << 8) | 0x04,
        Full = (39 << 8) | 0x05,

        MoveCharging = (21 << 8) | 0xEE,
        MoveNotCharging = (21 << 8) | 0xF1,
        MoveShutdown = (21 << 8) | 0x01,
        MoveDying = (21 << 8) | 0x02,
        MoveLow = (21 << 8) | 0x03,
        MoveHigh = (21 << 8) | 0x04,
        MoveFull = (21 << 8) | 0x05,

        CableRumble = (40 << 8) | 0x10, // Operating by USB and rumble is turned on
        Cable = (40 << 8) | 0x12, // Operating by USB and rumble is turned off
        BluetoothRumble = (40 << 8) | 0x14, // Operating by Bluetooth and rumble is turned on
        Bluetooth = (40 << 8) | 0x16, // Operating by Bluetooth and rumble is turned off
};

#endif