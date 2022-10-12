#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "packet.h"

size_t create_packet(uint8_t client_id, uint16_t type,
                    uint8_t tech, uint32_t src_sub_no, uint8_t **packet)
{
    if(type != PACKET_TYPE_ACC_PER && type != PACKET_TYPE_NOT_PAID 
        && type != PACKET_TYPE_NOT_EXIST && type != PACKET_TYPE_ACC_OK )
    {
        if(type == PACKET_TYPE_EXIST_NO_TECHNO )
            {}else
        return 0;
    }

    uint8_t *ret = (uint8_t *)malloc(PACKET_SIZE);
    if (!ret)
    {
        return 0;
    }

    *(uint16_t *)ret = htons(PACKET_IDENTIFIER); 
    ret[CLIENT_ID_OFFSET] = client_id;
    *(uint16_t *)(ret + TYPE_OFFSET) = htons(type); 
    ret[SEG_NO_OFFSET] = 0;
    ret[LENGTH_OFFSET] = 5;
    ret[TECH_OFFSET] = tech;
    *(uint32_t *)(ret + SRC_SUB_NO_OFFSET) = htonl(src_sub_no);
    *(uint16_t *)(ret + END_PACKET_OFFSET) = htons(PACKET_IDENTIFIER);

    *packet = ret;
    return PACKET_SIZE;
}



uint8_t get_client_id(const uint8_t *packet)
{
    return packet[CLIENT_ID_OFFSET];
}

uint16_t get_type(const uint8_t *packet)
{
    return ntohs(*(uint16_t *)(packet + TYPE_OFFSET));
}

uint8_t get_length(const uint8_t *packet)
{
    return packet[LENGTH_OFFSET];
}


uint8_t get_segment_no(const uint8_t *packet)
{
    return packet[SEG_NO_OFFSET];
}

uint8_t get_tech(const uint8_t *packet)
{
    return packet[TECH_OFFSET];
}

uint32_t get_src_sub_no(const uint8_t *packet)
{
    return ntohl(*(uint32_t *)(packet + SRC_SUB_NO_OFFSET));
}

uint16_t check_packet(const uint8_t *packet, size_t sz)
{
    if (sz != PACKET_SIZE)
    {
        return -1;
    }
    return 0;
}

void print_buffer(const uint8_t *buffer, size_t sz)
{
    printf("[DEBUG] buffer size: %lu\n", sz);
    printf("[DEBUG] buffer data: ");
	size_t i;
    for (i = 0; i < sz; ++i)
    {
        printf("%02X ", buffer[i]);
    }
    printf("\n");
}

long long parse_sub_no(char *str, int l)
{
    long long ret = 0;
int i;
    for(i = 0; i < l; i++)
    {
        if(str[i] == '-')
        {
            continue;
        }
        else if(str[i] < '0' || str[i] > '9')
        {
            return -1;
        }
        ret = ret * 10 + (str[i] - '0');
    }
    return ret;
}

