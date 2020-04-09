#ifndef NULL
#define NULL (0)
#endif
#include <stdio.h>
#include <stdlib.h>
#include "dberror.h"
#include "btree_mgr.h"
#include "storage_mgr.h"
#include "buffer_mgr.h"
#include "tables.h"
#include "btree_execute.h"

BTreeMgr *bTreeMgr = NULL;

RC initIndexManager(void *mgmtData) {
    initStorageManager();
        return RC_OK;
}

RC shutdownIndexManager() {
    bTreeMgr = NULL;
        return RC_OK;
}

RC createBtree(char *idxId, DataType keyType, int n) {
    int maxNumberOfNodes = PAGE_SIZE / sizeof(Nodes);

        if (n > maxNumberOfNodes) {
        printf("\n n = %d > Max. Nodes = %d \n", n, maxNumberOfNodes);
        return RC_ORDER_TOO_HIGH_FOR_PAGE;
    }

        bTreeMgr = (BTreeMgr *) malloc(sizeof(BTreeMgr));
    bTreeMgr->orderOfTree = n + 2;            
    bTreeMgr->numOfNodes = 0;            
    bTreeMgr->numOfEntries = 0;        
    bTreeMgr->rootNode = NULL;            
    bTreeMgr->queue = NULL;            
    bTreeMgr->keyType = keyType;    
        BM_BufferPool * bm = (BM_BufferPool *) malloc(sizeof(BM_BufferPool));
    bTreeMgr->bPool = *bm;

    SM_FileHandle fileHandle;
    RC output;

    char data[PAGE_SIZE];

        if ((output = createPageFile(idxId)) != RC_OK)
        return output;

        if ((output = openPageFile(idxId, &fileHandle)) != RC_OK)
        return output;

        if ((output = writeBlock(0, &fileHandle, data)) != RC_OK)
        return output;

        if ((output = closePageFile(&fileHandle)) != RC_OK)
        return output;

        return (RC_OK);
}

RC openBtree(BTreeHandle **tree, char *idxId) {
        *tree = (BTreeHandle *) malloc(sizeof(BTreeHandle));
    (*tree)->mgmtData = bTreeMgr;

        RC output = initBufferPool(&bTreeMgr->bPool, idxId, 1000, RS_FIFO, NULL);

    if (output == RC_OK) {
                return RC_OK;
    }
    return output;
}

RC closeBtree(BTreeHandle *tree) {
        BTreeMgr * bTreeMgr = (BTreeMgr*) tree->mgmtData;

        markDirty(&bTreeMgr->bPool, &bTreeMgr->pageHandle);

        shutdownBufferPool(&bTreeMgr->bPool);

        free(bTreeMgr);
    free(tree);

        return RC_OK;
}

RC deleteBtree(char *idxId) {
    RC output;
    if ((output = destroyPageFile(idxId)) != RC_OK)
        return output;
        return RC_OK;
}

RC insertKey(BTreeHandle *tree, Value *key, RID rid) {
        BTreeMgr *bTreeMgr = (BTreeMgr *) tree->mgmtData;
    Data * ptr;
    Nodes * leafNode;

    int orderOfBTree = bTreeMgr->orderOfTree;

        
        if (identifyRecord(bTreeMgr->rootNode, key) != NULL) {
        printf("\n The insert Key already EXISTS");
        return RC_IM_KEY_ALREADY_EXISTS;
    }

        ptr = buildRecord(&rid);

        if (bTreeMgr->rootNode == NULL) {
        bTreeMgr->rootNode = NewTree(bTreeMgr, key, ptr);
                        return RC_OK;
    }

        leafNode = getLeafNode(bTreeMgr->rootNode, key);

    if (leafNode->numOfSearchKeys < orderOfBTree - 1) {
                leafNode = insertIntoLeafNode(bTreeMgr, leafNode, key, ptr);
    } else {
                bTreeMgr->rootNode = insertIntoLeafNodeAfterSplit(bTreeMgr, leafNode, key, ptr);
    }

                return RC_OK;
}

extern RC findKey(BTreeHandle *tree, Value *key, RID *output) {
        BTreeMgr *bTreeMgr = (BTreeMgr *) tree->mgmtData;

        Data * record = identifyRecord(bTreeMgr->rootNode, key);

        if (record == NULL) {
        return RC_IM_KEY_NOT_FOUND;
    } else {
            }

        *output = record->recId;
    return RC_OK;
}

RC getNumNodes(BTreeHandle *tree, int *output) {
        BTreeMgr * bTreeMgr = (BTreeMgr *) tree->mgmtData;
    
        *output = bTreeMgr->numOfNodes;
    return RC_OK;
}

RC getNumEntries(BTreeHandle *tree, int *output) {
        BTreeMgr * bTreeMgr = (BTreeMgr *) tree->mgmtData;
    
        *output = bTreeMgr->numOfEntries;
    return RC_OK;
}

