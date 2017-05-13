// calculating finger tables for Chord-like distributed hash table.

// the size of the ring, as 2^m;
var m=9, ringSize = Math.pow(2,m);

// the nodes we know about
var nodes = [1, 12, 123, 234, 345, 456, 501];

// just make sure it's sorted
nodes = nodes.sort( (a,b) => a-b );

console.log('the finger tables for a ', ringSize,'-virtual-node system');
console.log('========');
console.log('nodes: ', nodes);

// calculate the finger table for each node
nodes.forEach(n => {

 var fingerTable = [];
 i = 1;
 for(var x = 0; x < m; x++) {
 var nodeId = (n + i) % ringSize;
 // find the node after nodeId;
 var ftEntry = nodes.filter(x => x > nodeId).concat(nodes)[0];
 fingerTable.push(ftEntry);
 i = i << 1;
 }

 console.log(n + ':', fingerTable);
});
