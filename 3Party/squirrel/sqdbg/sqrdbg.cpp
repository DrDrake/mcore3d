/*
	see copyright notice in sqrdbg.h
*/
#include <winsock.h>
#include "sqrdbg.h"
#include "sqdbgserver.h"
SQInteger debug_hook(HSQUIRRELVM v);
SQInteger error_handler(HSQUIRRELVM v);

#include "serialize_state.inl"

#pragma comment(lib, "WSOCK32.LIB")

static bool beginlisten(SQDbgServer *rdbg)
{
	if(rdbg->_accept != INVALID_SOCKET)
		sqdbg_closesocket(rdbg->_accept);

	if(rdbg->_endpoint != INVALID_SOCKET) {
		sqdbg_closesocket(rdbg->_endpoint);
		rdbg->_endpoint = INVALID_SOCKET;
	}

	sockaddr_in bindaddr;
	rdbg->_accept = socket(AF_INET,SOCK_STREAM,0);
	bindaddr.sin_family = AF_INET;
	bindaddr.sin_port = htons(rdbg->_port);
	bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	unsigned long nonBlocking = 1;
	if(ioctlsocket(rdbg->_accept,FIONBIO,&nonBlocking)==SOCKET_ERROR){
		sq_throwerror(rdbg->_v,_SC("failed to set non-blocking mode"));
		return false;
	}

	if(bind(rdbg->_accept,(sockaddr*)&bindaddr,sizeof(bindaddr))==SOCKET_ERROR){
		sq_throwerror(rdbg->_v,_SC("failed to bind the socket"));
		return false;
	}

	if(listen(rdbg->_accept,0)==SOCKET_ERROR) {
		sq_throwerror(rdbg->_v,_SC("error on listen(socket)"));
		return false;
	}

	return true;
}

HSQREMOTEDBG sq_rdbg_init(HSQUIRRELVM v,unsigned short port,SQBool autoupdate)
{
	WSADATA wsadata;
#ifdef _WIN32
	if (WSAStartup (MAKEWORD(1,1), &wsadata) != 0){
		return NULL;
	}	
#endif

	SQDbgServer *rdbg = new SQDbgServer(v);
	rdbg->_autoupdate = autoupdate?true:false;
	rdbg->_port = port;

	if(!beginlisten(rdbg)) {
		delete rdbg;
		return NULL;
	}
	if(!rdbg->Init()) {
		delete rdbg;
		sq_throwerror(v,_SC("failed to initialize the debugger"));
		return NULL;
	}

	if(SQ_FAILED(sq_compilebuffer(rdbg->_v,serialize_state_nut,(SQInteger)scstrlen(serialize_state_nut),_SC("SERIALIZE_STATE"),SQFalse))) {
		delete rdbg;
		sq_throwerror(rdbg->_v,_SC("error compiling the serialization function"));
		return NULL;
	}
	sq_getstackobj(rdbg->_v,-1,&rdbg->_serializefunc);
	sq_addref(rdbg->_v,&rdbg->_serializefunc);
	sq_pop(rdbg->_v,1);

	return rdbg;
}

SQRESULT sq_rdbg_waitforconnections(HSQREMOTEDBG rdbg)
{
	while(!rdbg->_ready){
		sq_rdbg_update(rdbg);
	}
	return SQ_OK;
}

// Returns true if there exist a connected client
static bool acceptConnectionsIfNeeded(HSQREMOTEDBG rdbg)
{
	if(rdbg->_endpoint != INVALID_SOCKET)
		return true;

	sockaddr_in cliaddr;
	int addrlen=sizeof(cliaddr);
	rdbg->_endpoint = accept(rdbg->_accept,(sockaddr*)&cliaddr,&addrlen);
	if(rdbg->_endpoint==INVALID_SOCKET)
		return false;

	//do not accept any other connection
	sqdbg_closesocket(rdbg->_accept);
	rdbg->_accept = INVALID_SOCKET;

	//clear any break points and watches
	rdbg->_breakpoints.clear();
	rdbg->_watches.clear();

	rdbg->_ready = false;
	sq_rdbg_waitforconnections(rdbg);
//	rdbg->_state = SQDbgServer::eDBG_Suspended;

	return true;
}

