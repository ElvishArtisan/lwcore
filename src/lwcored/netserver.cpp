// netserver.cpp
//
// LWCP server for lwcore
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

#include "netserver.h"

NetConnection::NetConnection(QTcpSocket *sock)
{
  socket=sock;
  buffer="";
  zombie=false;
}


NetConnection::~NetConnection()
{
  delete socket;
}



NetServer::NetServer(uint16_t port,QObject *parent)
  : QObject(parent)
{
  //
  // TCP Server
  //
  net_tcp_server=new QTcpServer(this);
  connect(net_tcp_server,SIGNAL(newConnection()),
	  this,SLOT(newConnectionData()));
  if(!net_tcp_server->listen(QHostAddress::Any,port)) {
    syslog(LOG_WARNING,"unable to bind LWCP port %u",port);
  }

  //
  // Connection Mappers
  //
  net_tcp_ready_mapper=new QSignalMapper(this);
  connect(net_tcp_ready_mapper,SIGNAL(mapped(int)),
	  this,SLOT(readyReadData(int)));

  net_tcp_discon_mapper=new QSignalMapper(this);
  connect(net_tcp_discon_mapper,SIGNAL(mapped(int)),
	  this,SLOT(disconnectedData(int)));

  //
  // Garbage Collection Timer
  //
  net_garbage_timer=new QTimer(this);
  net_garbage_timer->setSingleShot(true);
  connect(net_garbage_timer,SIGNAL(timeout()),this,SLOT(garbageData()));
}


NetServer::~NetServer()
{
  for(unsigned i=0;i<net_connections.size();i++) {
    delete net_connections[i];
  }
  delete net_tcp_server;
}


void NetServer::send(const QString &cmd,int id)
{
  if(id<0) {
    for(unsigned i=0;i<net_connections.size();i++) {
      send(cmd,i);
    }
  }
  else {
    if(net_connections[id]!=NULL) {
      net_connections[id]->socket->write((cmd+"\r\n").toUtf8(),cmd.length()+2);
    }
  }
}


void NetServer::newConnectionData()
{
  int id=NewConnectionId();
  net_connections[id]=
    new NetConnection(net_tcp_server->nextPendingConnection());
  net_tcp_ready_mapper->
    setMapping(net_connections[id]->socket,id);
  connect(net_connections[id]->socket,SIGNAL(readyRead()),
	  net_tcp_ready_mapper,SLOT(map()));
  net_tcp_discon_mapper->
    setMapping(net_connections[id]->socket,id);
  connect(net_connections[id]->socket,SIGNAL(disconnected()),
	  net_tcp_discon_mapper,SLOT(map()));
}


void NetServer::readyReadData(int id)
{
  char data[1500];
  int n;
  NetConnection *conn=net_connections[id];

  while((n=conn->socket->read(data,1500))>0) {
    for(int i=0;i<n;i++) {
      switch(0xFF&data[i]) {
      case 10:
	processCommand(id,conn->buffer);
	conn->buffer="";
	break;

      case 13:
	break;

      default:
	conn->buffer+=data[i];
	break;
      }
    }
  }
}


void NetServer::disconnectedData(int id)
{
  net_connections[id]->zombie=true;
  net_garbage_timer->start(0);
}


void NetServer::garbageData()
{
  for(unsigned i=0;i<net_connections.size();i++) {
    if(net_connections[i]!=NULL) {
      if(net_connections[i]->zombie) {
	delete net_connections[i];
	net_connections[i]=NULL;
      }
    }
  }
}


unsigned NetServer::NewConnectionId()
{
  for(unsigned i=0;i<net_connections.size();i++) {
    if(net_connections[i]==NULL) {
      return i;
    }
  }
  net_connections.push_back(NULL);
  return net_connections.size()-1;
}
