# PFM ABE - lib/utility

The PFM ABE suite of tools consists of many individual components that each carry a version number.  When put all together it is considered PFM ABE.The components can fall into 1 of 4 groups.
- ABE
- Utilities
- Libraries
- Open Source Libraries and tools

**utility** is part of the **lib** group.

## Version History

|App Version|Release Date|ABE Version|Notes|
|-------|------------|-----|---|
|V2.2.43|08/07/19|V7.0.0.0|JCD -Added SHAPE file support to get_area_mbr.c  |
|V2.2.44|01/22/20|V7.0.0.1|JCD - Removed getSystemInfo.cpp and .hpp because they were not being used and they caused compiler warnings.<br>Defined a couple of integer variables in get_area_mbr.c because they were causing errors when not using the c99 option to the compiler.|
    - 

## Notes

- I created a NBMakefile to be used for compiling with NetBeans
- The libnvutility.a ends up in the release directory.
	- TODO: Move it to the libs directory in the compile folder.
