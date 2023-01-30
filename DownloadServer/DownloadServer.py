import socket
import os

int_max_value = 4294967295

filename = "ota.bin"
port = 8082

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



with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
    s.bind(('', port))
    print("Listening at port "+str(port))
    s.listen()
    while True:
        c, addr =s.accept()
        print(f"Connected by {addr}")
        #data = c.recv(1024)
        c.send(bytes)
        print("Sent header")
        f = open(filename, 'rb')
        c.send(f.read())
        f.close()
        print("Sent binary")
        c.close()