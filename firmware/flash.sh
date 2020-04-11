# Convert hex to package
nrfutil pkg generate --hw-version 52 --sd-req=0x00 --application .pio/build/nrf52840_dk/firmware.hex --application-version 1 firmware.zip

# Send to device via DFU
nrfutil dfu usb-serial -pkg firmware.zip -p $1
