//
// GTULTRA V1.00
// Based on source code of GOATTRACKER v2.76 Stereo
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//

#define GOATTRK2_C

#ifdef __WIN32__
#include <windows.h>
#endif

#include "goattrk2.h"
#include "bme.h"

int menu = 0;
int recordmode = 1;
int followplay = 0;
int hexnybble = -1;
int stepsize = 4;
int autoadvance = 0;
int defaultpatternlength = 64;
int cursorflash = 0;
int cursorcolortable[] = { 1,2,7,2 };
int exitprogram = 0;
int eacolumn = 0;
int eamode = 0;
int paletteChanged = 0;

unsigned keypreset = KEY_TRACKER;
unsigned playerversion = 0;
int fileformat = FORMAT_PRG;
int zeropageadr = 0xfc;
int playeradr = 0x1000;
unsigned sidmodel = 0;
unsigned multiplier = 1;
unsigned adparam = 0x0f00;
unsigned ntsc = 0;
unsigned patterndispmode = 0;
unsigned sidaddress = 0xd400d420;
unsigned finevibrato = 1;
unsigned optimizepulse = 1;
unsigned optimizerealtime = 1;
unsigned customclockrate = 0;
unsigned usefinevib = 0;
unsigned b = DEFAULTBUF;
unsigned mr = DEFAULTMIXRATE;
unsigned writer = 0;
unsigned hardsid = 0;
unsigned catweasel = 0;
unsigned interpolate = 0;
unsigned residdelay = 0;
unsigned hardsidbufinteractive = 20;
unsigned hardsidbufplayback = 400;
unsigned monomode = 0;
float basepitch = 0.0f;
float equaldivisionsperoctave = 12.0f;
int tuningcount = 0;
double tuning[96];
extern unsigned bigwindow;
int checkUndoFlag = 0;
unsigned int lmanMode = 1;
unsigned int editPaletteMode = 0;
unsigned int enablekeyrepeat = 0;

float masterVolume = 1.0f;
float detuneCent = 0;


char configbuf[MAX_PATHNAME];
char loadedsongfilename[MAX_FILENAME];
char songfilename[MAX_FILENAME];
char songfilter[MAX_FILENAME];
char songpath[MAX_PATHNAME];
char instrfilename[MAX_FILENAME];
char instrfilter[MAX_FILENAME];
char instrpath[MAX_PATHNAME];
char packedpath[MAX_PATHNAME];

extern char *notename[];
char *programname = "$VER: GTUltra V1.1.2";
char specialnotenames[186];
char scalatuningfilepath[MAX_PATHNAME];
char tuningname[64];

char textbuffer[MAX_PATHNAME];
char infoTextBuffer[256];

char transportPolySIDEnabled[4];	// 0 = OFF 1 = ON (all OFF = mono)
char transportLoopPattern = 0;
char transportRecord = 1;
char transportPlay = 1;
char transportShowKeyboard = 0;
char jdebugPlaying = 0;

int selectedMIDIPort = 0;


