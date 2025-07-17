import time, re
from machine import UART, Pin, I2C

# Convert the latitude & longitude data to degrees, returning the values
def convertToDegree(data):
    if data == "":
        return 0
    raw = float(re.sub("[^0-9,.]", "", data))
    firstdigits = int(raw/100) 
    nexttwodigits = raw - float(firstdigits*100) 
    
    convertedData = float(firstdigits + nexttwodigits/60.0)
    convertedData = '{0:.6f}'.format(convertedData) 
    return str(convertedData)

# Get NMEA data from I2C and return the data
def obtainI2CData(i2c):
    data = b''
    size = int.from_bytes(i2c.readfrom_mem(0x42, 0xFD, 1), 8) << 8 | int.from_bytes(i2c.readfrom_mem(0x42, 0xFE, 1), 8)
    for i in range(size):
        data += i2c.readfrom_mem(0x42, 0xFF, 1)
    data = data.split(b'\r\n')
    return data

# Get NMEA data from UART and return the data
def obtainUARTData(uart):
    data = b''
    if (uart.any()):
        data = uart.read()
    data = data.split(b'\r\n')
    if (len(data) > 0):
        if (data[len(data) - 1][-1:] == b'\r'): # Get rid of trailing \r
            data[len(data) - 1] = data[len(data) - 1][:-1]
    return data

# Do checksum on NMEA data and return True / False
def nmeaChecksum(dataLine):
    if (len(dataLine) > 9): # 6 for the $GXXXX and 3 for *XX
        try:
            check = dataLine[-2:].decode("utf-8")
            data = dataLine[1:-3]
            value = 0
            for char in data:
                value = value ^ char
            checksum = str(hex((value & 0xF0) >> 4)[2:]) + str(hex((value & 0x0F))[2:]).upper()
            return checksum == check
        except Exception as e:
            print("Fatal error" + str(dataLine))
            return False
    return False

# Check if the data is valid and return True / False
def verifyValidData(dataLine):
    correct = False
    data = dataLine.decode("utf-8").split(",")
    head = data[0][-3:]
    #print(dataLine)
    if head == "GSA":
        if not (data[2] == "1"):
            correct = True
    elif head=="GSV":
        correct = True
    elif head == "VTG":
        if data[2] == "T" and data[4] == "M" and data[6] == "N" and data[8] == "K" and not (data[9] == "1"):
            correct = True
    elif head == "GGA":
        if data[10] == "M" and data[12] == "M" and not (data[6] == "0"):
            correct = True
    elif head == "RMC":
        if data[2] == "A" and not (data[12] == "N"):
            correct = True
        elif not data[1] == '': # When time data is obtained only, and still allow the data to be utilised
            correct = True
    elif head == "GLL":
        if data[6] == "A":
            correct = True
    elif head == "TXT":
        print(dataLine)
    else:
        print("CANNOT VERIFY DATA: " + str(dataLine))
    return correct

# Verifies data separated by NMEA messages and removes invalid or incorrect data
def verifyData(data):
    incorrect = 0;
    invalid = 0;
    verifiedData = []
    for dataLine in data:
        if (len(dataLine) > 9): # 6 for the $GXXXX and 3 for *XX
            if nmeaChecksum(dataLine):
                if verifyValidData(dataLine):
                    verifiedData.append(dataLine)
                else:
                    if not (dataLine.decode("utf-8")).startswith("$GNTXT"): #Exlude GNTXT as it doesn't pass valid data test
                        invalid += 1
                    print("Invalid: " + str(dataLine))
            else:
                incorrect += 1
                print("Incorrect: " + str(dataLine))
    if incorrect > 0:
        print(str(incorrect) + " - INCORRECT DATA FOUND")
    if invalid > 0:
        print(str(invalid) + " - INVALID DATA FOUND")
    return verifiedData

