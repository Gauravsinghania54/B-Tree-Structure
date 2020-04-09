#include "btree_execute.h"
#include "dt.h"
#include "string.h"
#include <stdlib.h>


Data *  buildRecord(RID * recid) {
    Data * rec = (Data *) malloc(sizeof(Data));
    if (rec == NULL) {
        perror("Data creation.");
        exit(RC_INSERT_ERROR);
    } else {
                        rec->recId.page = recid->page;
        rec->recId.slot = recid->slot;
    }
        return rec;
}

Nodes * NewTree(BTreeMgr * bTreeMgr, Value * key, Data * pointer) {

    Nodes * rootNode = createLeaf(bTreeMgr);
    int bTreeOrder = bTreeMgr->orderOfTree;
    
    rootNode->keys[0] = key;
    rootNode->pointers[0] = pointer;
    rootNode->pointers[bTreeOrder - 1] = NULL;
    rootNode->numOfSearchKeys++;
    rootNode->parent = NULL;

    bTreeMgr->numOfEntries++;
    
    return rootNode;
}

Nodes * insertIntoLeafNode(BTreeMgr * bTreeMgr, Nodes * leaf, Value * key, Data * pointer) {
    
    int i, insertPoint;
    bTreeMgr->numOfEntries++;

    insertPoint = 0;
    while (insertPoint < leaf->numOfSearchKeys && isLess(leaf->keys[insertPoint], key))
        insertPoint++;

    for (i = leaf->numOfSearchKeys; i > insertPoint; i--) {
        leaf->keys[i] = leaf->keys[i - 1];
        leaf->pointers[i] = leaf->pointers[i - 1];
    }
    leaf->keys[insertPoint] = key;
    leaf->numOfSearchKeys++;
    leaf->pointers[insertPoint] = pointer;
    return leaf;
}

Nodes * insertIntoLeafNodeAfterSplit(BTreeMgr * bTreeMgr, Nodes * leaf, Value * key, Data * pointer) {


    Nodes * newLeafNode;
    Value ** tempSearchKeys;
    void ** tempPtr;
    int insertIndex, split, newSearchKey, i, j;

    newLeafNode = createLeaf(bTreeMgr);
    int bTreeOrder = bTreeMgr->orderOfTree;

    tempSearchKeys = malloc(bTreeOrder * sizeof(Value));
    if (tempSearchKeys == NULL) {
        perror("Temporary keys array.");
        exit(RC_INSERT_ERROR);
    }

    tempPtr = malloc(bTreeOrder * sizeof(void *));
    if (tempPtr == NULL) {
        perror("Temporary pointers array.");
        exit(RC_INSERT_ERROR);
    }

        insertIndex = 0;
        while (insertIndex < bTreeOrder - 1 && isLess(leaf->keys[insertIndex], key))
       insertIndex++;
i=0;
j=0;
    while(i < leaf->numOfSearchKeys) {
        if (j == insertIndex)
            j++;
        tempSearchKeys[j] = leaf->keys[i];
        tempPtr[j] = leaf->pointers[i];
        i++;
        j++;
    }

    tempSearchKeys[insertIndex] = key;
    tempPtr[insertIndex] = pointer;

    leaf->numOfSearchKeys = 0;

        if ((bTreeOrder - 1) % 2 == 0)
        split = (bTreeOrder - 1) / 2;
    else
        split = (bTreeOrder - 1) / 2 + 1;

    i=0;

    while (i < split) {
        leaf->pointers[i] = tempPtr[i];
        leaf->keys[i] = tempSearchKeys[i];
        leaf->numOfSearchKeys++;
        i++;
    }

i=split;
j=0;
    while (i < bTreeOrder) {
        newLeafNode->pointers[j] = tempPtr[i];
        newLeafNode->keys[j] = tempSearchKeys[i];
        newLeafNode->numOfSearchKeys++;
        i++;
        j++;
    }

    free(tempPtr);
    free(tempSearchKeys);

    newLeafNode->pointers[bTreeOrder - 1] = leaf->pointers[bTreeOrder - 1];
    leaf->pointers[bTreeOrder - 1] = newLeafNode;

i = leaf->numOfSearchKeys;
    while (i < bTreeOrder - 1)
    {
        leaf->pointers[i] = NULL;
        i++;
    }
    for (i = newLeafNode->numOfSearchKeys; i < bTreeOrder - 1; i++)
        newLeafNode->pointers[i] = NULL;

    newLeafNode->parent = leaf->parent;
    newSearchKey = newLeafNode->keys[0];
    bTreeMgr->numOfEntries++;
    return insertIntoParentNode(bTreeMgr, leaf, newSearchKey, newLeafNode);
}

