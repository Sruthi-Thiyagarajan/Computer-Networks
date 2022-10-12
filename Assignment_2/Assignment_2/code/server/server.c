#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>
#include "../package/packet.h"

#define BUFF_SIZE 26
#define CLIENT_SIZE 256
#define DB_MAX 100

typedef struct db_entry 
{
    uint32_t sub_no;
    uint8_t tech;
    bool paid;
} DB_ENTRY;


int main(int argc, char *argv[])
{
    setbuf(stdout, NULL);

    if (argc != 3)
    {
        fprintf(stderr,"usage: %s [PORT] [DB FILE]\n", argv[0]);
        exit(1);
    }

    char *port = argv[1];
    int sock;
    struct sockaddr_in server;
    struct sockaddr_storage addr;
    socklen_t fromlen;
    ssize_t rval;
    uint8_t buff[BUFF_SIZE];
    uint8_t *reply_packet;
    size_t reply_packet_sz;
    DB_ENTRY db_data[DB_MAX];
    int db_len = 0;
    char *db_file = argv[2];

    FILE *file;
    file = fopen(db_file, "r");
    if(file)
    {
        char sub_no[13];
        int tech, paid;
        long long parsed;
        while(fscanf(file, "%[^,],%d,%d\n", sub_no, &tech, &paid) != EOF)
        {
            parsed = parse_sub_no(sub_no, strlen(sub_no));
            if (parsed > MAX_SUB_NO || parsed < 0)
            {
                fprintf(stderr, "Skip invalid line %s,%d,%d\n", sub_no, tech, paid);
                continue;
            }
            db_data[db_len].sub_no = (uint32_t) parsed;
            db_data[db_len].tech = (uint8_t) tech;
            db_data[db_len].paid = (bool) paid;
            ++db_len;
            if (db_len == DB_MAX)
            {
                printf("Max num reached. Stop reading DB");
                break;
            }
        }
    }
    else
    {
        perror("Unable to read database file\n");
        exit(1);
    }

    printf("Socket connection is in-progress\n");
    sock = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock < 0)
    {
        perror("Failed to create socket connection\n");
        exit(1);
    }

    server.sin_family = PF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(atoi(port));

    
    if (bind(sock, (struct sockaddr *) & server, sizeof(server)))
    {
        perror("Unable to create connection to port\n");
        exit(1);
    }
    
    printf("Start receiving........\n\n");

    while (1)
    {
        fromlen = sizeof(struct sockaddr);
        memset(buff, 0, sizeof(buff));
        rval = recvfrom(sock, buff, sizeof(buff), 0, (struct sockaddr *)&addr, &fromlen);
        printf(" ---------- Received packet ---------- \n");
        if (rval < 0)
        {
            perror("Receieved error\n");
            continue;
        }
        else if (check_packet(buff, (size_t)rval) != 0)
        {
            perror("Skip invalid message\n");
            continue;
        }

        print_buffer(buff, rval);

        uint16_t packet_type = get_type(buff);
        if (packet_type != PACKET_TYPE_ACC_PER)
        {
            printf("Skip non-request packet\n");
            continue;
        }
        uint8_t client_id = get_client_id(buff);
        uint8_t tech = get_tech(buff);
        uint32_t src_sub_no = get_src_sub_no(buff);
        
        printf("Client id is: %u\n", client_id);
        printf("Selected technology is: %hhu\n", tech);
        printf("Your subscriber number is: %u\n", src_sub_no);

        bool found = false;
    int i;

        for(i = 0; i < db_len; i++)
        {
            if(db_data[i].sub_no == src_sub_no )
            {
                if(db_data[i].tech == tech){
                            if(db_data[i].paid)
                            {
                                printf("Subscriber is permitted to access\n");                   
                                reply_packet_sz = create_packet(client_id, PACKET_TYPE_ACC_OK, tech, src_sub_no, &reply_packet);
                            }
                            else
                            {
                                printf("Subscriber's payment is not done yet \n");
                                reply_packet_sz = create_packet(client_id, PACKET_TYPE_NOT_PAID, tech, src_sub_no, &reply_packet);
                            }
                            found = true;
                            break;
                }else{
                            printf("Technology mismatch found\n");
                            reply_packet_sz = create_packet(client_id, PACKET_TYPE_EXIST_NO_TECHNO, tech, src_sub_no, &reply_packet);
                            found = true;
                }
                
            }
        }
        if(!found)
        {
            printf("Subscriber isn't present in the database\n");
            reply_packet_sz = create_packet(client_id, PACKET_TYPE_NOT_EXIST, tech, src_sub_no, &reply_packet);
        }

        if(reply_packet_sz == 0)
        {
            perror("Packet creation got failed\n");
            continue;
        }

        sendto(sock, reply_packet, reply_packet_sz, 0, (struct sockaddr*)&addr, fromlen);
        printf("Packet has been sent from server\n");
        free(reply_packet);
        printf("\n");
    }
    return 0;
}
