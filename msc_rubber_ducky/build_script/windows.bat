:: using bin2array to convert fat12_disk.ima in a byte array

cd ..
if not exist bin2array (git clone https://github.com/TheLivingOne/bin2array.git)
gcc bin2array\bin2array.c -o bin2array\bin2array.exe
bin2array\bin2array.exe fat12_disk.ima ramdisk msc_disk
mkdir include
move /y ramdisk* include

pause