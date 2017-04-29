/**********************************
 * FILE NAME: MP1Node.cpp
 *
 * DESCRIPTION: Membership protocol run by this Node.
 * 				Definition of MP1Node class functions.
 **********************************/

#include <typeinfo>
#include "MP1Node.h"


/**
 * Fisher–Yates shuffle Algorithm, random shuffle taken from github
 * It is used to shuffle the vector in order to use round-robin ring
 * https://gist.github.com/sundeepblue/10501662
 */

void shuffle(vector<MemberListEntry> &a) {
    int N = a.size();
    for(int i=N-1; i>0; --i) {  // gist, note, i>0 not i>=0
        int r = rand() % (i+1); // gist, note, i+1 not i. "rand() % (i+1)" means
                                // generate rand numbers from 0 to i
        swap(a[i], a[r]);
    }
}


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
  this->pos = 0;
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
	// memberNode->nnb = 0;
	// memberNode->heartbeat = 0;
	// memberNode->pingCounter = TFAIL;
	memberNode->timeOutCounter = TPERIOD;

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
    case PING:
      return recvPing(msg, size);
    case ACK:
      return recvAck(msg, size);
    case PINGREQ:
      return recvPingReq(msg, size);
    default:
      break;
  }
  return false;
}

/**
 * FUNCTION NAME: recvJoinReq
 *
 * DESCRIPTION: Send a JOINREP message to the src node
 *              Propagate the membership list
 */
bool MP1Node::recvJoinReq(MessageHdr *msg, int size) {
  // Add the requested node to the current node entries
  MemberListEntry entry = MemberListEntry((int)msg->src.addr[0], (short)msg->src.addr[4]);
  memberNode->memberList.push_back(entry);

#ifdef DEBUGLOG
  log->logNodeAdd(&memberNode->addr, &msg->src);
#endif

  Address *dst = &msg->src;
  // send JOINREP message to the requested node
  size_t msgsize = sizeof(MessageHdr);
  msg = createMessage(JOINREP, &memberNode->addr, &memberNode->memberList);
  emulNet->ENsend(&memberNode->addr, dst, (char *)msg, msgsize);

  free(msg);

  return true;
}

bool MP1Node::recvJoinReply(MessageHdr *msg, int size) {
  Address joinaddr;
  joinaddr = getJoinAddress();

  // Add the joined node to the current node entries
  MemberListEntry entry = MemberListEntry((int)msg->src.addr[0], (short)msg->src.addr[4]);
  memberNode->memberList.push_back(entry);

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
      dst = buildAddress(remote.id, remote.port);

      // send JOINREQ message to the remote node
      msg = createMessage(JOINREQ, &memberNode->addr, &memberNode->memberList);
      emulNet->ENsend(&memberNode->addr, &dst, (char *)msg, size);

      free(msg);
    }
  }

  return true;
}

/**
 * FUNCTION NAME: recvPing
 *
 * DESCRIPTION: Received an ping and should send an ACK to msg source
 *
 */
bool MP1Node::recvPing(MessageHdr *msg, int size) {
  msg = createMessage(ACK, &memberNode->addr, &msg->opt);
  emulNet->ENsend(&memberNode->addr, &msg->src, (char *)msg, size);

  free(msg);

  return true;
}


/**
 * FUNCTION NAME: recvPingReq
 *
 * DESCRIPTION: This is func only triggered when those helper Received PINGREQ
 *              Received an recvPingReq and should send an ping to dst in order to get an ACK
 */
bool MP1Node::recvPingReq(MessageHdr *msg, int size) {
  // cout << "current node: " << (int) memberNode->addr.addr[0]<<endl;
  // printAddress(&pingTarget);
  // cout << "TIME: " << par->getcurrtime()<< endl;
  // message foramt (ping, src address, dst address)
  msg = createMessage(PING, &memberNode->addr, &msg->opt);
  emulNet->ENsend(&memberNode->addr, &msg->src, (char *)msg, size);

  free(msg);

  return true;
}


/**
 * FUNCTION NAME: recvAck
 *
 * DESCRIPTION: Received an ACK and check its destantion is the current node
 *     if not, then send an ACK to the requested node
 *
 */
