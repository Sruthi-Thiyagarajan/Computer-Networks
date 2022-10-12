//Author: Jay Joshi
//Student ID: 159322


#define PORTNO 8081
#define PACKETID 0XFFFF
#define CLIENTID 0XFF
#define DATATYPE 0XFFF1
#define ENDPACKETID 0XFFFF
#define TIMEOUT 3
#define ACKPACKET 0XFFF2
#define REJECTPACKETCODE 0XFFF3
#define LENGTHMISMATCHCODE 0XFFF5
#define ENDPACKETIDMISSINGCODE 0XFFF6
#define OUTOFSEQUENCECODE 0XFFF4
#define DUPLICATECODE 0XFFF7

#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include<stdlib.h>
#include<strings.h>
#include<string.h>
#include<time.h>
#include<stdint.h>

struct Datapacket{
	uint16_t packetID;
	uint8_t clientID;
	uint16_t type;
	uint8_t sequence_number;
	uint8_t length;
	char payload[255];
	uint16_t endpacketID;
};
struct ackpacket {
	uint16_t packetID;
	uint8_t clientID;
	uint16_t type;
	uint8_t sequence_number;
	uint16_t endpacketID;
};
struct rejectpacket {
	uint16_t packetID;
	uint8_t clientID;
	uint16_t type;
	uint16_t subcode;
	uint8_t sequence_number;
	uint16_t endpacketID;
};

struct Datapacket initialise() {
	struct Datapacket data;
	data.packetID = PACKETID;
	data.clientID = CLIENTID;
	data.type = DATATYPE;
	data.endpacketID = ENDPACKETID;
	return data;
}
struct ackpacket ackinitialise() {
	struct ackpacket data;
	data.packetID = PACKETID;
	data.clientID = CLIENTID;
	data.type = ACKPACKET;
	data.endpacketID = ENDPACKETID;
	return data;
}
struct rejectpacket rejinitialise() {
	struct rejectpacket data;
	data.packetID = PACKETID;
	data.clientID = CLIENTID;
	data.type = ACKPACKET;
	data.endpacketID = ENDPACKETID;
	return data;
}

void print(struct Datapacket data) {
	printf("packetID: %x\n",data.packetID);
	printf("Client id : %hhx\n",data.clientID);
	printf("data: %x\n",data.type);
	printf("sequence no : %d \n",data.sequence_number);
	printf("length %d\n",data.length);
	printf("payload: %s",data.payload);
	printf("end of datapacket id : %x\n",data.endpacketID);
	printf("\n");
}

void ackprint(struct ackpacket adata) {
	printf("packetID: %x\n",adata.packetID);
	printf("Client id : %x\n",adata.clientID);
	printf("data: %x\n",adata.type);
	printf("sequence no : %d \n",adata.sequence_number+1);
	printf("end of datapacket id : %x\n",adata.endpacketID);
	printf("\n");
}
void rejprint(struct rejectpacket rdata) {
	printf("packetID: %x\n",rdata.packetID);
	printf("Client id : %hhx\n",rdata.clientID);
	printf("data: %x\n",rdata.type);
	printf("sequence no : %d \n",rdata.sequence_number+1);
	printf("Reject : fff3\n");
	printf("Subcode : %x\n",rdata.subcode);
	printf("end of datapacket id : %x\n",rdata.endpacketID);
	printf("\n");
}
int main(){
	struct Datapacket data;
	struct rejectpacket recievedpacket;
	struct sockaddr_in cliaddr;
	struct ackpacket ackdata;
	struct rejectpacket rej;
	socklen_t addr_size;
	FILE *fp;

	char line[255];
	int sockfd;
	int n = 0;
	int counter = 0;
	int sequenceNo = 1;

	int check = 0;
	
	sockfd = socket(AF_INET,SOCK_DGRAM,0);
	if(sockfd < 0) {
		printf("socket failed\n");
	}
	bzero(&cliaddr,sizeof(cliaddr));
	cliaddr.sin_family = AF_INET;
	cliaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	cliaddr.sin_port=htons(PORTNO);
	addr_size = sizeof cliaddr ;
	struct timeval tv;
	tv.tv_sec = TIMEOUT;  // 3 Secs Timeout
	tv.tv_usec = 0;
	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval));
	fp = fopen("input.txt", "rt");
	if(fp == NULL)
	{
		printf("cannot open file\n");
		exit(0);
	}
	
	while(1)
	{
	   printf("1. Send packet to server\n");
	   printf("2. Length Mismatch\n");
	   printf("3. End of packet error\n");
	   printf("4. Out of sequence error or Duplicate Packet\n");
	   int i;
	   scanf("%d",&i);
	   check = 0;
	   data = initialise();
	   ackdata = ackinitialise();
	   rej=rejinitialise();
	   
	   if(fgets(line, sizeof(line), fp) != NULL) {
		n = 0;
		counter = 0;
		printf("%s",line);
		data.sequence_number = sequenceNo;
		strcpy(data.payload,line);
		data.length = strlen(data.payload);
		data.endpacketID = ENDPACKETID;
	   }
		
	   switch (i)
	   {
		case 1:
			break;		
		case 2:
			data.length++;
			break;
		case 3:
			data.endpacketID= 0;
			break;
		case 4:
			data.sequence_number = 4; 
			break;
		default:
			printf("Invalid option selected");
			check = 1;
		}
		if(check == 0){
			while(n<=0 && counter<3){
				sendto(sockfd,&data,sizeof(struct Datapacket),0,(struct sockaddr *)&cliaddr,addr_size);
				n = recvfrom(sockfd,&recievedpacket,sizeof(struct rejectpacket),0,NULL,NULL);
				if(n <= 0 )
				{
					printf("No response from server for three seconds sending the packet again\n");
					counter ++;
				}
				else if(recievedpacket.type == ACKPACKET  ) {
					print(data);
					printf("Ack packet recieved \n ");
					ackdata.sequence_number = data.sequence_number-1;
					ackprint(ackdata);
				}
				else if(recievedpacket.type == REJECTPACKETCODE ) {
					printf("Reject Packet recieved \n");
						rej.subcode=recievedpacket.subcode;
						rej.sequence_number = data.sequence_number-1;

					if(recievedpacket.subcode == LENGTHMISMATCHCODE ) {
					printf("Length mismatch error\n");

					}
					if(recievedpacket.subcode == ENDPACKETIDMISSINGCODE ) {
						printf("END OF PACKET IDENTIFIER MISSING \n");
					}
						if(recievedpacket.subcode == OUTOFSEQUENCECODE ) {
						printf("OUT OF SEQUENCE ERROR \n");
					}
					if(recievedpacket.subcode == DUPLICATECODE) {
						printf("DUPLICATE PACKET RECIEVED BY THE SERVER \n");
					}
					rejprint(rej);
				}
			}
			if(counter >= 3 ) {
				printf("Server does not respond");
				exit(0);
			}
			sequenceNo++;
			printf("========================================================================\n");
		}
	}
}