#GP for GPS
#GL for GLONASS
#GA for Galileo
#GB/BD for BeiDou
#GN for NMEA Talker ID with exception of GPGSV
def getConstellation(prefix):
    constellation = ""
    if prefix == "GP":
        constellation = "GPS"
    elif prefix == "GL":
        constellation = "GLONASS"
    elif prefix == "GA":
        constellation = "GALILEO"
    elif prefix == "GB" or prefix == "BD":
        constellation = "BEIDOU"
    elif prefix == "GN":
        constellation = "N/A"
    else:
        constellation = "INVALID"
        print("UNKNOWN CONSTELLATION: " + prefix)
    return constellation

# This function determines if two arrays have values which are the same, assuming that both arrays have the same size
def checkEqualOrCombine(data1, data2):
    equal = True
    for i in range(len(data1)):
        if (data1[i] == '') and (data2[i] != ''):
            data1[i] = data2[i]
        elif (data1[i] != '') and (data2[i] == ''):
            pass
        elif (data1[i] != data2[i]):
            equal = False
    return (equal, data1)

# This function recieves a 2D array of parsedData and tries to combine duplicate data values together, returning a condensed 2D array
def combineData(parsedData):
    combined = []
    combinedIndex = 0
    for data in parsedData:
        if (combined == []):
            combined.append(data)
        else:
            result = checkEqualOrCombine(combined[combinedIndex], data)
            if (result[0]):
                combined[combinedIndex] = result[1]
            else:
                combined.append(data)
                combinedIndex += 1
#                print("INCONSISTENCY: " + combined[i] + " and " + data[i] + " in data: " + str(data) + " full data: " + str(processingData))
    return combined

# Parses the data from the NMEA messages
def parseData(data):
    parsedData = []
    for dataLine in data:
        data = dataLine.decode("utf-8").split(",")
        head = data[0][-3:]
        constellation = getConstellation(data[0][1:3])
        GSV = False
        #DATA START
        latitude = ""
        longitude = ""
        satellites = ""
        GPStime = ""
        date = ""
        quality = ""
        
        diffTime = ""
        diffID = ""
        
        PRN = ""
        ID = ""
        
        PDOP = ""
        HDOP = ""
        VDOP = ""
        
        altitude = ""
        geoidSep = ""
        
        angle = ""
        speed = ""
        magnetic = ""
        #DATA END
        
        if head == "GSA":
            if data[1] == "M":
                quality = "Manual"
            elif data[1] == "A":
                quality = "Auto"
            else:
                print("UNKNOWN QUALITY: " + str(dataLine))
            if data[2] == "2":
                quality += " two dimensional positioning"
            elif data[2] == "3":
                quality += " three dimensional positioning"
            else:
                print("UNKNOWN QUALITY: " + str(dataLine))
            PRN = data[3:-5]
            PDOP = data[-4]
            HDOP = data[-3]
            VDOP = data[-2]
            if data[-1] == "1":
                ID = "GPS"
            elif data[-1] == "2":
                ID = "GLONASS"
            elif data[-1] == "3":
                ID = "GALILEO"
            elif data[-1] == "5":
                ID = "BEIDOU"
        elif head == "GSV":
            #print(dataLine)
            GSV = True
        elif head == "VTG":
            angle = data[1]
            magnetic = data[3]
            speed = data[5]# + " " + str(float(data[7]) / 1.852)
            if data[9][0] == "A":
                quality = "Autonomous positioning"
            elif data[9][0] == "D":
                quality = "Differential"
            elif data[9][0] == "E":
                quality = "Estimation"
            else:
                print("UNKNOWN QUALITY: " + str(dataLine))
        elif head == "GGA":
            GPStime = data[1]
            latitude = str(convertToDegree(data[2]) + data[3])
            longitude = str(convertToDegree(data[4]) + data[5])
            if data[6] == "1":
                quality = "Single Point positioning"
            elif data[6] == "2":
                quality = "Differential"
            elif data[6] == "4":
                quality = "Fixed solution"
            elif data[6] == "5":
                quality = "Floating point solution"
            else:
                print("UNKNOWN QUALITY: " + str(dataLine))
            satellites = data[7]
            HDOP = data[8]
            altitude = data[9]
            geoidSep = data[11]
            diffTime = data[13]
            diffID = data[14][:-3]
        elif head == "RMC":
            GPStime = data[1]
            if data[3] != '':
                latitude = str(convertToDegree(data[3]) + data[4])
                longitude = str(convertToDegree(data[5]) + data[6])
                speed = data[7]
                angle = data[8]
                date = data[9]
                magnetic = data[10] + data[11]
                if data[12] == "A":
                    quality = "Autonomous positioning"
                elif data[12] == "D":
                    quality = "Differential"
                elif data[12] == "E":
                    quality = "Estimation"
                else:
                    print("UNKNOWN QUALITY: " + str(dataLine))
        elif head == "GLL":
            latitude = str(convertToDegree(data[1]) + data[2])
            longitude = str(convertToDegree(data[3]) + data[4])
            GPStime = data[5]
        else:
            print("CANNOT PARSE DATA: " + str(dataLine))
        if not GSV:
            #print(constellation, end=",")
            
            #print(latitude, end=", ")
            #print(longitude, end=", ")
            #print(satellites, end=", ")
            #print(GPStime, end=", ")
            
            #print(date, end=", ")
            #print(quality, end=", ")
            if (diffTime != ''):
                print("DIFFTime: " + diffTime)
            if (diffID != '') and (diffID != '0000'):
                print("DIFFID: " + diffID) 
            #print(PRN, end=", ")
            #print(ID, end=", ")
            #print(PDOP, end=", ")
            #print(HDOP, end=", ")
            #print(VDOP, end=", ")
            #print(altitude, end=", ")
            #print(geoidSep, end=", ")
            #print(angle, end=", ")
            #print(speed, end=", ")
            if (magnetic != ''):
                print("Magnetic: " + magnetic)