bool MP1Node::recvAck(MessageHdr *msg, int size) {
  if (memberNode->addr.getAddress() == msg->opt.getAddress()) {
    this->finished = true;
  } else {
    msg = createMessage(ACK, &memberNode->addr, &msg->opt);
    emulNet->ENsend(&memberNode->addr, &msg->src, (char *)msg, size);

    free(msg);
  }
  return true;
}

/**
 * FUNCTION NAME: startCycleRun
 *
 * DESCRIPTION: Start one cycle run during time period
 *     if not, then send an ACK to the requested node
 *
 */
bool MP1Node::startCycleRun() {
  MessageHdr *msg;
  size_t msgsize = sizeof(MessageHdr);

  this->pingTarget = buildAddress(memberNode->memberList[pos].getid(), memberNode->memberList[pos].getport());
  this->finished = false;

  // memberList and dst is not needed becasuse we just wanna an ACK from the
  msg = createMessage(PING, &memberNode->addr, &pingTarget);
  emulNet->ENsend(&memberNode->addr, &pingTarget, (char *)msg, msgsize);
  free(msg);

  pos++;

  return true;
}

/**
 * FUNCTION NAME: startCycleRun
 *
 * DESCRIPTION: Ask the helper to handle the communication when pingTarget is not responding
 *
 */
bool MP1Node::startHelperRun() {
  MessageHdr *msg;
  size_t msgsize = sizeof(MessageHdr);

  Address helper = buildAddress(memberNode->memberList[pos].getid(), memberNode->memberList[pos].getport());
  msg = createMessage(PINGREQ, &memberNode->addr, &pingTarget);
  emulNet->ENsend(&memberNode->addr, &helper, (char *)msg, msgsize);

  free(msg);

  pos++;

  return true;
}


/**
 * FUNCTION NAME: nodeLoopOps
 *
 * DESCRIPTION: Check if any node hasn't responded within a timeout period and then delete
 * 				the nodes
 *        SWIM protocol is used here and robin cycle is used as protocol improvement
 */
void MP1Node::nodeLoopOps() {
  // we skip nodes those who have no neighours to chose to dectect failures
  if (memberNode->memberList.empty()){
    return;
  }

  shuffle(memberNode->memberList);

  if (pos == memberNode->memberList.size()) {
    pos = 0;
  }



  if (memberNode->timeOutCounter == 0) {
    memberNode->timeOutCounter = TPERIOD;
    if (!finished) {
      //remove the pingTarget & Propagate its dead info

    }
    return;
  }

  if (memberNode->timeOutCounter == TPERIOD) {
    if (!startCycleRun()) {
      #ifdef DEBUGLOG
        log->LOG(&memberNode->addr, "startCycleRun failed! Wierd!");
      #endif
    };
  }

  if ((memberNode->timeOutCounter == TPERIOD - TPERIOD / SUBGROUPNUM ) || (memberNode->timeOutCounter == TPERIOD - TPERIOD / 2 * SUBGROUPNUM)){
    if (!finished) {
      if (!startHelperRun()) {
        #ifdef DEBUGLOG
          log->LOG(&memberNode->addr, "startHelperRun failed! Wierd!");
        #endif
      }
    }
  }

  memberNode->timeOutCounter--;
  return;
}

/**
 * FUNCTION NAME: createMessage
 *
 * DESCRIPTION: Create a MessageHdr
 *
 */
MessageHdr* MP1Node::createMessage(MsgTypes msgType, Address *src, vector<MemberListEntry> *lst) {
  MessageHdr *msg = new MessageHdr();
  msg->msgType = msgType;
  msg->src = *src;
  msg->memberList = *lst;
  return msg;
}
/**
 * FUNCTION NAME: createMessage
 *
 * DESCRIPTION: Create a MessageHdr
 *
 */
MessageHdr* MP1Node::createMessage(MsgTypes msgType, Address *src, Address *opt) {
  MessageHdr *msg = new MessageHdr();
  msg->msgType = msgType;
  msg->src = *src;
  msg->opt = *opt;
  return msg;
}

/**
 * FUNCTION NAME: buildAddress
 *
 * DESCRIPTION: Build an Address using id and port
 *
 */

Address MP1Node::buildAddress(int id, short port) {
  Address addr;
  addr.init();
  addr.addr[0] = (char) id;
  addr.addr[4] = (char) port;
  return addr;
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
