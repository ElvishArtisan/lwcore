// lwcored.h
//
// lwcored(8) facility router daemon
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

#ifndef LWCORED_H
#define LWCORED_H

#include <vector>

#include <QObject>
#include <QTimer>

#include "audioqueue.h"
#include "lwcpserver.h"

#define LWCORED_USAGE "[options]\n"

class MainObject : public QObject
{
 Q_OBJECT;
 public:
  MainObject(QObject *parent=0);

 private slots:
  void lengthChangedData(unsigned slot,unsigned frames);
  void maxLengthChangedData(unsigned slot,unsigned frames);
  void stopTimerData();
  void stoppedData(unsigned slot);

 private:
  bool InitShmSegment();
  std::vector<AudioQueue *> main_queues;
  LwcpServer *main_lwcp_server;
  QTimer *main_exit_timer;
  int main_stopped_queues;
  LwShm *main_shm;
  int main_shm_id;
};


#endif  // LWCORED_H
