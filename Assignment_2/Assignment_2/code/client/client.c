#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <netdb.h>
#include <time.h>
#include <stdbool.h>
#include "../package/packet.h"

#define CLIENT_ID 5
#define TIMEOUT 3
#define RETRY 3
#define BUFF_SIZE 512

int send_packet(int sockfd, struct addrinfo *p, uint8_t client_id,
                uint8_t tech, char* sub_no_str)
{
    int SIZE;
    uint8_t *packet;
    struct sockaddr_storage addr;
    uint8_t buff[BUFF_SIZE];
    // Packet recieved from server
    ssize_t rval;
    socklen_t fromlen;
    // Timeout
    struct timeval tv;
    
    long long parsed = parse_sub_no(sub_no_str, strlen(sub_no_str));
    if (parsed > MAX_SUB_NO || parsed < 0)
    {
        fprintf(stderr, "invalid sub_no %s", sub_no_str);
        return -1;
    }
    uint32_t sub_no = (uint32_t) parsed;

    memset(buff, 0, sizeof(buff));
    printf("---------- sending packet ----------\n");
   
    SIZE = create_packet(client_id,PACKET_TYPE_ACC_PER, 
                            tech, sub_no, &packet);
                                
    if (!SIZE)
    {
        perror("failed to create request packet\n");
        free(packet);
        return -1;
    }
    tv.tv_sec = TIMEOUT;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("error on setsockopt\n");
        free(packet);
        return -1;
    }
    print_buffer(packet, SIZE);
    int i;
    
    for (i = 0; i < RETRY; ++i)
    {
        printf("Packet sending to server in-progress: %d trying...\n", i + 1);
        sendto(sockfd, packet, SIZE, 0, p->ai_addr, p->ai_addrlen);
        rval = recvfrom(sockfd, buff, sizeof(buff), 0, (struct sockaddr *)&addr, &fromlen);
        if (rval >= 0)
        {
            printf("Packet has been received from server\n");
            print_buffer(buff, rval);
            if(rval == PACKET_SIZE)
            {
                uint16_t type = get_type(buff);
                if(type == PACKET_TYPE_NOT_PAID)
                {
                    printf("Not Paid\n");
                }
                else if(type == PACKET_TYPE_NOT_EXIST)
                {
                    printf("Subscriber isn't present in the database\n");
                }
                else if(type == PACKET_TYPE_ACC_OK)
                {
                    printf("Subscriber is permitted to access\n");
                }else if(type == PACKET_TYPE_EXIST_NO_TECHNO)
                {
                    printf("Subscriber is found but technology do not match\n");
                }
            }
            break;
        }
        printf("ack_timer timeout\n");
    }
    if (rval < 0)
    {
        perror("Server is not responding\n");
        free(packet);
        exit(1);
    }
    printf("\n");
    free(packet);
    return rval;
}

int main(int argc, char *argv[])
{
    setbuf(stdout, NULL);

    if (argc != 3)
    {
        fprintf(stderr,"Specify the arguments in this format : %s [IP/HOSTNAME] [PORT]\n", argv[0]);
        exit(1);
    }

    char *server_ip = argv[1];
    char *port = argv[2];
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rval;
    int numbytes;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rval = getaddrinfo(server_ip, port, &hints, &servinfo)) != 0)
    {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rval));
        return 1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next)
    {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
        {
            continue;
        }
        break;
    }

    if (p == NULL)
    {
        fprintf(stderr, "Unable to create socket connection. \n");
        return 2;
    }

    while (1) {
        char TECH[5];
    int choice;
    char str[20];
        setbuf(stdout, NULL);
        printf("");
        
        printf("Please enter the mobile number:\n");
        scanf("%s",&str);
        printf("Please enter your selection for technology (2G - 2, 3G -3 , 4G - 4, 5G - 5):\n");
        scanf("%s",&TECH);
        printf("----------\n");
    send_packet(sockfd, p, CLIENT_ID,(uint8_t)atoi(TECH), str);
    
    printf("Do you want to continue?? y/n: ");
    char reply;
    scanf("%s",&reply);
    if(reply=='n'){
       break;
    }
    }
    return 0;
}