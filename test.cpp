#include <iostream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

using namespace std;

typedef struct msg{
  int size;
  char a[8];
  char b[8];
  char c[8];
}msg;

enum MsgTypes{
    JOINREQ,
    JOINREP,
    DUMMYLASTMSGTYPE
};

/**
 * STRUCT NAME: MessageHdr
 *
 * DESCRIPTION: Header and content of a message
 */
typedef struct MessageHdr {
	enum MsgTypes msgType;
}MessageHdr;


class Box {
public:
  int length;
  int matrix[5][9];
  Box(): length(9) {};
  virtual ~Box() {};
  int test(char *a, char *b, char *data, int size) {
    msg *e;
    e = (msg *)malloc(sizeof (msg) + size);
    e->size = size;
    memcpy(e->a, a, sizeof(e->a));
  	memcpy(e->b, b, sizeof(e->b));
  	memcpy(e + 1, data, size);
    cout << (char)(e->a[0]) << (char)(e->b[1]) << char(e->c[1]) << endl;
    return 0;
  }
};

int main() {
  // class obj test
  Box *box = new Box();
  box->length = 99;
  cout << box->length << endl;
  // obj function test
  char a[8] = {'1', '2'};
  char b[8] = {'3', '4'};
  char c[8] = {'5', '6'};
  box->test(a, b, c, 1);
  // memcpy test
  char buffer1[] = "abc";
  char buffer2[] = "abc1";
  cout << memcmp(buffer1, buffer2, sizeof(buffer1)) << endl;
  //test



  MessageHdr *msg;

  size_t msgsize = sizeof(MessageHdr) + sizeof(buffer1) + 1;
  msg = (MessageHdr *) malloc(msgsize * sizeof(char));

  // create JOINREQ message: format of data is {struct Address myaddr}
  msg->msgType = JOINREQ;
  cout << msg->msgType << endl;
  cout << (char *) (msg+1) << endl;

  return 0;
}
