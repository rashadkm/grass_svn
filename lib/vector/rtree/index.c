/*!
   \file lib/vector/rtree/index.c

   \brief R-Tree library - Multidimensional index

   Higher level functions for managing R*-Trees.

   (C) 2010-2012 by the GRASS Development Team

   This program is free software under the 
   GNU General Public License (>=v2). 
   Read the file COPYING that comes with GRASS
   for details.

   \author Antonin Guttman - original code
   \author Daniel Green (green@superliminal.com) - major clean-up
	       and implementation of bounding spheres
   \author Markus Metz - file-based and memory-based R*-tree
 */

/* Read these articles first before attempting to modify the code
 * 
 * R-Tree reference:
 * Guttman, A. (1984). "R-Trees: A Dynamic Index Structure for Spatial
 * Searching". Proceedings of the 1984 ACM SIGMOD international 
 * conference on Management of data - SIGMOD '84. pp. 47.
 * DOI:10.1145/602259.602266
 * ISBN 0897911288
 *  
 * R*-Tree reference:
 * Beckmann, N.; Kriegel, H. P.; Schneider, R.; Seeger, B. (1990).
 * "The R*-tree: an efficient and robust access method for points and 
 * rectangles". Proceedings of the 1990 ACM SIGMOD international 
 * conference on Management of data - SIGMOD '90. pp. 322.
 * DOI:10.1145/93597.98741
 * ISBN 0897913655
 */

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include <grass/gis.h>
#include "index.h"

/*!
  \brief Create new empty R*-Tree

  This method creates a new RTree, either in memory (fd < 0) or in file.
  If the file descriptor is positive, the corresponding file must have 
  been opened for reading and writing.
  This method must also be called if an existing tree previously saved
  to file is going to be accessed.

  \param fd file descriptor to hold data, negative toggles memory mode
  \param rootpos offset in file to root node (past any header info)
  \param ndims number of dimensions for the new tree: min 2, max 20

  \return pointer to new RTree structure
*/
struct RTree *RTreeCreateTree(int fd, off_t rootpos, int ndims)
{
    struct RTree *new_rtree;
    struct RTree_Node *n;
    int i, j;
    
    new_rtree = (struct RTree *)malloc(sizeof(struct RTree));

    new_rtree->fd = fd;
    new_rtree->rootpos = rootpos;
    new_rtree->ndims = ndims;
    new_rtree->nsides = 2 * ndims;
    /* hack to keep compatibility */
    if (ndims < 3)
	new_rtree->ndims_alloc = 3;
    else
	new_rtree->ndims_alloc = ndims;

    new_rtree->nsides_alloc = 2 * new_rtree->ndims_alloc;

    /* init free nodes */
    new_rtree->free_nodes.avail = 0;
    new_rtree->free_nodes.alloc = 0;
    new_rtree->free_nodes.pos = NULL;

    new_rtree->rectsize = new_rtree->nsides_alloc * sizeof(RectReal);
    new_rtree->nodesize = sizeof(struct RTree_Node) -
                          MAXCARD * sizeof(RectReal *) +
			  MAXCARD * new_rtree->rectsize;

    new_rtree->branchsize = sizeof(struct RTree_Branch) -
                            sizeof(RectReal *) + new_rtree->rectsize;
    
    /* create empty root node */
    n = RTreeAllocNode(new_rtree, 0);
    new_rtree->rootlevel = n->level = 0;       /* leaf */
    new_rtree->root = NULL;

