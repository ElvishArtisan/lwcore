// capture.cpp
//
// Capture routines for the AudioQueue class
//
//   (C) Copyright 2015 Fred Gleason <fredg@paravelsystems.com>
//
//   This program is free software; you can redistribute it and/or modify
//   it under the terms of the GNU General Public License version 2 as
//   published by the Free Software Foundation.
//
//   This program is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.
//
//   You should have received a copy of the GNU General Public
//   License along with this program; if not, write to the Free Software
//   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//

#include <errno.h>
#include <sched.h>
#include <syslog.h>

#include <samplerate.h>
#include <soundtouch/SoundTouch.h>

#include "audioqueue.h"
#include "ringbuffer.h"

bool AudioQueue::StartCaptureDevice(const QString &dev)
{
  snd_pcm_hw_params_t *hwparams;
  snd_pcm_sw_params_t *swparams;
  int aerr;
  LwShm *shm=queue_shm+queue_slot;
  struct sched_param sched;

  if(snd_pcm_open(&shm->capture_pcm,dev.toUtf8(),SND_PCM_STREAM_CAPTURE,0)!=0) {
    syslog(LOG_ERR,(tr("unable to open ALSA device")+" \""+dev+"\"").toUtf8());
    return false;
  }
  snd_pcm_hw_params_alloca(&hwparams);
  snd_pcm_hw_params_any(shm->capture_pcm,hwparams);

  if(!SetAlsaParameters(shm->capture_pcm,hwparams)) {
    snd_pcm_close(shm->capture_pcm);
    shm->capture_pcm=NULL;
    return false;
  }

  //
  // Fire It Up
  //
  if((aerr=snd_pcm_hw_params(shm->capture_pcm,hwparams))<0) {
    syslog(LOG_ERR,(tr("ALSA device error")+": "+snd_strerror(aerr)).toUtf8());
    snd_pcm_close(shm->capture_pcm);
    shm->capture_pcm=NULL;
    return false;
  }

  //
  // Set Wake-up Timing
  //
  snd_pcm_sw_params_alloca(&swparams);
  snd_pcm_sw_params_current(shm->capture_pcm,swparams);
  snd_pcm_sw_params_set_avail_min(shm->capture_pcm,swparams,
				  ALSA_SAMPRATE/2);
  if((aerr=snd_pcm_sw_params(shm->capture_pcm,swparams))<0) {
    syslog(LOG_ERR,(tr("ALSA device error")+": "+snd_strerror(aerr)).toUtf8());
    snd_pcm_close(shm->capture_pcm);
    shm->capture_pcm=NULL;
    return false;
  }

  //
  // Start the Capture Process
  //
  if(fork()!=0) {
    memset(&sched,0,sizeof(sched));
    if((sched.sched_priority=sched_get_priority_min(SCHED_FIFO))>=0) {
      if(sched_setscheduler(0,SCHED_FIFO,&sched)<0) {
	syslog(LOG_WARNING,"unable to set realtime priority [%s]",
	       strerror(errno));
      }
    }
    else {
      syslog(LOG_WARNING,"SCHED_FIFO policy does not exist");
    }
    RunCapture(shm);
    exit(0);
  }
  queue_capture_running=true;

  return true;
}


void AudioQueue::RunCapture(LwShm *shm)
{
  snd_pcm_sframes_t n;
  Ringbuffer *ring=
    new Ringbuffer(QUEUE_MAX_DELAY*ALSA_SAMPRATE,QUEUE_CHANNELS);
  int pcm32[QUEUE_PERIOD_SIZE*QUEUE_CHANNELS];
  float pcm[QUEUE_PERIOD_SIZE*QUEUE_CHANNELS];
  soundtouch::SoundTouch *st=new soundtouch::SoundTouch();

  st->setSampleRate(ALSA_SAMPRATE);
  st->setChannels(QUEUE_CHANNELS);
  st->setRateChange(0.0);

  while(!shm->exiting) {
    shm->delay=ring->readSpace();
    /*
    if(delay<shm->set_max_delay) {
      st->setTempoChange(-100.0*shm->set_max_offset);
    }
    else {
      st->setTempoChange(0.0);
    }
    */
    n=snd_pcm_readi(shm->capture_pcm,pcm32,240);
    if((n<0)&&(snd_pcm_state(shm->capture_pcm)!=SND_PCM_STATE_RUNNING)&&
       (!shm->exiting)) {  // Recover from an xrun
      snd_pcm_drop(shm->capture_pcm);
      snd_pcm_prepare(shm->capture_pcm);
    }
    else {
      src_int_to_float_array(pcm32,pcm,n*QUEUE_CHANNELS);
 
      st->putSamples(pcm,n);
      n=st->receiveSamples(pcm,QUEUE_PERIOD_SIZE);

      ring->write(pcm,n);
      memset(pcm,0,sizeof(float)*QUEUE_PERIOD_SIZE*QUEUE_CHANNELS);
      if(ring->readSpace()>=240) {  // Mute it until we have enough to start
	ring->read(pcm,240);
      }
      src_float_to_int_array(pcm,shm->pcm+PCM_OFFSET(shm->period),
			     240*QUEUE_CHANNELS);
      shm->period++;
    }
  }
  snd_pcm_close(shm->capture_pcm);
  shm->capture_pcm=NULL;

  exit(0);
}
