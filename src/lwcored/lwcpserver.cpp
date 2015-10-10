// lwcpserver.cpp
//
// LWCP Server for lwcore.
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

#include <stdio.h>

#include <QStringList>

#include "lwcore.h"
#include "lwcpserver.h"

LwcpServer::LwcpServer(QObject *parent)
  : NetServer(LWCORESERVER_PORT,parent)
{
}


LwcpServer::~LwcpServer()
{
}


void LwcpServer::delaySet(unsigned slotnum,unsigned frames)
{
  send("EVENT "+SlotString(slotnum)+" Delay="+DelayString(frames));
}


void LwcpServer::maxDelaySet(unsigned slotnum,unsigned frames)
{
  send("EVENT "+SlotString(slotnum)+" Max_Delay="+DelayString(frames));
}


void LwcpServer::processCommand(int id,const AString &cmd)
{
  printf("processCommand(%d,%s)\n",id,(const char *)cmd.toUtf8());

  bool ok=false;
  QStringList f0=cmd.split(" ","\"");
  unsigned slot;
  unsigned frames;
  
  if(f0[0].toLower()=="set") {
    if(cmd.size()>=3) {
      slot=SlotValue(f0[1],&ok);
      if(ok&&(slot<LWCORE_SLOT_QUAN)) {
	for(int i=2;i<f0.size();i++) {
	  QStringList f1=((AString)f0[i]).split("=","\"");
	  if(f1.size()==2) {
	    if(f1[0].toLower()=="delay") {
	      frames=DelayValue(f1[1],&ok);
	      if(ok) {
		emit setDelay(slot,frames);
	      }
	    }
	  }
	  if(f1.size()==2) {
	    if(f1[0].toLower()=="max_delay") {
	      frames=DelayValue(f1[1],&ok);
	      if(ok) {
		emit setMaxDelay(slot,frames);
	      }
	    }
	  }
	}
      }
    }
  }
}


QString LwcpServer::SlotString(unsigned slotnum) const
{
  return QString().sprintf("Slot#%u",((slotnum)+1));
}


unsigned LwcpServer::SlotValue(const QString &slot,bool *ok) const
{
  unsigned ret=0;
  QStringList f0=slot.split("#");
  
  *ok=false;
  if(f0.size()==2) {
    if(f0[0].toLower()=="slot") {
      ret=f0[1].toUInt(ok)-1;
    }
  }
  return ret;
}


QString LwcpServer::DelayString(unsigned frames) const
{
  return QString().sprintf("%5.1lf",(double)(frames)/(double)LWCORE_SAMPRATE);
}


unsigned LwcpServer::DelayValue(const QString &str, bool *ok) const
{
  unsigned ret=0;

  *ok=false;
  double delay=str.toDouble(ok);
  if(ok&&(delay>=0.0)&&(delay<=LWCORE_MAX_DELAY)) {
    ret=delay*(double)LWCORE_SAMPRATE;
  }
  else {
    ok=false;
  }

  return ret;
}
