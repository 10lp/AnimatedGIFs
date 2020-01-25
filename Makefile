img3.ffat: data/gifs64/*
	# Get from https://github.com/marcmerlin/esp32_fatfsimage/
	# noota_3gffat.csv 0x2F0000 is 3008KB
	fatfsimage img3.ffat 3008 data/

img12.ffat: data/gifs64/*
	# Get from https://github.com/marcmerlin/esp32_fatfsimage/
	# ffat.csv 0xBEF000 is 12220KB
	fatfsimage img12.ffat 12220 data/

flash: flash3

flash3: img3.ffat
	# IF you aren't using a 1/3GB split, 0x110000 will need to be updated
	for baud in 921600 460800; do esptool.py --chip esp32 --port /dev/ttyUSB0 --baud $$baud write_flash 0x110000 img3.ffat  && exit; done

flash16: img12.ffat
	# offset comes from espressif/esp32/tools/partitions/ffat.csv defined in espressif/esp32/boards.txt
	for baud in 921600 460800; do esptool.py --chip esp32 --port /dev/ttyUSB0 --baud $$baud write_flash 0x410000 img12.ffat && exit; done

