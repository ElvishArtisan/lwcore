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

#include "lwcpserver.h"

LwcpServer::LwcpServer(QObject *parent)
  : NetServer(LWCORESERVER_PORT,parent)
{
}


LwcpServer::~LwcpServer()
{
}


void LwcpServer::processCommand(int id,const QString &cmd)
{
  printf("processCommand(%d,%s)\n",id,(const char *)cmd.toUtf8());
}
