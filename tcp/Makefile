default: sender receiver received_dir

sender: sender_tcp.c Aufgabe2.h
	gcc -o sender_tcp sender_tcp.c -lssl -lcrypto

receiver: receiver_tcp.c Aufgabe2.h
	gcc -o receiver_tcp receiver_tcp.c -lssl -lcrypto

received_dir:
	mkdir -p received

clean:
	rm -rf sender_tcp receiver_tcp received *gz

valgrind_sender:
	valgrind --leak-check=yes ./sender 9090 send_dir

valgrind_receiver:
	valgrind --leak-check=yes ./receiver 127.0.0.1 9090
