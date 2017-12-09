default: sender receiver received_dir

sender: sender_tcp.c 
	gcc -o sender sender_tcp.c -Wall

receiver: receiver_tcp.c
	gcc -o receiver receiver_tcp.c -Wall

received_dir:
	mkdir -p received

clean:
	rm -rf sender
	rm -rf receiver
	rm -rf received
