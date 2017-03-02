//
// Created by Zhe Wang on 3/1/17.
//

#ifndef ECHO_CONSTANTS_H
#define ECHO_CONSTANTS_H

#include <string>
using std::string;

namespace Constants{
    // command type
    const int MOVE = 1;
    const int ROTATE = 2;
    const int STOP = 3;

    // command string
    const string CMDMOVE = "MOV";
    const string CMDROTATE = "ROTATE";
    const string CMDSTOP = "STOP";

    // key
    const string speed = "speed";
    const string time = "time";
    const string stop = "stop";


}



#endif //ECHO_CONSTANTS_H