Nodes * insertIntoNodesAfterSplit(BTreeMgr * bTreeMgr, Nodes * old_Nodes, int leftIndex, Value * key, Nodes * right) {

    int i, j, split, prime;
    Nodes * newSearchNodes, *childNode;
    Value ** tempSearchKeys;
    Nodes ** tempPtr;

    int bTreeOrder = bTreeMgr->orderOfTree;


    tempPtr = malloc((bTreeOrder + 1) * sizeof(Nodes *));
    if (tempPtr == NULL) {
        perror("Temporary pointers array for splitting Nodess.");
        exit(RC_INSERT_ERROR);
    }
    tempSearchKeys = malloc(bTreeOrder * sizeof(Value *));
    if (tempSearchKeys == NULL) {
        perror("Temporary keys array for splitting Nodess.");
        exit(RC_INSERT_ERROR);
    }

    for (i = 0, j = 0; i < old_Nodes->numOfSearchKeys + 1; i++, j++) {
        if (j == leftIndex + 1)
            j++;
        tempPtr[j] = old_Nodes->pointers[i];
    }

    for (i = 0, j = 0; i < old_Nodes->numOfSearchKeys; i++, j++) {
        if (j == leftIndex)
            j++;
        tempSearchKeys[j] = old_Nodes->keys[i];
    }

    tempPtr[leftIndex + 1] = right;
    tempSearchKeys[leftIndex] = key;


    if ((bTreeOrder - 1) % 2 == 0)
        split = (bTreeOrder - 1) / 2;
    else
        split = (bTreeOrder - 1) / 2 + 1;

    newSearchNodes = createNodes(bTreeMgr);
    old_Nodes->numOfSearchKeys = 0;
    for (i = 0; i < split - 1; i++) {
        old_Nodes->pointers[i] = tempPtr[i];
        old_Nodes->keys[i] = tempSearchKeys[i];
        old_Nodes->numOfSearchKeys++;
    }
    old_Nodes->pointers[i] = tempPtr[i];
    prime = tempSearchKeys[split - 1];
    for (++i, j = 0; i < bTreeOrder; i++, j++) {
        newSearchNodes->pointers[j] = tempPtr[i];
        newSearchNodes->keys[j] = tempSearchKeys[i];
        newSearchNodes->numOfSearchKeys++;
    }
    newSearchNodes->pointers[j] = tempPtr[i];
    free(tempPtr);
    free(tempSearchKeys);
    newSearchNodes->parent = old_Nodes->parent;
    for (i = 0; i <= newSearchNodes->numOfSearchKeys; i++) {
        childNode = newSearchNodes->pointers[i];
        childNode->parent = newSearchNodes;
    }

    bTreeMgr->numOfEntries++;
    return insertIntoParentNode(bTreeMgr, old_Nodes, prime, newSearchNodes);
}

Nodes * insertIntoParentNode(BTreeMgr * bTreeMgr, Nodes * left, Value * key, Nodes * right) {

        int leftIndex;
    Nodes * parent = left->parent;
    int bTreeOrder = bTreeMgr->orderOfTree;

        if (parent == NULL)
        return insertIntoNewRootNode(bTreeMgr, left, key, right);

        leftIndex = LeftIndex(parent, left);

        if (parent->numOfSearchKeys < bTreeOrder - 1) {
        return insertIntoNodes(bTreeMgr, parent, leftIndex, key, right);
    }

        return insertIntoNodesAfterSplit(bTreeMgr, parent, leftIndex, key, right);
}

int LeftIndex(Nodes * parent, Nodes * left) {
    int leftIndex = 0;
    while (leftIndex <= parent->numOfSearchKeys && parent->pointers[leftIndex] != left)
        leftIndex++;
    return leftIndex;
}

Nodes * insertIntoNodes(BTreeMgr * bTreeMgr, Nodes * parent, int leftIndex, Value * key, Nodes * right) {
    int i;
    for (i = parent->numOfSearchKeys; i > leftIndex; i--) {
        parent->pointers[i + 1] = parent->pointers[i];
        parent->keys[i] = parent->keys[i - 1];
    }

    parent->pointers[leftIndex + 1] = right;
    parent->keys[leftIndex] = key;
    parent->numOfSearchKeys++;

    return bTreeMgr->rootNode;
}

