/*
*
* This example reads standard from input and writes
* to the default PCM device for RUNTIME seconds of data.
* 
* Based on code from https://www.cnblogs.com/dong1/p/10565722.html
*
* Requires libasound2-dev to be installed (already installed on sbitx)
*
*/

/* Use the newer ALSA API */
#define ALSA_PCM_NEW_HW_PARAMS_API
#define RUNTIME 10        // Seconds
#define NUMFRAMES 1024     // Buffer Size
#define SAMPLERATE 96000  // Sample Rate

#include <alsa/asoundlib.h>

int main() {
  long loops;
  int rc;
  int size;
  snd_pcm_t *handle;
  snd_pcm_hw_params_t *params;
  unsigned int val;
  int dir;
  snd_pcm_uframes_t frames;
  char *buffer;
  int offset;

  /* Open PCM device for playback. */
  /* Use "plughw:0,0" for an sbitx host, and "default" for a PC host */  
  rc = snd_pcm_open(&handle, "plughw:0,0",
                    SND_PCM_STREAM_PLAYBACK, 0); // Change to  SND_PCM_NONBLOCK and re-build. It will run in 7-8 seconds!!
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
  snd_pcm_hw_params_set_channels(handle, params, 2);

  /* SAMPLERATE bits/second sampling rate (CD quality) */
  val = SAMPLERATE;
  snd_pcm_hw_params_set_rate_near(handle, params,
                                  &val, &dir);

  /* Set period size to NUMFRAMES frames. */
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

  /* Use a buffer large enough to hold one period */
  snd_pcm_hw_params_get_period_size(params, &frames,
                                    &dir);
  size = frames * 4; /* 2 bytes/sample, 2 channels */
  buffer = (char *) malloc(size);

  /* We want to loop for RUNTIME seconds */
  snd_pcm_hw_params_get_period_time(params,
                                    &val, &dir);
  /* RUNTIME seconds in microseconds divided by
   * period time */
  loops = RUNTIME * 1000000 / val;
  
  printf("Run Time: %d\n", RUNTIME);
  printf("Buffer Size: %d\n", NUMFRAMES);
  printf("Sample Rate: %d\n", SAMPLERATE);
  printf("Executing %ld loops.\n",loops);

  while (loops > 0) {
    loops--;
    rc = read(0, buffer, size);
    if (rc == 0) {
      fprintf(stderr, "end of file on input\n");
      break;
    } else if (rc != size) {
      fprintf(stderr,
              "short read: read %d bytes\n", rc);
    }
    
// ************************************************************* The Write portion.    
    do
    {
      rc = snd_pcm_avail(handle);
    } while (rc == -11);
    
    frames = NUMFRAMES;
    offset = 0;
    
    while (frames > 0)
    {
      printf("Loop: %ld, Frames to write: %ld, Frames available: %d\n", loops, frames, rc);
      
      do
      {
        rc = snd_pcm_writei(handle, buffer + offset, frames);
      } while (rc == -11);
      
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
        offset += (rc * 2);
      }
    } // End of while (frames > 0) loop
  }

  snd_pcm_drain(handle);
  snd_pcm_close(handle);
  free(buffer);

  return 0;
}

