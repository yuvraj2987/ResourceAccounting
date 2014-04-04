

import deviceModule

fd = deviceModule.open()

print "Returned fd from open ", fd
ret2 = deviceModule.get_device(fd)
print "value passed to get_devices:", ret2
print "passing 2 values"
success = deviceModule.set_device_info(fd, 2)
print "success: ", success
