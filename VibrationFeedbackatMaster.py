import RPi.GPIO as GPIO
import datetime
import time
import socket
import sys

ip="IP"
port="PORT"
# Create a TCP socket
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

# Bind the address to the port
server_address = (ip, port)
s.bind(server_address)
#GPIO pin that connects with the vibration motor
vibration_pin = 27

GPIO.setmode(GPIO.BCM)
GPIO.setup(vibration_pin, GPIO.OUT)
GPIO.setwarnings(False)
s.listen(1)
#Main function for vibration motor
def vibrate():
    while True:
       
       connectionSocket, addr = s.accept()
       data = connectionSocket.recv(1024)
       #convert recieved data to acceptable format
       strings = data.decode('utf8')
       value = int(strings)
       # 1 is presence of vibration, 0 is no vibrations received
       if value == 1:
           GPIO.output(vibration_pin, GPIO.HIGH)
       elif value == 0:
           GPIO.output(vibration_pin, GPIO.LOW)
       else:
           current_time= round(time.time()*1000)
           # label the packet
           print(current_time,"packet_label:",value)
       connectionSocket.send(data)
       connectionSocket.close()

vibrate()

GPIO.cleanup()
