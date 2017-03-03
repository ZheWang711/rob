# Rob

# System Design
The system can be break into 2 pieces, a server writing in C++ running on raspberry pi, and a python client writing in Python running in my laptop. The client and server communicate with each other using socket. Our design principle is to let the C++ be easy, and Python be complex. So, the server just provides basic interfaces like `move`, `rotate`, and `get_data`; whereas the client takes cares of both maintaining the system state and providing another abstraction layer for compleicated commands such as `goto`, and `return_init` for the user.


# How to run
## On Raspberry PI
* put the `server` folder into `~/kobuki_core/src/kobuki_core/kobuki_driver/src/test/`
* add the following lines into `~/kobuki_core/src/kobuki_core/kobuki_driver/src/test/CMakeList.txt`
```
add_executable(simple_test server/initialisation.cpp)
target_link_libraries(simple_test kobuki)

add_executable(demo_kobuki_simple_server server/server.cpp server/Command.h server/Constants.h server/Controller.h)
target_link_libraries(demo_kobuki_simple_server kobuki)
```
* cd to `~/kobuki_core/`
* execute the following command to add path
```
export PATH=~/kobuki_core/src/catkin/bin/:${PATH}
export LD_LIBRARY_PATH=~/kobuki_core/install/lib
```
* execute `/home/pi/kobuki_core/devel/lib/kobuki_driver/demo_kobuki_simple_server 10080 10081`

## On Client
* execute `python RobManager.py`