    if (fd > -1) {  /* file based */
	/* nodecard and leafcard can be adjusted, must NOT be larger than MAXCARD */
	new_rtree->nodecard = MAXCARD;
	new_rtree->leafcard = MAXCARD;

	/* initialize node buffer */
	for (i = 0; i < MAXLEVEL; i++) {
	    new_rtree->nb[i][0].dirty = 0;
	    new_rtree->nb[i][1].dirty = 0;
	    new_rtree->nb[i][2].dirty = 0;
	    new_rtree->nb[i][0].pos = -1;
	    new_rtree->nb[i][1].pos = -1;
	    new_rtree->nb[i][2].pos = -1;
	    /* usage order */
	    new_rtree->used[i][0] = 2;
	    new_rtree->used[i][1] = 1;
	    new_rtree->used[i][2] = 0;

	    /* alloc memory for rectangles */
	    for (j = 0; j < MAXCARD; j++) {
		RTreeAllocBoundary(&(new_rtree->nb[i][0].n.branch[j].rect), new_rtree);
		RTreeAllocBoundary(&(new_rtree->nb[i][1].n.branch[j].rect), new_rtree);
		RTreeAllocBoundary(&(new_rtree->nb[i][2].n.branch[j].rect), new_rtree);

		RTreeAllocBoundary(&(new_rtree->fs[i].sn.branch[j].rect), new_rtree);
	    }
	}

	/* write empty root node */
	lseek(new_rtree->fd, rootpos, SEEK_SET);
	RTreeWriteNode(n, new_rtree);
	new_rtree->nb[0][0].n = *n;
	new_rtree->nb[0][0].pos = rootpos;
	new_rtree->used[0][0] = 0;
	new_rtree->used[0][2] = 2;
	RTreeFreeNode(n);

	new_rtree->insert_rect = RTreeInsertRectF;
	new_rtree->delete_rect = RTreeDeleteRectF;
	new_rtree->search_rect = RTreeSearchF;
	new_rtree->valid_child = RTreeValidChildF;
    }
    else {    /* memory based */
	new_rtree->nodecard = MAXCARD;
	new_rtree->leafcard = MAXCARD;

	new_rtree->insert_rect = RTreeInsertRectM;
	new_rtree->delete_rect = RTreeDeleteRectM;
	new_rtree->search_rect = RTreeSearchM;
	new_rtree->valid_child = RTreeValidChildM;

	new_rtree->root = n;
    }

    /* minimum number of remaining children for RTreeDeleteRect */
    /* NOTE: min fill can be changed if needed, must be < nodecard and leafcard. */
    new_rtree->min_node_fill = (new_rtree->nodecard - 2) / 2;
    new_rtree->min_leaf_fill = (new_rtree->leafcard - 2) / 2;

    /* balance criteria for node splitting */
    new_rtree->minfill_node_split = (new_rtree->nodecard - 1) / 2;
    new_rtree->minfill_leaf_split = (new_rtree->leafcard - 1) / 2;

    new_rtree->n_nodes = 1;
    new_rtree->n_leafs = 0;

    /* initialize temp variables */
    RTreeAllocBoundary(&(new_rtree->p.cover[0]), new_rtree);
    RTreeAllocBoundary(&(new_rtree->p.cover[1]), new_rtree);
    
    RTreeAllocBoundary(&(new_rtree->tmpb1.rect), new_rtree);
    RTreeAllocBoundary(&(new_rtree->tmpb2.rect), new_rtree);
    RTreeAllocBoundary(&(new_rtree->c.rect), new_rtree);
    for (i = 0; i <= MAXCARD; i++) {
	RTreeAllocBoundary(&(new_rtree->BranchBuf[i].rect), new_rtree);
    }
    RTreeAllocBoundary(&(new_rtree->rect_0), new_rtree);
    RTreeAllocBoundary(&(new_rtree->rect_1), new_rtree);
    RTreeAllocBoundary(&(new_rtree->upperrect), new_rtree);
    RTreeAllocBoundary(&(new_rtree->orect), new_rtree);
    new_rtree->center_n = (RectReal *)malloc(new_rtree->ndims_alloc * sizeof(RectReal));

    return new_rtree;
}

/*!
  \brief Destroy an R*-Tree

  This method releases all memory allocated to a RTree. It deletes all 
  rectangles and all memory allocated for internal support data.
  Note that for a file-based RTree, the file is not deleted and not 
  closed. The file can thus be used to permanently store an RTree.

  \param t pointer to RTree structure

  \return nothing
*/

