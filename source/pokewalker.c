#include "pokewalker.h"
#include "ir.h"
#include "utils.h"
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <3ds/types.h>

static u32 inited_session_id; // Big endian

void set_watts(u16 watts)
{
	add_watts_payload[3] = watts >> 8;
	add_watts_payload[4] = watts & 0xFF;
}

// packet checksum must be zero
u16 compute_checksum(const poke_packet *pkt)
{
	const u8 *data = (u8 *) pkt;
    u16 checksum = 0x0002, size = sizeof(packet_header) + pkt->payload_size;

    for (size_t i = 1; i < size; i += 2)
        checksum += data[i];

    for (size_t i = 0; i < size; i += 2)
    {
        if ((data[i] << 8) > UINT16_MAX - checksum)
            ++checksum;

        checksum += data[i] << 8;
    }

    // Swap the bytes
    checksum = ((checksum << 8) & 0xFF00) | ((checksum >> 8) & 0xFF);

    return checksum;
}


void create_poke_packet(poke_packet *out, u8 opcode, u8 extra, const u8 *payload,
						u16 payload_size)
{
	out->header.opcode = opcode;
	out->header.extra = extra;
	out->header.checksum = 0;
	out->header.session_id = inited_session_id;
	out->payload_size = payload_size;

	if (payload)
		memcpy(out->payload, payload, payload_size);

	out->header.checksum = compute_checksum(out);
}

void send_pokepacket(poke_packet *pkt)
{
	pkt->header.checksum = convert_endian16(pkt->header.checksum);
	xor_data(pkt, sizeof(packet_header) + pkt->payload_size);
	ir_send_data(pkt, sizeof(packet_header) + pkt->payload_size);
}

bool recv_pokepacket(poke_packet *pkt)
{
	u16 checksum;
	u32 tc;

	tc = ir_recv_data(pkt, MAX_PACKET_SIZE);
	if (tc < sizeof(packet_header))
		return false;
	pkt->payload_size = tc - sizeof(packet_header);

	// XOR data
	xor_data(pkt, tc);

	// Validate checksum
	checksum = pkt->header.checksum;
	pkt->header.checksum = 0;
	if (convert_endian16(checksum) != compute_checksum(pkt))
		return false;
	pkt->header.checksum = checksum;

	return true;
}

bool wait_adv()
{
	u8 adv = 0;

	ir_recv_data(&adv, 1);

	return adv == 0x56;
}

bool poke_init_session(void)
{
	poke_packet pkt_syn, pkt_synack;

	//inited_session_id = rand();
	inited_session_id = 0;

	printf("Open the Comms app on the Pokewalker\n");
	printf("Waiting for PokeWalker...\n");
	if (!wait_adv()) {
		printf("No PokeWalker found\n");
		return false;
	}

	// Send SYN packet
	create_poke_packet(&pkt_syn, CMD_SYN, MASTER_EXTRA, NULL, 0);
	send_pokepacket(&pkt_syn);

	// Receive SYNACK packet
	if (!recv_pokepacket(&pkt_synack) || pkt_synack.header.opcode != CMD_SYNACK) {
		printf("Error while receiving SYNACK\n");
		return false;
	}

	// Compute session id
	//inited_session_id = convert_endian32(inited_session_id) ^ pkt_synack.header.session_id;
	inited_session_id = inited_session_id ^ pkt_synack.header.session_id;

	return true;
}

void print_identity_data(const identity_data *data)
{
	char trainer_name[8];

	decode_string(trainer_name, data->trainer_name);

	printf("Trainer name: %s\n", trainer_name);
	printf("Trainer TID: %04X\n", data->trainer_tid);
	printf("Trainer SID: %04X\n", data->trainer_sid);
	printf("Is paired: %s\n", data->flags & BIT(0) ? "yes" : "no");
	printf("Has pokemon: %s\n", data->flags & BIT(1) ? "yes" : "no");
	printf("Last sync time: %lu\n", convert_endian32(data->last_synctime));
	printf("Step count: %lu\n", convert_endian32(data->stepcount));
}

void poke_get_data(void)
{
	poke_packet pkt_askdata, pkt_data, pkt_disconnect;

	ir_enable();

	if (!poke_init_session()) {
		printf("Error while establishing session\n");
		ir_disable();
		return;
	}

	create_poke_packet(&pkt_askdata, CMD_ASKDATA, MASTER_EXTRA, NULL, 0);
	send_pokepacket(&pkt_askdata);

	if (!recv_pokepacket(&pkt_data) || pkt_data.header.opcode != CMD_DATA) {
		printf("Error while receiving identity_data\n");
		ir_disable();
		return;
	}

	create_poke_packet(&pkt_disconnect, CMD_DISC, MASTER_EXTRA, NULL, 0);
	send_pokepacket(&pkt_disconnect);

	print_identity_data((identity_data *) pkt_data.payload);

	ir_disable();
}

void poke_add_watts(u16 watts)
{
	poke_packet pkt_addwatts, pkt_addwatts_res, pkt_trigger, pkt_trigger_res;

	ir_enable();

	if (!poke_init_session()) {
		printf("Error while establishing session\n");
		ir_disable();
		return;
	}

	set_watts(watts);
	create_poke_packet(&pkt_addwatts, CMD_WRITE, 0xF9, add_watts_payload, sizeof(add_watts_payload));
	send_pokepacket(&pkt_addwatts);

	if (!recv_pokepacket(&pkt_addwatts_res) || pkt_addwatts_res.header.opcode != CMD_WRITE) {
		printf("Error while sending exploit\n");
		ir_disable();
		return;
	}

	create_poke_packet(&pkt_trigger, CMD_WRITE, 0xF7, trigger_exploit, sizeof(trigger_exploit));
	send_pokepacket(&pkt_trigger);

	if (!recv_pokepacket(&pkt_trigger_res) || pkt_trigger_res.header.opcode != CMD_WRITE) {
		printf("Error while triggering the exploit\n");
		ir_disable();
		return;
	}

	printf("SUCCESS!\n");

	ir_disable();
}
