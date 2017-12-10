default: sender_udp receiver_udp

sender_udp: sender_udp.c
	gcc -o sender_udp sender_udp.c -lssl -lcrypto

receiver_udp: receiver_udp.c
	gcc -o receiver_udp receiver_udp.c -lssl -lcrypto

clean:
	rm sender_udp
