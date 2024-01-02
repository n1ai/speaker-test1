README for spkr-test1

The purpose of this program is to demonstrate how to use the ALSA sound
system to send audio to the speaker (and to the IF chain transmit mixer)
of the sBitx SDR.

To build the program -- 

Be sure that execute permission has been set on the build-spkr-test1.sh
script by entering the command:

	chmod +x build-spkr-test1.sh

CAUTION - the noise generated from the next step will be LOUD!!
execute the build script by entering the command:

	./build-spkr-test1.sh
	
Run the program by entering the command:

	./spkr-test1 << /dev/urandom
	
Note that the program can accept any file that's firmatted in 32 bit
stereo, sampled ad 96000 samples/second.

