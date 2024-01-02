# speaker-test1

The purpose of this program is to demonstrate how to use the ALSA sound
system to send audio to the speaker (and to the IF chain transmit mixer)
of the sBitx SDR.

## Building the program

To build the program -- 

Be sure that execute permission has been set on the build-spkr-test1.sh
script by entering the command:

	chmod +x build-spkr-test1.sh

CAUTION - the noise generated from the next step will be LOUD!!
execute the build script by entering the command:

	./build-spkr-test1.sh
	
## Running the program

Run the program by entering the command:

	./spkr-test1 << /dev/urandom
	
Note that the program can accept any file that's formatted in 32 bit
stereo (two signed 16 bit little-endian samples per 32 bits), sampled
at 96000 samples/second.

## Using Audacity to generate the expected audio file format

You can use Audacity to export 32 bit stereo sampled at 96000 samples/second.

Install it using 'sudo apt install audacity'.

Once Audacity is opened:
  - Select from the main menu: File -> Import
  - The dialog allows you to open audio files on your computer
  - At the bottom of the main screen, set Project Sample Rate to 96000
  - Select from the main menu: File -> Export Audio
  - At the bottom of the dialog, select 'Other uncompressed files' as 
    the format of the exported file
  - Then for Header: select 'Raw (header-less)' and for Encoding select
    'Signed 16 bit PCM'
  - Finally, select the file name for the output file, by convention using
    .raw as the extension
  - If the Edit Metadata Tags dialog pops up, just hit Clear then OK

