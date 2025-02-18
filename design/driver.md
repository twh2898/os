# Drivers

There are 2 core concepts, a driver and a device. Drivers are registered
for later use in creating a device. The device will have a copy of the driver
function calls to operate. Both can be searched by string name.

You can think of the driver as a class definition while a device is an instance
of that class. Drivers are registered so they can be later retrieved by name.

## Example

Registering a driver

```c
register_driver("ata", DRIVER_TYPE_IO, driver_struct);
```

Register a device. Because the device takes a void * for driver data, it can be
any object or value used by the driver's functions (eg. an ata driver can take
an ata disk object as it's driver data).

```c
void * ata_disk = ata_create(0);
register_device("/dev/ata/0", ata_disk, "ata");
```
