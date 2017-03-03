import socket
import time
from math import pi

#HOST = '192.168.0.20'    # OD's home wifi
HOST = '172.20.10.4'    # my iphone
PORT = 10080
DATAPORT = 10081

rotate_time = 5
move_speed = 0.1


def send_msg(msg):
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((HOST, PORT))
    s.send(msg)
    s.close()

# x[0] = (data.buttons & 0x01) >> 0; // botton0
# x[1] = (data.buttons & 0x02) >> 1; // botton1
# x[2] = (data.buttons & 0x04) >> 2; // botton2
# x[3] = (data.wheel_drop & 0x02) >> 1; // left wheel
# x[4] = (data.wheel_drop & 0x01) >> 0; // right wheel
# x[5] = (data.bumper & 0x04) >> 2; // left bumper
# x[6] = (data.bumper & 0x02) >> 1; // center bumper
# x[7] = (data.bumper & 0x01) >> 0; // right bumper

def get_data():
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((HOST, DATAPORT))
    s.send("getdata" + '\0')
    data = s.recv(1024)
    #print data, len(data)
    X = [int(x) for x in data[:-1].split(',')]
    print "botton0:", X[0]
    print "botton1:", X[1]
    print "botton2:", X[2]
    print "left wheel drop:", X[3]
    print "right wheel drop:", X[4]
    print "left bumper touched", X[5]
    print "center bumper touched", X[6]
    print "right bumper touched", X[7]

    s.close()


def make_command(cmd, para):
    """
    :param string cmd: command (MOV, ROTATE, STOP)
    :param dict string->string para: command parameter (double speed, int time, bool stop)
    :param string res: command string
    """
    params = ["{0}:{1}".format(k, v) for k,v in para.iteritems()]
    return cmd + "|" + "|".join(params) + '\0'


class RobManager:

    def status(self):
        print self.cmd_stack
        print self.drive_time_stack
        print self.last_drive

    def _record_last_drive_time(self):
        if self.last_drive:
            print "time: ", time.time() - self.timer
            self.drive_time_stack.append(time.time() - self.timer)
            self.cmd_stack.append(make_command("MOV", {"speed": str(-move_speed), "time": str(int(self.drive_time_stack.pop())), "stop": "true"}))

    def __init__(self):
        self.cmd_stack = [] # reversed command for return the starting point
        self.drive_time_stack = [] # drive time
        self.timer = time.time() # timer to record when the last command starts
        self.last_drive = False # last command: drive or not

    def drive(self):
        self._record_last_drive_time()
        cmd = make_command("MOV", {"speed": str(move_speed), "time": str(1), "stop": "false"})
        send_msg(cmd)
        self.timer = time.time()
        self.last_drive = True


    def turn(self, degree):
        '''
        :param degree double, negative for clockwise
        '''
        self._record_last_drive_time()
        rotate_speed = (pi * degree / 180.0) / rotate_time
        cmd = make_command("ROTATE", {"speed": str(rotate_speed), "time": str(rotate_time), "stop": "true"})
        send_msg(cmd)
        self.timer = time.time()
        self.last_drive = False
        self.cmd_stack.append(make_command("ROTATE", {"speed": str(-rotate_speed), "time": str(rotate_time), "stop": "true"}))


    def read_sensor(self):
        pass

    def goto(self, x, y):
        self._record_last_drive_time()
        duration_x = int(1.0 * x / move_speed)
        duration_y = int(1.0 * y / move_speed)
        cmd_x = make_command("MOV", {"speed": str(move_speed), "time": str(duration_x), "stop": "true"})
        cmd_y = make_command("MOV", {"speed": str(move_speed), "time": str(duration_y), "stop": "true"})

        rotate_speed = pi * 90.0 / 180.0 / rotate_time
        cmd_turn = make_command("ROTATE", {"speed": str(rotate_speed), "time": str(rotate_time), "stop": "true"})

        send_msg(cmd_x)
        send_msg(cmd_turn)
        send_msg(cmd_y)

        self.timer = time.time()
        self.last_drive = False
        self.cmd_stack.append(make_command("MOV", {"speed": str(-move_speed), "time": str(duration_x), "stop": "true"}))
        self.cmd_stack.append(make_command("ROTATE", {"speed": str(-rotate_speed), "time": str(rotate_time), "stop": "true"}))
        self.cmd_stack.append(make_command("MOV", {"speed": str(-move_speed), "time": str(duration_y), "stop": "true"}))


    def return_init(self):
        self._record_last_drive_time()
        while len(self.cmd_stack) != 0:
            print self.cmd_stack[-1]
            send_msg(self.cmd_stack.pop())
        print "lalala"
        self.timer = time.time()
        self.last_drive = False

    def reset(self):
        self.cmd_stack = []  # reversed command for return the starting point
        self.drive_time_stack = []  # drive time
        self.timer = time.time()  # timer to record when the last command starts
        self.last_drive = False  # last command: drive or not

    def stop(self):
        send_msg(make_command("STOP", {}))
        self._record_last_drive_time()
        self.timer = time.time()
        self.last_drive = False


if __name__ == "__main__":
    rob = RobManager()

    def drive():
        rob.drive()
        rob.status()

    def turn(degree):

        r = abs(degree) / 180
        c = abs(degree) % 180
        couter_clock = 1 if degree > 0 else -1

        for i in range(r):
            rob.turn(couter_clock * 180)
        rob.turn(couter_clock * c)

        rob.status()

    def goto(x, y):
        rob.goto(x, y)
        rob.status()

    def return_init():
        rob.return_init()
        rob.status()

    def reset():
        rob.reset()
        rob.status()

    def stop():
        rob.stop()
        rob.status()


    def help():
        print (
            "drive() -- let the robot drive without stop\n" +
            "turn(degree) -- turn left (positive degree) or right (negative degree)\n" +
            "goto(x, y) -- go to location x, y\n" +
            "return_init() -- return to initial location\n" +
            "reset() -- reset initial location\n" +
            "stop() -- stop robot\n"
        )


    while True:
        input("Enter command: ")
