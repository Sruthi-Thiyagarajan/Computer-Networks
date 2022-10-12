#ifndef PACKET_H
#define PACKET_H

#define PACKET_IDENTIFIER 0xFFFF
#define PACKET_TYPE_ACC_PER 0xFFF8
#define PACKET_TYPE_NOT_PAID 0xFFF9
#define PACKET_TYPE_NOT_EXIST 0xFFFA
#define PACKET_TYPE_ACC_OK 0xFFFB
#define PACKET_TYPE_EXIST_NO_TECHNO 0xFFF2



#define CLIENT_ID_OFFSET 2
#define TYPE_OFFSET 3
#define SEG_NO_OFFSET 5
#define LENGTH_OFFSET 6
#define TECH_OFFSET 7
#define SRC_SUB_NO_OFFSET 8
#define END_PACKET_OFFSET 12

#define PACKET_SIZE 14
#define TECH_2G 2
#define TECH_3G 3
#define TECH_4G 4
#define TECH_5G 5
#define MAX_SUB_NO 0xFFFFFFFF


size_t create_packet(uint8_t client_id, uint16_t type,
                     uint8_t tech, uint32_t src_sub_no, 
                     uint8_t **packet);

uint8_t get_client_id(const uint8_t *packet);
uint16_t get_type(const uint8_t *packet);
uint8_t get_length(const uint8_t *packet);
uint8_t get_segment_no(const uint8_t *packet);
uint8_t get_tech(const uint8_t *packet);
uint32_t get_src_sub_no(const uint8_t *packet);


uint16_t check_packet(const uint8_t *packet, size_t sz);

void print_buffer(const uint8_t *buffer, size_t sz);

long long parse_sub_no(char * str, int l);

#endif 

