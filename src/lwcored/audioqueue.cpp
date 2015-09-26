// audioqueue.cpp
//
// Audio queue for lwcore
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
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <unistd.h>

#include "audioqueue.h"

AudioQueue::AudioQueue(int shm_id,unsigned slot,QObject *parent)
  : QObject(parent)
{
  queue_slot=slot;
  queue_shm_id=shm_id;
  queue_capture_running=false;
  queue_playout_running=false;

  queue_shm=(struct LwShm *)shmat(shm_id,NULL,0);

  queue_stop_timer=new QTimer(this);
  connect(queue_stop_timer,SIGNAL(timeout()),this,SLOT(stopData()));
}


AudioQueue::~AudioQueue()
{
  delete queue_stop_timer;
}


bool AudioQueue::start(const QString &dev)
{
  if(!StartCaptureDevice(dev)) {
    return false;
  }
  if(!StartPlayoutDevice(dev)) {
    return false;
  }
  queue_device=dev;

  return true;
}


void AudioQueue::stop()
{
  queue_shm->exiting=true;
  queue_stop_timer->start(100);
}


void AudioQueue::stopData()
{
  if((queue_shm->capture_pcm==NULL)&&(queue_shm->playout_pcm==NULL)) {
    queue_stop_timer->stop();
    emit stopped(queue_slot);
  }
}


bool AudioQueue::SetAlsaParameters(snd_pcm_t *pcm,snd_pcm_hw_params_t *hwparams)
{
  //
  // Access Type
  //
  if(snd_pcm_hw_params_test_access(pcm,hwparams,
				   SND_PCM_ACCESS_RW_INTERLEAVED)<0) {
    syslog(LOG_ERR,tr("interleaved access not supported").toUtf8());
    return false;
  }
  snd_pcm_hw_params_set_access(pcm,hwparams,SND_PCM_ACCESS_RW_INTERLEAVED);

  //
  // Sample Format
  //
  if(snd_pcm_hw_params_set_format(pcm,hwparams,SND_PCM_FORMAT_S32_LE)!=0) {
    syslog(LOG_ERR,tr("unsupported sample format").toUtf8());
    return false;
  }

  //
  // Sample Rate
  //
  if(snd_pcm_hw_params_set_rate(pcm,hwparams,ALSA_SAMPRATE,0)!=0) {
    syslog(LOG_ERR,tr("unsupported sample rate").toUtf8());
    return false;
  }

  //
  // Channels
  //
  if(snd_pcm_hw_params_set_channels(pcm,hwparams,ALSA_CHANNELS)!=0) {
    syslog(LOG_ERR,tr("unsupported channel count").toUtf8());
    return false;
  }

  //
  // Buffer Parameters
  //
  if(snd_pcm_hw_params_set_periods(pcm,hwparams,ALSA_PERIOD_QUAN,0)!=0) {
    syslog(LOG_ERR,tr("unsupported period quantity").toUtf8());
    return false;
  }
  if(snd_pcm_hw_params_set_buffer_size(pcm,hwparams,240*ALSA_PERIOD_QUAN)!=0) {
    syslog(LOG_ERR,tr("unsupported buffer size").toUtf8());
    return false;
  }

  return true;
}
