/*
*
* This example reads standard from input and writes
* to the default PCM device for RUNTIME seconds of data.
* 
* Based on code from https://www.cnblogs.com/dong1/p/10565722.html
* See also: https://www.linuxjournal.com/article/6735
*
* Requires libasound2-dev to be installed (already installed on sbitx)
*
*/

/* Use the newer ALSA API */
#define ALSA_PCM_NEW_HW_PARAMS_API
#define RUNTIME 10        // Seconds
#define NUMFRAMES 1024    // Buffer Size in Frames (not bytes!)
#define SAMPLERATE 96000  // Sample Rate
#define CHANNELS 2        // Stereo audio

#include <alsa/asoundlib.h>

int main() {
  long loops;
  int rc;
  int buffsize;
  snd_pcm_t *handle;
  snd_pcm_hw_params_t *params;
  unsigned int val;
  int dir;
  snd_pcm_uframes_t frames;
  char *buffer;
  int offset;

  // Use zero for the default (blocking) stream mode. If you change it to
  // SND_PCM_NONBLOCK and re-build, it will run in 7-8 seconds not 10!!
  // int mode = 0;
  int mode = SND_PCM_NONBLOCK;

  // For a PC host, use "default".
  // For sbitx, use "plughw:0,0".
  // For sbitx, using "hw:0,0" for direct access worked with mode = 0
  // but not with mode = SND_PCM_NONBLOCK which suggests "plughw" is 
  // doing some time correction / rebuffering in nonblocking mode.
  // "the plug plugin performs channel duplication, sample value conversion and"
  // "resampling when necessary": https://www.volkerschatz.com/noise/alsa.html
  // char *device = "default";
  // char *device = "hw:0,0";
  // char *device = "plughw:0,0";

  // On sbitx, instead of logical names, we can try physical names 
  // This avoids where the system assigns logical names in an unexpected way
  // For instance, "hw:0,0" some times gets assigned to HDMI-attached speakers
  // The more specific we are, the more likely we are to get what we want
  // To get the list of such devices on your sbitx:
  // $ aplay -L | grep CARD= | grep DEV=0 | egrep '^hw:|^plughw:' | sort
  // Note the Loopback devices are used by sbitx to send audio to wsjtx and fldigi.
  // char *device = "hw:CARD=audioinjectorpi,DEV=0";
  char *device = "plughw:CARD=audioinjectorpi,DEV=0";

  /* Open PCM device for playback. */
  rc = snd_pcm_open(&handle, device, SND_PCM_STREAM_PLAYBACK, mode); 
  if (rc < 0) {
    fprintf(stderr,
            "unable to open pcm device: %s\n",
            snd_strerror(rc));
    exit(1);
  }

  /* Allocate a hardware parameters object. */
  snd_pcm_hw_params_alloca(&params);

  /* Fill it in with default values. */
  snd_pcm_hw_params_any(handle, params);

  /* Set the desired hardware parameters. */

  /* Interleaved mode */
  snd_pcm_hw_params_set_access(handle, params,
                      SND_PCM_ACCESS_RW_INTERLEAVED);

  /* Signed 16-bit little-endian format */
  snd_pcm_hw_params_set_format(handle, params,
                              SND_PCM_FORMAT_S16_LE);

  /* Two channels (stereo) */
  snd_pcm_hw_params_set_channels(handle, params, CHANNELS);

  /* SAMPLERATE bits/second sampling rate */
  val = SAMPLERATE;
  snd_pcm_hw_params_set_rate_near(handle, params,
                                  &val, &dir);

  /* Set period size to NUMFRAMES frames (not bytes). */
  /* The set_period_size_near code may round it up or down */
  frames = NUMFRAMES; 
  snd_pcm_hw_params_set_period_size_near(handle,
                              params, &frames, &dir);

  /* Write the parameters to the driver */
  rc = snd_pcm_hw_params(handle, params);
  if (rc < 0) {
    fprintf(stderr,
            "unable to set hw parameters: %s\n",
            snd_strerror(rc));
    exit(1);
  }

  /* Each frame has two sixteen-bit samples, one for each channel */
  int bytes_per_frame = CHANNELS * sizeof(int16_t); 
  buffsize = frames * bytes_per_frame;

  /* The buffer holds "frames" number of frames */
  buffer = (char *) malloc(buffsize);

  /* Find out how many microseconds it will take to play period_size frames. */
  /* This is the period time.  We set the period_size to NUMFRAMES frames. */
  snd_pcm_hw_params_get_period_time(params, &val, &dir);

  /* Find how many loops it will take to play for RUNTIME seconds by */
  /* converting RUNTIME to microseconds and dividing by period time */
  loops = RUNTIME * 1000000 / val;
  
  printf("Run Time: %d Seconds\n", RUNTIME);
  printf("Buffer Size: %d Frames (%d Bytes)\n", NUMFRAMES, buffsize);
  printf("Period Time: %d Microseconds\n", val);
  printf("Sample Rate: %d Samples/Second \n", SAMPLERATE);
  printf("Executing %ld loops.\n",loops);

  while (loops > 0) {
    loops--;
    
// ************************************************************* The Read portion.    

    rc = read(0, buffer, buffsize);
    if (rc == 0) {
      fprintf(stderr, "end of file on input\n");
      break;
    } else if (rc != buffsize) {
      fprintf(stderr,
              "short read: read %d bytes\n", rc);
    }
    
// ************************************************************* The Write portion.    

    /* wait for hardware to be available */
    /* (EAGAIN means resource not available) */
    /* result indicates number of frames we can write */
    do
    {
      rc = snd_pcm_avail(handle);
    } while (rc == -EAGAIN);
    
    frames = NUMFRAMES;
    offset = 0;
    
    while (frames > 0)
    {
      printf("Loop: %4ld: Our frames to write: %4ld, System frames available: %d\n", loops, frames, rc);
      
      do
      {
        rc = snd_pcm_writei(handle, buffer + offset, frames);
      } while (rc == -EAGAIN);
      
      if (rc == -EPIPE)
      {
        /* EPIPE means underrun */
        fprintf(stderr, "Loop # %ld, Underrun occurred ", loops);
        fprintf(stderr, "Error Number %d\n", rc);
        snd_pcm_prepare(handle);
      }
      else if (rc < 0)
      {
        fprintf(stderr,
                "Loop # %ld, error %d from writei: %s\n", loops, rc,
                snd_strerror(rc));
      }
      else if (rc != (int)frames)
      {
        fprintf(stderr,
                "Loop # %ld, short write, write %d frames\n", loops, rc);
      }
      if (rc > 0)
      {
        frames -= rc;
        offset += (rc * 2); // why 2 when frames are 4 bytes long?
      }
    } // End of while (frames > 0) loop

  } // End of while (loops > 0) loop

  snd_pcm_drain(handle);
  snd_pcm_close(handle);
  free(buffer);

  return 0;
}

