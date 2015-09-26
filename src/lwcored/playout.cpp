// playout.cpp
//
// Playout routines for the AudioQueue class
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

#include <syslog.h>

#include "audioqueue.h"

bool AudioQueue::StartPlayoutDevice(const QString &dev)
{
  snd_pcm_hw_params_t *hwparams;
  snd_pcm_sw_params_t *swparams;
  int aerr;
  LwShm *shm=queue_shm+queue_slot;

  if(snd_pcm_open(&shm->playout_pcm,dev.toUtf8(),
		  SND_PCM_STREAM_PLAYBACK,0)!=0) {
    syslog(LOG_ERR,(tr("unable to open ALSA device")+" \""+dev+"\"").toUtf8());
    return false;
  }
  snd_pcm_hw_params_alloca(&hwparams);
  snd_pcm_hw_params_any(queue_shm->playout_pcm,hwparams);

  if(!SetAlsaParameters(shm->playout_pcm,hwparams)) {
    snd_pcm_close(shm->playout_pcm);
    shm->playout_pcm=NULL;
    return false;
  }

  //
  // Fire It Up
  //
  if((aerr=snd_pcm_hw_params(shm->playout_pcm,hwparams))<0) {
    syslog(LOG_ERR,(tr("ALSA device error")+": "+snd_strerror(aerr)).toUtf8());
    snd_pcm_close(shm->playout_pcm);
    shm->playout_pcm=NULL;
    return false;
  }

  //
  // Set Wake-up Timing
  //
  snd_pcm_sw_params_alloca(&swparams);
  snd_pcm_sw_params_current(shm->playout_pcm,swparams);
  snd_pcm_sw_params_set_avail_min(shm->playout_pcm,swparams,
				  ALSA_SAMPRATE/2);
  if((aerr=snd_pcm_sw_params(shm->playout_pcm,swparams))<0) {
    syslog(LOG_ERR,(tr("ALSA device error")+": "+snd_strerror(aerr)).toUtf8());
    snd_pcm_close(shm->playout_pcm);
    shm->playout_pcm=NULL;
    return false;
  }

  //
  // Start the Playout Process
  //
  if(fork()!=0) {
    struct LwShm *full_shm=(struct LwShm *)shmat(queue_shm_id,NULL,0);
    RunPlayout(full_shm);
    exit(0);
  }
  queue_playout_running=true;

  return true;
}


void AudioQueue::RunPlayout(LwShm *full_shm)
{
  snd_pcm_sframes_t n;
  LwShm *shm=full_shm+queue_slot;

  while(!shm->exiting) {
    n=snd_pcm_writei(shm->playout_pcm,shm->pcm+PCM_OFFSET(shm->period-2),240);
    if((n<0)&&(snd_pcm_state(shm->playout_pcm)!=SND_PCM_STATE_RUNNING)&&
       (!shm->exiting)) {   // Recover from an xrun
      snd_pcm_drop(shm->playout_pcm);
      snd_pcm_prepare(shm->playout_pcm);
    }
  }
  snd_pcm_close(shm->playout_pcm);
  shm->playout_pcm=NULL;

  exit(0);
}
