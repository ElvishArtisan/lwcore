// audioqueue.h
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

#ifndef AUDIOQUEUE_H
#define AUDIOQUEUE_H

#include <sys/ipc.h>
#include <sys/shm.h>

#include <QObject>
#include <QString>
#include <QTimer>

#include <alsa/asoundlib.h>

#define QUEUE_CHANNELS 2
#define QUEUE_PERIOD_SIZE 960
#define QUEUE_PERIOD_QUAN 2
#define QUEUE_SHM_KEY_BASE_KEY 
#define ALSA_CHANNELS 2
#define ALSA_SAMPRATE 48000
#define ALSA_FORMAT S32_LE
#define ALSA_PERIOD_QUAN 4

#define PCM_OFFSET(x) ((x)%QUEUE_PERIOD_QUAN)*QUEUE_PERIOD_SIZE*QUEUE_CHANNELS

struct LwShm {
  snd_pcm_t *capture_pcm;
  snd_pcm_t *playout_pcm;
  int pcm[QUEUE_CHANNELS*QUEUE_PERIOD_SIZE*QUEUE_PERIOD_QUAN];
  unsigned period;
  bool exiting;
};

class AudioQueue : public QObject
{
 Q_OBJECT;
 public:
  AudioQueue(int shm_id,unsigned slot,QObject *parent=0);
  ~AudioQueue();
  bool start(const QString &dev);
  void stop();

 signals:
  void stopped(unsigned slot);

 private slots:
  void stopData();

 private:
  bool StartCaptureDevice(const QString &dev);
  void RunCapture(LwShm *shm);
  bool StartPlayoutDevice(const QString &dev);
  void RunPlayout(LwShm *full_shm);
  bool SetAlsaParameters(snd_pcm_t *pcm,snd_pcm_hw_params_t *hwparams);
  unsigned queue_slot;
  struct LwShm *queue_shm;
  int queue_shm_id;
  QString queue_device;
  bool queue_capture_running;
  bool queue_playout_running;
  QTimer *queue_stop_timer;
};


#endif  // LWCORED_H
