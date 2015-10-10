// lwcpserver.h
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

#ifndef LWCPSERVER_H
#define LWCPSERVER_H

#include "audioqueue.h"
#include "netserver.h"

#define LWCORESERVER_PORT 4010

class LwcpServer : public NetServer
{
 Q_OBJECT;
 public:
  LwcpServer(QObject *parent=0);
  ~LwcpServer();

 public slots:
  void delaySet(unsigned slotnum,unsigned frames);
  void maxDelaySet(unsigned slotnum,unsigned frames);

 signals:
  void setDelay(unsigned slotnum,unsigned frames);
  void setMaxDelay(unsigned slotnum,unsigned frames);

 protected:
  void processCommand(int id,const AString &cmd);

 private:
  QString SlotString(unsigned slotnum) const;
  unsigned SlotValue(const QString &slot,bool *ok) const;
  QString DelayString(unsigned frames) const;
  unsigned DelayValue(const QString &str,bool *ok) const;
};


#endif  // LWCPSERVER_H
