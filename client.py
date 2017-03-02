import socket

#HOST = '172.20.10.4'    # The remote host
HOST = '192.168.0.20'
PORT = 10086              # The same port as used by the server
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))

msg = "ROTATE|speed:0.5|time:5|stop:true" + '\0'
#msg = "STOP" + '\0'
s.send(msg)
print "sent"
data = s.recv(1024)
s.close()
print 'Received', repr(data)
