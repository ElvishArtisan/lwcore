// netserver.h
//
// Base class for a telnet-style server.
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

#ifndef NETSERVER_H
#define NETSERVER_H

#include <stdint.h>

#include <vector>

#include <QObject>
#include <QSignalMapper>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>

#include "astring.h"

class NetConnection
{
 public:
  NetConnection(QTcpSocket *sock);
  ~NetConnection();
  QTcpSocket *socket;
  QString buffer;
  bool zombie;
};


class NetServer : public QObject
{
 Q_OBJECT;
 public:
  NetServer(uint16_t port,QObject *parent=0);
  ~NetServer();

 public slots:
  void send(const QString &cmd,int id=-1);

 protected:
  virtual void processCommand(int id,const AString &cmd)=0;

 private slots:
  void newConnectionData();
  void readyReadData(int id);
  void disconnectedData(int id);
  void garbageData();

 private:
  unsigned NewConnectionId();
  std::vector<NetConnection *> net_connections;
  QTcpServer *net_tcp_server;
  QSignalMapper *net_tcp_ready_mapper;
  QSignalMapper *net_tcp_discon_mapper;
  QTimer *net_garbage_timer;
};


#endif  // NETSERVER_H
