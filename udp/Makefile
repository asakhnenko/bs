default: sender_udp receiver_udp received_dir

sender_udp: sender_udp.c
	gcc -o sender_udp sender_udp.c -lssl -lcrypto

receiver_udp: receiver_udp.c
	gcc -o receiver_udp receiver_udp.c -lssl -lcrypto

received_dir:
	mkdir -p received

clean:
	rm -rf sender_udp receiver_udp received
