default: sender receiver received_dir

sender: sender_tcp.c Aufgabe2.h
	gcc -o sender sender_tcp.c -lssl -lcrypto -Wall

receiver: receiver_tcp.c Aufgabe2.h
	gcc -o receiver receiver_tcp.c -lssl -lcrypto -Wall

received_dir:
	mkdir -p received

clean:
	rm -rf sender receiver received *gz