unsigned char hexkeytbl[] = { '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

extern unsigned char datafile[];

int currentPalettePreset = 0;

unsigned char backupPaletteSong[MAX_CHN][MAX_SONGLEN + 2];
unsigned char paletteRGB[MAX_PALETTE_PRESETS][3][MAX_PALETTE_ENTRIES];

unsigned short tableBackgroundColors[MAX_TABLES][MAX_TABLELEN];
unsigned char paletteR[256];
unsigned char paletteG[256];
unsigned char paletteB[256];

int maxSIDChannels = 12;
int gMIDINote = -1;

WAVEFORM_INFO waveformDisplayInfo;


int main(int argc, char **argv)
{
	char filename[MAX_PATHNAME];
	char palettename[MAX_PATHNAME];

	FILE *configfile;
	int c, d;

	// JP: SDL2 produces no audio for Windows32 without explicitly setting this (otherwise, it's set to "dummy sound" as the output)
#ifdef __WIN32__
	SDL_setenv("SDL_AUDIODRIVER", "directsound", 1);
#endif


	programname += sizeof "$VER:";
	// Open datafile
	io_openlinkeddatafile(datafile);

	// load palette
	configfile = fopen("gtskins.bin", "rb");		// rb write binary.
	if (configfile)
	{
		fread(&paletteRGB, MAX_PALETTE_PRESETS * 3 * MAX_PALETTE_ENTRIES, 1, configfile);
		fclose(configfile);
	}
	else
	{
		int handle = io_open("gtskins.bin");
		if (handle == -1) return 0;
		io_read(handle, &paletteRGB, 384);
		io_close(handle);
	}

	// Load configuration
#ifdef __WIN32__
	GetModuleFileName(NULL, filename, MAX_PATHNAME);
	filename[strlen(filename) - 3] = 'c';
	filename[strlen(filename) - 2] = 'f';
	filename[strlen(filename) - 1] = 'g';
#elif __amigaos__
	strcpy(filename, "PROGDIR:goattrk2.cfg");
#else
	char* xdg_home = getenv("XDG_CONFIG_HOME");
	if (xdg_home)
	{
		strcpy(filename, xdg_home);
		strcat(filename, "/goattrk/gt2stereo.cfg");
	}
	else
	{
		strcpy(filename, getenv("HOME"));
		strcat(filename, "/.config/goattrk/gt2stereo.cfg");
	}
#endif
	configfile = fopen(filename, "rt");
	if (configfile)
	{
		getparam(configfile, &b);
		getparam(configfile, &mr);
		getparam(configfile, &hardsid);
		getparam(configfile, &sidmodel);
		getparam(configfile, &ntsc);
		getparam(configfile, (unsigned *)&fileformat);
		getparam(configfile, (unsigned *)&playeradr);
		getparam(configfile, (unsigned *)&zeropageadr);
		getparam(configfile, &playerversion);
		getparam(configfile, &keypreset);
		getparam(configfile, (unsigned *)&stepsize);
		getparam(configfile, &multiplier);
		getparam(configfile, &catweasel);
		getparam(configfile, &adparam);
		getparam(configfile, &interpolate);
		getparam(configfile, &patterndispmode);
		getparam(configfile, &sidaddress);
		getparam(configfile, &finevibrato);
		getparam(configfile, &optimizepulse);
		getparam(configfile, &optimizerealtime);
		getparam(configfile, &residdelay);
		getparam(configfile, &customclockrate);
		getparam(configfile, &hardsidbufinteractive);
		getparam(configfile, &hardsidbufplayback);
		getfloatparam(configfile, &filterparams.distortionrate);
		getfloatparam(configfile, &filterparams.distortionpoint);
		getfloatparam(configfile, &filterparams.distortioncfthreshold);
		getfloatparam(configfile, &filterparams.type3baseresistance);
		getfloatparam(configfile, &filterparams.type3offset);
		getfloatparam(configfile, &filterparams.type3steepness);
		getfloatparam(configfile, &filterparams.type3minimumfetresistance);
		getfloatparam(configfile, &filterparams.type4k);
		getfloatparam(configfile, &filterparams.type4b);
		getfloatparam(configfile, &filterparams.voicenonlinearity);
		getparam(configfile, (unsigned*)&win_fullscreen);
		getparam(configfile, &bigwindow);
		getfloatparam(configfile, &basepitch);
		getfloatparam(configfile, &equaldivisionsperoctave);
		getstringparam(configfile, specialnotenames);
		getstringparam(configfile, scalatuningfilepath);
		getparam(configfile, &maxSIDChannels);
		getparam(configfile, &currentPalettePreset);
		getfloatparam(configfile, &masterVolume);
		getfloatparam(configfile, &detuneCent);
		getparam(configfile, &enablekeyrepeat);
		getparam(configfile, &selectedMIDIPort);
		fclose(configfile);
	}

	// Init pathnames
	initpaths();

	// Scan command line
	for (c = 1; c < argc; c++)
	{
#ifdef __WIN32__
		if ((argv[c][0] == '-') || (argv[c][0] == '/'))
#else
		if (argv[c][0] == '-')
#endif
		{
			int y = 0;
			switch (argv[c][1]) //switch (toupper(argv[c][1]))
			{
			case '?':
				if (!initscreen())
					return 1;
				if (argv[c][2] == '?') {
					onlinehelp(1, 0, &gtObject);
					return 0;
				}

				printtext(0, y++, getColor(15, 0), "Usage: GT2STEREO [songname] [options]");
				printtext(0, y++, getColor(15, 0), "Options:");
				printtext(0, y++, getColor(15, 0), "-Axx Set ADSR parameter for hardrestart in hex. DEFAULT=0F00");
				printtext(0, y++, getColor(15, 0), "-Bxx Set sound buffer length in milliseconds DEFAULT=100");
				printtext(0, y++, getColor(15, 0), "-Cxx Use CatWeasel MK3 PCI SID (0 = off, 1 = on)");
				printtext(0, y++, getColor(15, 0), "-Dxx Pattern row display (0 = decimal, 1 = hex, 2 = decimal w/dots, 3 = hex w/dots)");
				printtext(0, y++, getColor(15, 0), "-Exx Set emulated SID model (0 = 6581 1 = 8580) DEFAULT=6581");
				printtext(0, y++, getColor(15, 0), "-Fxx Set custom SID clock cycles per second (0 = use PAL/NTSC default)");
				printtext(0, y++, getColor(15, 0), "-Gxx Set pitch of A-4 in Hz (0 = use default frequencytable, close to 440Hz)");
				printtext(0, y++, getColor(15, 0), "-Hxx Use HardSID (0 = off, 1 = HardSID ID0 2 = HardSID ID1 etc.)");
				printtext(0, y++, getColor(15, 0), "     Use high nybble (it's hexadecimal) to specify right HardSID ID");
				printtext(0, y++, getColor(15, 0), "-Ixx Set reSID interpolation (0 = off, 1 = on, 2 = distortion, 3 = distortion & on) DEFAULT=off");
				printtext(0, y++, getColor(15, 0), "-Jxx Set special note names (2 chars for every note in an octave/cycle, e.g. C-DbD-EbE-F-GbG-AbA-BbB-)");
				printtext(0, y++, getColor(15, 0), "-Kxx Note-entry mode (0 = PROTRACKER 1 = DMC) DEFAULT=PROTRK.");
				printtext(0, y++, getColor(15, 0), "-Lxx SID memory locations in hex. DEFAULT=D500D400");
				printtext(0, y++, getColor(15, 0), "-Mxx Set sound mixing rate DEFAULT=44100");
				printtext(0, y++, getColor(15, 0), "-Oxx Set pulseoptimization/skipping (0 = off, 1 = on) DEFAULT=on");
				printtext(0, y++, getColor(15, 0), "-Qxx Set equal divisions per octave (12 = default, 8.2019143 = Bohlen-Pierce)");
				printtext(0, y++, getColor(15, 0), "-Rxx Set realtime-effect optimization/skipping (0 = off, 1 = on) DEFAULT=on");
				printtext(0, y++, getColor(15, 0), "-Sxx Set speed multiplier (0 for 25Hz, 1 for 1x, 2 for 2x etc.)");
				printtext(0, y++, getColor(15, 0), "-Txx Set HardSID interactive mode sound buffer length in milliseconds DEFAULT=20, max.buffering=0");
				printtext(0, y++, getColor(15, 0), "-Uxx Set HardSID playback mode sound buffer length in milliseconds DEFAULT=400, max.buffering=0");
				printtext(0, y++, getColor(15, 0), "-Vxx Set finevibrato conversion (0 = off, 1 = on) DEFAULT=on");
				printtext(0, y++, getColor(15, 0), "-Xxx Set window type (0 = window, 1 = fullscreen) DEFAULT=window");
				printtext(0, y++, getColor(15, 0), "-Yxx Path to a Scala tuning file .scl");
				printtext(0, y++, getColor(15, 0), "-Zxx Set random reSID write delay in cycles (0 = off) DEFAULT=off");
				printtext(0, y++, getColor(15, 0), "-wxx Set window scale factor (1 = no scaling, 2 to 4 = 2 to 4 times bigger window) DEFAULT=1");
				printtext(0, y++, getColor(15, 0), "-N   Use NTSC timing");
				printtext(0, y++, getColor(15, 0), "-P   Use PAL timing (DEFAULT)");
				printtext(0, y++, getColor(15, 0), "-W   Write sound output to a file SIDAUDIO.RAW");
				printtext(0, y++, getColor(15, 0), "-cxx SID channel count (3,6,9 or 12) DEFAULT=6");
				printtext(0, y++, getColor(15, 0), "-pxx set UI Skin (0-3) DEFAULT=0");
				printtext(0, y++, getColor(15, 0), "-vxx Master Volume (floating point) DEFAULT=1(large values may cause clipping / distortion)");
				printtext(0, y++, getColor(15, 0), "-dxxx Detune Pitchtable (-1 > 1 0 = no detune. -1 = -1 semitone 1 = +1 semitone");
				printtext(0, y++, getColor(15, 0), "-kx   Enable key repeat (0=only on selected keys. 1= on everything (DEFAULT 0)");
				printtext(0, y++, getColor(15, 0), "-mxx  MIDI Port (DEFAULT 0)");
				printtext(0, y++, getColor(15, 0), "-?   Show this info again");
				printtext(0, y++, getColor(15, 0), "-??  Standalone online help window");
				waitkeynoupdate();
				return 0;

			case 'Z':
				sscanf(&argv[c][2], "%u", &residdelay);
				break;

			case 'A':
				sscanf(&argv[c][2], "%x", &adparam);
				break;

			case 'S':
				sscanf(&argv[c][2], "%u", &multiplier);
				break;

			case 'B':
				sscanf(&argv[c][2], "%u", &b);
				break;

			case 'D':
				sscanf(&argv[c][2], "%u", &patterndispmode);
				break;

			case 'E':
				sscanf(&argv[c][2], "%u", &sidmodel);
				break;

			case 'I':
				sscanf(&argv[c][2], "%u", &interpolate);
				break;

			case 'K':
				sscanf(&argv[c][2], "%u", &keypreset);
				break;

			case 'L':
				sscanf(&argv[c][2], "%x", &sidaddress);
				break;

			case 'N':
				ntsc = 1;
				customclockrate = 0;
				break;

			case 'P':
				ntsc = 0;
				customclockrate = 0;
				break;

			case 'F':
				sscanf(&argv[c][2], "%u", &customclockrate);
				break;

			case 'M':
				sscanf(&argv[c][2], "%u", &mr);
				break;

			case 'O':
				sscanf(&argv[c][2], "%u", &optimizepulse);
				break;

			case 'R':
				sscanf(&argv[c][2], "%u", &optimizerealtime);
				break;

			case 'H':
				sscanf(&argv[c][2], "%x", &hardsid);
				break;

			case 'V':
				sscanf(&argv[c][2], "%u", &finevibrato);
				break;

			case 'T':
				sscanf(&argv[c][2], "%u", &hardsidbufinteractive);
				break;

			case 'U':
				sscanf(&argv[c][2], "%u", &hardsidbufplayback);
				break;

			case 'W':
				writer = 1;
				break;

			case 'X':
				sscanf(&argv[c][2], "%u", &win_fullscreen);
				break;

			case 'C':
				sscanf(&argv[c][2], "%u", &catweasel);
				break;

			case 'G':
				sscanf(&argv[c][2], "%f", &basepitch);
				break;

			case 'Q':
				sscanf(&argv[c][2], "%f", &equaldivisionsperoctave);
				break;

			case 'J':
				sscanf(&argv[c][2], "%s", specialnotenames);
				break;

			case 'Y':
				sscanf(&argv[c][2], "%s", scalatuningfilepath);
				break;

			case 'w':
				sscanf(&argv[c][2], "%u", &bigwindow);
				break;

			case 'c':
				sscanf(&argv[c][2], "%d", &maxSIDChannels);

			case 'p':
				sscanf(&argv[c][2], "%d", &currentPalettePreset);

			case 'v':
				sscanf(&argv[c][2], "%f", &masterVolume);

			case 'd':
				sscanf(&argv[c][2], "%f", &detuneCent);

			case 'k':
				sscanf(&argv[c][2], "%d", &enablekeyrepeat);

			case 'm':
				sscanf(&argv[c][2], "%d", &selectedMIDIPort);
			}
		}
		else
		{
			char startpath[MAX_PATHNAME];

			strcpy(songfilename, argv[c]);
			for (d = strlen(argv[c]) - 1; d >= 0; d--)
			{
				if ((argv[c][d] == '/') || (argv[c][d] == '\\'))
				{
					strcpy(startpath, argv[c]);
					startpath[d + 1] = 0;
					chdir(startpath);
					initpaths();
					strcpy(songfilename, &argv[c][d + 1]);
					break;
				}
			}
		}
	}


	// Validate parameters

	if (currentPalettePreset > 3)
		currentPalettePreset = 0;

	if (maxSIDChannels != 3 && maxSIDChannels != 6 && maxSIDChannels != 9 && maxSIDChannels != 12)
		maxSIDChannels = 6;

	sidmodel &= 1;
	adparam &= 0xffff;
	zeropageadr &= 0xff;
	playeradr &= 0xff00;
	if (!stepsize) stepsize = 4;
	if (multiplier > 16) multiplier = 16;
	if (keypreset > 2) keypreset = 0;
	if ((finevibrato == 1) && (multiplier < 2)) usefinevib = 1;
	if (finevibrato > 1) usefinevib = 1;
	if (optimizepulse > 1) optimizepulse = 1;
	if (optimizerealtime > 1) optimizerealtime = 1;
	if (residdelay > 63) residdelay = 63;
	if (customclockrate < 100) customclockrate = 0;

	if ((detuneCent < -1) || (detuneCent > 1))
	{
		detuneCent = 0;
	}

	if (enablekeyrepeat > 1)
		enablekeyrepeat = 0;

	// Read Scala tuning file
	if (scalatuningfilepath[0] != '0' && scalatuningfilepath[1] != '\0')
	{
		readscalatuningfile();
	}

	// Calculate frequencytable if necessary
	if (basepitch < 0.0f)
		basepitch = 0.0f;
	if (basepitch > 0.0f || detuneCent != 1)
		calculatefreqtable();

	// Set special note names
	if (specialnotenames[1] != '\0')
	{
		setspecialnotenames();
	}

	// JP - Init MIDI (yes. MIDI)
	selectedMIDIPort = initMidi(selectedMIDIPort);

	// Set screenmode
	if (!initscreen())
		return 1;


	waveformDisplayInfo.displayOnOff = 0;

	initPaletteDisplay();
	setTableBackgroundColours(0);

	initPolyKeyboard();
	// Reset channels/song
	initchannels(&gtObject);
	clearsong(1, 1, 1, 1, 1, &gtObject);

	initAreaListFlag = 0;
	initUndoBufferFlag = 0;
	undoInitAllAreas(&gtObject);	// Must be called after clearSong. Creates undo buffers, containing duplicates of each GT area.

	// Init sound
	if (!sound_init(b, mr, writer, hardsid, sidmodel, ntsc, multiplier, catweasel, interpolate, customclockrate))
	{
		printtextc(MAX_ROWS / 2 - 1, getColor(15, 0), "Sound init failed. Press any key to run without sound (notice that song timer won't start)");
		waitkeynoupdate();
	}


	// JP - Init Editor info
	editorInfo.editmode = EDIT_PATTERN;
	editorInfo.epoctave = 2;
	editorInfo.epmarkchn = -1;
	editorInfo.esmarkchn = -1;
	editorInfo.etlock = 0;		// was 1. changed to 0 for LMAN mode (tables unlocked)
	editorInfo.etmarknum = -1;

	editorInfo.einum = 1;	//jp

	// JP - Init GTObject
	gtObject.masterfader = 0xf;
	gtObject.controlEditor = 1;
	gtObject.noSIDWrites = 0;
	gtEditorObject.noSIDWrites = 1;
	gtLoopObject.noSIDWrites = 1;
	gtEditorLoopObject.noSIDWrites = 1;

	initSID(&gtObject);



	//-------------------------------------------------------------
#if 0
	strcpy(songfilename, "ultestura.sng");
	maxSIDChannels = 3;

	// Load song if applicable
	if (strlen(songfilename))
	{
		int ok = loadsong(&gtObject);
		if (ok)
		{
			undoInitAllAreas(&gtObject);	// recreate undo buffers, using the loaded song as the original info
			countInstruments();
			setTableBackgroundColours(editorInfo.einum);
		}
	}

#endif

#if 0
	initsong(editorInfo.esnum, PLAY_BEGINNING, &gtObject);
	followplay = shiftpressed;
	while (!exitprogram)
	{
		//waitkeymouse(&gtObject);
		if (key)
		{
			// Shutdown sound output now
			sound_uninit();
			return 0;
}

	}

#endif

	playUntilEnd();	// Get length of time of loaded or empty song

	// Start editor mainloop
	printmainscreen(&gtObject);


	while (!exitprogram)
	{
		int ch = checkFor3ChannelSong();

		waitkeymouse(&gtObject);
		docommand();
	}

	// Shutdown sound output now
	sound_uninit();


#ifndef __WIN32__
#ifdef __amigaos__
	strcpy(filename, "PROGDIR:gtskins.bin");
#else
	xdg_home = getenv("XDG_CONFIG_HOME");
	if (xdg_home)
	{
		strcpy(filename, xdg_home);
		strcat(filename, "/goattrk");
	}
	else
	{
		strcpy(filename, getenv("HOME"));
		strcat(filename, "/.config/goattrk");
	}
	mkdir(filename, S_IRUSR | S_IWUSR | S_IXUSR);
	strcat(filename, "/gtskins.bin");
#endif
#endif

	if (paletteChanged)
	{
		configfile = fopen("gtskins.bin", "wb");		// wb write binary. wt = write text
		if (configfile)
		{
			fwrite(&paletteRGB, MAX_PALETTE_PRESETS * 3 * MAX_PALETTE_ENTRIES, 1, configfile);
			fclose(configfile);
		}
	}


	// Save configuration
#ifndef __WIN32__
#ifdef __amigaos__
	strcpy(filename, "PROGDIR:goattrk2.cfg");
#else
	xdg_home = getenv("XDG_CONFIG_HOME");
	if (xdg_home)
	{
		strcpy(filename, xdg_home);
		strcat(filename, "/goattrk");
	}
	else
	{
		strcpy(filename, getenv("HOME"));
		strcat(filename, "/.config/goattrk");
	}
	mkdir(filename, S_IRUSR | S_IWUSR | S_IXUSR);
	strcat(filename, "/gt2stereo.cfg");
#endif
#endif
	configfile = fopen(filename, "wt");
	if (configfile)
	{
		fprintf(configfile, ";------------------------------------------------------------------------------\n"
			";GT2 config file. Rows starting with ; are comments. Hexadecimal parameters are\n"
			";to be preceded with $ and decimal parameters with nothing.                    \n"
			";------------------------------------------------------------------------------\n"
			"\n"
			";reSID buffer length (in milliseconds)\n%d\n\n"
			";reSID mixing rate (in Hz)\n%d\n\n"
			";Hardsid device number (0 = off)\n%d\n\n"
			";reSID model (0 = 6581, 1 = 8580)\n%d\n\n"
			";Timing mode (0 = PAL, 1 = NTSC)\n%d\n\n"
			";Packer/relocator fileformat (0 = SID, 1 = PRG, 2 = BIN)\n%d\n\n"
			";Packer/relocator player address\n$%04x\n\n"
			";Packer/relocator zeropage baseaddress\n$%02x\n\n"
			";Packer/relocator player type (0 = standard ... 3 = minimal)\n%d\n\n"
			";Key entry mode (0 = Protracker, 1 = DMC, 2 = Janko)\n%d\n\n"
			";Pattern highlight step size\n%d\n\n"
			";Speed multiplier (0 = 25Hz, 1 = 1X, 2 = 2X etc.)\n%d\n\n"
			";Use CatWeasel SID (0 = off, 1 = on)\n%d\n\n"
			";Hardrestart ADSR parameter\n$%04x\n\n"
			";reSID interpolation (0 = off, 1 = on, 2 = distortion, 3 = distortion & on)\n%d\n\n"
			";Pattern display mode (0 = decimal, 1 = hex, 2 = decimal w/dots, 3 = hex w/dots)\n%d\n\n"
			";SID baseaddresses\n$%08x\n\n"
			";Finevibrato mode (0 = off, 1 = on)\n%d\n\n"
			";Pulseskipping (0 = off, 1 = on)\n%d\n\n"
			";Realtime effect skipping (0 = off, 1 = on)\n%d\n\n"
			";Random reSID write delay in cycles (0 = off)\n%d\n\n"
			";Custom SID clock cycles per second (0 = use PAL/NTSC default)\n%d\n\n"
			";HardSID interactive mode buffer size (in milliseconds, 0 = maximum/no flush)\n%d\n\n"
			";HardSID playback mode buffer size (in milliseconds, 0 = maximum/no flush)\n%d\n\n"
			";reSID-fp distortion rate\n%f\n\n"
			";reSID-fp distortion point\n%f\n\n"
			";reSID-fp distortion CF threshold\n%f\n\n"
			";reSID-fp type 3 base resistance\n%f\n\n"
			";reSID-fp type 3 base offset\n%f\n\n"
			";reSID-fp type 3 base steepness\n%f\n\n"
			";reSID-fp type 3 minimum FET resistance\n%f\n\n"
			";reSID-fp type 4 k\n%f\n\n"
			";reSID-fp type 4 b\n%f\n\n"
			";reSID-fp voice nonlinearity\n%f\n\n"
			";Window type (0 = window, 1 = fullscreen)\n%d\n\n"
			";window scale factor (1 = no scaling, 2 to 4 = 2 to 4 times bigger window)\n%d\n\n"
			";Base pitch of A-4 in Hz (0 = use default frequencytable)\n%f\n\n"
			";Equal divisions per octave (12 = default, 8.2019143 = Bohlen-Pierce)\n%f\n\n"
			";Special note names (2 chars for every note in an octave/cycle)\n%s\n\n"
			";Path to a Scala tuning file .scl\n%s\n\n"
			";Default SID channel playback\n%d\n\n"
			";UI Skin\n%d\n\n"
			";Master Volume scaler (1 = normal volume. 2 = twice as loud 0.5 = half volume..)\n%f\n\n"
			";Detune Cent (0-2... 1 = no detune. 0 =-100 cents. 2=+100 cents)\n%f\n\n"
			";Enable Key repeat (0-1... 0=only on specific keys. 1=on all keys)\n%d\n\n"
			";MIDI Port\n%d\n\n",
			b,
			mr,
			hardsid,
			sidmodel,
			ntsc,
			fileformat,
			playeradr,
			zeropageadr,
			playerversion,
			keypreset,
			stepsize,
			multiplier,
			catweasel,
			adparam,
			interpolate,
			patterndispmode,
			sidaddress,
			finevibrato,
			optimizepulse,
			optimizerealtime,
			residdelay,
			customclockrate,
			hardsidbufinteractive,
			hardsidbufplayback,
			filterparams.distortionrate,
			filterparams.distortionpoint,
			filterparams.distortioncfthreshold,
			filterparams.type3baseresistance,
			filterparams.type3offset,
			filterparams.type3steepness,
			filterparams.type3minimumfetresistance,
			filterparams.type4k,
			filterparams.type4b,
			filterparams.voicenonlinearity,
			win_fullscreen,
			bigwindow,
			basepitch,
			equaldivisionsperoctave,
			specialnotenames,
			scalatuningfilepath,
			maxSIDChannels,
			currentPalettePreset,
			masterVolume,
			detuneCent,
			enablekeyrepeat,
			selectedMIDIPort);

		fclose(configfile);
	}



	// Exit
	return 0;
}

void waitkey(GTOBJECT *gt)
{
	for (;;)
	{

		if (!jdebugPlaying)
		{
			displayupdate(gt);
		}
		getkey();
		if ((rawkey) || (key)) break;
		if (win_quitted) break;


	}

	converthex();
}

MIDI_MESSAGE midiMessage;

int refreshSongTime = 0;
int refreshSongInfoDeltaTime = 0;
int refreshCount = 0;

void waitkeymouse(GTOBJECT *gt)
{
	int jc = 0;
	int rk = 0;

	for (;;)
	{
		msDelta = SDL_GetTicks() - lastMS;
		lastMS = SDL_GetTicks();

		if (!jdebugPlaying)
			displayupdate(gt);

		getkey();
		if (mouseb)
		{
			break;
		}
		else if (prevmouseb)
		{
			break;		// Handle modifying values when hold / dragging. We've released the mouse
		}

		editorInfo.mouseTrack = 0;

		if ((rawkey) || (key))
		{
			break;
		}
		if (win_quitted) break;


		win_enableKeyRepeat();

		//	SDL_Delay(50);

		midiMessage.size = 0;

		int portOpen = 123;

		/*
		Allow MIDI Jamming if not editing PATTERN (so also enable if we've got cursor on other areas)
		*/
		gMIDINote = -1;

		if (!jdebugPlaying)
		{
			if (recordmode && editorInfo.editmode == EDIT_PATTERN)
			{
				portOpen = checkForMidiInput(&midiMessage, selectedMIDIPort);
				int i = 0;
				for (int c = 0;c < midiMessage.size / 3;c++)
				{
					unsigned char midiInstruction = midiMessage.message[i];
					unsigned char midiNote = midiMessage.message[i + 1];
					unsigned char midiVel = midiMessage.message[i + 2];
					i += 3;

					if (midiInstruction == 0x90 && midiVel > 0)	// key on
					{
						gMIDINote = midiNote + FIRSTNOTE;	// editing pattern data and have received keyon from MIDI device
						key = 0;
						rawkey = 0;
						return;

					}
				}
			}
			else if ((!recordmode) || (recordmode && editorInfo.editmode != EDIT_PATTERN))
			{
				do {

					portOpen = checkForMidiInput(&midiMessage, selectedMIDIPort);
					handleMIDIPolykeyboard(&gtObject, midiMessage);

				} while (midiMessage.size);

				int noKeysPressed = handlePolyphonicKeyboard(&gtObject);	// update for QWERTY too

	// Need to change this so that it checks actual keyed on channels, rather than keys pressed


				if (!checkAnyPolyPlaying())
				{
					for (int i = 0;i < KEYBOARD_POLYPHONY;i++)
					{
						clearPolyChannel(i, gt);
					}
					if (clearInfoLine)
					{
						clearInfoLine = 0;
						if (editorInfo.editmode == EDIT_PATTERN)
						{
							lastInfoPatternCh = -1;	// force text
							displayPatternInfo(gt);
						}
						else
						{
							sprintf(&keyOffsetText[0], "                        ");
							sprintf(infoTextBuffer, keyOffsetText);
						}
					}
				}
				else
				{
					calculateNoteOffsets();
					sprintf(infoTextBuffer, keyOffsetText);
				}
			}
		}
	}
	converthex();

}

void waitkeymousenoupdate(void)
{
	for (;;)
	{
		fliptoscreen();
		getkey();
		if ((rawkey) || (key)) break;
		if (win_quitted) break;
		if (mouseb) break;
	}

	converthex();
}

void waitkeynoupdate(void)
{
	for (;;)
	{
		fliptoscreen();
		getkey();
		if ((rawkey) || (key)) break;
		if ((mouseb) && (!prevmouseb)) break;
		if (win_quitted) break;

	}
}

void converthex()
{
	int c;

	hexnybble = -1;
	for (c = 0; c < 16; c++)
	{
		if (tolower(key) == hexkeytbl[c])
		{
			if (c >= 10)
			{
				if (!shiftpressed) hexnybble = c;
			}
			else
			{
				hexnybble = c;
			}
		}
	}
}


void docommand(void)
{

	int i = 0;
	//	for (int i = 0; i < SDL_GetNumAudioDrivers(); ++i) {
	//		sprintf(textbuffer, "Audio driver %d: %s\n", i,  SDL_GetAudioDriver(0));
	//		printtext(70, 36, 0xe, textbuffer);
	//	}

	int c2;
	GTOBJECT *gt;

	gt = &gtObject;

	// "GUI" operation :)
	int m = mousebDoubleClick;
	mousecommands(gt);
	if (m)
		mousebDoubleClick = 0;


	GTUNDO_OBJECT *ed = undoCreateEditorInfo();

	// Mode-specific commands
	switch (editorInfo.editmode)
	{

	case EDIT_ORDERLIST:

		// We need to check all channels in order list incase user presses shift1-6 to swap them around
		// (we could just set this for the other channe in orderlistcommands - but this is just safer overall..)
		for (int i = 0;i < MAX_CHN;i++)
		{
			undoAreaSetCheckForChange(UNDO_AREA_ORDERLIST, i + (editorInfo.esnum*MAX_CHN), UNDO_AREA_DIRTY_CHECK);
		}
		c2 = getActualChannel(editorInfo.esnum, editorInfo.eschn);

		//	undoAreaSetCheckForChange(UNDO_AREA_CHANNEL_EDITOR_INFO, c2, UNDO_AREA_DIRTY_CHECK);

		orderlistcommands(gt);
		displayOrderTableInfo(gt);
		break;

	case EDIT_INSTRUMENT:



		if (mouseTrackModify(EDIT_INSTRUMENT))
		{
			undoAreaSetCheckForChange(UNDO_AREA_INSTRUMENTS, editorInfo.einum, UNDO_AREA_DIRTY_CHECK);
		}
		instrumentcommands(gt);
		displayInstrumentInfo(gt);
		break;

	case EDIT_TABLES:

		if (mouseTrackModify(EDIT_TABLES))
		{
			undoAreaSetCheckForChange(UNDO_AREA_TABLES + editorInfo.etnum, 0, UNDO_AREA_DIRTY_CHECK);	// left table
			undoAreaSetCheckForChange(UNDO_AREA_TABLES + editorInfo.etnum, 1, UNDO_AREA_DIRTY_CHECK);	// right table
		}

		tablecommands(gt);
		displayTableInfo(gt);
		break;

	case EDIT_PATTERN:

		c2 = getActualChannel(editorInfo.esnum, editorInfo.epchn);
		undoAreaSetCheckForChange(UNDO_AREA_PATTERN, gt->editorInfo[c2].epnum, UNDO_AREA_DIRTY_CHECK);
		undoAreaSetCheckForChange(UNDO_AREA_PATTERN_LEN, 0, UNDO_AREA_DIRTY_CHECK);

		for (int i = 0;i < MAX_CHN;i++)
		{
			undoAreaSetCheckForChange(UNDO_AREA_ORDERLIST, i + (editorInfo.esnum*MAX_CHN), UNDO_AREA_DIRTY_CHECK);
		}

		// JP REMOVED THIS. SEEMS TO CAUSE PROBLEMS..
	//	undoAreaSetCheckForChange(UNDO_AREA_CHANNEL_EDITOR_INFO, c2, UNDO_AREA_DIRTY_CHECK);

		// if gMIDINote!=-1, then use this as input instead of QWERTY note input
		// Also, if this is the case, set key and rawkey=0 so that only note input is recognised - just in case..
		patterncommands(gt, gMIDINote);

		displayPatternInfo(gt);
		countInstrumentsInPattern(gt->editorInfo[c2].epnum);
		calculateTotalInstrumentsFromAllPatterns();
		break;

	case EDIT_NAMES:
		namecommands(gt);
		break;
	}


	if (!editPaletteMode)
	{
		if (undoValidateUndoAreas(ed) == 0)
		{
			undoFreeUndoObject((GTUNDO_OBJECT*)ed);
		}
	}
	else
		undoFreeUndoObject((GTUNDO_OBJECT*)ed);

	// General commands
	generalcommands(gt);
}

void mousecommands(GTOBJECT *gt)
{
	int c;
	int c2 = getActualChannel(editorInfo.esnum, editorInfo.epchn);	// 0-12
	int songNum = getActualSongNumber(editorInfo.esnum, c2);
	int c3 = c % 6;


	if (!mouseb)
	{
	}
	else if (editorInfo.mouseTrack)
	{
		return;
	}

	if (mouseTransportBar(gt))
		return;


	if (editPaletteMode)
	{

		if (((!prevmouseb) || (mouseheld > HOLDDELAY)) && (mousey == 2) && (mousex >= 63 + 21) && (mousex <= 64 + 21))
		{
			if (mouseb & MOUSEB_LEFT) nextsong(gt);
			if (mouseb & MOUSEB_RIGHT) prevsong(gt);
		}

		// Song editpos & songnumber selection
		if ((mousey >= 3) && (mousey <= 5) && (mousex >= 40 + 21) && mouseb)
		{
			if (editorInfo.editmode != EDIT_ORDERLIST && prevmouseb)
				return;

			// editing palette, so don't allow user to click elsewhere.

			int newpos = editorInfo.esview + (mousex - 44 - 21) / 3;
			int newcolumn = (mousex - 44 - 21) % 3;
			int newchn = mousey - 3;
			if (newcolumn < 0) newcolumn = 0;
			if (newcolumn > 1) newcolumn = 1;
			if (newpos < 0)
			{
				newpos = 0;
				newcolumn = 0;
			}

			int maxPaletteText = getPaletteTextArraySize();

			if (newpos >= maxPaletteText / 2)
			{
				newpos = (maxPaletteText / 2) - 1;
				newcolumn = 1;
			}

			editorInfo.eschn = newchn;
			editorInfo.eseditpos = newpos;
			editorInfo.escolumn = newcolumn;

			editorInfo.editmode = EDIT_ORDERLIST;

		}
		return;
	}

	// Pattern editpos & pattern number selection
	for (c = 0; c < MAX_CHN; c++)
	{
		if (maxSIDChannels == 3 && c >= 3)
			break;
		if (maxSIDChannels == 9 && c >= 3 && (editorInfo.esnum & 1))
			break;

		if (mousey == PATTERN_Y)
		{
			if ((mousex >= PATTERN_X + 11 + c * 9) && (mousex <= PATTERN_X + 12 + c * 9))
			{
				if ((!prevmouseb) || (mouseheld > HOLDDELAY))
				{
					if (mouseb & MOUSEB_LEFT)
					{
						editorInfo.epchn = c;
						nextpattern(gt);
					}
					if (mouseb & MOUSEB_RIGHT)
					{
						editorInfo.epchn = c;
						prevpattern(gt);
					}
				}
			}
			else if (!prevmouseb)
			{
				if ((mousex >= PATTERN_X + 5 + c * 9) && (mousex <= PATTERN_X + 7 + c * 9))
					mutechannel(c, gt);
			}

		}
		else
		{
			if ((mousey >= PATTERN_Y) && (mousey <= PATTERN_Y + VISIBLEPATTROWS + 0) && (mousex >= PATTERN_X + 5 + c * 9) && (mousex <= PATTERN_X + 12 + c * 9))
			{
				if (!mouseb)
					return;
				if (editorInfo.editmode != EDIT_PATTERN && prevmouseb)	// Don't allow hold/drag to select another panel
					return;

				int x = mousex - (PATTERN_X + 5) - c * 9;
				int newpos = mousey - PATTERN_Y + 1 + 12 + editorInfo.epview - VISIBLEPATTROWS / 2;

				if (newpos < 0) newpos = 0;
				if (newpos > pattlen[gt->editorInfo[c2].epnum])
					newpos = pattlen[gt->editorInfo[c2].epnum];

				editorInfo.editmode = EDIT_PATTERN;

				if ((mouseb & (MOUSEB_RIGHT | MOUSEB_MIDDLE)) && (!prevmouseb))
				{
					if ((editorInfo.epmarkchn != c) || (newpos != editorInfo.epmarkend))
					{
						editorInfo.epmarkchn = c;
						editorInfo.epmarkstart = editorInfo.epmarkend = newpos;
					}
				}

				if (mouseb & MOUSEB_LEFT)
				{
					editorInfo.epchn = c;
					if (x < 3) editorInfo.epcolumn = 0;
					if (x >= 3) editorInfo.epcolumn = x - 2;

					setMasterLoopChannel(gt);
				}

				if (!prevmouseb)
				{
					if (mouseb & MOUSEB_LEFT)
						editorInfo.eppos = newpos;
				}

				if (editorInfo.eppos < 0) editorInfo.eppos = 0;
				if (editorInfo.eppos > pattlen[gt->editorInfo[c2].epnum])
					editorInfo.eppos = pattlen[gt->editorInfo[c2].epnum];

				if (mouseb & (MOUSEB_RIGHT | MOUSEB_MIDDLE)) editorInfo.epmarkend = newpos;
			}
		}
	}


	int maxCh = 5;
	if ((maxSIDChannels == 3) || (maxSIDChannels == 9 && (editorInfo.esnum & 1)))
		maxCh = 2;

	// Song editpos & songnumber selection
	if ((mousey >= 3) && (mousey <= 3 + maxCh) && (mousex >= 40 + 21))
	{
		if (editorInfo.editmode != EDIT_ORDERLIST && prevmouseb)	// Don't allow hold/drag to select another panel
			return;

		if (!mouseb)
			return;

		int newpos = editorInfo.esview + (mousex - 44 - 21) / 3;
		int newcolumn = (mousex - 44 - 21) % 3;
		int newchn = mousey - 3;
		if (newcolumn < 0) newcolumn = 0;
		if (newcolumn > 1) newcolumn = 1;
		if (newpos < 0)
		{
			newpos = 0;
			newcolumn = 0;
		}
		if (newpos == songlen[editorInfo.esnum][editorInfo.eschn])
		{
			newpos++;
			newcolumn = 0;
		}
		if (newpos > songlen[editorInfo.esnum][editorInfo.eschn] + 1)
		{
			newpos = songlen[editorInfo.esnum][editorInfo.eschn] + 1;
			newcolumn = 1;
		}

		editorInfo.editmode = EDIT_ORDERLIST;

		if ((mouseb & (MOUSEB_RIGHT | MOUSEB_MIDDLE)) && (!prevmouseb) && (newpos < songlen[editorInfo.esnum][editorInfo.eschn]))
		{

			if ((editorInfo.esmarkchn != newchn) || (newpos != editorInfo.esmarkend))
			{
				editorInfo.esmarkchn = newchn;
				editorInfo.esmarkstart = editorInfo.esmarkend = newpos;
			}

		}

		if (mouseb & MOUSEB_LEFT)
		{
			int m = mousebDoubleClick;
			int s = shiftpressed;


			if (mouseheld > HOLDDELAY && !editPaletteMode)
			{
				editorInfo.eschn = newchn;
				editorInfo.eseditpos = newpos;
				editorInfo.escolumn = newcolumn;
				setMasterLoopChannel(gt);
				orderSelectPatternsFromSelected(gt);
			}
			else if (m && !editPaletteMode)	// double click?
			{

				editorInfo.eschn = newchn;
				editorInfo.eseditpos = newpos;
				editorInfo.escolumn = newcolumn;

				setMasterLoopChannel(gt);
				orderPlayFromPosition(gt, 0, editorInfo.eseditpos, editorInfo.eschn);

			}
			else
			{
				editorInfo.eschn = newchn;
				editorInfo.eseditpos = newpos;
				editorInfo.escolumn = newcolumn;

				setMasterLoopChannel(gt);
			}
		}

		if ((mouseb & (MOUSEB_RIGHT | MOUSEB_MIDDLE)) && (newpos < songlen[editorInfo.esnum][editorInfo.eschn]))
			editorInfo.esmarkend = newpos;
	}
	if (((!prevmouseb) || (mouseheld > HOLDDELAY)) && (mousey == 2) && (mousex >= 64 + 20) && (mousex <= 65 + 20))
	{
		if (mouseb & MOUSEB_LEFT) nextsong(gt);
		if (mouseb & MOUSEB_RIGHT) prevsong(gt);
	}

	// Instrument editpos & instrument number selection
	if ((mousey >= 8 + 3) && (mousey <= 12 + 3) && (mousex >= 56 + 20) && (mousex <= 57 + 20))
	{
		if (editorInfo.editmode != EDIT_INSTRUMENT && prevmouseb)	// Don't allow hold/drag to select another panel
			return;

		if (!mouseb)
			return;

		if (!prevmouseb)
		{
			editorInfo.editmode = EDIT_INSTRUMENT;
			editorInfo.eipos = mousey - 8 - 3;
			editorInfo.eicolumn = mousex - 56 - 20;
			mouseTrack();	// MUST DO AFTER SETTING ABOVE VALUES
		}
	}
	if ((mousey >= 8 + 3) && (mousey <= 11 + 3) && (mousex >= 76 + 20) && (mousex <= 77 + 20))
	{
		if (editorInfo.editmode != EDIT_INSTRUMENT && prevmouseb)	// Don't allow hold/drag to select another panel
			return;

		if (!mouseb)
			return;

		if (!prevmouseb)
		{
			editorInfo.editmode = EDIT_INSTRUMENT;
			editorInfo.eipos = mousey - 8 - 3 + 5;
			editorInfo.eicolumn = mousex - 76 - 20;
			mouseTrack();	// MUST DO AFTER SETTING ABOVE VALUES
		}
	}
	if ((mousey == 7 + 3) && (mousex >= 60 + 20))
	{
		if (editorInfo.editmode != EDIT_INSTRUMENT && prevmouseb)	// Don't allow hold/drag to select another panel
			return;

		if (!mouseb)
			return;

		if (!prevmouseb)
		{
			editorInfo.editmode = EDIT_INSTRUMENT;
			editorInfo.eipos = 9;
			mouseTrack();	// MUST DO AFTER SETTING ABOVE VALUES
		}
	}

	if (((!prevmouseb) || (mouseheld > HOLDDELAY)) && (mousey == 7 + 3) && (mousex >= 56 + 20) && (mousex <= 57 + 20))
	{
		if (editorInfo.editmode != EDIT_INSTRUMENT && prevmouseb)	// Don't allow hold/drag to select another panel
			return;

		if (mouseb & MOUSEB_LEFT) nextinstr();
		if (mouseb & MOUSEB_RIGHT) previnstr();
	}

	// Table editpos
	for (c = 0; c < MAX_TABLES - 1; c++)
	{
		if (mouseb && (mousey == 14 + 3 && (mousex >= 40 + 20 + c * 10) && (mousex <= 47 + 20 + c * 10)))
		{
			// JP - I've no idea why I added this..
			if (editorInfo.editmode != EDIT_TABLE_WAVE && prevmouseb)	// Don't allow hold/drag to select another panel
				return;

			if (prevmouseb)
				return;

			if (editorInfo.editTableMode == EDIT_TABLE_WAVE + c)
				editorInfo.editTableMode = EDIT_TABLE_NONE;
			else if (editorInfo.editTableMode == EDIT_TABLE_NONE)
				editorInfo.editTableMode = EDIT_TABLE_WAVE + c;
			return;
		}
	}

	if (editorInfo.editTableMode == EDIT_TABLE_NONE)
	{
		for (c = 0; c < MAX_TABLES; c++)
		{
			checkForMouseInTable(c);
		}
	}
	else if (editorInfo.editTableMode == EDIT_TABLE_WAVE)
	{
		checkForMouseInDetailedWaveTable();
		checkForMouseInTable(EDIT_TABLE_SPEED - 1);
	}
	else if (editorInfo.editTableMode == EDIT_TABLE_FILTER)
	{
		checkForMouseInDetailedFilterTable();
	}
	else if (editorInfo.editTableMode == EDIT_TABLE_PULSE)
	{
		checkForMouseInDetailedPulseTable();
	}

	// Name editpos
	if ((mousey >= (21 + 3 + 9) && mousey < (21 + 3 + 9 + 3)) && (mousex >= 47 + 20))
	{
		if (!mouseb)
			return;

		editorInfo.nameIndex = mousey - (21 + 3 + 9);
		editorInfo.editmode = EDIT_NAMES;

	}


//	if ((!prevmouseb) && (mousex <= 7) && (mousey == TRANSPORT_BAR_Y))
//	{
//		recordmode ^= 1;
//	}
	for (c = 0; c < MAX_CHN; c++)
	{
		if ((!prevmouseb) && (mousey >= 23 + 3 + 10) && (mousex >= 59 + 7 * c) && (mousex <= 64 + 7 * c))
			mutechannel(c, gt);
	}


	checkMouseInWaveformInfo();

	// Titlebar actions
	if (!menu)
	{
		if ((mousey == 0) && (!prevmouseb) && (mouseb == MOUSEB_LEFT))
		{
			if ((mousex >= 38 + 20) && (mousex <= 39 + 20))
			{
				monomode ^= 1;
			}
			if ((mousex >= 40 + 20) && (mousex <= 41 + 20))
			{
				usefinevib ^= 1;
			}
			if ((mousex >= 43 + 20) && (mousex <= 44 + 20))
			{
				optimizepulse ^= 1;
			}
			if ((mousex >= 46 + 20) && (mousex <= 47 + 20))
			{
				optimizerealtime ^= 1;
			}
			if ((mousex >= 49 + 20) && (mousex <= 52 + 20))
			{
				ntsc ^= 1;
				sound_init(b, mr, writer, hardsid, sidmodel, ntsc, multiplier, catweasel, interpolate, customclockrate);
			}
			if ((mousex >= 54 + 20) && (mousex <= 57 + 20))
			{
				sidmodel ^= 1;
				sound_init(b, mr, writer, hardsid, sidmodel, ntsc, multiplier, catweasel, interpolate, customclockrate);
			}
			if ((mousex >= 62 + 20) && (mousex <= 65 + 20)) editadsr(gt);
			if ((mousex >= 67 + 20) && (mousex <= 68 + 20)) prevmultiplier();
			if ((mousex >= 69 + 20) && (mousex <= 70 + 20)) nextmultiplier();
		}
	}
	else
	{
		if ((!mousey) && (mouseb & MOUSEB_LEFT) && (!(prevmouseb & MOUSEB_LEFT)))
		{
			if ((mousex >= 0) && (mousex <= 5))
			{
				initsong(editorInfo.esnum, PLAY_BEGINNING, gt);
				followplay = shiftpressed;
			}
			if ((mousex >= 7) && (mousex <= 15))
			{
				initsong(editorInfo.esnum, PLAY_POS, gt);
				followplay = shiftpressed;
			}
			if ((mousex >= 17) && (mousex <= 26))
			{
				initsong(editorInfo.esnum, PLAY_PATTERN, gt);
				followplay = shiftpressed;
			}
			if ((mousex >= 28) && (mousex <= 33))
				stopsong(gt);
			if ((mousex >= 35) && (mousex <= 40))
				load(gt);
			if ((mousex >= 42) && (mousex <= 47))
				save(gt);
			if ((mousex >= 49) && (mousex <= 57))
				relocator(gt);
			if ((mousex >= 59) && (mousex <= 64))
				onlinehelp(0, 0, gt);
			if ((mousex >= 66) && (mousex <= 72))
				clear(gt);
			if ((mousex >= 74) && (mousex <= 79))
				quit(gt);
		}
	}
}

void generalcommands(GTOBJECT *gt)
{
	int c;
	int songNum;
	int ac = getActualChannel(editorInfo.esnum, editorInfo.epchn);

	switch (key)
	{
	case '?':
	case '-':
		if ((editorInfo.editmode != EDIT_NAMES) && (editorInfo.editmode != EDIT_ORDERLIST))
		{
			if (!((editorInfo.editmode == EDIT_INSTRUMENT) && (editorInfo.eipos == 9))) previnstr();
		}
		break;

	case '+':
	case '_':
		if ((editorInfo.editmode != EDIT_NAMES) && (editorInfo.editmode != EDIT_ORDERLIST))
		{
			if (!((editorInfo.editmode == EDIT_INSTRUMENT) && (editorInfo.eipos >= 9))) nextinstr();

		}
		break;

	case '*':
		if (editorInfo.editmode != EDIT_NAMES)
		{
			if (!((editorInfo.editmode == EDIT_INSTRUMENT) && (editorInfo.eipos >= 9)))
			{
				if (editorInfo.epoctave < 7) editorInfo.epoctave++;
			}
		}
		break;

	case '/':
	case '\'':
		if (editorInfo.editmode != EDIT_NAMES)
		{
			if (!((editorInfo.editmode == EDIT_INSTRUMENT) && (editorInfo.eipos >= 9)))
			{
				if (editorInfo.epoctave > 0) editorInfo.epoctave--;
			}
		}
		break;

	case '<':
		if (((editorInfo.editmode == EDIT_INSTRUMENT) && (editorInfo.eipos != 9)) || (editorInfo.editmode == EDIT_TABLES))
			previnstr();
		break;

	case '>':
		if (((editorInfo.editmode == EDIT_INSTRUMENT) && (editorInfo.eipos != 9)) || (editorInfo.editmode == EDIT_TABLES))
			nextinstr();
		break;

	case ';':
		previousSongPos(gt, 1);
		break;

	case ':':

		nextSongPos(gt);
		break;

	}
	if (win_quitted) exitprogram = 1;
	switch (rawkey)
	{
	case KEY_ESC:
		if (!shiftpressed)
			quit(gt);
		else
			clear(gt);
		break;

	case KEY_KPMULTIPLY:
		if ((editorInfo.editmode != EDIT_NAMES) && (!key))
		{
			if (!((editorInfo.editmode == EDIT_INSTRUMENT) && (editorInfo.eipos >= 9)))
			{
				if (editorInfo.epoctave < 7) editorInfo.epoctave++;
			}
		}
		break;

	case KEY_KPDIVIDE:
		if ((editorInfo.editmode != EDIT_NAMES) && (!key))
		{
			if (!((editorInfo.editmode == EDIT_INSTRUMENT) && (editorInfo.eipos >= 9)))
			{
				if (editorInfo.epoctave > 0) editorInfo.epoctave--;
			}
		}
		break;


	case KEY_S:
		if (!ctrlpressed) break;
		int s = quickSave();
		if (s)
			sprintf(infoTextBuffer, "quick save: %d", s);
		else
			save(gt);
		return;

	case KEY_Z:
		if (!ctrlpressed) break;

		if (!editPaletteMode)
			undoPerform();
		return;

	case KEY_F12:

	case SDLK_HELP:
		onlinehelp(0, shiftpressed, gt);
		break;

	case KEY_TAB:
		if (!shiftpressed) editorInfo.editmode++;
		else editorInfo.editmode--;
		if (editorInfo.editmode > EDIT_NAMES) editorInfo.editmode = EDIT_PATTERN;
		if (editorInfo.editmode < EDIT_PATTERN) editorInfo.editmode = EDIT_NAMES;

		setMasterLoopChannel(gt);
		break;

	case KEY_F1:
		if (editPaletteMode)
			break;

		// JP - Shift_F1  changed to just turn looping on/off
		playUntilEnd();
		initsong(editorInfo.esnum, PLAY_BEGINNING, gt);
		//	followplay = shiftpressed;

		break;

		// PLAY FROM START OF SELECTED PATTERN
	case KEY_F2:

		if (shiftpressed)
			followplay = 1 - followplay;
		else
		{
			transportLoopPattern = 1 - transportLoopPattern;
			if (!transportLoopPattern)
			{
				editorInfo.highlightLoopChannel = 999;			// remove from display
				editorInfo.highlightLoopPatternNumber = -1;
				editorInfo.highlightLoopStart = editorInfo.highlightLoopEnd = 0;
			}
		}
		break;


		// LOOP PATTERN, PLAYING FROM SELECTED
	case KEY_F3:
		if (editPaletteMode)
			break;
		playFromCurrentPosition(gt);
		break;

	case KEY_F4:
		if (shiftpressed)
			mutechannel(editorInfo.epchn, gt);
		else
		{
			if (gt->songinit != PLAY_STOPPED)
			{
				stopsong(gt);
				setMasterLoopChannel(gt);
			}
		}

		break;

	case KEY_F5:
		if (!shiftpressed)
			editorInfo.editmode = EDIT_PATTERN;
		else prevmultiplier();
		break;

	case KEY_F6:
		if (!shiftpressed)
			editorInfo.editmode = EDIT_ORDERLIST;
		else nextmultiplier();
		break;

	case KEY_F7:
		if (!shiftpressed)
		{
			if (editorInfo.editmode == EDIT_INSTRUMENT)
				editorInfo.editmode = EDIT_TABLES;
			else
				editorInfo.editmode = EDIT_INSTRUMENT;
		}
		else editadsr(gt);
		break;

	case KEY_F8:
		if (!shiftpressed)
			editorInfo.editmode = EDIT_TABLES;		// 'Cos JAMMAR SAID SO!
		else
		{
			sidmodel ^= 1;
			sound_init(b, mr, writer, hardsid, sidmodel, ntsc, multiplier, catweasel, interpolate, customclockrate);
		}
		break;

	case KEY_F9:
		if (!shiftpressed)
			relocator(gt);
		else
		{
			monomode ^= 1;
		}
		break;

	case KEY_F10:
		load(gt);

		// Set up song 1 and then 0... This allows editor pattern numbers to be complete, so that F3 works from the very start.
		// (Bit of a nasty hack..Meh. Never mind)
		editorInfo.esnum = 1;
		songchange(gt, 1);
		editorInfo.esnum = 0;
		songchange(gt, 1);

		playUntilEnd();
		break;

	case KEY_F11:
		save(gt);
		break;
	}
}

int loadedSongFlag = 0;

void load(GTOBJECT *gt)
{
	win_enableKeyRepeat();

	if ((editorInfo.editmode != EDIT_INSTRUMENT) && (editorInfo.editmode != EDIT_TABLES))
	{
		int ok = 0;
		if (!shiftpressed)
		{
			if (fileselector(songfilename, songpath, songfilter, "LOAD SONG", 0, gt, CEDIT))
				ok = loadsong(gt);
		}
		else
		{
			if (fileselector(songfilename, songpath, songfilter, "MERGE SONG", 0, gt, CEDIT))
				ok = mergesong(gt);
		}
		if (ok)
		{
			loadedSongFlag = 1;
			undoInitAllAreas(&gtObject);	// recreate undo buffers, using the loaded song as the original info
			countInstruments();
			setTableBackgroundColours(editorInfo.einum);
		}

	}
	else
	{
		if (editorInfo.einum)
		{
			if (fileselector(instrfilename, instrpath, instrfilter, "LOAD INSTRUMENT", 0, gt, 15))
				loadinstrument(gt);
		}
	}
	key = 0;
	rawkey = 0;
}

int quickSave()
{
	if (loadedSongFlag)
	{
		if (strlen(loadedsongfilename))
			strcpy(songfilename, loadedsongfilename);
		savesong();
	}
	return loadedSongFlag;
}

void save(GTOBJECT *gt)
{
	win_enableKeyRepeat();

	if ((editorInfo.editmode != EDIT_INSTRUMENT) && (editorInfo.editmode != EDIT_TABLES))
	{
		int done = 0;

		// Repeat until quit or save successful
		while (!done)
		{
			if (strlen(loadedsongfilename)) strcpy(songfilename, loadedsongfilename);
			if (fileselector(songfilename, songpath, songfilter, "SAVE SONG", 3, gt, 12))
				done = savesong();
			else done = 1;
		}
	}
	else
	{
		if (editorInfo.einum)
		{
			int done = 0;
			int useinstrname = 0;
			char tempfilename[MAX_FILENAME];

			// Repeat until quit or save successful
			while (!done)
			{
				if ((!strlen(instrfilename)) && (strlen(instr[editorInfo.einum].name)))
				{
					useinstrname = 1;
					strcpy(instrfilename, instr[editorInfo.einum].name);
					strcat(instrfilename, ".ins");
					strcpy(tempfilename, instrfilename);
				}

				if (fileselector(instrfilename, instrpath, instrfilter, "SAVE INSTRUMENT", 3, gt, 12))
					done = saveinstrument();
				else done = 1;

				if (useinstrname)
				{
					if (!strcmp(tempfilename, instrfilename))
						memset(instrfilename, 0, sizeof instrfilename);
				}
			}
		}
	}
	key = 0;
	rawkey = 0;
}

void quit(GTOBJECT *gt)
{
	if ((!shiftpressed) || (mouseb))
	{
		printtextcp(78, 36, getColor(CINFO_FOREGROUND, CGENERAL_BACKGROUND), "Really Quit (y/n)?");
		waitkey(gt);

		printtextcp(78, 36, getColor(CINFO_FOREGROUND, CGENERAL_BACKGROUND), "                  ");
		if ((key == 'y') || (key == 'Y')) exitprogram = 1;
	}
	key = 0;
	rawkey = 0;
}

void clear(GTOBJECT *gt)
{
	int cs = 0;
	int cp = 0;
	int ci = 0;
	int ct = 0;
	int cn = 0;

	printtextcp(78, 36, getColor(15, CGENERAL_BACKGROUND), "Optimize everything (y/n)?");
	waitkey(gt);
	printbyterow(60, 36, getColor(15, CGENERAL_BACKGROUND), 32, 39);

	if ((key == 'y') || (key == 'Y'))
	{
		optimizeeverything(1, 1, &gtObject);
		key = 0;
		rawkey = 0;
		return;
	}

	printtextcp(78, 36, getColor(15, CGENERAL_BACKGROUND), "Clear orderlists (y/n)?");
	waitkey(gt);
	printbyterow(60, 36, getColor(15, CGENERAL_BACKGROUND), 32, 39);
	if ((key == 'y') || (key == 'Y')) cs = 1;

	printtextcp(78, 36, getColor(15, CGENERAL_BACKGROUND), "Clear patterns (y/n)?");
	waitkey(gt);
	printbyterow(60, 36, getColor(15, CGENERAL_BACKGROUND), 32, 39);
	if ((key == 'y') || (key == 'Y')) cp = 1;

	printtextcp(78, 36, getColor(15, CGENERAL_BACKGROUND), "Clear instruments (y/n)?");
	waitkey(gt);
	printbyterow(60, 36, getColor(15, CGENERAL_BACKGROUND), 32, 39);
	if ((key == 'y') || (key == 'Y')) ci = 1;

	printtextcp(78, 36, getColor(15, CGENERAL_BACKGROUND), "Clear tables (y/n)?");
	waitkey(gt);
	printbyterow(60, 36, getColor(15, CGENERAL_BACKGROUND), 32, 39);
	if ((key == 'y') || (key == 'Y')) ct = 1;

	printtextcp(78, 36, getColor(15, CGENERAL_BACKGROUND), "Clear songname (y/n)?");
	waitkey(gt);
	printbyterow(60, 36, getColor(15, CGENERAL_BACKGROUND), 32, 39);
	if ((key == 'y') || (key == 'Y')) cn = 1;

	if (cp == 1)
	{
		int selectdone = 0;
		int olddpl = defaultpatternlength;

		printtext(60, 36, getColor(15, CGENERAL_BACKGROUND), "Pattern length:");
		while (!selectdone)
		{
			sprintf(textbuffer, "%02d ", defaultpatternlength);
			printtext(60 + 15, 36, getColor(15, CGENERAL_BACKGROUND), textbuffer);

			waitkey(gt);
			switch (rawkey)
			{
			case KEY_LEFT:
				defaultpatternlength -= 7;
			case KEY_DOWN:
				defaultpatternlength--;
				if (defaultpatternlength < 1) defaultpatternlength = 1;
				break;

			case KEY_RIGHT:
				defaultpatternlength += 7;
			case KEY_UP:
				defaultpatternlength++;
				if (defaultpatternlength > MAX_PATTROWS) defaultpatternlength = MAX_PATTROWS;
				break;

			case KEY_ESC:
				defaultpatternlength = olddpl;
				selectdone = 1;
				break;

			case KEY_ENTER:
				selectdone = 1;
				break;
			}
		}
		printbyterow(60, 36, getColor(15, CGENERAL_BACKGROUND), 32, 39);
	}

	if (cs | cp | ci | ct | cn)
	{
		loadedSongFlag = 0;
		memset(songfilename, 0, sizeof songfilename);
	}
	clearsong(cs, cp, ci, ct, cn, &gtObject);

	key = 0;
	rawkey = 0;
}

void editadsr(GTOBJECT *gt)
{
	eamode = 1;
	eacolumn = 0;

	for (;;)
	{
		waitkeymouse(gt);

		if (win_quitted)
		{
			exitprogram = 1;
			key = 0;
			rawkey = 0;
			return;
		}

		if (hexnybble >= 0)
		{
			switch (eacolumn)
			{
			case 0:
				adparam &= 0x0fff;
				adparam |= hexnybble << 12;
				break;

			case 1:
				adparam &= 0xf0ff;
				adparam |= hexnybble << 8;
				break;

			case 2:
				adparam &= 0xff0f;
				adparam |= hexnybble << 4;
				break;

			case 3:
				adparam &= 0xfff0;
				adparam |= hexnybble;
				break;
			}
			eacolumn++;
		}

		switch (rawkey)
		{

		case KEY_F7:
			if (!shiftpressed) break;

		case KEY_ESC:
		case KEY_ENTER:
		case KEY_TAB:
			eamode = 0;
			key = 0;
			rawkey = 0;
			return;

		case KEY_BACKSPACE:
			if (!eacolumn) break;
		case KEY_LEFT:
			eacolumn--;
			break;

		case KEY_RIGHT:
			eacolumn++;
		}
		eacolumn &= 3;

		if ((mouseb) && (!prevmouseb))
		{
			eamode = 0;
			return;
		}
	}
}

void getparam(FILE *handle, unsigned *value)
{
	char *configptr;

	for (;;)
	{
		if (feof(handle)) return;
		fgets(configbuf, MAX_PATHNAME, handle);
		if ((configbuf[0]) && (configbuf[0] != ';') && (configbuf[0] != ' ') && (configbuf[0] != 13) && (configbuf[0] != 10)) break;
	}

	configptr = configbuf;
	if (*configptr == '$')
	{
		*value = 0;
		configptr++;
		for (;;)
		{
			char c = tolower(*configptr++);
			int h = -1;

			if ((c >= 'a') && (c <= 'f')) h = c - 'a' + 10;
			if ((c >= '0') && (c <= '9')) h = c - '0';

			if (h >= 0)
			{
				*value *= 16;
				*value += h;
			}
			else break;
		}
	}
	else
	{
		*value = 0;
		for (;;)
		{
			char c = tolower(*configptr++);
			int d = -1;

			if ((c >= '0') && (c <= '9')) d = c - '0';

			if (d >= 0)
			{
				*value *= 10;
				*value += d;
			}
			else break;
		}
	}
}

void getfloatparam(FILE *handle, float *value)
{
	char *configptr;

	for (;;)
	{
		if (feof(handle)) return;
		fgets(configbuf, MAX_PATHNAME, handle);
		if ((configbuf[0]) && (configbuf[0] != ';') && (configbuf[0] != ' ') && (configbuf[0] != 13) && (configbuf[0] != 10)) break;
	}

	configptr = configbuf;
	*value = 0.0f;
	sscanf(configptr, "%f", value);
}

void getstringparam(FILE *handle, char *value)
{
	char *configptr;

	int foundSemi = 0;

	for (;;)
	{

		if (feof(handle)) return;

		int currentoffset = ftell(handle);


		fgets(configbuf, MAX_PATHNAME, handle);
		if (configbuf[0] == ';')	// Found comment (should be the comment for this string param)
		{
			if (foundSemi)	// Already found a comment? Means that there was no string inbetween...
			{
				fseek(handle, currentoffset, SEEK_SET);	// seek back to this comment.
				return;
			}
			foundSemi++;
		}

		if ((configbuf[0]) && (configbuf[0] != ';') && (configbuf[0] != ' ') && (configbuf[0] != 13) && (configbuf[0] != 10)) break;
	}

	configptr = configbuf;

	sscanf(configptr, "%s", value);
}

void prevmultiplier(void)
{
	if (multiplier > 0)
	{
		multiplier--;
		sound_init(b, mr, writer, hardsid, sidmodel, ntsc, multiplier, catweasel, interpolate, customclockrate);
		playUntilEnd();
	}
}

void nextmultiplier(void)
{
	if (multiplier < 16)
	{
		multiplier++;
		sound_init(b, mr, writer, hardsid, sidmodel, ntsc, multiplier, catweasel, interpolate, customclockrate);
		playUntilEnd();
	}
}

void calculatefreqtable()
{
	float bp = basepitch;
	if (!bp)
		bp = 440.0f;

	double basefreq = (double)bp * (16777216.0 / 985248.0) * pow(2.0, 0.25) / 32.0;
	double cyclebasefreq = basefreq;
	double freq = basefreq;
	int c;
	int i;


	if (tuningcount)
	{
		c = 0;
		while (c < 96)
		{
			for (i = 0; i < tuningcount; i++)
			{
				if (c < 96)
				{
					int intfreq = freq + 0.5;
					if (intfreq > 0xffff)
						intfreq = 0xffff;
					freqtbllo[c] = intfreq & 0xff;
					freqtblhi[c] = intfreq >> 8;
					freq = cyclebasefreq * tuning[i];
					c++;
				}
			}
			cyclebasefreq = freq;
		}
	}
	else
	{
		for (c = 0; c < 8 * 12; c++)
		{
			double note = c * 100;			// * 100 so we can handle detune by +/- 100 cents
			note += (double)detuneCent * 100;
			double freq = basefreq * pow(2.0, note / (double)(equaldivisionsperoctave * 100));
			int intfreq = freq + 0.5;
			if (intfreq > 0xffff)
				intfreq = 0xffff;
			freqtbllo[c] = intfreq & 0xff;
			freqtblhi[c] = intfreq >> 8;
		}
	}
}

void setspecialnotenames()
{
	int i;
	int j;
	int oct;
	char *name;
	char octave[11];

	i = 0;
	oct = 0;
	while (i < 93)
	{
		for (j = 0; j < 186; j += 2)
		{
			if (specialnotenames[j] == '\0')
				break;
			if (i < 93)
			{
				name = malloc(4);
				strncpy(name, specialnotenames + j, 2);
				sprintf(octave, "%d", oct);
				strcpy(name + 2, octave);
				notename[i] = name;
				i++;
			}
		}
		oct++;
	}
}

void readscalatuningfile()
{
	FILE *scalatuningfile;
	char *configptr;
	char strbuf[64];
	char name[3];
	int i;
	double numerator;
	double denominator;
	double centvalue;

	scalatuningfile = fopen(scalatuningfilepath, "rt");
	if (scalatuningfile)
	{
		// Tuning name
		for (;;)
		{
			if (feof(scalatuningfile)) return;
			fgets(configbuf, MAX_PATHNAME, scalatuningfile);
			if ((configbuf[0]) && (configbuf[0] != '!') && (configbuf[0] != 13) && (configbuf[0] != 10)) break;
		}
		configptr = configbuf;
		sscanf(configptr, "%63[^\t\n]", tuningname);

		// Tuning count
		for (;;)
		{
			if (feof(scalatuningfile)) return;
			fgets(configbuf, MAX_PATHNAME, scalatuningfile);
			if ((configbuf[0]) && (configbuf[0] != '!') && (configbuf[0] != 13) && (configbuf[0] != 10)) break;
		}
		configptr = configbuf;
		sscanf(configptr, "%d", &tuningcount);

		// Tunings 
		for (i = 0; i < tuningcount; i++)
		{
			for (;;)
			{
				if (feof(scalatuningfile)) return;
				fgets(configbuf, MAX_PATHNAME, scalatuningfile);
				if ((configbuf[0]) && (configbuf[0] != '!') && (configbuf[0] != 13) && (configbuf[0] != 10)) break;
			}
			configptr = configbuf;
			name[0] = '\0';
			sscanf(configptr, "%63s %2s", strbuf, name);
			if (!i)
			{
				strcpy(specialnotenames, name);
			}
			else
			{
				if (i == tuningcount - 1)
				{
					char *tmp = strdup(specialnotenames);
					strcpy(specialnotenames, name);
					strcat(specialnotenames, tmp);
					free(tmp);
				}
				else
				{
					strcat(specialnotenames, name);
				}
			}
			if (!strchr(strbuf, '.'))
			{
				sscanf(strbuf, "%lf", &numerator);
				if (strchr(strbuf, '/'))
				{
					sscanf(strchr(strbuf, '/') + 1, "%lf", &denominator);
					tuning[i] = numerator / denominator;
				}
			}
			else
			{
				sscanf(configptr, "%lf", &centvalue);
				tuning[i] = pow(2.0, centvalue / 1200.0);
			}
		}
		fclose(scalatuningfile);
	}
}

/*
Foreground / Background for each column display for editing palette RGB
For each entry, the RGB for index 0-n will set the paletteRGB entry (likey with an offset).
So, modifying RGB for index 0 will set the CPATTERN_BACKGROUND1. modifying for index 1 will set CPATTERN_FOREGROUND1
*/

void initPaletteDisplay()
{
	setSkin(currentPalettePreset);	// set the actual gfx_ palette colours (using the loaded gtskin.bin file)
}

void setSkin(int palettePreset)
{
	for (int i = 0;i < MAX_PALETTE_ENTRIES;i++)
	{
		setGFXPaletteRGBFromPaletteRGB(palettePreset, i);
	}
}

void setPaletteRGB(int presetIndex, int paletteIndex, int r, int g, int b)
{
	paletteRGB[presetIndex][0][paletteIndex] = r;
	paletteRGB[presetIndex][1][paletteIndex] = g;
	paletteRGB[presetIndex][2][paletteIndex] = b;

	setGFXPaletteRGBFromPaletteRGB(presetIndex, paletteIndex);
}

int isMatchingRGB(int presetIndex, int color)
{
	int c1 = color & 0xff;
	int c2 = (color >> 8) & 0xff;

	for (int i = 0;i < 3;i++)
	{
		if ((paletteR[c1] != paletteR[c2]) || (paletteG[c1] != paletteG[c2]) || (paletteB[c1] != paletteB[c2]))
		{
			return 0;
		}
	}
	return 1;
}


void setGFXPaletteRGBFromPaletteRGB(int presetIndex, int paletteIndex)
{
	int r = paletteRGB[presetIndex][0][paletteIndex];
	int g = paletteRGB[presetIndex][1][paletteIndex];
	int b = paletteRGB[presetIndex][2][paletteIndex];

	int r1 = r & 0xf0;
	int g1 = g & 0xf0;
	int b1 = b & 0xf0;
	int r2 = (r << 4) & 0xf0;
	int g2 = (g << 4) & 0xf0;
	int b2 = (b << 4) & 0xf0;

	gfx_setPaletteRGB(FIRST_UI_COLOR + (paletteIndex * 2), r1, g1, b1);
	gfx_setPaletteRGB(FIRST_UI_COLOR + (paletteIndex * 2) + 1, r2, g2, b2);

	paletteR[FIRST_UI_COLOR + (paletteIndex * 2)] = r1;
	paletteG[FIRST_UI_COLOR + (paletteIndex * 2)] = g1;
	paletteB[FIRST_UI_COLOR + (paletteIndex * 2)] = b1;

	paletteR[FIRST_UI_COLOR + (paletteIndex * 2) + 1] = r2;
	paletteG[FIRST_UI_COLOR + (paletteIndex * 2) + 1] = g2;
	paletteB[FIRST_UI_COLOR + (paletteIndex * 2) + 1] = b2;
}


void handlePaletteDisplay(GTOBJECT *gt, int palettePreset)
{
	stopsong(gt);
	SDL_Delay(50);


	// backup song 0
	for (int c = 0;c < MAX_CHN;c++)
	{
		for (int p = 0;p < MAX_SONGLEN;p++)
		{
			if (editPaletteMode)
				backupPaletteSong[c][p] = songorder[0][c][p];
			else
			{
				songorder[0][c][p] = backupPaletteSong[c][p];
			}
		}
	}


	if (editPaletteMode)
	{
		copyPaletteToOrderList(palettePreset);

	}

	editorInfo.eseditpos = 0;
	editorInfo.eppos = 0;
	editorInfo.esnum = 0;
	editorInfo.eschn = 0;
	editorInfo.editmode = EDIT_ORDERLIST;
	songchange(gt, 1);

}

void copyPaletteToOrderList(int palettePreset)
{
	for (int c = 0;c < MAX_CHN;c++)
	{
		for (int p = 0;p < MAX_PALETTE_ENTRIES;p++)
		{
			songorder[0][0][p] = paletteRGB[palettePreset][0][p];
			songorder[0][1][p] = paletteRGB[palettePreset][1][p];
			songorder[0][2][p] = paletteRGB[palettePreset][2][p];
		}
	}
}

int highlightTableBuffer[MAX_TABLELEN];


void setTableBackgroundColours(int currentInstrument)
{

	for (int t = 0;t < MAX_TABLES;t++)
	{
		int alternateTableColor = 0;
		int needNewTable = 1;
		int startTableOffset;
		int instrumentTablePtr = instr[currentInstrument].ptr[t];
		instrumentTablePtr--;

		highlightInstrument(t, instrumentTablePtr);

		for (int i = 0;i < MAX_TABLELEN;i++)
		{

			if (needNewTable)
			{
				startTableOffset = i;
				needNewTable = 0;
			}

			int foundEnd = 0;
			if (t != 3)		// not the speed table?
			{

				if (ltable[t][i] == 0xff)	// end marker?
				{
					setTableColour(instrumentTablePtr, t, startTableOffset, i, getColor(CTABLE_FOREGROUND1 + (alternateTableColor * 2), CTABLE_BACKGROUND1 + (alternateTableColor * 2)));
					alternateTableColor = 1 - alternateTableColor;
					needNewTable = 1;
				}
			}
			else
			{
				if (ltable[t][i] != 0 || rtable[t][i] != 0)	// end marker?
				{
					setTableColour(instrumentTablePtr, t, startTableOffset, i, getColor(CTABLE_FOREGROUND1 + (alternateTableColor * 2), CTABLE_BACKGROUND1 + (alternateTableColor * 2)));
					needNewTable = 1;
				}
			}

		}

		setTableColour(instrumentTablePtr, t, startTableOffset, MAX_TABLELEN - 1, getColor(CTABLE_UNUSED_FOREGROUND, CTABLE_UNUSED_BACKGROUND));

		if (instrumentTablePtr < startTableOffset)	// Valid table data
			highlightInstrument(t, instrumentTablePtr);
	}
}



void highlightInstrument(int t, int instrumentTablePtr)
{
	for (int i = 0;i < MAX_TABLELEN;i++)
	{
		highlightTableBuffer[i] = 0;
	}

	if (instrumentTablePtr < 0)
		return;

	if (t == 3)
	{
		highlightTableBuffer[instrumentTablePtr] = 1;
		return;
	}
	for (int i = 0;i < MAX_TABLELEN;i++)
	{
		if (highlightTableBuffer[instrumentTablePtr] == 1)
			return;	// we've looped to a previously played table slot

		highlightTableBuffer[instrumentTablePtr] = 1;
		if ((ltable[t][instrumentTablePtr] == 0xff))
		{
			if ((rtable[t][instrumentTablePtr] == 0))
				break;
			else
				instrumentTablePtr = rtable[t][instrumentTablePtr] - 1;
		}
		else
			instrumentTablePtr++;
	}

}

void setTableColour(int instrumentTablePtr, int t, int startTableOffset, int endTableOffset, int color)
{
	for (int j = startTableOffset;j <= endTableOffset;j++)
	{
		if (highlightTableBuffer[j])
		{
			color &= 0xff;
			color |= (CTABLE_SELECTED_INSTRUMENT_BACKGROUND << 8);
		}
		tableBackgroundColors[t][j] = color;
	}
}

// Used to get time of overall length of song
// Either when first channel hits an END SONG or when last channel has looped
void playUntilEnd()
{
	int sng = getActualSongNumber(editorInfo.esnum, 0);
	GTOBJECT *gte = &gtEditorObject;

	initsong(sng, PLAY_BEGINNING, gte);	// JP FEB
	gte->loopEnabledFlag = 0;

	int allDone;
	do {

		playroutine(gte);
		if (gte->songinit == PLAY_STOPPED)	// Error in song data
		{
			break;
		}

		allDone = 1;
		for (int i = 0;i < maxSIDChannels;i++)
		{
			if (gte->chn[i].loopCount == 0)	// wait until all channels have looped (or song ends)
			{
				allDone = 0;	// hasn't looped
				break;
			}
		}
	} while (allDone == 0);

	setSongLengthTime(gte);
}

int mouseTransportBar(GTOBJECT *gt)
{
	if (!mouseb)
		return 0;


	if (checkMouseRange(TRANSPORT_BAR_X + 37, TRANSPORT_BAR_Y, 3, 2))
	{
		if (mouseb == MOUSEB_RIGHT)
		{
			detuneCent -= (msDelta / 2000.0f);
			if (detuneCent < -1)
				detuneCent = -1;
		}
		else
		{
			detuneCent += (msDelta / 2000.0f);
			if (detuneCent > 1)
				detuneCent = 1;
		}

		calculatefreqtable();
		return 0;


	}


	if (checkMouseRange(TRANSPORT_BAR_X + 12, TRANSPORT_BAR_Y, 3, 2))
	{
		if (mouseheld > HOLDDELAY)
		{
			setSongToBeginning(&gtObject);
			return 1;
		}
	}

	if (checkMouseRange(8, TRANSPORT_BAR_Y, 3, 2))
	{
		if (mouseb == MOUSEB_RIGHT)
		{
			masterVolume -= (msDelta / 500.0f);
			if (masterVolume < 0)
				masterVolume = 0;
		}
		else if (mouseb)
		{
			masterVolume += (msDelta / 500.0f);	// half a second to change int
			if (masterVolume > 6)
				masterVolume = 6;
		}
	}

	if (prevmouseb)
		return 0;

	int change = 1;
	if (mouseb == MOUSEB_RIGHT)
		change = -change;

	if (checkMouseRange(0, TRANSPORT_BAR_Y, 3, 2))
	{
		if (ctrlpressed)
		{
			if (bothShiftAndCtrlPressed)
			{
				stopsong(gt);
				displayCharWindow();
			}
			else
			{
				editPaletteMode = 1 - editPaletteMode;
				handlePaletteDisplay(gt, currentPalettePreset);
				if (editPaletteMode)
				{
					stopsong(gt);
				}
			}
		}
		else
		{
			currentPalettePreset += change;
			if (currentPalettePreset >= MAX_PALETTE_PRESETS)
				currentPalettePreset = MAX_PALETTE_PRESETS - 1;
			else if (currentPalettePreset < 0)
				currentPalettePreset = 0;

			setSkin(currentPalettePreset);
		}

		return 1;
	}

	//return 0;

	if (checkMouseRange(4, TRANSPORT_BAR_Y, 3, 2))
	{
		if (editPaletteMode)
			return 1;

		int newCh = maxSIDChannels + change * 3;
		if (newCh < 3)
			newCh = 3;
		else if (newCh > 12)
			newCh = 12;

		if (newCh != maxSIDChannels)
		{
			handleSIDChannelCountChange(&gtObject);
			maxSIDChannels = newCh;
		}
		return 1;
	}

	if (checkMouseRange(TRANSPORT_BAR_X, TRANSPORT_BAR_Y, 3, 2))
	{
		editorInfo.epoctave += change;
		if (editorInfo.epoctave < 0)
			editorInfo.epoctave = 0;
		else if (editorInfo.epoctave > 6)
			editorInfo.epoctave = 6;

		return 1;
	}

	if (checkMouseRange(TRANSPORT_BAR_X + 4, TRANSPORT_BAR_Y, 3, 2))
	{
		if (editPaletteMode)
			return 1;

		followplay = 1 - followplay;
		if (followplay)
			resetOrderView(&gtObject);
		return 1;
	}

	if (checkMouseRange(TRANSPORT_BAR_X + 8, TRANSPORT_BAR_Y, 3, 2))
	{
		transportLoopPattern = 1 - transportLoopPattern;
		return 1;
	}



	if (checkMouseRange(TRANSPORT_BAR_X + 12, TRANSPORT_BAR_Y, 3, 2))
	{
		if (editPaletteMode)
			return 1;

		if (mousebDoubleClick)
			previousSongPos(&gtObject, 1);
		else
		{
			if (editorInfo.eppos)
				previousSongPos(&gtObject, 0);
			else
				previousSongPos(&gtObject, 1);
		}
		return 1;
	}


	if (checkMouseRange(TRANSPORT_BAR_X + 16, TRANSPORT_BAR_Y, 3, 2))
	{
		recordmode = 1 - recordmode;
		return 1;
	}

	if (checkMouseRange(TRANSPORT_BAR_X + 20, TRANSPORT_BAR_Y, 3, 2))
	{
		if (editPaletteMode)
			return 1;
		if (gt->songinit == PLAY_STOPPED)
			playFromCurrentPosition(gt);
		else
			stopsong(gt);
		return 1;
	}



	if (checkMouseRange(TRANSPORT_BAR_X + 24, TRANSPORT_BAR_Y, 3, 2))
	{
		if (editPaletteMode)
			return 1;

		nextSongPos(&gtObject);
		return 1;
	}

	if (checkMouseRange(TRANSPORT_BAR_X + 28, TRANSPORT_BAR_Y, 4, 2))
	{
		int index = 0;
		if (mousey >= TRANSPORT_BAR_Y + 1)
			index += 2;
		if (mousex >= TRANSPORT_BAR_X + 28 + 2)
			index += 1;

		transportPolySIDEnabled[index] = 1 - transportPolySIDEnabled[index];
		return 1;
	}

	if (checkMouseRange(TRANSPORT_BAR_X + 33, TRANSPORT_BAR_Y, 3, 2))
	{
		if (shiftpressed)
			displayMIDISelectWindow();
		else
			transportShowKeyboard = 1 - transportShowKeyboard;
	}

	return 0;

}



int checkMouseRange(int x, int y, int w, int h)
{
	if (mousex >= x && mousex < x + w && mousey >= y && mousey < y + h)
		return 1;
	return 0;
}

void handleSIDChannelCountChange(GTOBJECT *gt)
{
	stopsong(gt);
	SDL_Delay(100);	// ensure that GT player has done an update, so that playing channels are now silent prior to setting new channel count


	for (int i = 0;i < MAX_PLAY_CH;i++)
	{
		int c2 = getActualChannel(editorInfo.esnum, i);

		gt->editorInfo[c2].espos = 0;	// reset current channel pos
		if (songlen[i / 6][i % 6] > 0)
			gt->editorInfo[c2].epnum = songorder[i / 6][i % 6][0];
		else
			gt->editorInfo[c2].epnum = 0;
	}
	// overkill??
	editorInfo.esnum = 1;
	songchange(gt, 1);
	editorInfo.esnum = 0;
	songchange(gt, 1);
}

void nextSongPos(GTOBJECT *gt)
{
	int songNum = getActualSongNumber(editorInfo.esnum, editorInfo.epchn);
	int ac = getActualChannel(editorInfo.esnum, editorInfo.epchn);	// 0-12
	int c3 = ac % 6;

	if (gt->songinit == PLAY_STOPPED)
	{
		if (gt->editorInfo[ac].espos < songlen[songNum][c3] - 1)
		{
			editorInfo.eseditpos = gt->editorInfo[ac].espos + 1;
			orderSelectPatternsFromSelected(gt);
			if (gt->editorInfo[ac].espos - editorInfo.esview >= VISIBLEORDERLIST)
			{
				editorInfo.esview = gt->editorInfo[ac].espos - VISIBLEORDERLIST + 1;
				editorInfo.eseditpos = gt->editorInfo[ac].espos;
			}
			updateviewtopos(gt);
		}
	}
	else
	{
		if (gt->chn[gt->masterLoopChannel].songptr < songlen[songNum][c3])
			orderPlayFromPosition(gt, 0, gt->chn[gt->masterLoopChannel].songptr, gt->masterLoopChannel);
	}
}


void previousSongPos(GTOBJECT *gt, int songDffset)
{
	int songNum = getActualSongNumber(editorInfo.esnum, editorInfo.epchn);
	int ac = getActualChannel(editorInfo.esnum, editorInfo.epchn);	// 0-12
	int c3 = ac % 6;

	if (gt->songinit == PLAY_STOPPED)
	{

		editorInfo.eseditpos = gt->editorInfo[ac].espos - songDffset;
		if (editorInfo.eseditpos < 0)
			editorInfo.eseditpos = 0;
		orderSelectPatternsFromSelected(gt);

		if (gt->editorInfo[ac].espos < editorInfo.esview)
		{
			editorInfo.esview = gt->editorInfo[ac].espos;
			editorInfo.eseditpos = gt->editorInfo[ac].espos;
		}
		updateviewtopos(gt);
	}
	else
	{
		if (gt->chn[gt->masterLoopChannel].songptr)
			orderPlayFromPosition(gt, 0, gt->chn[gt->masterLoopChannel].songptr - 1 - songDffset, gt->masterLoopChannel);
	}
}

void setSongToBeginning(GTOBJECT *gt)
{
	if (editPaletteMode)
		return;

	editorInfo.eseditpos = 0;
	editorInfo.eschn = editorInfo.epchn;
	if (gt->songinit == PLAY_STOPPED)
		orderSelectPatternsFromSelected(gt);
	else
		orderPlayFromPosition(gt, 0, 0, 0);

	editorInfo.esview = 0;
	editorInfo.eseditpos = 0;

	updateviewtopos(gt);
}

void playFromCurrentPosition(GTOBJECT *gt)
{
	if (editPaletteMode)
		return;

	int t1 = followplay;
	int t2 = gt->interPatternLoopEnabledFlag;
	gt->loopEnabledFlag = 0;
	gt->interPatternLoopEnabledFlag = 0;
	int c2 = getActualChannel(editorInfo.esnum, editorInfo.epchn);
	handleShiftSpace(gt, c2, editorInfo.eppos * 4, 0, 1);

	gt->loopEnabledFlag = transportLoopPattern;
	gt->interPatternLoopEnabledFlag = t2;
	followplay = t1;
}

void mouseTrack()
{
	editorInfo.mouseTrack = 1;
	editorInfo.mouseTrackX = mousex;
	editorInfo.mouseTrackY = mousey;
	ModifyTrackGetOriginalValue();
}


void ModifyTrackGetOriginalValue()
{
	if (editorInfo.editmode == EDIT_TABLES)
	{
		if (editorInfo.etcolumn < 2)	// columns 0+1 = lefttable value
			editorInfo.mouseTrackOriginalValue = ltable[editorInfo.etnum][editorInfo.etpos];
		else
			editorInfo.mouseTrackOriginalValue = rtable[editorInfo.etnum][editorInfo.etpos];
	}
	if (editorInfo.editmode == EDIT_INSTRUMENT)
	{
		unsigned char *ptr = &instr[editorInfo.einum].ad;
		ptr += editorInfo.eipos;
		editorInfo.mouseTrackOriginalValue = *ptr;
	}

}

// Hold left mouse button + move mouse = modify value under cursor
int mouseTrackModify(int editorWindow)
{


	if (!mouseb || (editorWindow == EDIT_TABLES && editorInfo.editTableMode != EDIT_TABLE_NONE))
	{
		editorInfo.mouseTrackDoUndo = 0;
		editorInfo.mouseTrack = 0;
		return 1;
	}

	if (editorInfo.mouseTrack == 0)	// This ensures that we only track hold/move when we start to hold on the same value that the cursor is on
		return 1;


	int xdiff = mousex - editorInfo.mouseTrackX;
	int ydiff = mousey - editorInfo.mouseTrackY;

	if (editorWindow == EDIT_TABLES)
	{

		char *dptr = &ltable[editorInfo.etnum][editorInfo.etpos];
		if (editorInfo.etcolumn > 1)	// columns 0+1 = lefttable value
			dptr = &rtable[editorInfo.etnum][editorInfo.etpos];

		int v = editorInfo.mouseTrackOriginalValue << 1;
		v += xdiff;
		v >>= 1;

		*dptr = v;

		if (*dptr != editorInfo.mouseTrackOriginalValue)
		{
			editorInfo.mouseTrackDoUndo = 1;
			return 0;	// 0 = stop recording change in UNDO. We only record the last value when releasing button for a hold/drag
		}
		return 1;
	}
	if (editorWindow == EDIT_INSTRUMENT)
	{
		if (editorInfo.eicolumn < 2 && editorInfo.eipos < 2)	// Editing ADSR. Only hold/drag on nybbles
		{
			int v = editorInfo.mouseTrackOriginalValue;

			char *dptr = &instr[editorInfo.einum].ad;
			if (editorInfo.eipos == 1)
				dptr = &instr[editorInfo.einum].sr;

			if (editorInfo.eicolumn == 0)	// high nybble			
				v >>= 4;
			else
				v &= 0xf;

			//		v <<= 1;
			v += xdiff;
			//		v >>= 1;

			if (v < 0)
				v = 0;
			else if (v > 0xf)
				v = 0xf;
			if (editorInfo.eicolumn == 0)	// high nybble
			{
				v <<= 4;
				v |= (*dptr & 0xf);
			}
			else
				v |= (*dptr & 0xf0);

			*dptr = v;
			if (*dptr != editorInfo.mouseTrackOriginalValue)
			{
				editorInfo.mouseTrackDoUndo = 1;
				return 0;	// 0 = stop recording change in UNDO. We only record the last value when releasing button for a hold/drag
			}
			return 1;
		}
		else
		{
			unsigned char *dptr = &instr[editorInfo.einum].ad;
			dptr += editorInfo.eipos;

			int v = editorInfo.mouseTrackOriginalValue << 1;
			v += xdiff;
			*dptr = v >> 1;

			setTableBackgroundColours(editorInfo.einum);

			if (*dptr != editorInfo.mouseTrackOriginalValue)
			{
				editorInfo.mouseTrackDoUndo = 1;
				return 0;	// 0 = stop recording change in UNDO. We only record the last value when releasing button for a hold/drag
			}
			return 1;

		}
	}

}

int checkForMouseInTable(int c)
{
	if ((mousey > 14 + 3) && (mousey <= 20 + 3 + 9) && (mousex >= 43 + 20 + c * 10) && (mousex <= 47 + 20 + c * 10))
	{
		if (editorInfo.editmode != EDIT_TABLES && prevmouseb)	// Don't allow hold/drag to select another panel
			return 0;

		if (!mouseb)
			return 0;

		int newpos = mousey - 15 - 3 + editorInfo.etview[editorInfo.etnum];
		if (newpos < 0) newpos = 0;
		if (newpos >= MAX_TABLELEN) newpos = MAX_TABLELEN - 1;

		editorInfo.editmode = EDIT_TABLES;

		if ((mouseb & (MOUSEB_RIGHT | MOUSEB_MIDDLE)) && (!prevmouseb))
		{
			if ((editorInfo.etmarknum != editorInfo.etnum) || (newpos != editorInfo.etmarkend))
			{
				editorInfo.etmarknum = c;
				editorInfo.etmarkstart = editorInfo.etmarkend = newpos;
			}
		}
		if (mouseb & MOUSEB_LEFT && (!prevmouseb))
		{
			editorInfo.etnum = c;
			editorInfo.etpos = mousey - 15 - 3 + editorInfo.etview[editorInfo.etnum];
			editorInfo.etcolumn = mousex - 43 - 20 - c * 10;
			if (editorInfo.etcolumn > 2) editorInfo.etcolumn--;
			mouseTrack();	// MUST DO AFTER SETTING ABOVE VALUES

		}

		if (editorInfo.etpos < 0) editorInfo.etpos = 0;
		if (editorInfo.etpos > MAX_TABLELEN - 1) editorInfo.etpos = MAX_TABLELEN - 1;

		if (mouseb & (MOUSEB_RIGHT | MOUSEB_MIDDLE)) editorInfo.etmarkend = newpos;

		return 1;
	}
}


int checkForMouseInDetailedFilterTable()
{

	if ((mousey > 14 + 3) && (mousey <= 20 + 3 + 9))
	{
		int newpos = mousey - 15 - 3 + editorInfo.etview[editorInfo.etnum];
		if (newpos < 0) newpos = 0;
		if (newpos >= MAX_TABLELEN) newpos = MAX_TABLELEN - 1;

		if (mousex >= 64 && mousex <= 67 && mouseb && !prevmouseb)
		{
			detailedFilterTableChangeCommand(mousex - 64, newpos);	// select Filter Cutoff, modify,filterinfo, jump/end option
			return 1;
		}
		else if (mousex >= 80 && mousex <= 82 && mouseb && !prevmouseb)
		{
			detailedFilterTableChangeSign(mousex - 80, newpos);
		}
		else if (mousex >= 84 && mousex <= 89 && mouseb && !prevmouseb)
		{
			detailedFilterTableChangeFilterType((mousex - 84) / 2, newpos);
		}

		int col = -1;
		if (mousex > 68 && mousex < 77)
		{
			col = 0;
		}
		else if (mousex >= 77 && mousex <= 78)
			col = mousex - 77;
		else if (mousex >= 81 && mousex <= 82)
			col = 2 + (mousex - 81);
		if (col == -1)
			return 0;

		if (editorInfo.editmode != EDIT_TABLES && prevmouseb)	// Don't allow hold/drag to select another panel
			return 0;

		if (!mouseb)
			return 0;



		editorInfo.editmode = EDIT_TABLES;

		if ((mouseb & (MOUSEB_RIGHT | MOUSEB_MIDDLE)) && (!prevmouseb))
		{
			if ((editorInfo.etmarknum != editorInfo.etnum) || (newpos != editorInfo.etmarkend))
			{
				editorInfo.etmarknum = 0;
				editorInfo.etmarkstart = editorInfo.etmarkend = newpos;
			}
		}
		if (mouseb & MOUSEB_LEFT && (!prevmouseb))
		{

			editorInfo.etnum = FTBL;
			editorInfo.etpos = mousey - 15 - 3 + editorInfo.etview[editorInfo.etnum];
			editorInfo.etcolumn = col;
			mouseTrack();	// MUST DO AFTER SETTING ABOVE VALUES
		}
		if (editorInfo.etpos < 0) editorInfo.etpos = 0;
		if (editorInfo.etpos > MAX_TABLELEN - 1) editorInfo.etpos = MAX_TABLELEN - 1;

		if (mouseb & (MOUSEB_RIGHT | MOUSEB_MIDDLE)) editorInfo.etmarkend = newpos;

		return 1;
	}
}



int checkForMouseInDetailedPulseTable()
{

	if ((mousey > 14 + 3) && (mousey <= 20 + 3 + 9))
	{
		int newpos = mousey - 15 - 3 + editorInfo.etview[editorInfo.etnum];
		if (newpos < 0) newpos = 0;
		if (newpos >= MAX_TABLELEN) newpos = MAX_TABLELEN - 1;

		if (mousex >= 64 && mousex <= 66 && mouseb && !prevmouseb)
		{
			detailedPulseTableChangeCommand(mousex - 64, newpos);
			return 1;
		}
		else if (mousex == 81 && mouseb && !prevmouseb)
		{
			detailedPulseTableChangeSign(mousex - 81, newpos);
		}


		int col = -1;
		if (mousex > 68 && mousex < 77)
		{
			col = 0;
		}
		else if (mousex >= 77 && mousex <= 79)	// 0-2 = left column (2 only used for setting pulse width)
			col = mousex - 77;
		else if (mousex >= 82 && mousex <= 83)	// 3-4 = right column
			col = 3 + (mousex - 82);
		if (col == -1)
			return 0;

		if (editorInfo.editmode != EDIT_TABLES && prevmouseb)	// Don't allow hold/drag to select another panel
			return 0;

		if (!mouseb)
			return 0;

		editorInfo.editmode = EDIT_TABLES;

		if ((mouseb & (MOUSEB_RIGHT | MOUSEB_MIDDLE)) && (!prevmouseb))
		{
			if ((editorInfo.etmarknum != editorInfo.etnum) || (newpos != editorInfo.etmarkend))
			{
				editorInfo.etmarknum = 0;
				editorInfo.etmarkstart = editorInfo.etmarkend = newpos;
			}
		}
		if (mouseb & MOUSEB_LEFT && (!prevmouseb))
		{
			editorInfo.etnum = PTBL;
			editorInfo.etpos = mousey - 15 - 3 + editorInfo.etview[editorInfo.etnum];
			editorInfo.etcolumn = col;
			mouseTrack();	// MUST DO AFTER SETTING ABOVE VALUES
		}
		if (editorInfo.etpos < 0) editorInfo.etpos = 0;
		if (editorInfo.etpos > MAX_TABLELEN - 1) editorInfo.etpos = MAX_TABLELEN - 1;

		if (mouseb & (MOUSEB_RIGHT | MOUSEB_MIDDLE)) editorInfo.etmarkend = newpos;

		return 1;
	}
}



/*
Mouse clicking on the various -WDCJ / AB / + options in the detailed wavetable view?
If so, initialise data accordingly.
*/
int checkForMouseInDetailedWaveTable()
{
	if ((mousey > 14 + 3) && (mousey <= 20 + 3 + 9))
	{
		int newpos = mousey - 15 - 3 + editorInfo.etview[editorInfo.etnum];
		if (newpos < 0) newpos = 0;
		if (newpos >= MAX_TABLELEN) newpos = MAX_TABLELEN - 1;

		if (mousex >= 64 && mousex <= 68 && mouseb && !prevmouseb)
		{
			detailedWaveTableChangeCommand(mousex - 64, newpos);	// Clicked on either -WDCJ
			return 1;
		}
		else if (mousex >= 70 && mousex <= 71 && mouseb && !prevmouseb)
		{
			detailedWaveTableChangeData(mousex - 70, newpos);		// Clicked on either R or A (relative / Absolute)
			return 1;
		}
		else if (mousex == 81 && mouseb && !prevmouseb)
		{
			detailedWaveTableChangeRelativeNote(mousex - 70, newpos);	// Clicked on +/- to change sign of relative note
			return 1;
		}
		int col = -1;
		if (mousex > 72 && mousex < 78)
		{
			col = 0;
		}
		else if (mousex >= 78 && mousex <= 79)
			col = mousex - 78;
		else if (mousex >= 82 && mousex <= 83)
			col = 2 + (mousex - 82);
		if (col == -1)
			return 0;

		if (editorInfo.editmode != EDIT_TABLES && prevmouseb)	// Don't allow hold/drag to select another panel
			return 0;

		if (!mouseb)
			return 0;



		editorInfo.editmode = EDIT_TABLES;

		if ((mouseb & (MOUSEB_RIGHT | MOUSEB_MIDDLE)) && (!prevmouseb))
		{
			if ((editorInfo.etmarknum != editorInfo.etnum) || (newpos != editorInfo.etmarkend))
			{
				editorInfo.etmarknum = 0;
				editorInfo.etmarkstart = editorInfo.etmarkend = newpos;
			}
		}
		if (mouseb & MOUSEB_LEFT && (!prevmouseb))
		{
			editorInfo.etnum = 0;
			editorInfo.etpos = mousey - 15 - 3 + editorInfo.etview[editorInfo.etnum];
			editorInfo.etcolumn = col;
			mouseTrack();	// MUST DO AFTER SETTING ABOVE VALUES
		}
		if (editorInfo.etpos < 0) editorInfo.etpos = 0;
		if (editorInfo.etpos > MAX_TABLELEN - 1) editorInfo.etpos = MAX_TABLELEN - 1;

		if (mouseb & (MOUSEB_RIGHT | MOUSEB_MIDDLE)) editorInfo.etmarkend = newpos;

		return 1;
	}
}




/*
Wavetable right side:  00-5F Relative notes
			   60-7F Negative relative notes (lower pitch)
			   80    Keep frequency unchanged
			   81-DF Absolute notes C#0 - B-7

*/

/*
User has clicked on +/- to change relative note sign
*/
void detailedWaveTableChangeRelativeNote(int x, int y)
{
	unsigned char v = ltable[0][y];
	if (v < 1 || v >= 0xf0)	// Does right table contain abs or relative note?
		return;	// No..

	int vr = rtable[0][y];
	if (vr >= 0x80 || vr == 0)	// Does right table contain relative note (or a non-zero relative note)?
		return;	// No..

	if (vr <= 0x5f)	// positive relative?
	{
		vr = 0x80 - vr;
		if (vr < 0x60)
			vr = 0x60;
		rtable[0][y] = vr;
	}
	else
	{
		vr -= 0x60;
		vr = 0x20 - vr;
		rtable[0][y] = vr;
	}

}

/*
User has clicked on R/A to change wavetable relative / absolute
*/
void detailedWaveTableChangeData(int x, int y)
{
	unsigned char v = ltable[0][y];
	if (v < 1 || v >= 0xf0)	// Does right table contain abs or relative note?
		return;	// No..

	v = rtable[0][y];


	if (v >= 0x81)	// was abs note?
	{
		if (x == 1)
			rtable[0][y] = 0x80;	// Was absolute already. So turn off (make "no change")
		else
			rtable[0][y] = 0;	// set to Relative
	}
	else if (v < 0x80)	// relative note?
	{
		if (x == 0)
			rtable[0][y] = 0x80;	// Was Relative already. So turn off (make "no change")
		else
			rtable[0][y] = 0x81;	// Set to Absolute
	}
	else if (v == 0x80)	// Was "no change"
	{
		if (x == 0)
			rtable[0][y] = 0;	// Set to Relative
		else
			rtable[0][y] = 0x81;	// Set to Absolute
	}

}


/*
Wavetable left side:   00    Leave waveform unchanged
					   01-0F Delay this step by 1-15 frames
					   10-DF Waveform values
					   E0-EF Inaudible waveform values $00-$0F
					   F0-FE Execute command 0XY-EXY. Right side is parameter.
					   FF    Jump. Right side tells position ($00 = stop)

*/

/*
If user changes the command type, we need to reset the data within the table so that it's initialised in range for that specific command
*/
void detailedWaveTableChangeCommand(int x, int y)
{

	unsigned char v = ltable[WTBL][y];

	int currentCommand = 0;
	if (v >= 1 && v <= 0xf)
		currentCommand = 2;
	else if (v >= 0x10 && v <= 0xef)
		currentCommand = 1;
	else if (v >= 0xf0 && v <= 0xfe)
		currentCommand = 3;
	else if (v == 0xff)
		currentCommand = 4;

	if (x == currentCommand)	// selecting the same command?
		return;

	if (x == 0)
		ltable[WTBL][y] = 0;
	else if (x == 2)
		ltable[WTBL][y] = 1;
	else if (x == 1)
	{
		ltable[WTBL][y] = 0x41;
	}
	else if (x == 3)
		ltable[WTBL][y] = 0xf0;
	else if (x == 4)
	{
		ltable[WTBL][y] = 0xff;
		rtable[WTBL][y] = 0;
	}
}



/*
Filtertable left side: 00    Set cutoff, indicated by right side
					   01-7F Filter modulation step. Left side indicates time
							 and right side the speed (signed 8-bit value)
					   80-F0 Set filter parameters. Left side high nybble
							 tells the passband ($90 = lowpass, $A0 = bandpass
							 etc.) and right side tells resonance/channel
							 bitmask, as in command BXY.
					   FF    Jump. Right side tells position ($00 = stop)
*/

/*
If user changes the command type, we need to reset the data within the table so that it's initialised in range for that specific command
*/

void detailedFilterTableChangeFilterType(int x, int y)
{

	unsigned char v = ltable[FTBL][y];

	if (v >= 0x80 && v <= 0xfe)
	{
		v ^= (0x10 << x);
		ltable[FTBL][y] = v;
	}
}

void detailedFilterTableChangeSign(int x, int y)
{
	unsigned char v = ltable[FTBL][y];
	if (v >= 0x1 && v <= 0x7f)
	{
		if (x == 0)// filter modulation
		{
			v = rtable[FTBL][y];
			if (v >= 0x80)
			{
				v = 0x100 - v;
				rtable[FTBL][y] = v;
			}
			else
			{
				if (v == 0)
					v = 1;
				v = 0x100 - v;
				rtable[FTBL][y] = v;
			}
		}
	}
	else if (v >= 0x80)
	{
		int rv = rtable[FTBL][y];
		rv ^= 1 << x;
		rtable[FTBL][y] = rv;
	}
}



void detailedPulseTableChangeSign(int x, int y)
{
	unsigned char v = ltable[PTBL][y];
	if (v >= 0x1 && v <= 0x7f)
	{
		if (x == 0)// filter modulation
		{
			v = rtable[PTBL][y];
			if (v >= 0x80)
			{
				v = 0x100 - v;
				rtable[PTBL][y] = v;
			}
			else
			{
				if (v == 0)
					v = 1;
				v = 0x100 - v;
				rtable[PTBL][y] = v;
			}
		}
	}
}


void detailedFilterTableChangeCommand(int x, int y)
{

	unsigned char v = ltable[FTBL][y];

	// CMFJ
	int currentCommand = 0;
	if (v >= 1 && v <= 0x7f)
		currentCommand = 1;
	else if (v >= 0x80 && v <= 0xfe)
		currentCommand = 2;
	else if (v == 0xff)
		currentCommand = 3;

	if (x == currentCommand)	// selecting the same command?
		return;

	if (x == 0)
	{
		ltable[FTBL][y] = 0;
		rtable[FTBL][y] = 0;
	}
	else if (x == 1)
	{
		ltable[FTBL][y] = 0x1;
		rtable[FTBL][y] = 0x0;
	}
	else if (x == 2)
	{
		ltable[FTBL][y] = 0x90;		// low pass
		rtable[FTBL][y] = 0x7;		// all channels
	}
	else if (x == 3)
	{
		ltable[FTBL][y] = 0xff;
		rtable[FTBL][y] = 0;
	}

}


void detailedPulseTableChangeCommand(int x, int y)
{

	unsigned char v = ltable[PTBL][y];

	// CMFJ
	int currentCommand = 0;
	if (v >= 1 && v <= 0x7f)
		currentCommand = 1;
	else if (v >= 0x80 && v <= 0xfe)
		currentCommand = 0;
	else if (v == 0xff)
		currentCommand = 2;

	if (x == currentCommand)	// selecting the same command?
		return;

	if (x == 1)	// Modify
	{
		ltable[PTBL][y] = 0x1;
		rtable[PTBL][y] = 0x0;
	}
	else if (x == 0) //Set
	{
		ltable[PTBL][y] = 0x88;		// Set pulse to 0x800 as default
		rtable[PTBL][y] = 0x0;		//
	}
	else if (x == 2)
	{
		ltable[PTBL][y] = 0xff;		// stop
		rtable[PTBL][y] = 0;
	}

}

int checkMouseInWaveformInfo()
{
	if (!mouseb)
		return 0;
	if (prevmouseb)
		return 0;
	if (mousey != TRANSPORT_BAR_Y - 1)
		return 0;
	if (waveformDisplayInfo.displayOnOff == 0)
		return 0;

	int x = mousex - (TRANSPORT_BAR_X - 5);
	if (x < 0)
		return 0;
	if (x % 5 == 0)
		return 0;

	x /= 5;
	if (x > 7)
		return 0;

	waveformDisplayInfo.value ^= (0x80 >> x);

	if (editorInfo.editmode == EDIT_TABLES)
	{
		if (ltable[WTBL][editorInfo.etpos] < 0xf0)
		{
			int data = waveformDisplayInfo.value;	// wavetable value in left table
			if (data < 0x10)
				data += 0xe0;
			*waveformDisplayInfo.destAddress = (unsigned char)data;
		}
		else
		{
			*waveformDisplayInfo.destAddress = (unsigned char)waveformDisplayInfo.value;
		}
	}
	else if (editorInfo.editmode == EDIT_INSTRUMENT)
	{
		*waveformDisplayInfo.destAddress = (unsigned char)waveformDisplayInfo.value;
	}
	else if (editorInfo.editmode == EDIT_PATTERN)
	{
		*waveformDisplayInfo.destAddress = (unsigned char)waveformDisplayInfo.value;

	}

	return 1;
}


// Wrote all this, then realised I could just easily modify the existing calculatefreqtable 
// Will leave it here anyway. May use it again one day...
float noteToHz(int note)
{
	note *= 100;
	return centToHz(note);
}

// 1200 per octave (100 per semitone)
float centToHz(int cent)
{
	float a = 440.0f; //frequency of A (coomon value is 440Hz)
	return (a / 32) * pow(2, ((cent - 900) / 1200.0));
}


int HzToSIDFreq(float hz)
{
	float phi = 985248;	// PAL

//	phi = 1022727;	//NTSC

	float freqCons = (256 * 256 * 256) / phi;
	float sidFreq = freqCons * hz;
	return sidFreq;
}

void detunePitchTable()
{
	for (int i = 0;i < 0x60;i++)
	{
		int cent = (12 + i) * 100;
		cent += detuneCent - 100;	// -99 > + 99

		float hz = centToHz(cent);
		int SIDFreq = HzToSIDFreq(hz);
		freqtbllo[i] = SIDFreq & 0xff;
		freqtblhi[i] = (SIDFreq >> 8) & 0xff;
	}

}

