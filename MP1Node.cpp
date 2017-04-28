/**********************************
 * FILE NAME: MP1Node.cpp
 *
 * DESCRIPTION: Membership protocol run by this Node.
 * 				Definition of MP1Node class functions.
 **********************************/

#include <typeinfo>
#include "MP1Node.h"

/*
 * Note: You can change/add any functions in MP1Node.{h,cpp}
 */

/**
 * Overloaded Constructor of the MP1Node class
 * You can add new members to the class if you think it
 * is necessary for your logic to work
 */
MP1Node::MP1Node(Member *member, Params *params, EmulNet *emul, Log *log, Address *address) {
	for( int i = 0; i < 6; i++ ) {
		NULLADDR[i] = 0;
	}
	this->memberNode = member;
	this->emulNet = emul;
	this->log = log;
	this->par = params;
	this->memberNode->addr = *address;
}

/**
 * Destructor of the MP1Node class
 */
MP1Node::~MP1Node() {}

/**
 * FUNCTION NAME: recvLoop
 *
 * DESCRIPTION: This function receives message from the network and pushes into the queue
 * 				This function is called by a node to receive messages currently waiting for it
 */
int MP1Node::recvLoop() {
    if ( memberNode->bFailed ) {
    	return false;
    }
    else {
      return emulNet->ENrecv(&(memberNode->addr), enqueueWrapper, NULL, 1, &(memberNode->mp1q));
    }
}

/**
 * FUNCTION NAME: enqueueWrapper
 *
 * DESCRIPTION: Enqueue the message from Emulnet into the queue
 */
int MP1Node::enqueueWrapper(void *env, char *buff, int size) {
	Queue q;
	return q.enqueue((queue<q_elt> *)env, (void *)buff, size);
}

/**
 * FUNCTION NAME: nodeStart
 *
 * DESCRIPTION: This function bootstraps the node
 * 				All initializations routines for a member.
 * 				Called by the application layer.
 */
void MP1Node::nodeStart(char *servaddrstr, short servport) {
    Address joinaddr;
    joinaddr = getJoinAddress();

    // Self booting routines
    if( initThisNode(&joinaddr) == -1 ) {
#ifdef DEBUGLOG
        log->LOG(&memberNode->addr, "init_thisnode failed. Exit.");
#endif
        exit(1);
    }

    if( !introduceSelfToGroup(&joinaddr) ) {
        finishUpThisNode();
#ifdef DEBUGLOG
        log->LOG(&memberNode->addr, "Unable to join self to group. Exiting.");
#endif
        exit(1);
    }

    return;
}

/**
 * FUNCTION NAME: initThisNode
 *
 * DESCRIPTION: Find out who I am and start up
 */
int MP1Node::initThisNode(Address *joinaddr) {
	/*
	 * This function is partially implemented and may require changes
	 */
	// int id = *(int*)(&memberNode->addr.addr);
	// int port = *(short*)(&memberNode->addr.addr[4]);

	memberNode->bFailed = false;
	memberNode->inited = true;
	memberNode->inGroup = false;
  // node is up!
	memberNode->nnb = 0;
	memberNode->heartbeat = 0;
	memberNode->pingCounter = TFAIL;
	memberNode->timeOutCounter = -1;

  initMemberListTable(memberNode);

  return 0;
}

/**
 * FUNCTION NAME: introduceSelfToGroup
 *
 * DESCRIPTION: Join the distributed system
 */
int MP1Node::introduceSelfToGroup(Address *joinaddr) {
	MessageHdr *msg;
#ifdef DEBUGLOG
    static char s[1024];
#endif

    if ( 0 == memcmp((char *)&(memberNode->addr.addr), (char *)&(joinaddr->addr), sizeof(memberNode->addr.addr))) {
        // I am the group booter (first process to join the group). Boot up the group
#ifdef DEBUGLOG
        log->LOG(&memberNode->addr, "Starting up group...");
#endif
        memberNode->inGroup = true;
    }
    else {
        // create JOINREQ message: format of data is {struct Address myaddr}
        size_t msgsize = sizeof(MessageHdr);
        // initialize new struct
        msg = new MessageHdr();
        msg->msgType = JOINREQ;
        msg->src = memberNode->addr;

#ifdef DEBUGLOG
        sprintf(s, "Trying to join...");
        log->LOG(&memberNode->addr, s);
#endif
        // send JOINREQ message to introducer member
        emulNet->ENsend(&memberNode->addr, joinaddr, (char *)msg, msgsize);

        free(msg);
    }

    return 1;

}

/**
 * FUNCTION NAME: finishUpThisNode
 *
 * DESCRIPTION: Wind up this node and clean up state
 */
int MP1Node::finishUpThisNode(){
   /*
    * Your code goes here
    */
    return 0;
}

/**
 * FUNCTION NAME: nodeLoop
 *
 * DESCRIPTION: Executed periodically at each member
 * 				Check your messages in queue and perform membership protocol duties
 */
void MP1Node::nodeLoop() {
    if (memberNode->bFailed) {
    	return;
    }

    // Check my messages
    checkMessages();

    // Wait until you're in the group...
    if( !memberNode->inGroup ) {
    	return;
    }

    // ...then jump in and share your responsibilites!
    nodeLoopOps();

    return;
}

