import socket

HOST = '172.20.10.4'    # The remote host
PORT = 10088              # The same port as used by the server
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
s.connect((HOST, PORT))

msg = "Hello World" + '\0'
s.send(msg)
print "sent"
data = s.recv(1024)
s.close()
print 'Received', repr(data)
