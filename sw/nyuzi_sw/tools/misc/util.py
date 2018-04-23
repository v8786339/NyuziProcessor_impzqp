import os
import re

# Calculate checksum like specified in Intel HEX standard
def checksum(frame):

	byte_string = ""
	sum = 0
	
	length = len(frame)
	i = 0
	
	# Get sum of bytes of current record
	while i < (length - 1):
		byte_string = frame[length - 1 - i] + byte_string
		if len(byte_string) == 2 or i+2 >= length:
			sum = sum + s2h(byte_string)
			byte_string = ""
		i = i+1
	
	# Take two last bytes of sum (LSBs), invert them and add 1
	return (((~(sum % 0x100)) & 0x000000FF) + 1) & 0x000000FF

# Conversion hex value to hex string
def h2s(hex_value):
	return format(hex_value, '02X')

# Conversion hex string to hex value
def s2h(hex_string):
	return int(hex_string, 16)
	
# Build ihex entry for setting the extended linear address to base_address
def get_extended_linear_address_frame(base_address_h, base_address_l):
	# number of bytes
	frame = ":02"
	# padding
	frame = frame + "0000"
	# op code = Extended Linear Address
	frame = frame + "04"
	# address
	frame = frame + h2s(base_address_h) + h2s(base_address_l)
	# checksum
	frame = frame + h2s(checksum(frame))
	# add newline
	frame = frame + "\n"
	return frame
	
# Import all modules of a specified path
def loadImports(path):
    files = os.listdir(path)
    imps = []

    for i in range(len(files)):
        name = files[i].split('.')
        if len(name) > 1:
            if name[1] == 'py' and name[0] != '__init__':
               name = name[0]
               imps.append(name)

    file = open(path+'__init__.py','w')

    toWrite = '__all__ = '+str(imps)

    file.write(toWrite)
    file.close()