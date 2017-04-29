#include <iostream>
#include <vector>
#include <stdlib.h>

using namespace std;

void func() {
  vector<int> array;
  array.push_back(6);
  array.push_back(9090);

  for (vector<int>::iterator it=array.begin();it!=array.end(); it++ ) {
    cout << *it << endl;
  }
}

int main(int argc, char const *argv[]) {
  func();

  return 0;
}

log->LOG(&pingTarget, "Node removed in Node %d.0.0.0:0 memberList", memberNode->addr.addr[0]);
log->LOG(&pingTarget, "Node failed at %d", par->getcurrtime());



// check if the iterator pointer reaches the end
// if (pos == (int) memberNode->memberList.size()) {
//   // set the iterator index to 0 and shuffle the vector
//   pos = 0;
//   shuffle(memberNode->memberList);
// }
//
// // for (memberNode->myPos=memberNode->memberList.begin(); memberNode->myPos != memberNode->memberList.end(); ++memberNode->myPos) {
// //   cout << "current node: " << (int) memberNode->addr.addr[0];
// //   cout << " loop over node "<< memberNode->myPos->id << endl;
// //
// // }
//
// if (memberNode->timeOutCounter == TPERIOD) {
//   MessageHdr *msg;
//
//   pingTarget = buildAddress(memberNode->memberList[pos].getid(), memberNode->memberList[pos].getport());
//
//   size_t msgsize = sizeof(MessageHdr);
//   // memberList and dst is not needed becasuse we just wanna an ACK from the
//   msg = createMessage(PING, &memberNode->addr, NULL, NULL);
//
//
//   emulNet->ENsend(&memberNode->addr, &pingTarget, (char *)msg, msgsize);
//
//   free(msg);
//   // pos++;
// }

//
// cout << " after shuffle "<< endl;
//
// for (memberNode->myPos=memberNode->memberList.begin(); memberNode->myPos != memberNode->memberList.end(); ++memberNode->myPos) {
//   cout << "current node: " << (int) memberNode->addr.addr[0];
//   cout << " loop over node "<< memberNode->myPos->id << endl;
//
// }
//
// cout << "current node: " << (int) memberNode->addr.addr[0];
// cout << " loop over node " << memberNode->memberList[pos].getid() << endl;
// pos++;
// return;
