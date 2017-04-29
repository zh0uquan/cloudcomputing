/**********************************
 * FILE NAME: MP1Node.cpp
 *
 * DESCRIPTION: Membership protocol run by this Node.
 * 				Header file of MP1Node class.
 **********************************/

#ifndef _MP1NODE_H_
#define _MP1NODE_H_

#include "stdincludes.h"
#include "Log.h"
#include "Params.h"
#include "Member.h"
#include "EmulNet.h"
#include "Queue.h"

/**
 * Macros
 */
#define TPERIOD 20
#define TFAIL 3
/*
 * Note: You can change/add any functions in MP1Node.{h,cpp}
 */

/**
 * Message Types
 */
enum MsgTypes{
    JOINREQ,
    JOINREP,
    PING,
    PINGREQ,
    ACK,
    REMOVE
};

/**
 * STRUCT NAME: MessageHdr
 *
 * DESCRIPTION: Header and content of a message
 *   Address optional is used as destantion if MsgTypes is PINGREQ or ACK
 *   And it is used as remove target if MsgTypes is REMOVE
 */
typedef struct MessageHdr {
	enum MsgTypes msgType;
  Address src;
  Address opt;
  vector<MemberListEntry> memberList;
}MessageHdr;

/**
 * CLASS NAME: MP1Node
 *
 * DESCRIPTION: Class implementing Membership protocol functionalities for failure detection
 */
class MP1Node {
private:
	EmulNet *emulNet;
	Log *log;
	Params *par;
	Member *memberNode;
	char NULLADDR[6];
  //added values
  size_t pos;
  bool finished;
  Address pingTarget;

public:
	MP1Node(Member *, Params *, EmulNet *, Log *, Address *);
	Member * getMemberNode() {
		return memberNode;
	}
	int recvLoop();
	static int enqueueWrapper(void *env, char *buff, int size);
	void nodeStart(char *servaddrstr, short serverport);
	int initThisNode(Address *joinaddr);
	int introduceSelfToGroup(Address *joinAddress);
	int finishUpThisNode();
	void nodeLoop();
	void checkMessages();
	bool recvCallBack(Member *memberNode, MessageHdr *msg, int size);
	void nodeLoopOps();
	int isNullAddress(Address *addr);
	Address getJoinAddress();
	void initMemberListTable(Member *memberNode);
	void printAddress(Address *addr);
  bool recvJoinReq(MessageHdr *msg, int size);
  bool recvJoinReply(MessageHdr *msg, int size);
  bool recvPing(MessageHdr *msg, int size);
  bool recvPingReq(MessageHdr *msg, int size);
  bool recvAck(MessageHdr *msg, int size);
  bool recvRmv(MessageHdr *msg, int size);
  MessageHdr* createMessage(MsgTypes, Address *, vector<MemberListEntry> *);
  MessageHdr* createMessage(MsgTypes, Address *, Address *);
  virtual ~MP1Node();
  bool startCycleRun();
  bool startHelperRun();
  bool sendFailedNode();
  bool removeFailedNode(Address *);
  Address buildAddress(int id, short port);
};

#endif /* _MP1NODE_H_ */