Nodes * insertIntoNewRootNode(BTreeMgr * bTreeMgr, Nodes * left, Value * key, Nodes * right) {
    Nodes * rootNode = createNodes(bTreeMgr);
    rootNode->keys[0] = key;
    rootNode->pointers[0] = left;
    rootNode->pointers[1] = right;
    rootNode->numOfSearchKeys++;
    rootNode->parent = NULL;
    left->parent = rootNode;
    right->parent = rootNode;
    return rootNode;
}

Nodes * createNodes(BTreeMgr * bTreeMgr) {
        bTreeMgr->numOfNodes++;
    int bTreeOrder = bTreeMgr->orderOfTree;

    Nodes * newSearchNodes = malloc(sizeof(Nodes));
    if (newSearchNodes == NULL) {
        perror("Nodes creation.");
        exit(RC_INSERT_ERROR);
    }
    
    newSearchNodes->keys = malloc((bTreeOrder - 1) * sizeof(Value *));
    if (newSearchNodes->keys == NULL) {
        perror("New Nodes keys array.");
        exit(RC_INSERT_ERROR);
    }
    
    newSearchNodes->pointers = malloc(bTreeOrder * sizeof(void *));
    if (newSearchNodes->pointers == NULL) {
        perror("New Nodes pointers array.");
        exit(RC_INSERT_ERROR);
    }
    
    newSearchNodes->isLeafNode = false;
    newSearchNodes->numOfSearchKeys = 0;
    newSearchNodes->parent = NULL;
    newSearchNodes->next = NULL;
    return newSearchNodes;
}

Nodes * createLeaf(BTreeMgr * bTreeMgr) {
    Nodes * leaf = createNodes(bTreeMgr);
    leaf->isLeafNode = true;
    return leaf;
}

Nodes * getLeafNode(Nodes * rootNode, Value * key) {
        int i = 0;
    Nodes * c = rootNode;
    if (c == NULL) {
                return c;
    }
    while (!c->isLeafNode) {
        i = 0;
        while (i < c->numOfSearchKeys) {
                        if (isGreater(key, c->keys[i]) || isEqual(key, c->keys[i])) {
                i++;
            } else
                break;
        }
        c = (Nodes *) c->pointers[i];
    }
    return c;
}

Data * identifyRecord(Nodes * rootNode, Value *key) {
        int i = 0;
    Nodes * c = getLeafNode(rootNode, key);
    if (c == NULL)
        return NULL;
    for (i = 0; i < c->numOfSearchKeys; i++) {
                if (isEqual(c->keys[i], key))
            break;
    }
    if (i == c->numOfSearchKeys)
        return NULL;
    else
        return (Data *) c->pointers[i];
}


int getNeighbor(Nodes * node) {

    int i;


    for (i = 0; i <= node->parent->numOfSearchKeys; i++)
        if (node->parent->pointers[i] == node)
            return i - 1;

            printf("Nodes:  %#lx\n", (unsigned long) node);
    exit(RC_ERROR);
}

Nodes * removeEntry(BTreeMgr * bTreeMgr, Nodes * node, Value * key, Nodes * pointer) {

        int i, num_pointers;
    int bTreeOrder = bTreeMgr->orderOfTree;

        i = 0;

    while (!isEqual(node->keys[i], key))
        i++;

    for (++i; i < node->numOfSearchKeys; i++)
        node->keys[i - 1] = node->keys[i];

            num_pointers = node->isLeafNode ? node->numOfSearchKeys : node->numOfSearchKeys + 1;
    i = 0;
    while (node->pointers[i] != pointer)
        i++;
    for (++i; i < num_pointers; i++)
        node->pointers[i - 1] = node->pointers[i];

        node->numOfSearchKeys--;
    bTreeMgr->numOfEntries--;

            if (node->isLeafNode)
        for (i = node->numOfSearchKeys; i < bTreeOrder - 1; i++)
            node->pointers[i] = NULL;
    else
        for (i = node->numOfSearchKeys + 1; i < bTreeOrder; i++)
            node->pointers[i] = NULL;

    return node;
}

Nodes * adjustRootNode(Nodes * rootNode) {

    Nodes * new_root;

            if (rootNode->numOfSearchKeys > 0)
        return rootNode;

    if (!rootNode->isLeafNode) {
                        new_root = rootNode->pointers[0];
        new_root->parent = NULL;
    } else {
                new_root = NULL;
    }

        free(rootNode->keys);
    free(rootNode->pointers);
    free(rootNode);

    return new_root;
}