#            print("{:<10}, {:<10}, {:<6}, {:<2}, {:<5}, {:<6}, {:<6}, {:<5}, {:<4}, {:<4}, {:<4}, {:<5}".format(latitude, longitude, GPStime, satellites, altitude, date, angle, speed, PDOP, HDOP, VDOP, geoidSep))
        
            parsedData.append([latitude, longitude, GPStime, satellites, altitude, date, angle, speed, PDOP, HDOP, VDOP, geoidSep])
        else:
            print(data)
    return combineData(parsedData)

# Write the data into a file
def writeData(combinedData, File):
    for data in combinedData:
        d = "{:<10}, {:<10}, {:<6}, {:<2}, {:<5}, {:<6}, {:<6}, {:<5}, {:<4}, {:<4}, {:<4}, {:<5}".format(data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11])
        File.write(d + "\n")
        print("Data:" + d)
        
#-----------------------------------------------------
# Example code
def run():
    #i2c = I2C(0, sda=Pin(4), scl=Pin(5), freq=400000, timeout=2000)
    uart = UART(1, baudrate=38400, tx=Pin(4), rx=Pin(5), timeout=1)
    #print("Start")
    #print("Started with addr: " + str(i2c.scan()[0]))

    File = open("data01.txt", "a") # Open text file
    File.write("Latitude, Longitude, Time, Satellites, Altitude, Date, Angle, Speed, PDOP, HDOP, VDOP, GeoidSep\n")
    print("Data:Latitude, Longitude, Time, Satellites, Altitude, Date, Angle, Speed, PDOP, HDOP, VDOP, GeoidSep")
    try:
        while True:
            time.sleep(0.9)
            # Get the data, verify it, then parse it and write to a file
            writeData(parseData(verifyData(obtainUARTData(uart))), File)
            #writeData(parseData(verifyData(obtainI2CData(uart))), File)
            File.flush()
    except KeyboardInterrupt:
        File.flush()
        print("Ended program")

run()