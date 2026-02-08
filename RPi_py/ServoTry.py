from gpiozero import Servo
from time import sleep

# The Pi 5 uses the RP1 chip; GPIO Zero handles this automatically
# Connect the signal wire to GPIO 18
servo = Servo(18)

try:
    while True:
        print("Moving to Min")
        servo.min()
        sleep(1)
        
        print("Moving to Mid")
        servo.mid()
        sleep(1)
        
        print("Moving to Max")
        servo.max()
        sleep(1)
        
except KeyboardInterrupt:
    print("Program stopped")