/**
 * FUNCTION NAME: checkMessages
 *
 * DESCRIPTION: Check messages in the queue and call the respective message handler
 */
void MP1Node::checkMessages() {
    void *ptr;
    int size;

    // Pop waiting messages from memberNode's mp1q
    while ( !memberNode->mp1q.empty() ) {
    	ptr = memberNode->mp1q.front().elt;
    	size = memberNode->mp1q.front().size;
    	memberNode->mp1q.pop();
    	recvCallBack((Member *)memberNode, (MessageHdr *)ptr, size);
    }
    return;
}

/**
 * FUNCTION NAME: recvCallBack
 *
 * DESCRIPTION: Message handler for different message types
 */
bool MP1Node::recvCallBack(Member *memberNode, MessageHdr *msg, int size ) {
	/*
	 * Handle requests based on msgType
	 */

  switch (msg->msgType) {
    case JOINREQ:
      return recvJoinReq(msg, size);
    case JOINREP:
      return recvJoinReply(msg, size);
    default:
      break;
  }
  return false;
}

/**
 * FUNCTION NAME: recvJoinReq
 *
 * DESCRIPTION: Send a JOINREP message to the src node
 */
bool MP1Node::recvJoinReq(MessageHdr *msg, int size) {
  // Add the requested node to the current node entries
  MemberListEntry entry = MemberListEntry((int)msg->src.addr[0], (short)msg->src.addr[4]);
  memberNode->memberList.push_back(entry);

#ifdef DEBUGLOG
  log->logNodeAdd(&memberNode->addr, &msg->src);
#endif

  Address *src = &memberNode->addr;
  Address *dst = &msg->src;

  size_t msgsize = sizeof(MessageHdr);
  msg = new MessageHdr();
  msg->msgType = JOINREP;
  msg->src = memberNode->addr;
  msg->memberList = memberNode->memberList;
  // send JOINREP message to the requested node
  emulNet->ENsend(src, dst, (char *)msg, msgsize);

  free(msg);


  return true;
}

bool MP1Node::recvJoinReply(MessageHdr *msg, int size) {
  Address joinaddr;
  joinaddr = getJoinAddress();

  // Only set in Group when first time
  if ((int) msg->src.addr[0] == (int) joinaddr.addr[0]) {
    memberNode->inGroup = true;

#ifdef DEBUGLOG
  log->LOG(&memberNode->addr, "Now in the group...");
#endif

  }
  
#ifdef DEBUGLOG
  log->logNodeAdd(&memberNode->addr, &msg->src);
#endif

  int thisId = *(int*)(&memberNode->addr.addr);
  bool exist;
  //Add every node in memberList
  for (auto remote: msg->memberList) {
    exist = false;

    // ignore the remote node if it is current node
    if (thisId == remote.id) {
      continue;
    }

    // ignore the remote node if it is already in memberList
    for (auto local: memberNode->memberList) {
      if (local.id == remote.id)  {
        exist = true;
        break;
      }
    }

    // add the remote node to the memberList
    if (!exist) {
      MemberListEntry entry = MemberListEntry(remote);
      memberNode->memberList.push_back(entry);

      Address dst;
      dst.init();
      dst.addr[0] = (char) remote.id;
      dst.addr[4] = (char) remote.port;

      // send JOINREP message to the remote node
      size_t msgsize = sizeof(MessageHdr);
      msg = new MessageHdr();
      msg->msgType = JOINREQ;
      msg->src = memberNode->addr;
      msg->memberList = memberNode->memberList;
      emulNet->ENsend(&memberNode->addr, &dst, (char *)msg, msgsize);

      free(msg);
    }
  }

  return true;
}


/**
 * FUNCTION NAME: nodeLoopOps
 *
 * DESCRIPTION: Check if any node hasn't responded within a timeout period and then delete
 * 				the nodes
 * 				Propagate your membership list
 */
void MP1Node::nodeLoopOps() {

	/*
	 * Your code goes here
	 */

    return;
}

/**
 * FUNCTION NAME: isNullAddress
 *
 * DESCRIPTION: Function checks if the address is NULL
 */
int MP1Node::isNullAddress(Address *addr) {
	return (memcmp(addr->addr, NULLADDR, 6) == 0 ? 1 : 0);
}

/**
 * FUNCTION NAME: getJoinAddress
 *
 * DESCRIPTION: Returns the Address of the coordinator
 */
Address MP1Node::getJoinAddress() {
    Address joinaddr;

    memset(&joinaddr, 0, sizeof(Address));
    *(int *)(&joinaddr.addr) = 1;
    *(short *)(&joinaddr.addr[4]) = 0;

    return joinaddr;
}

/**
 * FUNCTION NAME: initMemberListTable
 *
 * DESCRIPTION: Initialize the membership list
 */
void MP1Node::initMemberListTable(Member *memberNode) {
	memberNode->memberList.clear();
}

/**
 * FUNCTION NAME: printAddress
 *
 * DESCRIPTION: Print the Address
 */
void MP1Node::printAddress(Address *addr)
{
    printf("%d.%d.%d.%d:%d \n",  addr->addr[0],addr->addr[1],addr->addr[2],
                                                       addr->addr[3], *(short*)&addr->addr[4]) ;
}
