#pragma once

#include <3ds/types.h>

#define MAX_PAYLOAD_SIZE 128
#define MAX_PACKET_SIZE (sizeof(packet_header) + MAX_PAYLOAD_SIZE)

// Commands
#define CMD_SYN		0xFA
#define CMD_SYNACK	0xF8
#define CMD_WRITE	0x06
#define CMD_ASKDATA	0x20
#define CMD_DATA	0x22
#define CMD_DISC	0xF4

#define MASTER_EXTRA 0x01
#define SLAVE_EXTRA 0x02

typedef struct {
	u8 opcode;
	u8 extra;
	u16 checksum;
	u32 session_id;
} packet_header;

typedef struct {
	packet_header header;
	u8 payload[MAX_PAYLOAD_SIZE];
	u16 payload_size;
} poke_packet;

typedef struct {
	u8 unk_0[12];
	u16 trainer_tid;
	u16 trainer_sid;
	u8 unique_data[56];
	u16 trainer_name[8];
	u8 unk_1[3];
	u8 flags;			// bit 0 = is registered, bit 1 = has pokemon
	u8 unk_2[4];
	u32 last_synctime;	// Big endian
	u32 stepcount;		// Big endian
} identity_data;

// exploits from https://dmitry.gr/?r=05.Projects&proj=28.%20pokewalker
// upload to 0xF956
static u8 add_watts_payload[] = {
	0x56,
	0x79, 0x00, 0x00, 0x64,	// mov.w #100, r0
	0x5E, 0x00, 0x1F, 0x40, // jsr addWatts_word ( == addWatts + 2 )
	0x79, 0x00, 0x08, 0xD6, // mov.w @&irHandleRxedByteIfAnyHasBeenRxed, r0
	0x5A, 0x00, 0x69, 0x3A, // jmp setProcToCallByMainInLoop
};

// upload to 0xF7E0
static const u8 trigger_exploit[] = {
	0xE0,
	0xF9, 0x56,
};

void poke_get_data(void);
void poke_add_watts(u16 watts);
