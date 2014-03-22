
import smbus
import time

bus = smbus.SMBus(1)    # 0 = /dev/i2c-0 (port I2C0), 1 = /dev/i2c-1 (port I2C1)

DEVICE_ADDRESS = 0x40      #7 bit address (will be left shifted to add the read write bit)
DEVICE_REG_MODE1 = 0x04
DEVICE_REG_LEDOUT0 = 0x1d

def printValues():
    print "--- current Sensor Testing ---"
    #Write a single register
    #bus.write_byte_data(DEVICE_ADDRESS, DEVICE_REG_MODE1, 0x80)
    #var = bus.read_byte_data(DEVICE_ADDRESS, DEVICE_REG_MODE1);
    #bus.write_word_data(DEVICE_ADDRESS, 0x00, 0x1020);
    var  = bus.read_word_data(DEVICE_ADDRESS, 0x04);
    current = ((var&0xFF00) >> 8 )|((var&0x00FF)<<8)  
    print ('Current  value:',  current)
    var = bus.read_word_data(DEVICE_ADDRESS, 0x02)#Bus vge register
    temp = ((var&0xFF00) >> 8 ) | ((var&0x00FF)<<8)
    if temp>65000:
        temp=0
    bus_vge = (temp>>3)*4
    print ('Bus Voltage: ', bus_vge)
    time.sleep(0.1)
    var = bus.read_word_data(DEVICE_ADDRESS, 0x01) #shunt voltage
    shunt_vge = ((var&0xFF00) >>8 ) | ((var&0x00FF)<<8)
    if shunt_vge > 65000:
       shunt_vge=0
    print('Shunt Voltage: ', shunt_vge)

    c=shunt_vge+0.0
    c=c/10    
    print('Calculated Current: ', c)
    #Write an array of registers
    #ledout_values = [0xff, 0xff, 0xff, 0xff, 0xff, 0xff]
    #bus.write_i2c_block_data(DEVICE_ADDRESS, DEVICE_REG_LEDOUT0, ledout_values)
    print ""

def main():
    # Get the name from the command line, using 'World' as a fallback.
    print "--- current seinor configuration ---"
    bus.write_word_data (DEVICE_ADDRESS, 0x00, 0x9F08)


    print "--- current sensor i2c testing ---"
    count = 100
    while count > 0:
        printValues()
	time.sleep(0.5)
        count-=1

    print "--- program ends ---"

# This is the standard boilerplate that calls the main() function.
if __name__ == '__main__':
      main()