Nodes * mergeNodes(BTreeMgr * bTreeMgr, Nodes * node, Nodes * neighbor, int neighborIndex, int prime) {

    int i, j, neighbor_insertion_index, n_end;
    Nodes * tmp;
    int bTreeOrder = bTreeMgr->orderOfTree;

        if (neighborIndex == -1) {
        tmp = node;
        node = neighbor;
        neighbor = tmp;
    }

            neighbor_insertion_index = neighbor->numOfSearchKeys;

            if (!node->isLeafNode) {
        neighbor->keys[neighbor_insertion_index] = prime;
        neighbor->numOfSearchKeys++;

        n_end = node->numOfSearchKeys;

        for (i = neighbor_insertion_index + 1, j = 0; j < n_end; i++, j++) {
            neighbor->keys[i] = node->keys[j];
            neighbor->pointers[i] = node->pointers[j];
            neighbor->numOfSearchKeys++;
            node->numOfSearchKeys--;
        }

        neighbor->pointers[i] = node->pointers[j];

                for (i = 0; i < neighbor->numOfSearchKeys + 1; i++) {
            tmp = (Nodes *) neighbor->pointers[i];
            tmp->parent = neighbor;
        }
    } else {
                        for (i = neighbor_insertion_index, j = 0; j < node->numOfSearchKeys; i++, j++) {
            neighbor->keys[i] = node->keys[j];
            neighbor->pointers[i] = node->pointers[j];
            neighbor->numOfSearchKeys++;
        }
        neighbor->pointers[bTreeOrder - 1] = node->pointers[bTreeOrder - 1];
    }

    bTreeMgr->rootNode = deleteEntry(bTreeMgr, node->parent, prime, node);

        free(node->keys);
    free(node->pointers);
    free(node);
    return bTreeMgr->rootNode;
}

Nodes * deleteEntry(BTreeMgr * bTreeMgr, Nodes * node, Value * key, void * pointer) {
        int min_keys;
    Nodes * neighbor;
    int neighborIndex;
    int primeIndex, prime;
    int capacity;
    int bTreeOrder = bTreeMgr->orderOfTree;

        node = removeEntry(bTreeMgr, node, key, pointer);

        if (node == bTreeMgr->rootNode)
        return adjustRootNode(bTreeMgr->rootNode);

        if (node->isLeafNode) {
        if ((bTreeOrder - 1) % 2 == 0)
            min_keys = (bTreeOrder - 1) / 2;
        else
            min_keys = (bTreeOrder - 1) / 2 + 1;
    } else {
        if ((bTreeOrder) % 2 == 0)
            min_keys = (bTreeOrder) / 2;
        else
            min_keys = (bTreeOrder) / 2 + 1;
        min_keys--;
    }

        if (node->numOfSearchKeys >= min_keys)
        return bTreeMgr->rootNode;

                neighborIndex = getNeighbor(node);
    primeIndex = neighborIndex == -1 ? 0 : neighborIndex;
    prime = node->parent->keys[primeIndex];
    neighbor =
            (neighborIndex == -1) ? node->parent->pointers[1] : node->parent->pointers[neighborIndex];

    capacity = node->isLeafNode ? bTreeOrder : bTreeOrder - 1;

    if (neighbor->numOfSearchKeys + node->numOfSearchKeys < capacity)
                return mergeNodes(bTreeMgr, node, neighbor, neighborIndex, prime);
    else
                return redistributeNodes(bTreeMgr->rootNode, node, neighbor, neighborIndex, primeIndex, prime);
}

Nodes * deleteTheKey(BTreeMgr * bTreeMgr, Value * key) {
    
    Nodes * record = identifyRecord(bTreeMgr->rootNode, key);
    Data * key_leaf = getLeafNode(bTreeMgr->rootNode, key);

    if (record != NULL && key_leaf != NULL) {
        bTreeMgr->rootNode = deleteEntry(bTreeMgr, key_leaf, key, record);
        free(record);
    }
        return bTreeMgr->rootNode;
}