void RTreeDestroyTree(struct RTree *t)
{
    int i, j;

    assert(t);

    if (t->fd > -1) {
	if (t->free_nodes.alloc)
	    free(t->free_nodes.pos);
    }
    else if (t->root)
	RTreeDestroyNode(t->root, t->root->level ? t->nodecard : t->leafcard);

    if (t->fd > -1) {  /* file based */
	/* free node buffer */
	for (i = 0; i < MAXLEVEL; i++) {

	    /* free memory for rectangles */
	    for (j = 0; j < MAXCARD; j++) {
		RTreeFreeBoundary(&(t->nb[i][0].n.branch[j].rect));
		RTreeFreeBoundary(&(t->nb[i][1].n.branch[j].rect));
		RTreeFreeBoundary(&(t->nb[i][2].n.branch[j].rect));
		RTreeFreeBoundary(&(t->fs[i].sn.branch[j].rect));
	    }
	}
    }

    /* free temp variables */
    RTreeFreeBoundary(&(t->p.cover[0]));
    RTreeFreeBoundary(&(t->p.cover[1]));
    
    RTreeFreeBoundary(&(t->tmpb1.rect));
    RTreeFreeBoundary(&(t->tmpb2.rect));
    RTreeFreeBoundary(&(t->c.rect));
    for (i = 0; i <= MAXCARD; i++) {
        RTreeFreeBoundary(&(t->BranchBuf[i].rect));
    }
    RTreeFreeBoundary(&(t->rect_0));
    RTreeFreeBoundary(&(t->rect_1));
    RTreeFreeBoundary(&(t->upperrect));
    RTreeFreeBoundary(&(t->orect));
    free(t->center_n);

    free(t);
    
    return;
}

/*!
  \brief Search an R*-Tree

  Search in an RTree for all data retangles that overlap or touch the 
  argument rectangle.
  Return the number of qualifying data rectangles.
  The search stops if the SearchHitCallBack function returns 0 (zero)
  or if there are no more qualifying data rectangles.

  \param t pointer to RTree structure
  \param r pointer to rectangle to use for searching
  \param shcb Search Hit CallBack function
  \param cbarg custom pointer used as argument for the shcb fn

  \return number of qualifying data rectangles
*/
/*
 * 
 * add option to select operator to select rectangles ?
 * current: overlap
 * possible alternatives: 
 *  - select all rectangles that are fully contained in r
 *  - select all rectangles that fully contain r
 */
int RTreeSearch(struct RTree *t, struct RTree_Rect *r,
                SearchHitCallback *shcb, void *cbarg)
{
    assert(r && t);

    return t->search_rect(t, r, shcb, cbarg);
}

/*!
  \brief Insert an item into a R*-Tree

  \param r pointer to rectangle to use for searching
  \param tid data id stored with rectangle, must be > 0
  \param t pointer to RTree structure

  \return number of qualifying data rectangles
*/
int RTreeInsertRect(struct RTree_Rect *r, int tid, struct RTree *t)
{
    union RTree_Child newchild;

    assert(r && t && tid > 0);
    
    t->n_leafs++;
    newchild.id = tid;
    
    return t->insert_rect(r, newchild, 0, t);
}

/*!
  \brief Delete an item from a R*-Tree
  
  This method deletes an item from the RTree. The rectangle passed to 
  this method does not need to be the exact rectangle, the only
  requirement is that this rectangle overlaps with the rectangle to 
  be deleted. The rectangle to be deleted is identified by its id.

  \param r pointer to rectangle to use for searching
  \param tid id of the data to be deleted, must be > 0
  \param t pointer to RTree structure

  \return 0 on success
  \return 1 if data item not found
*/
int RTreeDeleteRect(struct RTree_Rect *r, int tid, struct RTree *t)
{
    union RTree_Child child;
    
    assert(r && t && tid > 0);

    child.id = tid;

    return t->delete_rect(r, child, t);
}


/***********************************
 *    internally used functions    *
 ***********************************/ 


/*
 * Allocate space for a node in the list used in DeleteRect to
 * store Nodes that are too empty.
 */
struct RTree_ListNode *RTreeNewListNode(void)
{
    return (struct RTree_ListNode *)malloc(sizeof(struct RTree_ListNode));
}

void RTreeFreeListNode(struct RTree_ListNode *p)
{
    free(p);
}

/* 
 * Add a node to the reinsertion list.  All its branches will later
 * be reinserted into the index structure.
 */
void RTreeReInsertNode(struct RTree_Node *n, struct RTree_ListNode **ee)
{
    struct RTree_ListNode *l = RTreeNewListNode();

    l->node = n;
    l->next = *ee;
    *ee = l;
}

/* 
 * Free ListBranch, used by R*-type forced reinsertion
 */
void RTreeFreeListBranch(struct RTree_ListBranch *p)
{
    RTreeFreeBoundary(&(p->b.rect));
    free(p);
}



