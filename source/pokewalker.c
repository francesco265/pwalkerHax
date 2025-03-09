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
	pkt->header.checksum = swap16(pkt->header.checksum);
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
	if (swap16(checksum) != compute_checksum(pkt))
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
	//inited_session_id = swap32(inited_session_id) ^ pkt_synack.header.session_id;
	inited_session_id = inited_session_id ^ pkt_synack.header.session_id;

	return true;
}

// Session must be already established
bool poke_eeprom_write(u16 addr, const void *data, u16 size)
{
	poke_packet pkt_write, pkt_write_ack;
	u8 buf[MAX_PAYLOAD_SIZE];
	int i = 0, buf_size = 0;

	while (size) {
		buf_size = size > MAX_PAYLOAD_SIZE - 1? MAX_PAYLOAD_SIZE - 1 : size;

		memcpy(buf + 1, data + i, buf_size);
		buf[0] = addr;

		create_poke_packet(&pkt_write, CMD_EEPROMWRITE, addr >> 8, buf, buf_size + 1);
		send_pokepacket(&pkt_write);

		if (!recv_pokepacket(&pkt_write_ack) || pkt_write_ack.header.opcode != CMD_EEPROMWRITE_ACK)
			return false;

		i += buf_size;
		addr += buf_size;
		size -= buf_size;
	}

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
	printf("Last sync time: %u\n", swap32(data->last_synctime));
	printf("Step count: %u\n", swap32(data->stepcount));
}

void poke_get_data(void)
{
	poke_packet pkt_req, pkt_idata;

	ir_enable();

	if (!poke_init_session()) {
		printf("Error while establishing session\n");
		ir_disable();
		return;
	}

	create_poke_packet(&pkt_req, CMD_ASKDATA, MASTER_EXTRA, NULL, 0);
	send_pokepacket(&pkt_req);

	if (!recv_pokepacket(&pkt_idata) || pkt_idata.header.opcode != CMD_DATA) {
		printf("Error while receiving identity_data\n");
		ir_disable();
		return;
	}

	create_poke_packet(&pkt_req, CMD_DISC, MASTER_EXTRA, NULL, 0);
	send_pokepacket(&pkt_req);

	print_identity_data((identity_data *) pkt_idata.payload);

	ir_disable();
}

void poke_add_watts(u16 watts)
{
	//poke_packet pkt_addwatts, pkt_addwatts_res, pkt_trigger, pkt_trigger_res;
	poke_packet pkt_req, pkt_ack;

	ir_enable();

	if (!poke_init_session()) {
		printf("Error while establishing session\n");
		ir_disable();
		return;
	}

	set_watts(watts);
	create_poke_packet(&pkt_req, CMD_WRITE, 0xF9, add_watts_payload, sizeof(add_watts_payload));
	send_pokepacket(&pkt_req);

	if (!recv_pokepacket(&pkt_ack) || pkt_ack.header.opcode != CMD_WRITE) {
		printf("Error while sending exploit\n");
		ir_disable();
		return;
	}

	create_poke_packet(&pkt_req, CMD_WRITE, 0xF7, trigger_exploit, sizeof(trigger_exploit));
	send_pokepacket(&pkt_req);

	if (!recv_pokepacket(&pkt_ack) || pkt_ack.header.opcode != CMD_WRITE) {
		printf("Error while triggering the exploit\n");
		ir_disable();
		return;
	}

	printf("SUCCESS!\n");

	ir_disable();
}

void poke_gift_item(u16 item)
{
	poke_packet pkt_req, pkt_ack;
	u8 item_name[384];
	u8 item_data[8] = {0};

	// Prepare data to send
	// This must be sent to 0xBD48
	string_to_img(item_name, 96, item_list[item], false);
	// This must be sent to 0xBD40
	item_data[6] = item;
	item_data[7] = item >> 8;

	ir_enable();

	if (!poke_init_session()) {
		printf("Error while establishing session\n");
		ir_disable();
		return;
	}

	if (!poke_eeprom_write(0xBD40, item_data, sizeof(item_data))) {
		printf("Error while writing item data on EEPROM\n");
		ir_disable();
		return;
	}

	if (!poke_eeprom_write(0xBD48, item_name, sizeof(item_name))) {
		printf("Error while writing item name bitmap on EEPROM\n");
		ir_disable();
		return;
	}

	create_poke_packet(&pkt_req, CMD_EVENTITEM, MASTER_EXTRA, NULL, 0);
	send_pokepacket(&pkt_req);

	if (!recv_pokepacket(&pkt_ack) || pkt_ack.header.opcode != CMD_EVENTITEM) {
		printf("Error while triggering the item event\n");
		ir_disable();
		return;
	}

	printf("SUCCESS!\n");

	ir_disable();
}

void poke_gift_pokemon(pokemon_data poke_data, pokemon_extradata poke_extra)
{
	poke_packet pkt_req, pkt_idata, pkt_ack;
	identity_data *idata;
	u8 animation[384], poke_name[320];
	char poke_str[24];

	string_upper(poke_str, poke_list[poke_data.poke]);
	// temporary
	memset(animation, 0, 192);
	memset(animation + 192, 0xFF, 192);
	string_to_img(poke_name, 80, poke_str, false);


	ir_enable();

	if (!poke_init_session()) {
		printf("Error while establishing session\n");
		ir_disable();
		return;
	}

	// Get identity data
	create_poke_packet(&pkt_req, CMD_ASKDATA, MASTER_EXTRA, NULL, 0);
	send_pokepacket(&pkt_req);

	if (!recv_pokepacket(&pkt_idata) || pkt_idata.header.opcode != CMD_DATA) {
		printf("Error while receiving identity_data\n");
		ir_disable();
		return;
	}
	idata = (identity_data *) pkt_idata.payload;

	// Write pokemon_data struct to 0xBA44
	if (!poke_eeprom_write(0xBA44, &poke_data, sizeof(pokemon_data))) {
		printf("Error while writing pokemon_data on EEPROM\n");
		ir_disable();
		return;
	}

	// Write pokemon_extradata struct to 0xBA54
	poke_extra.ot_tid = idata->trainer_tid;
	poke_extra.ot_sid = idata->trainer_sid;
	memcpy(poke_extra.trainer_name, idata->trainer_name, sizeof(poke_extra.trainer_name));
	if (!poke_eeprom_write(0xBA54, &poke_extra, sizeof(pokemon_extradata))) {
		printf("Error while writing pokemon_extradata on EEPROM\n");
		ir_disable();
		return;
	}

	// Write animation to 0xBA80
	if (!poke_eeprom_write(0xBA80, animation, sizeof(animation))) {
		printf("Error while writing animation on EEPROM\n");
		ir_disable();
		return;
	}

	// Write pokemon name image to 0xBC00
	if (!poke_eeprom_write(0xBC00, poke_name, sizeof(poke_name))) {
		printf("Error while writing pokemon name on EEPROM\n");
		ir_disable();
		return;
	}

	// Finally, trigger the event
	create_poke_packet(&pkt_req, CMD_EVENTPOKE, MASTER_EXTRA, NULL, 0);
	send_pokepacket(&pkt_req);

	if (!recv_pokepacket(&pkt_ack) || pkt_ack.header.opcode != CMD_EVENTPOKE) {
		printf("Error while triggering the pokemon event\n");
		ir_disable();
		return;
	}

	printf("SUCCESS!\n");

	ir_disable();
}