Nodes * redistributeNodes(Nodes * rootNode, Nodes * node, Nodes * neighbor, int neighborIndex, int primeIndex, int prime) {
    int i;
    Nodes * tmp;

    if (neighborIndex != -1) {
                if (!node->isLeafNode)
            node->pointers[node->numOfSearchKeys + 1] = node->pointers[node->numOfSearchKeys];
        i = node->numOfSearchKeys;
        while (i > 0) {
            node->keys[i] = node->keys[i - 1];
            node->pointers[i] = node->pointers[i - 1];
            i--;
        }
        if (!node->isLeafNode) {
            node->pointers[0] = neighbor->pointers[neighbor->numOfSearchKeys];
            tmp = (Nodes *) node->pointers[0];
            tmp->parent = node;
            neighbor->pointers[neighbor->numOfSearchKeys] = NULL;
            node->keys[0] = prime;
            node->parent->keys[primeIndex] = neighbor->keys[neighbor->numOfSearchKeys - 1];
        } else {
            node->pointers[0] = neighbor->pointers[neighbor->numOfSearchKeys - 1];
            neighbor->pointers[neighbor->numOfSearchKeys - 1] = NULL;
            node->keys[0] = neighbor->keys[neighbor->numOfSearchKeys - 1];
            node->parent->keys[primeIndex] = node->keys[0];
        }
    } else {
                        if (node->isLeafNode) {
            node->keys[node->numOfSearchKeys] = neighbor->keys[0];
            node->pointers[node->numOfSearchKeys] = neighbor->pointers[0];
            node->parent->keys[primeIndex] = neighbor->keys[1];
        } else {
            node->keys[node->numOfSearchKeys] = prime;
            node->pointers[node->numOfSearchKeys + 1] = neighbor->pointers[0];
            tmp = (Nodes *) node->pointers[node->numOfSearchKeys + 1];
            tmp->parent = node;
            node->parent->keys[primeIndex] = neighbor->keys[0];
        }
        for (i = 0; i < neighbor->numOfSearchKeys - 1; i++) {
            neighbor->keys[i] = neighbor->keys[i + 1];
            neighbor->pointers[i] = neighbor->pointers[i + 1];
        }
        if (!node->isLeafNode)
            neighbor->pointers[i] = neighbor->pointers[i + 1];
    }

        node->numOfSearchKeys++;
    neighbor->numOfSearchKeys--;

    return rootNode;
}


void enqueue(BTreeMgr * bTreeMgr, Nodes * newSearchNodes) {
    Nodes * c;
    if (bTreeMgr->queue == NULL) {
        bTreeMgr->queue = newSearchNodes;
        bTreeMgr->queue->next = NULL;
    } else {
        c = bTreeMgr->queue;
        while (c->next != NULL) {
            c = c->next;
        }
        c->next = newSearchNodes;
        newSearchNodes->next = NULL;
    }
}

Nodes * dequeue(BTreeMgr * bTreeMgr) {
    Nodes * node = bTreeMgr->queue;
    bTreeMgr->queue = bTreeMgr->queue->next;
    node->next = NULL;
    return node;
}

int pathRootNode(Nodes * rootNode, Nodes * childNode) {
    int length = 0;
    Nodes * c = childNode;
    while (c != rootNode) {
        c = c->parent;
        length++;
    }
    return length;
}


bool isLess(Value * key1, Value * key2) {
    switch (key1->dt) {
    case DT_INT:
        if (key1->v.intV < key2->v.intV) {
            return TRUE;
        } else {
            return FALSE;
        }
        break;
    case DT_FLOAT:
        if (key1->v.floatV < key2->v.floatV) {
            return TRUE;
        } else {
            return FALSE;
        }
        break;
    case DT_STRING:
        if (strcmp(key1->v.stringV, key2->v.stringV) == -1) {
            return TRUE;
        } else {
            return FALSE;
        }
        break;
    case DT_BOOL:
                return FALSE;
        break;
    }
}

bool isGreater(Value * key1, Value * key2) {
    switch (key1->dt) {
    case DT_INT:
        if (key1->v.intV > key2->v.intV) {
            return TRUE;
        } else {
            return FALSE;
        }
        break;
    case DT_FLOAT:
        if (key1->v.floatV > key2->v.floatV) {
            return TRUE;
        } else {
            return FALSE;
        }
        break;
    case DT_STRING:
        if (strcmp(key1->v.stringV, key2->v.stringV) == 1) {
            return TRUE;
        } else {
            return FALSE;
        }
        break;
    case DT_BOOL:
                return FALSE;
        break;
    }
}

bool isEqual(Value * key1, Value * key2) {
    switch (key1->dt) {
    case DT_INT:
        if (key1->v.intV == key2->v.intV) {
            return TRUE;
        } else {
            return FALSE;
        }
        break;
    case DT_FLOAT:
        if (key1->v.floatV == key2->v.floatV) {
            return TRUE;
        } else {
            return FALSE;
        }
        break;
    case DT_STRING:
        if (strcmp(key1->v.stringV, key2->v.stringV) == 0) {
            return TRUE;
        } else {
            return FALSE;
        }
        break;
    case DT_BOOL:
        if (key1->v.boolV == key2->v.boolV) {
            return TRUE;
        } else {
            return FALSE;
        }
        break;
    }
}


