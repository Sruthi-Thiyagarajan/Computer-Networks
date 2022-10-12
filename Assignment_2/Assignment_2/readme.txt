Please follow below steps:

Compile programming steps:
	1. 	cd src
	2. 	Compile server
    	gcc -o server/server server/server.c package/packet.c
	3. 	Compile client
    	gcc -o client/client client/client.c package/packet.c

Server & Client connection programming steps:
	1. 	Start server
    	server/server 5000 server/Verification_Database.txt

	2. 	Run client app for testing
    	client/client "localhost" 5000 or client/client 127.0.0.1 5000