RC getKeyType(BTreeHandle *tree, DataType *output) {
        BTreeMgr * bTreeMgr = (BTreeMgr *) tree->mgmtData;

        *output = bTreeMgr->keyType;
    return RC_OK;
}

RC deleteKey(BTreeHandle *tree, Value *key) {
        BTreeMgr *bTreeMgr = (BTreeMgr *) tree->mgmtData;

        bTreeMgr->rootNode = deleteTheKey(bTreeMgr, key);
        return RC_OK;
}

RC openTreeScan(BTreeHandle *tree, BT_ScanHandle **handle) {
        BTreeMgr *bTreeMgr = (BTreeMgr *) tree->mgmtData;

        ScanMgr *scanMetaData = malloc(sizeof(ScanMgr));

        *handle = malloc(sizeof(BT_ScanHandle));

    Nodes * nodes = bTreeMgr->rootNode;

    if (bTreeMgr->rootNode == NULL) {
                return RC_NO_RECORDS_TO_SCAN;
    } else {
                while (!nodes->isLeafNode)
            nodes = nodes->pointers[0];

                scanMetaData->searchKeyIndex = 0;
        scanMetaData->totalNoKeys = nodes->numOfSearchKeys;
        scanMetaData->nodes = nodes;
        scanMetaData->orderOfTree = bTreeMgr->orderOfTree;
        (*handle)->mgmtData = scanMetaData;
            }
    return RC_OK;
}

RC nextEntry(BT_ScanHandle *handle, RID *output) {
            ScanMgr * scanMetaData = (ScanMgr *) handle->mgmtData;

        int searchKeyIndex = scanMetaData->searchKeyIndex;
    int totalNoKeys = scanMetaData->totalNoKeys;
    int orderOfBTree = scanMetaData->orderOfTree;
    RID rid;

        Nodes * nodes = scanMetaData->nodes;

        if (nodes == NULL) {
        return RC_IM_NO_MORE_ENTRIES;
    }

    if (searchKeyIndex < totalNoKeys) {
                rid = ((Data *) nodes->pointers[searchKeyIndex])->recId;
                        scanMetaData->searchKeyIndex++;
    } else {
                if (nodes->pointers[orderOfBTree - 1] != NULL) {
            nodes = nodes->pointers[orderOfBTree - 1];
            scanMetaData->searchKeyIndex = 1;
            scanMetaData->totalNoKeys = nodes->numOfSearchKeys;
            scanMetaData->nodes = nodes;
            rid = ((Data *) nodes->pointers[0])->recId;
                    } else {
                        return RC_IM_NO_MORE_ENTRIES;
        }
    }
        *output = rid;
    return RC_OK;
}

extern RC closeTreeScan(BT_ScanHandle *handle) {
    handle->mgmtData = NULL;
    free(handle);
    return RC_OK;
}

extern char *printTree(BTreeHandle *tree) {
    BTreeMgr *bTreeMgr = (BTreeMgr *) tree->mgmtData;
    printf("\nPRINTING THE TREE:\n");
    Nodes * node = NULL;
    int i = 0;
    int rankOfTree = 0;
    int newRankOfTree = 0;

    if (bTreeMgr->rootNode == NULL) {
        printf("Empty tree.\n");
        return '\0';
    }
    bTreeMgr->queue = NULL;
    enqueue(bTreeMgr, bTreeMgr->rootNode);
    while (bTreeMgr->queue != NULL) {
        node = dequeue(bTreeMgr);
        if (node->parent != NULL && node == node->parent->pointers[0]) {
            newRankOfTree = pathRootNode(bTreeMgr->rootNode, node);
            if (newRankOfTree != rankOfTree) {
                rankOfTree = newRankOfTree;
                printf("\n");
            }
        }

        int index=0;
                while (index < node->numOfSearchKeys) {
            if(bTreeMgr->keyType==DT_INT)
                printf("%d ", (*node->keys[index]).v.intV);
            else if(bTreeMgr->keyType==DT_FLOAT)
                printf("%.02f ", (*node->keys[index]).v.floatV);
            else if(bTreeMgr->keyType==DT_STRING)
                printf("%s ", (*node->keys[index]).v.stringV);
            else if(bTreeMgr->keyType==DT_BOOL)
                printf("%d ", (*node->keys[index]).v.boolV);

            printf("(%d - %d) ", ((Data *) node->pointers[index])->recId.page, ((Data *) node->pointers[index])->recId.slot);
             index++;
        }
        if (!node->isLeafNode){
            index = 0;
            while (index <= node->numOfSearchKeys)
            {
                enqueue(bTreeMgr, node->pointers[index]);
                index++;
            }
        }


        printf("| ");
    }
    printf("\n");

    return '\0';
}

