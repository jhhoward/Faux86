/*
  Faux86: A portable, open-source 8086 PC emulator.
  Copyright (C)2018 James Howard
  Base on Fake86
  Copyright (C)2010-2013 Mike Chambers

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

/* packet.c: functions to interface with libpcap/winpcap for ethernet emulation. */

#include "config.h"

#ifdef NETWORKING_ENABLED
#define HAVE_REMOTE
#define WPCAP
#include <pcap.h>
#include <stdint.h>
#include <stdlib.h>
#include <memory.h>

#ifndef _WIN32
#define PCAP_OPENFLAG_PROMISCUOUS 1
#endif

#include "ram.h"
#include "i8259.h"

extern uint8_t verbose;
uint8_t ethif, net_enabled = 0;
uint8_t dopktrecv = 0;
uint16_t rcvseg, rcvoff, hdrlen, handpkt;

pcap_if_t *alldevs;
pcap_if_t *d;
pcap_t *adhandle;
const u_char *pktdata;
struct pcap_pkthdr *hdr;
int inum;
uint16_t curhandle = 0;
char errbuf[PCAP_ERRBUF_SIZE];
uint8_t maclocal[6] = { 0xDE, 0xAD, 0xBE, 0xEF, 0x13, 0x37 };

void initpcap() {
	int i=0;

	printf ("\nObtaining NIC list via libpcap...\n");

	/* Retrieve the device list from the local machine */
#if defined(_WIN32)
	if (pcap_findalldevs_ex (PCAP_SRC_IF_STRING, NULL /* auth is not needed */, &alldevs, errbuf) == -1)
#else
	if (pcap_findalldevs (&alldevs, errbuf) == -1)
#endif
		{
			printf ("Error in pcap_findalldevs_ex: %s\n", errbuf);
			exit (1);
		}

	/* Print the list */
	for (d= alldevs; d != NULL; d= d->next) {
			i++;
			if (ethif==255) {
					printf ("%d. %s", i, d->name);
					if (d->description) {
							printf (" (%s)\n", d->description);
						}
					else {
							printf (" (No description available)\n");
						}
				}
		}

	if (i == 0) {
			printf ("\nNo interfaces found! Make sure WinPcap is installed.\n");
			return;
		}

	printf ("\n");

	if (ethif==255) exit (0);
	else inum = ethif;
	printf ("Using network interface %u.\n", ethif);


	if (inum < 1 || inum > i) {
			printf ("\nInterface number out of range.\n");
			/* Free the device list */
			pcap_freealldevs (alldevs);
			return;
		}

	/* Jump to the selected adapter */
	for (d=alldevs, i=0; i< inum-1 ; d=d->next, i++);

	/* Open the device */
#ifdef _WIN32
	if ( (adhandle= pcap_open (d->name, 65536, PCAP_OPENFLAG_PROMISCUOUS, -1, NULL, errbuf) ) == NULL)
#else
	if ( (adhandle= pcap_open_live (d->name, 65535, 1, -1, NULL) ) == NULL)
#endif
		{
			printf ("\nUnable to open the adapter. %s is not supported by WinPcap\n", d->name);
			/* Free the device list */
			pcap_freealldevs (alldevs);
			return;
		}

	printf ("\nEthernet bridge on %s...\n", d->description);

	/* At this point, we don't need any more the device list. Free it */
	pcap_freealldevs (alldevs);
	net_enabled = 1;
}

void setmac() {
	memcpy (&RAM[0xE0000], &maclocal[0], 6);
}

#ifndef NETWORKING_OLDCARD
extern int ne2000_can_receive();
extern void ne2000_receive (const uint8_t *buf, int size);

uint8_t newrecv[5000];

void dispatch() {
	uint16_t i;

	if (pcap_next_ex (adhandle, &hdr, &pktdata) <=0) return;
	if (hdr->len==0) return;
	if (ne2000_can_receive() ) {
			for (i=0; i<hdr->len; i++) {
					newrecv[i<<1] = pktdata[i];
					newrecv[ (i<<1) +1] = 0;
				}
			ne2000_receive (newrecv, hdr->len);
		}
	return;
}

void sendpkt (uint8_t *src, uint16_t len) {
	uint16_t i;
	for (i=0; i<len; i++) {
			printf ("%02X ", src[i]);
		}
	printf ("\n");
	pcap_sendpacket (adhandle, src, len);
}
#else
extern struct netstruct {
	uint8_t enabled;
	uint8_t canrecv;
	uint16_t pktlen;
} net;

void dispatch() {
	if (pcap_next_ex (adhandle, &hdr, &pktdata) <=0) return;
	if (hdr->len==0) return;

	net.canrecv = 0;
	memcpy (&RAM[0xD0000], &pktdata[0], hdr->len);
	net.pktlen = (uint16_t) hdr->len;
	if (verbose) {
			printf ("Received packet of %u bytes.\n", net.pktlen);
		}
	doirq (6);
	return;
}

void sendpkt (uint8_t *src, uint16_t len) {
	pcap_sendpacket (adhandle, src, len);
}
#endif

#endif
