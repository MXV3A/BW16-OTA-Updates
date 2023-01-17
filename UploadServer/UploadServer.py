import socket
import os

int_max_value = 4294967295

filename = "ota.bin"
host = "192.168.32.224"
port = 8080

f = open(filename, "rb")
checksum = 0
length = os.stat(filename).st_size
empty = 0

byte = f.read(1)
while byte != b"":
    checksum = checksum+int.from_bytes(byte,'big')
    byte = f.read(1)
checksum = checksum % int_max_value
f.close()

print("Checksum: "+hex(checksum))
print("Length: "+str(length))

bytes = bytearray()
bytes = bytes + checksum.to_bytes(4, 'little')
bytes = bytes + empty.to_bytes(4, 'little')
bytes = bytes + length.to_bytes(4, 'little')


f = open(filename, 'rb')
with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.connect((host, port))
    s.send(bytes)
    s.send(f.read())

print("sent everything")

f.close()