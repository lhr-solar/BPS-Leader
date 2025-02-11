import can
import time
import serial

class pyCandapter:
    def __init__(self, port, baudrate = 9600) -> None:
        self.device = serial.Serial(port=port, baudrate=baudrate, timeout=None)

    def openCANBus(self, baudrate) -> bool:
        baudrateCodes = {10000: 0, 20000: 1, 50000: 2, 100000: 3, 125000: 4, 250000: 5, 500000: 6, 800000: 7, 1000000: 8}
        try:
            code = baudrateCodes[baudrate]
        except KeyError:
            raise ValueError('Invalid baudrate')
        status = self.sendSerialMessage('S{}'.format(code))
        if status == True:
            status = self.sendSerialMessage('O')
            if status == True:
                return True
            else:
                raise ValueError('Error opening CAN bus')
        else:
            raise ValueError('Error setting baudrate')

    def sendSerialMessage(self, message) -> bool:
        self.device.write('{}\n'.format(message).encode())
        time.sleep(0.1)
        if self.device.read() == b'\x06':
            return True
        else:
            return False

    def readCANMessage(self) -> can.Message:
        message = self.device.read_until(b'\r').decode('utf-8') #This is in the format Tiiilddddddddddddddd\r
        messageID = int(message[1:4], 16)
        messageLength = int(message[4])
        messageDataArr = []
        for i in range(messageLength):
            messageDataArr.append(int(message[5 + 2*i :5 + 2*i + 2], 16))
        timeStamp = time.time()
        return (can.Message(arbitration_id=messageID, data=messageDataArr, is_extended_id=False, timestamp=timeStamp))

    def sendCANMessage(self, message):
        dataString = ''
        for i in range(0, len(message.data)):
            messageDataString = str(hex(message.data[i]))[2:]
            if len(messageDataString) == 1:
                messageDataString = '0' + messageDataString
            dataString += messageDataString
        messageIDString = str(hex(message.arbitration_id))[2:]
        while len(messageIDString) < 3:
            messageIDString = '0' + messageIDString
        response = self.sendSerialMessage('T{id}{length}{data}'.format(id = messageIDString, length = len(message.data), data = dataString))
        if response != True:
            raise ValueError('Error sending CAN message')
        else:
            return True

    def closeCANBus(self) -> bool:
        response = self.sendSerialMessage('C')

    def closeDevice(self) -> None:
        self.device.close()
