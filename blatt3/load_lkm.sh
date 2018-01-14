# !/bin/bash
echo "Preparing to load LKM"
echo "Compiling LKM"
make
echo "Removing old version"
sudo rmmod reverse.ko
echo "Inserting new version"
sudo insmod reverse.ko
sudo chmod 666 /dev/reverse
echo "Compiling test file"
gcc -o test test.c
gcc -o elgamal elgamal.c
echo "Done"
