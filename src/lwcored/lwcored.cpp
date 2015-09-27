// lwcored.cpp
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

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <QCoreApplication>

#include "lwcored.h"

bool exiting=false;

void SigHandler(int signo)
{
  switch(signo) {
  case SIGINT:
  case SIGTERM:
    exiting=true;
    break;

  case SIGCHLD:
    waitpid(-1,NULL,WNOHANG);
    break;
  }
}


MainObject::MainObject(QObject *parent)
  : QObject(parent)
{
  //
  // Create Shared Memory Segment
  //
  main_shm=NULL;
  main_shm_id=-1;
  if(!InitShmSegment()) {
    fprintf(stderr,"lwcored: unable to create shared memory segment\n");
    exit(256);
  }

  //
  // Start Queues
  //
  for(int i=0;i<LWCORED_SLOT_QUAN;i++) {
    main_queues.push_back(new AudioQueue(main_shm_id,i,this));
    connect(main_queues.back(),SIGNAL(lengthChanged(unsigned,unsigned)),
	    this,SLOT(lengthChangedData(unsigned,unsigned)));
    connect(main_queues.back(),SIGNAL(stopped(unsigned)),
	    this,SLOT(stoppedData(unsigned)));
    QString dev=QString().sprintf("hw:Axia,%d",i);
    if(!main_queues.back()->start(dev)) {
	fprintf(stderr,"lwcored: unable to start device %s\n",
		(const char *)dev.toUtf8());
      exit(256);
    }
  }

  //
  // LWCP Server
  //
  main_lwcp_server=new LwcpServer(this);

  //
  // Exit Timer
  //
  main_stopped_queues=0;
  main_exit_timer=new QTimer(this);
  connect(main_exit_timer,SIGNAL(timeout()),this,SLOT(stopTimerData()));
  main_exit_timer->start(1000);
  /*
  main_queues[0]->setMaxTempoOffset(0.1);
  main_queues[0]->setMaxLength(480000);
  */
}


void MainObject::lengthChangedData(unsigned slot,unsigned frames)
{
  //  printf("lengthChangedData(%u,%u)\n",slot,frames);
}


void MainObject::stopTimerData()
{
  if(exiting) {
    for(int i=0;i<LWCORED_SLOT_QUAN;i++) {
      main_queues[i]->stop();
    }
  }
}


void MainObject::stoppedData(unsigned slot)
{
  if(++main_stopped_queues==LWCORED_SLOT_QUAN) {
    shmctl(main_shm_id,IPC_RMID,NULL);
    exit(0);
  }
}


bool MainObject::InitShmSegment()
{
  struct shmid_ds shmid_ds;

  /*
   * First try to create a new shared memory segment.
   */
  if((main_shm_id=
      shmget(IPC_PRIVATE,sizeof(struct LwShm)*LWCORED_SLOT_QUAN,
	     IPC_CREAT|IPC_EXCL|S_IRUSR|S_IWUSR|
	     S_IRGRP|S_IWUSR|S_IROTH|S_IWOTH))<0) {
    if(errno!=EEXIST) {
      return false;
    }
    /*
     * The shmget() error was due to an existing segment, so try to get it,
     *  release it, and re-get it.
     */
    main_shm_id=shmget(IPC_PRIVATE,sizeof(struct LwShm)*LWCORED_SLOT_QUAN,0);
    shmctl(main_shm_id,IPC_RMID,NULL);
    if((main_shm_id=shmget(IPC_PRIVATE,sizeof(struct LwShm)*LWCORED_SLOT_QUAN,
			   IPC_CREAT|IPC_EXCL|S_IRUSR|S_IWUSR|
			   S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH))<0) {
      return false;
    }
  }
  shmctl(main_shm_id,IPC_STAT,&shmid_ds);
  shmid_ds.shm_perm.uid=getuid();
  shmctl(main_shm_id,IPC_SET,&shmid_ds);
  main_shm=(struct LwShm *)shmat(main_shm_id,NULL,0);
  memset(main_shm,0,sizeof(struct LwShm)*LWCORED_SLOT_QUAN);

  return true;
}


int main(int argc,char *argv[])
{
  QCoreApplication a(argc,argv);
  new MainObject();
  return a.exec();
}
