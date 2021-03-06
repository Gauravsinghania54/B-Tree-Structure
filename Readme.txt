B+ - TreeIndex 

Project Modules
C source files: dberror.c, test_assign3_1.c, buffer_mgr_stat.c, test_expr.c, record_mgr.c,  rm_serializer.c, buffer_mgr.c,storage_mgr.c,expr.c, btree_execute.c, btree_mgr.c, 

Header files: storage_mgr.h, dberror.h, test_helper.h, buffer_mgr.h, buffer_mgr_stat.h, dt.h, expr.h, record_mgr.h, tables.h, btree_iexecute.h, btree_mgr.h, tables.h


Team Members: (Group 4)

1. Gaurav Singhania(A20445477)

2. Chaitanya Nagaram(A20450288)

3. Sachin Devatar(A20443522)


Abstract:

The main purpose is to implement a disk-based B+-tree index.

To Run:

With given default test cases:

Command: make test_assign4_1.c
Run: ./test_assign4_1.c
To remove object files use this command: make clean



-- B+ Tree  Functions--


--Custom B+ Tree functions (btree_execute.h)


These functions have bee defined to perform insert/delete/find/print operations on our B+ Tree.


getLeafNode() 
- This functions finds the leaf node which has the entry having the specified key in parameter.
- It is used when inserting an element as well as finding an entry.

identifyRecord()
- This function search for B+ Tree for an entry having the specified key in parameter.
- It returns the record if the key is present in the tree else returns null.

buildRecord()
- This function creates a new record element which encapsulates a RID.

insertIntoLeafNode()
- This function inserts a new pointer to the record and its corresponding key into a leaf.
- It returns the altered leaf node.

NewTree()
- This function creates a new tree when the first element/entry is inserted in the B+ tree. 

createNodes()
- This function creates a new general node, which can be adapted to serve as a leaf/internal/root node.

createLeaf()
-  This function creates a new leaf node.

insertIntoLeafNodeAfterSplit()
- This function inserts a new key and pointer to a new record into a leaf node so as to exceed the tree's order, causing the leaf to be split in half.
- It adjusts the tree after splitting so as maintain the B+ Tree properties.

insertIntoNodes()
-This function inserts a new key and pointer to a node into a node into which these can fit without violating the B+ tree properties.

insertIntoNodesAfterSplit()
- This function inserts a new key and pointer to a node into a non-leaf node, causing the node's size to exceed the order, and causing the node to split into two.

insertIntoParentNode()
-This function inserts a new node (leaf or internal node) into the B+ tree.
- It returns the root of the tree after insertion.

insertIntoNewRootNode()
- This function creates a new root for two subtrees and inserts the appropriate key into the new root.

LeftIndex()
- This function is used in insertIntoParent(..) function to findRecord the index of the parent's pointer to the node to the left of the key to be inserted.

adjustRootNode()
- This function adjusts the root after a record has been deleted from the B+ Tree and maintains the B+ Tree properties.

mergeNodes()
-This function combines (merges) a node that has become too small after deletion with a neighboring node that can accept the additional entries without exceeding the maximum.

redistributeNodes()
- This function redistributes the entries between two nodes when one has become too small after deletion but its neighbor is too big to append the small node's entries without exceeding the maximum.

deleteEntry()
- This function deletes an entry from the B+ tree.
- It removes the record having the specified key and pointer from the leaf, and then makes all appropriate changes to preserve the B+ tree properties.

deleteTheKey()
- This function deletes the the entry/record having the specified key.

removeEntry()
- This function removes a record having the specified key from the the specified node.

getNeighbor()
- This function returns the index of a node's nearest neighbor (sibling) to the left if one exists.
- It returns -1 if this is not the case i.e. the node is the leftmost child.


--init and shutdown index manager--

These functions are used to initialize the index manager and shut it down, freeing up all acquired resources.

initIndexManager()

-The index manager is initialized by initializing the storage manager

shutdownIndexManager()

- This function efeectively shuts down the index manager
- The effect of this function is that it frees up all resources/memory space being used by the Index Manager
- It does so by de-allocating all the resources allocated to the index manager

--B+-tree Functions--
These functions are used to create or delete a b-tree index

createBtree()

-This function creates a new B+ Tree
-Allocates memory to all elements of Btree struct
-It initializes the TreeManager structure which stores additional information of our B+ Tree


openBtree()

-This function opens an existing B+ Tree which is stored on the file specified by "idxId" parameter
-We retrieve our TreeManager and initialize the Buffer Pool

closeBtree()

-This Function closes the B tree index which is opened
-The index manager ensures that all new or modified pages of the index are flushed back to disk
-It then shuts down the buffer pool and frees up all the allocated resources

deleteBtree()

-This Function deletes a B tree index and removes the corresponding page file


--Functions to Access Information about B+- tree--

getNumNodes()

-This function returns the number of nodes present in our B+ Tree
-We store this information in our TreeManager structure in variable TotalNodes

getNumEntries()

-This function returns the number of entries/records/keys present in our B+ Tree
-We store this information in our TreeManager structure in variable TotalEntries

getKeyType()

-This function returns the datatype of the keys being stored in our B+ Tree
-We store this information in our TreeManager structure in variable KeyType

--Index Access Functions--

findKey()

-It searches the B+ Tree for the key specified in the parameter
-This Function returns the RID for the entry with the search key in the b-tree
-If the key does not exist it returns RC_IM_KEY_NOT_sFOUND

insertKey()

-Inserts the given key in a Btree
-If that key is already stored in the b-tree it returns the error code RC_IM_KEY_ALREADY_EXISTS

deleteKey()

-This Function removes a key and the corresponding record pointer from the index

openTreeScan()

-This function initializes the scan which is used to scan the entries in the B+ Tree in the sorted key order
-This function initializes the ScanManager structure for performing the scan operation

nextEntry()

-This Function reads the next entry in the Btree
-If all the entries have been scanned and there are no more entries left, then we return error code RC_IM_NO_MORE_ENTRIES

closeTreeScan()

-This Function closes the tree after scanning through all the elements of the B tree

--Debug and Test functions--

printTree()
- This Function is used to create a string representation of a b-tree