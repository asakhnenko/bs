all: sender receiver

sender: sender_tcp.c
	gcc -o sender sender_tcp.c -Wall

receiver: receiver_tcp.c
	gcc -o receiver receiver_tcp.c -Wall

clean:
	rm sender
	rm receiver
