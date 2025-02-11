import pyCandapter
import can
import signal

COMPORT = input('Enter COM number: ')
SERIALBAUDRATE = 9600
CANBAUDRATE = 250000

candapter = pyCandapter.pyCandapter('COM' + COMPORT, SERIALBAUDRATE)
candapter.openCANBus(CANBAUDRATE)

def signal_handler(sig, frame):
    candapter.closeCANBus()
    exit(0) 

signal.signal(signal.SIGINT, signal_handler)

while True:
    message = candapter.readCANMessage()
    print(message)
    candapter.sendCANMessage(message)

# pip install pyserial
# pip install python-can