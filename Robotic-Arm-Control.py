from subprocess import Popen, PIPE
import Adafruit_PCA9685	# Import the library used to communicate with PCA9685
import sys
import time
import datetime
from numpy import interp #To enable interpratation and interpolation of values from master at the slave motors
import RPi.GPIO as GPIO  # To enable reading out and writing to the Raspberry pi input/output pins
from  socket import *

Ec = 8
GPIO.setmode(GPIO.BCM)
GPIO.setup(Ec, GPIO.IN)
pwm = Adafruit_PCA9685.PCA9685()	# Instantiate the object used to control the pulse width modulation (PWM) for the motors
pwm.set_pwm_freq(50)	# Set the frequency of the PWM signal
program_path = "path to the executable socket initiating code"
dict={}
# Read values sent from robotic arm 
p = Popen([program_path], stdout=PIPE, stdin=PIPE, universal_newlines=True)
#This is initializing the minimum position values for the 4DOF robotic arm motors in every axis
xaxis=100x
yaxis1=100
yaxis2=100
zaxis=100
packet = 0


# Define TCP socket parameters
ip = " IP"  # The server's hostname or IP address
port = "PORT"  # The port used by the server
connectserver = (ip, port)


# function to scale values
def translate(value, leftmin, leftmax, rightmin, rightmax):
    leftspan = leftmax - leftmin
    rightspan = rightmax -rightmin

    valuescaled  = float(value-leftmin) / float(leftspan)
    return rightmin + (valuescaled * rightspan)

      
while True:
    #Create RCP socket
    s = socket(AF_INET, SOCK_STREAM)

    result= p.stdout.readline().strip()
    # Process output from robotic arm
    output=result.split(':')
    #store arm movement values in dictionary for easy organization and access
    dict[output[0]]=float(output[1])
    current_time= round(time.time()*1000)
    print (current_time,"dict1:",dict)     # DEBUG
    
    #Mapping of haptic device input to the motor input range, i.e. the input range at master is between -0.219729 and 0.209877, while robotic arm motors input ranges from 100 to 560

    if 'X' in dict:
        xaxis = int(translate(dict['X'], -0.219729, 0.209877, 100, 560)
    if 'Y' in dict:
        yaxis1=int(translate(dict['Y'], -0.219729, 0.209877, 100, 560))
        yaxis2=int(translate(dict['Y'], -0.219729, 0.209877, 100, 560))
    if 'Z' in dict:
        zaxis=int(translate(dict['Z'], -0.219729, 0.209877, 100, 560))
    if 'Packet_Label' in dict:
        packet=int(dict['Packet_Label'])
        
    
    print ("translated_xaxis:", xaxis, "translated_yaxis1:", yaxis1,"translated_yaxis2:",yaxis2,"translated_zaxis:",zaxis,"translated_packet_label",packet)
   

    pwm.set_pwm(0, 0, xaxis)
    pwm.set_pwm(1, 0, yaxis1)
    pwm.set_pwm(2, 0, yaxis2)
    #Enable connection between the master and slave  in the feedback loop and send data
    s.connect(connectserver)
    send_data = packet;
    s.send(str(send_data).encode('utf8'))
    s.close()
    s = socket(AF_INET, SOCK_STREAM)
    s.connect(connectserver)
    if GPIO.input(Ec):
       send_data =1;
       s.send(str(send_data).encode('utf8'))
       
    else:
       send_data = 0;
       s.send(str(send_data).encode('utf8'))
 # Enable acknowledgement when haptic feedback is received at the master controller
    answer = s.recv(1024)
    #Close TCP socket
    s.close()
