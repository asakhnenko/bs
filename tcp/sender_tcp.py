#!/usr/bin/env python
import os, os.path
import argparse
import socket
import struct
import hashlib
from shutil import make_archive

SHA512_CMP_OK=0
SHA512_CMP_ERROR=-1

sha512_ok 	= "\x1b[32mSHA512 OK \x1b[0m\n";
sha512_error 	= "\x1b[31mSHA512 Error\x1b[0m\n";
port_error 	= "\x1b[31mInvalid Port (%s) \x1b[0m\n";
address_error 	= "\x1b[31mInvalid Address (%s) or Port (%s) \x1b[0m\n";
receiver_sha512 	= "\x1b[34mReceiver SHA512: %s \x1b[0m\n";
sender_sha512 	= "\x1b[34mSender SHA512: %s \x1b[0m\n";
filename_str 	= "\x1b[33mFilename: %s \x1b[0m\n";
filesize_str 	= "\x1b[33mFilesize: %d bytes\x1b[0m\n";

packet_error = "\x1b[31mInvalid Packet received \x1b[0m\n";
order_error = "\x1b[31mInvalid Packet Order: received %d, expected %d \x1b[0m\n";
timeout_error = "\x1b[31mTimeout reached, aborting..\x1b[0m\n";

BUFFERSIZE = 1492
parser = argparse.ArgumentParser(description='PA2 TCP Test Sender')
parser.add_argument('port', metavar="Empfaengerport",type=int,help="Port des Senders")
parser.add_argument('inpath', metavar="Dateipfad",type=str,help="Zu sendender Ordner")
a = parser.parse_args()
if not os.path.isdir(a.inpath):
    print "Not a path\n"
    exit()
try:
    make_archive(a.inpath, 'zip', '.', a.inpath)
except IOError:
    print "Permission denied for target archive"
    exit()
a.inpath = a.inpath[:-1]
a.inpath += '.zip'
basename = os.path.basename(a.inpath)
filesize = os.stat(a.inpath).st_size
print filesize_str[0:-1] %filesize
print filename_str[0:-1] %basename
buff = ""
buff+= struct.pack("!H",len(basename))
buff+= basename
buff+= struct.pack("!I",filesize)
sock2 = socket.socket(socket.AF_INET,socket.SOCK_STREAM)
sock2.bind(('',a.port));
sock2.listen(0)
sock = sock2.accept()[0]
sock.settimeout(10)
sock.send(buff)
f = open(a.inpath,"rb")
sha512 = hashlib.sha512()
while True:
    data =  f.read(BUFFERSIZE-1)
    if data == "":
        break;
    sha512.update(data)
    buff = ""
    buff+=data

    sock.send(buff)
sha512sum = sha512.digest()
print sender_sha512[0:-1] %sha512.hexdigest()
buff = ""
buff+=sha512sum
sock.send(buff)
try:
    reply = sock.recv(1)
    if reply == "":
        print "Connection closed too early"
        exit()
except socket.timeout:
    print timeout_error[:-1]
    exit()
finally:
    sock.shutdown(socket.SHUT_RDWR)
    sock.close()
    sock2.shutdown(socket.SHUT_RDWR)
    sock2.close()
result =  struct.unpack("!B",reply[0])[0]
print sha512_ok[0:-1] if not result else sha512_error[0:-1]