SQRESULT sq_rdbg_update(HSQREMOTEDBG rdbg)
{
	if(!acceptConnectionsIfNeeded(rdbg))
		return SQ_OK;

	TIMEVAL time;
	time.tv_sec=0;
	time.tv_usec=0;
	fd_set read_flags;
	FD_ZERO(&read_flags);
	FD_SET(rdbg->_endpoint, &read_flags);
	select(NULL/*ignored*/, &read_flags, NULL, NULL, &time);

	if(FD_ISSET(rdbg->_endpoint,&read_flags)){
		char temp[1024];
		int size=0;
		char c,prev=NULL;
		memset(&temp,0,sizeof(temp));
		int res;
		FD_CLR(rdbg->_endpoint, &read_flags);
		while((res = recv(rdbg->_endpoint,&c,1,0))>0){
			if(c=='\n')break;
			if(c!='\r'){
				temp[size]=c;
				prev=c;
				size++;
			}
		}
		switch(res){
		case 0:
			beginlisten(rdbg);
			return sq_throwerror(rdbg->_v,_SC("disconnected"));
		case SOCKET_ERROR:
			beginlisten(rdbg);
			return sq_throwerror(rdbg->_v,_SC("socket error"));
		}
		
		temp[size]=NULL;
		temp[size+1]=NULL;
		rdbg->ParseMsg(temp);
	}
	return SQ_OK;
}

SQInteger debug_hook(HSQUIRRELVM v)
{
	SQUserPointer up;
	SQInteger event_type,line;
	const SQChar *src,*func;
	sq_getinteger(v,2,&event_type);
	sq_getstring(v,3,&src);
	sq_getinteger(v,4,&line);
	sq_getstring(v,5,&func);
	sq_getuserpointer(v,-1,&up);
	HSQREMOTEDBG rdbg = (HSQREMOTEDBG)up;
	rdbg->Hook(event_type,line,src,func);
	if(rdbg->_autoupdate) {
		if(SQ_FAILED(sq_rdbg_update(rdbg)))
			return sq_throwerror(v,_SC("socket failed"));
	}
	return 0;
}

SQInteger error_handler(HSQUIRRELVM v)
{
	SQUserPointer up;
	const SQChar *sErr=NULL;
	const SQChar *fn=_SC("unknown");
	const SQChar *src=_SC("unknown");
	SQInteger line=-1;
	SQStackInfos si;
	sq_getuserpointer(v,-1,&up);
	HSQREMOTEDBG rdbg=(HSQREMOTEDBG)up;
	if(SQ_SUCCEEDED(sq_stackinfos(v,1,&si)))
	{
		if(si.funcname)fn=si.funcname;
		if(si.source)src=si.source;
		line=si.line;
		scprintf(_SC("*FUNCTION [%s] %s line [%d]\n"),fn,src,si.line);
	}
	if(sq_gettop(v)>=1){
		if(SQ_SUCCEEDED(sq_getstring(v,2,&sErr)))	{
			scprintf(_SC("\nAN ERROR HAS OCCURED [%s]\n"),sErr);
			rdbg->Break(si.line,src,_SC("error"),sErr);
		}
		else{
			scprintf(_SC("\nAN ERROR HAS OCCURED [unknown]\n"));
			rdbg->Break(si.line,src,_SC("error"),_SC("unknown"));
		}
	}
	rdbg->BreakExecution();
	return 0;
}


SQRESULT sq_rdbg_shutdown(HSQREMOTEDBG rdbg)
{
	delete rdbg;
#ifdef _WIN32
	WSACleanup();
#endif
	return SQ_OK;
}
