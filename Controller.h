/**
 * @file /kobuki_driver/src/test/initialisation.cpp
 *
 * @brief Demo program for kobuki initialisation.
 **/
/*****************************************************************************
** Includes
*****************************************************************************/

#include <iostream>
#include <kobuki_driver/kobuki.hpp>
#include <ecl/time.hpp>
#include <unistd.h>
#include <cassert>

const double mistake = 0.00872665; // half a degree
const double angSpeed = 0.4; // about 10 degrees 1s
const int sleep_mocro_sec = 1000000 * 0.75 * mistake / angSpeed;

//assert(angSpeed * sleep_mocro_sec / 100000.0 < mistake * 0.8);

inline double abs(double x) {
   return (x > 0)? x : -x;
}


class KobukiController {
public:
    KobukiController() {
        kobuki::Parameters parameters;
        // change the default device port from /dev/kobuki to /dev/ttyUSB0
        parameters.device_port = "/dev/ttyUSB0";
        // Other parameters are typically happy enough as defaults
        // namespaces all sigslot connection names under this value, only important if you want to
        parameters.sigslots_namespace = "/kobuki";
        // Most people will prefer to do their own velocity smoothing/acceleration limiting.
        // If you wish to utilise kobuki's minimal acceleration limiter, set to true
        parameters.enable_acceleration_limiter = false;
        // If your battery levels are showing significant variance from factory defaults, adjust thresholds.
        // This will affect the led on the front of the robot as well as when signals are emitted by the driver.
        parameters.battery_capacity = 16.5;
        parameters.battery_low = 14.0;
        parameters.battery_dangerous = 13.2;

        // initialise - it will throw an exception if parameter validation or initialisation fails.
        try {
            kobuki.init(parameters);
        } catch ( ecl::StandardException &e ) {
            std::cout << e.what();
        }
    }

    // first argument: <double> angular velocity (rad/s); second argument: <int> time (second)
    void rotate(double radPerSec, int dur){
        rotate_angle(radPerSec * dur);
    }

    // first argument: <double> speed (m/s); second argument: <int> time (second)
    void move(double meterPerSec, int dur){
        kobuki.setBaseControl(meterPerSec, 0.0);
        kobuki.sendBaseControlCommand();
        ecl::Sleep()(dur);
    }

    void stop(){
        kobuki.setBaseControl(0.0, 0.0);
        kobuki.sendBaseControlCommand();
    }

    double getHeading(){
        kobuki.lockDataAccess();
        double ang = kobuki.getHeading();
        kobuki.unlockDataAccess();
        return ang;
    }

    void resetOdometry(){
        kobuki.resetOdometry();
    }

    void getSensorData(int* x){
        kobuki.lockDataAccess();
        kobuki::CoreSensors::Data data = kobuki.getCoreSensorData();
        kobuki.unlockDataAccess();

        x[0] = (data.buttons & 0x01) >> 0; // botton0
        x[1] = (data.buttons & 0x02) >> 1; // botton1
        x[2] = (data.buttons & 0x04) >> 2; // botton2
        x[3] = (data.wheel_drop & 0x02) >> 1; // left wheel
        x[4] = (data.wheel_drop & 0x01) >> 0; // right wheel
        x[5] = (data.bumper & 0x04) >> 2; // left bumper
        x[6] = (data.bumper & 0x02) >> 1; // center bumper
        x[7] = (data.bumper & 0x01) >> 0; // right bumper


        // printf("botton0: %x, botton1: %x, botton2: %x\n", (data.buttons & 0x01) >> 0, (data.buttons & 0x02) >> 1, (data.buttons & 0x04) >> 2);
        // printf("left wheel: %x, right wheel: %x\n", (data.wheel_drop & 0x02) >> 1, (data.wheel_drop & 0x01) >> 0);
        // printf("left bumper: %x, center bumper: %x, right bumper: %x\n", (data.bumper & 0x04) >> 2, (data.bumper & 0x02) >> 1, (data.bumper & 0x01) >> 0);
    }


private:
    kobuki::Kobuki kobuki;

    void rotate_angle(double rads){
        double heading = 0;
        do{ // move less than 0.5 degree
            if (rads > 0) {kobuki.setBaseControl(0.0, angSpeed);}
            else {kobuki.setBaseControl(0.0, -angSpeed);}
            kobuki.sendBaseControlCommand();
            usleep(sleep_mocro_sec); // sleep 30 ms, delta angle = 0.03 * 10 = 0.4 degree < 0.5 degree
            kobuki.lockDataAccess();
            heading = kobuki.getHeading();
            kobuki.unlockDataAccess();
        }while(abs(heading - rads) > mistake);
        kobuki.lockDataAccess();
        kobuki.resetOdometry(); // reset the base hading
        kobuki.unlockDataAccess();

    }


};
