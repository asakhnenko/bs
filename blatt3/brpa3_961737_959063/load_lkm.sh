# !/bin/bash
if [ -c /dev/brpa3_961737_959063 ]; then
	echo "Removing old version from kernel.."
	sudo rmmod brpa3_961737_959063.ko
fi

echo "Inserting new version.."
sudo insmod brpa3_961737_959063.ko
sudo chmod 666 /dev/brpa3_961737_959063
echo "Module brpa3_961737_959063 successfully embedded!"
