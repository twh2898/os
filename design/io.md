# IO

Opening devices / files takes a formatted path with `:` separating the driver
id and path. The registered drivers can return a struct with functions for
that driver given it's data pointer.

```
ata:1
file:/data/file
dir:/data
```

```c
register_disk(ata, 1)
register_
```

This needs a list of disks with their driver and id / path. Opening the rest is
based from here. A new fs is created by driver and disk id. it can be registered
for the open file, unless open_file can have a prefix with the fs name.2


## alt

There is only io driver with str prefix. To add disk and fs, they are all
devices with a path and driver id?
