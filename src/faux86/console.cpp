/*
  Faux86: A portable, open-source 8086 PC emulator.
  Copyright (C)2018 James Howard
  Based on Fake86
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

#include "Config.h"
// TODO
#ifdef WITH_DEBUG_CONSOLE
#include "VM.h"
#include <SDL/SDL.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "cpu.h"
#include "DriveManager.h"

#ifdef _WIN32
#include <conio.h>
#define strcmpi _strcmpi
#else
#define strcmpi strcasecmp
#endif

using namespace Faux86;

char inputline[1024];
uint16_t inputptr = 0;

void waitforcmd (char *dst, uint16_t maxlen) {
#ifdef _WIN32
	uint16_t inputptr;
	uint8_t cc;

	inputptr = 0;
	maxlen -= 2;
	inputline[0] = 0;
	while (vm.running) {
			if (_kbhit () ) {
					cc = (uint8_t) _getch ();
					switch (cc) {
							case 0:
							case 9:
							case 10:
								break;
							case 8: //backspace
								if (inputptr > 0) {
										printf ("%c %c", 8, 8);
										inputline[--inputptr] = 0;
									}
								break;
							case 13: //enter
								printf ("\n");
								return;
							default:
								if (inputptr < maxlen) {
										inputline[inputptr++] = cc;
										inputline[inputptr] = 0;
										printf ("%c",cc);
									}
						}
				}
			SDL_Delay(10); //don't waste CPU time while in the polling loop
		}
#else
	fgets (dst, maxlen, stdin);
#endif
}

void consolehelp () {
	printf ("\nConsole command summary:\n");
	printf ("  The console is not very robust yet. There are only a few commands:\n\n");
	printf ("    change fd0        Mount a new image file on first floppy drive.\n");
	printf ("                      Entering a blank line just ejects any current image file.\n");
	printf ("    change fd1        Mount a new image file on first floppy drive.\n");
	printf ("                      Entering a blank line just ejects any current image file.\n");
	printf ("    help              This help display.\n");
	printf ("    quit              Immediately abort emulation and close Faux86.\n");
}

#ifdef _WIN32
void runconsole (void *dummy) {
#else
void *runconsole (void *dummy) {
#endif
	printf ("\nFaux86 management console\n");
	printf ("Type \"help\" for a summary of commands.\n");
	while (vm.running) {
			printf ("\n>");
			waitforcmd (inputline, sizeof(inputline) );
			if (strcmpi ( (const char *) inputline, "change fd0") == 0) {
					printf ("Path to new image file: ");
					waitforcmd (inputline, sizeof(inputline) );
					if (strlen (inputline) > 0) {
							vm.drives.insertDisk (DRIVE_A, new ImagedDisk(inputline));
						}
					else {
							vm.drives.ejectDisk (DRIVE_A);
							printf ("Floppy image ejected from first drive.\n");
						}
				}
			else if (strcmpi ( (const char *) inputline, "change fd1") == 0) {
					printf ("Path to new image file: ");
					waitforcmd (inputline, sizeof(inputline) );
					if (strlen (inputline) > 0) {
							vm.drives.insertDisk (DRIVE_B, new ImagedDisk(inputline));
						}
					else {
							vm.drives.ejectDisk (DRIVE_B);
							printf ("Floppy image ejected from second drive.\n");
						}
				}
			else if (strcmpi ( (const char *) inputline, "help") == 0) {
					consolehelp ();
				}
			else if (strcmpi ( (const char *) inputline, "quit") == 0) {
					vm.running = false;
				}
			else printf("Invalid command was entered.\n");
		}
}

#endif