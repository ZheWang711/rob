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
        kobuki.setBaseControl(0.0, radPerSec);
        kobuki.sendBaseControlCommand();
        ecl::Sleep()(dur);
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

private:
    kobuki::Kobuki kobuki;
};
