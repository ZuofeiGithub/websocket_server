#include "ClientObject.h"

ClientObject::ClientObject(SOCKET sockfd)
{
	memset(_szMsgBuf,0,sizeof(_szMsgBuf));
	_curPos = 0;
	m_clientsocket = sockfd;
}


ClientObject::~ClientObject()
{
}

SOCKET ClientObject::getClientSocket()
{
	return m_clientsocket;
}


char * ClientObject::getMsgBuf()
{
	return _szMsgBuf;
}

size_t ClientObject::getCurPos()
{
	return _curPos;
}

void ClientObject::setCurPos(size_t pos)
{
	_curPos = pos;
}

