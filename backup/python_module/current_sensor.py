import smbus

bus = smbus.SMBus(1)    # 0 = /dev/i2c-0 (port I2C0), 1 = /dev/i2c-1 (port I2C1)

DEVICE_ADDRESS = 0x40      #7 bit address (will be left shifted to add the read write bit)
DEVICE_REG_MODE1 = 0x04
DEVICE_REG_LEDOUT0 = 0x1d

def printValues():
    print "--- current Sensor Testing ---"
    #Write a single register
    #bus.write_byte_data(DEVICE_ADDRESS, DEVICE_REG_MODE1, 0x80)
    #var = bus.read_byte_data(DEVICE_ADDRESS, DEVICE_REG_MODE1);
    bus.write_word_data(DEVICE_ADDRESS, 0x05, 0x1010);
    current = bus.read_word_data(DEVICE_ADDRESS, 0x04);
    print ('Current  value:',  current)
    bus_vge = bus.read_word_data(DEVICE_ADDRESS, 0x02)#Bus vge register
    print ('Bus Voltage: ', bus_vge)
    shunt_vge = bus.read_word_data(DEVICE_ADDRESS, 0x01) #shunt voltage
    print('Shunt Voltage: ', shunt_vge)
    #Write an array of registers
    #ledout_values = [0xff, 0xff, 0xff, 0xff, 0xff, 0xff]
    #bus.write_i2c_block_data(DEVICE_ADDRESS, DEVICE_REG_LEDOUT0, ledout_values)

def main():
    # Get the name from the command line, using 'World' as a fallback.
    print "--- current sensor i2c testing ---"
    count = 10
    while count > 0:
        printValues()
        count-=1

    print "--- program ends ---"

# This is the standard boilerplate that calls the main() function.
if __name__ == '__main__':
      main()
