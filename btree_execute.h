#ifndef BTREE_EXECUTE_H
#define BTREE_EXECUTE_H
#ifndef NULL
#define NULL (0)
#endif

#include "btree_mgr.h"
#include "buffer_mgr.h"

typedef struct Data {
    RID recId;
} Data;

typedef struct Nodes {
    void ** pointers;
    Value ** keys;
    struct Nodes * parent;
    bool isLeafNode;
    int numOfSearchKeys;
    struct Nodes * next; } Nodes;

typedef struct BTreeMgr {
    BM_BufferPool bPool;
    BM_PageHandle pageHandle;
    int orderOfTree;
    int numOfNodes;
    int numOfEntries;
    Nodes * rootNode;
    Nodes * queue;
    DataType keyType;
} BTreeMgr;

typedef struct ScanMgr {
    int searchKeyIndex;
    int totalNoKeys;
    int orderOfTree;
    Nodes * nodes;
} ScanMgr;

Nodes * getLeafNode(Nodes * rootNode, Value * key);
Data * identifyRecord(Nodes * rootNode, Value * key);

void enqueue(BTreeMgr * bTreeMgr, Nodes * new_Nodes);
Nodes * dequeue(BTreeMgr * bTreeMgr);
int pathRootNode(Nodes * rootNode, Nodes * child);

Nodes * createNodes(BTreeMgr * bTreeMgr);

Nodes * insertIntoLeafNode(BTreeMgr * bTreeMgr, Nodes * leaf, Value * key, Data * pointer);
Nodes * NewTree(BTreeMgr * bTreeMgr, Value * key, Data * pointer);
Nodes * insertIntoNodes(BTreeMgr * bTreeMgr, Nodes * parent, int leftIndex, Value * key, Nodes * right);

Nodes * insertIntoLeafNodeAfterSplit(BTreeMgr * bTreeMgr, Nodes * leaf, Value * key, Data * pointer);
Nodes * insertIntoNodesAfterSplit(BTreeMgr * bTreeMgr, Nodes * parent, int leftIndex, Value * key, Nodes * right);
Nodes * insertIntoParentNode(BTreeMgr * bTreeMgr, Nodes * left, Value * key, Nodes * right);
Nodes * insertIntoNewRootNode(BTreeMgr * bTreeMgr, Nodes * left, Value * key, Nodes * right);
Nodes * createLeaf(BTreeMgr * bTreeMgr);

int LeftIndex(Nodes * parent, Nodes * left);
Data * buildRecord(RID * recId);


Nodes * adjustRootNode(Nodes * rootNode);
Nodes * mergeNodes(BTreeMgr * bTreeMgr, Nodes * node, Nodes * neighbor, int neighborIndex, int prime);
Nodes * deleteTheKey(BTreeMgr * bTreeMgr, Value * key);
Nodes * redistributeNodes(Nodes * rootNode, Nodes * node, Nodes * neighbor, int neighborIndex, int primeIndex, int prime);
Nodes * deleteEntry(BTreeMgr * bTreeMgr, Nodes * node, Value * key, void * pointer);
Nodes * removeEntry(BTreeMgr * bTreeMgr, Nodes * node, Value * key, Nodes * pointer);
int getNeighbor(Nodes * node);

bool isLess(Value * key1, Value * key2);
bool isGreater(Value * key1, Value * key2);
bool isEqual(Value * key1, Value * key2);

#endif 
