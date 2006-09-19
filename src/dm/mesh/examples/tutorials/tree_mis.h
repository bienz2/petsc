/*-----------------------------------------------*\
|tree_mis.h - quad/octtree based ball coarsening  |
| - takes a mesh and patch number                 |
| - produces a list comprising a ball packing     |
|   over the volume                               |
| - right now only 2D                             |
\*-----------------------------------------------*/

#include <list>
#include <Distribution.hh>
//#include "petscmesh.h"
//#include "petscviewer.h"
//#include "src/dm/mesh/meshpcice.h"
//#include "src/dm/mesh/meshpylith.h"
#include <triangle.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <ctime>

namespace ALE {
  namespace Coarsener {
    struct mis_node {
      bool isLeaf;
      mis_node * parent;
      double * boundaries;
      double maxSpacing;  //we can only refine until the radius of the max spacing ball is hit.
      std::list<mis_node *> subspaces;
      int depth;
      std::list<ALE::Mesh::point_type> childPoints;
      std::list<ALE::Mesh::point_type> childColPoints;
    };
    bool isOverlap(mis_node *, mis_node *, int); //calculates if there is an intersection or overlap of these two domains
    extern PetscErrorCode TriangleToMesh(Obj<ALE::Mesh>, triangulateio *, ALE::Mesh::section_type::patch_type);
    void randPush(std::list<ALE::Mesh::point_type> *, ALE::Mesh::point_type); //breaks up patterns in the mesh that cause oddness

    PetscErrorCode tree_mis (Obj<ALE::Mesh>& mesh, int dim, ALE::Mesh::section_type::patch_type patch, bool includePrevious, double beta) {
      //build the quadtree
      PetscFunctionBegin;
      srand((unsigned int)time(0));
      ALE::Mesh::section_type::patch_type rPatch = 0; //the patch on which everything is stored.. we restrict to this patch
      Obj<ALE::Mesh::topology_type> topology = mesh->getTopologyNew();
      const Obj<ALE::Mesh::topology_type::label_sequence>& vertices = topology->depthStratum(rPatch, 0);
      Obj<ALE::Mesh::section_type> coords = mesh->getSection("coordinates");
      Obj<ALE::Mesh::section_type> spacing = mesh->getSection("spacing");
      const Obj<ALE::Mesh::topology_type::patch_label_type>& boundary = topology->getLabel(rPatch, "boundary");
      ostringstream txt;
	  //build the root node;
      mis_node * tmpPoint = new mis_node;
      tmpPoint->boundaries = new double[2*dim];
      tmpPoint->maxSpacing = 0;
      tmpPoint->depth = 0;
      bool * bound_init = new bool[2*dim];
      for (int i = 0; i < 2*dim; i++) {
	bound_init[i] = false;
      }
      ALE::Mesh::topology_type::label_sequence::iterator v_iter = vertices->begin();
      ALE::Mesh::topology_type::label_sequence::iterator v_iter_end = vertices->end();
      while (v_iter != v_iter_end) {
	double cur_space = *spacing->restrict(rPatch, *v_iter);
	const double * cur_coords = coords->restrict(rPatch, *v_iter); //I swear a lot when I code
	    //initialize the boundaries.
	for (int i = 0; i < dim; i++) {
	  if (bound_init[2*i] == false || cur_coords[i] < tmpPoint->boundaries[2*i]) {
	    bound_init[2*i] = true; 
	    tmpPoint->boundaries[2*i] = cur_coords[i];
	  }
	  if (bound_init[2*i+1] == false || cur_coords[i] > tmpPoint->boundaries[2*i+1]) {
	    bound_init[2*i+1] = true;
	    tmpPoint->boundaries[2*i+1] = cur_coords[i];
	  }
	}
	    //initialize the maximum spacing ball.
	if(tmpPoint->maxSpacing < cur_space) tmpPoint->maxSpacing = cur_space;

	    //if it's essential, push it to the ColPoints stack, which will be the pool that is compared with during the traversal-MIS algorithm.
	if (!includePrevious) {
	  if(topology->getValue(boundary, *v_iter) == dim) { randPush(&tmpPoint->childColPoints, *v_iter);
	  } else { randPush(&tmpPoint->childPoints, *v_iter);} 
	} else { //enforce the node-nested condition.
	  if(topology->getPatch(patch+1)->capContains(*v_iter)) {
	    randPush(&tmpPoint->childColPoints, *v_iter);
           // printf("Got one from the last");
	  } else {
	    randPush(&tmpPoint->childPoints, *v_iter);
	  }
	}
	v_iter++;
      }

      mis_node * root = tmpPoint;
      //PetscPrintf(mesh->comm(), "Root volume has dimensions:");
      //for (int d = 0; d < dim; d++) PetscPrintf(mesh->comm(), "(%f <-> %f), ", root->boundaries[2*d], root->boundaries[2*d+1]);
      //PetscPrintf(mesh->comm(), "\n");
      std::list<mis_node *> mis_queue;
      std::list<mis_node *> leaf_list;
      mis_queue.push_front(root);
      while (!mis_queue.empty()) {
	tmpPoint = *mis_queue.begin();
	mis_queue.pop_front();
	bool canRefine = true;

	    //define the criterion under which we cannot refine this particular section

	for (int i = 0; i < dim; i++) {
	  if((tmpPoint->boundaries[2*i+1] - tmpPoint->boundaries[2*i]) < 2*beta*tmpPoint->maxSpacing) { 
	    canRefine = false;
	    //PetscPrintf(mesh->comm(), "-- cannot refine: %f < %f\n", (tmpPoint->boundaries[2*i+1] - tmpPoint->boundaries[2*i]), beta*tmpPoint->maxSpacing);
	  }
	}
	if (tmpPoint->childPoints.size() + tmpPoint->childColPoints.size() < 20) canRefine = false;  //the threshhold at which we do not care to not do the greedy thing as comparison is cheap enough
	if (canRefine) {
	  //PetscPrintf(mesh->comm(), "-- refining an area containing %d nodes..\n", tmpPoint->childPoints.size() + tmpPoint->childColPoints.size());
	  tmpPoint->isLeaf = false;
	  int nblocks = (int)pow(2, dim);
	  mis_node ** newBlocks = new mis_node*[nblocks];
	  for (int i = 0; i < nblocks; i++) {
	    newBlocks[i] = new mis_node;
	    tmpPoint->subspaces.push_front(newBlocks[i]);
	    newBlocks[i]->parent = tmpPoint;
	    newBlocks[i]->boundaries = new double[2*dim];
	    newBlocks[i]->maxSpacing = 0;
	    newBlocks[i]->depth = tmpPoint->depth + 1;
	  }
	  int curdigit = 1;
	  for (int d = 0; d < dim; d++) {
	    curdigit = curdigit * 2;
	    for (int i = 0; i < nblocks; i++) {
	      if (i % curdigit == i) { //indicates that it is the one we'll have on the "left" boundary, otherwise is on the "right" boundary.
		newBlocks[i]->boundaries[2*d] = tmpPoint->boundaries[2*d];
		newBlocks[i]->boundaries[2*d+1] = (tmpPoint->boundaries[2*d] + tmpPoint->boundaries[2*d+1])/2;
	      } else {
		newBlocks[i]->boundaries[2*d] = (tmpPoint->boundaries[2*d] + tmpPoint->boundaries[2*d+1])/2;
		newBlocks[i]->boundaries[2*d+1] = tmpPoint->boundaries[2*d+1];
	      }
	    }
	  }
	  std::list<ALE::Mesh::point_type>::iterator p_iter = tmpPoint->childPoints.begin();
	  std::list<ALE::Mesh::point_type>::iterator p_iter_end = tmpPoint->childPoints.end();
	  while (p_iter != p_iter_end) {
	    double ch_space = *spacing->restrict(rPatch, *p_iter);
	    const double * cur_coords = coords->restrict(rPatch, *p_iter);
	    int index = 0, change = 1;
	    for (int d = 0; d < dim; d++) {
	      if ((tmpPoint->boundaries[2*d] + tmpPoint->boundaries[2*d+1])/2 > cur_coords[d]) index += change;
	      change = change * 2;
	    }
	    randPush(&newBlocks[index]->childPoints, *p_iter);
	    if(ch_space > newBlocks[index]->maxSpacing) newBlocks[index]->maxSpacing = ch_space;
	    p_iter++;
	  }
	  p_iter = tmpPoint->childColPoints.begin();
	  p_iter_end = tmpPoint->childColPoints.end();
		//add the points to the 
	  while (p_iter != p_iter_end) {
	    double ch_space = *spacing->restrict(rPatch, *p_iter);
	    const double * cur_coords = coords->restrict(rPatch, *p_iter);
	    int index = 0, change = 1;
	    for (int d = 0; d < dim; d++) {
	      if ((tmpPoint->boundaries[2*d] + tmpPoint->boundaries[2*d+1])/2 > cur_coords[d]) index += change;
	      change = change * 2;
	    }
	    if(ch_space > newBlocks[index]->maxSpacing) newBlocks[index]->maxSpacing = ch_space;
	    randPush(&newBlocks[index]->childColPoints, *p_iter);
	    p_iter++;
	  }
		//add all the new blocks to the refinement queue.
	  for (int i = 0; i < nblocks; i++) {
	    mis_queue.push_front(newBlocks[i]);
	  }

	} else {  //ending refinement if
	  tmpPoint->isLeaf = true;
	  leaf_list.push_front(tmpPoint); 
	} //ending leaf else
      } //ending while for building queue.

      std::list<ALE::Mesh::point_type> globalNodes; //the list of global nodes that have been accepted.

	//now, we must traverse the tree and determine what, if any conflicts exist in a greedy way.  First traverse all the way to the roots.
      std::list<mis_node *>::iterator leaf_iter = leaf_list.begin();
      std::list<mis_node *>::iterator leaf_iter_end = leaf_list.end();
      //PetscPrintf(mesh->comm(), "- created %d comparison spaces\n", leaf_list.size());
      while (leaf_iter != leaf_iter_end) {
	    //we must now traverse the tree in such a way as to determine what collides with this leaf and what to do about it.
	std::list<mis_node *> comparisons; //dump the spaces that will be directly compared to cur_point in here.
	std::list<mis_node *> mis_travQueue; //the traversal queue.
	    // go top-down with the comparisons.
	mis_node * cur_leaf = *leaf_iter;
	mis_travQueue.push_front(root);
	while(!mis_travQueue.empty()) {
	  mis_node * trav_node = *mis_travQueue.begin();
	  mis_travQueue.pop_front();
	  if (trav_node->isLeaf && (trav_node != *leaf_iter)) { //add this leaf to the comparison list.
	    comparisons.push_front(trav_node);
	  } else { //for non-leafs we compare the children using the same heuristic, namely if there could be any possible collision between the two.
	    std::list<mis_node *>::iterator child_iter = trav_node->subspaces.begin();
	    std::list<mis_node *>::iterator child_iter_end = trav_node->subspaces.end();
	    while(child_iter != child_iter_end) {
	      if(isOverlap(*child_iter, *leaf_iter, dim))mis_travQueue.push_front(*child_iter);
	      child_iter++;
	    }
	  } //end what to do for non-leafs
	} //end traversal of tree to determine adjacent sections
	//PetscPrintf(mesh->comm(), "Region has %d adjacent sections; comparing\n", comparisons.size());
	    //now loop over the adjacent areas we found to determine the MIS within *leaf_iter with respect to its neighbors.
	    //begin by looping over the vertices in the leaf.
	std::list<ALE::Mesh::point_type>::iterator l_points_iter = cur_leaf->childPoints.begin();
	std::list<ALE::Mesh::point_type>::iterator l_points_iter_end = cur_leaf->childPoints.end();
	while (l_points_iter != l_points_iter_end) {
	  bool l_is_ok = true;
	  double l_coords[dim];
	  PetscMemcpy(l_coords, coords->restrict(rPatch, *l_points_iter), dim*sizeof(double));
	  double l_space = *spacing->restrict(rPatch, *l_points_iter);

		//internal consistency check; keeps us from having to go outside if we don't have to.
	  std::list<ALE::Mesh::point_type>::iterator int_iter = cur_leaf->childColPoints.begin();
	  std::list<ALE::Mesh::point_type>::iterator int_iter_end = cur_leaf->childColPoints.end();
	  while (int_iter != int_iter_end && l_is_ok) {
	    double i_coords[dim];
	    double dist = 0;
	    PetscMemcpy(i_coords, coords->restrict(rPatch, *int_iter), dim*sizeof(double));
	    double i_space = *spacing->restrict(rPatch, *int_iter);
	    for (int d = 0; d < dim; d++) {
	      dist += (i_coords[d] - l_coords[d])*(i_coords[d] - l_coords[d]);
	    }
	    double mdist = i_space + l_space;
	    if (dist < beta*beta*mdist*mdist/4) l_is_ok = false;
	    int_iter++;
	  }
		//now we must iterate over the adjacent spaces as determined before.
	  std::list<mis_node *>::iterator comp_iter = comparisons.begin();
	  std::list<mis_node *>::iterator comp_iter_end = comparisons.end();
	  while (comp_iter != comp_iter_end && l_is_ok) {
	    mis_node * cur_comp = *comp_iter;
	    std::list<ALE::Mesh::point_type>::iterator adj_iter = cur_comp->childColPoints.begin();
	    std::list<ALE::Mesh::point_type>::iterator adj_iter_end = cur_comp->childColPoints.end();
	    while (adj_iter != adj_iter_end && l_is_ok) {
	      double a_coords[dim];
	      double dist = 0;
	      PetscMemcpy(a_coords, coords->restrict(rPatch, *adj_iter), dim*sizeof(double));
	      double a_space = *spacing->restrict(rPatch, *adj_iter);
	      for (int d = 0; d < dim; d++) {
		dist += (a_coords[d] - l_coords[d])*(a_coords[d] - l_coords[d]);
	      }
	      double mdist = l_space + a_space;
	      if (dist < beta*beta*mdist*mdist/4) l_is_ok = false;
	      adj_iter++;
	    }
	    comp_iter++;
	  }
	  if (l_is_ok) {  //this point has run the gambit... cool.
	    cur_leaf->childColPoints.push_front(*l_points_iter);
	  }
	  l_points_iter++;
	} //end while over points

	    //we can now dump the accepted list of points from the current leaf into the global list
	std::list<ALE::Mesh::point_type>::iterator accept_iter = cur_leaf->childColPoints.begin();
	std::list<ALE::Mesh::point_type>::iterator accept_iter_end = cur_leaf->childColPoints.end();
	while (accept_iter != accept_iter_end) {
	  globalNodes.push_front(*accept_iter);
	  accept_iter++;
	} 
	comparisons.clear(); //we need to remake this list the next time around.
	leaf_iter++;
      } //end while over leaf spaces; after this point we have a complete MIS in globalNodes
      PetscPrintf(mesh->comm(), "- Accepted %d nodes\n", globalNodes.size());
      
      //clean up the tree
      mis_queue.clear();
      mis_queue.push_front(root);
      while (!mis_queue.empty()) {
	mis_node * currentPoint = *mis_queue.begin();
	mis_queue.pop_front();
	std::list<mis_node *>::iterator clean_iter = currentPoint->subspaces.begin();
	std::list<mis_node *>::iterator clean_iter_end = currentPoint->subspaces.end();
	while(clean_iter != clean_iter_end) {
	  mis_queue.push_front(*clean_iter);
	  clean_iter++;
	}
	delete currentPoint;
      }
      
      //tree is properly deallocated... continue.
      
      triangulateio * input = new triangulateio;
      triangulateio * output = new triangulateio;
  
      input->numberofpoints = globalNodes.size();
      input->numberofpointattributes = 0;
      input->pointlist = new double[dim*input->numberofpoints];

  //copy the points over
      std::list<ALE::Mesh::point_type>::iterator c_iter = globalNodes.begin(), c_iter_end = globalNodes.end();

      int index = 0;
      while (c_iter != c_iter_end) {
	PetscMemcpy(input->pointlist + dim*index, coords->restrict(rPatch, *c_iter), dim*sizeof(double));
	c_iter++;
	index++;
      }

  //ierr = PetscPrintf(srcMesh->comm(), "copy is ok\n");
      input->numberofpointattributes = 0;
      input->pointattributelist = NULL;

//set up the pointmarkerlist to hold the names of the points

      input->pointmarkerlist = new int[input->numberofpoints];
      c_iter = globalNodes.begin();
      c_iter_end = globalNodes.end();
      index = 0;
      while(c_iter != c_iter_end) {
	input->pointmarkerlist[index] = *c_iter;
	c_iter++;
	index++;
      }


      input->numberoftriangles = 0;
      input->numberofcorners = 0;
      input->numberoftriangleattributes = 0;
      input->trianglelist = NULL;
      input->triangleattributelist = NULL;
      input->trianglearealist = NULL;

      input->segmentlist = NULL;
      input->segmentmarkerlist = NULL;
      input->numberofsegments = 0;

      input->holelist = NULL;
      input->numberofholes = 0;
  
      input->regionlist = NULL;
      input->numberofregions = 0;

      output->pointlist = NULL;
      output->pointattributelist = NULL;
      output->pointmarkerlist = NULL;
      output->trianglelist = NULL;
      output->triangleattributelist = NULL;
      output->trianglearealist = NULL;
      output->neighborlist = NULL;
      output->segmentlist = NULL;
      output->segmentmarkerlist = NULL;
      output->holelist = NULL;
      output->regionlist = NULL;
      output->edgelist = NULL;
      output->edgemarkerlist = NULL;
      output->normlist = NULL;

      string triangleOptions = "-zeQ"; //(z)ero indexing, output (e)dges, Quiet
      triangulate((char *)triangleOptions.c_str(), input, output, NULL);
      TriangleToMesh(mesh, output, patch);
      delete input->pointlist;
      delete output->pointlist;
      delete output->trianglelist;
      delete output->edgelist;
      delete input;
      delete output;
      PetscFunctionReturn(0);
    }  //ending tree_mis

    bool isOverlap(mis_node * a, mis_node * b, int dim) { //see if any two balls in the two sections could overlap at all.
      int sharedDim = 0;
      for (int i = 0; i < dim; i++) {
	if((a->boundaries[2*i] - a->maxSpacing <= b->boundaries[2*i+1] + b->maxSpacing) && (b->boundaries[2*i] - b->maxSpacing <= a->boundaries[2*i+1] + a->maxSpacing)) sharedDim++;
      }
      if (sharedDim == dim) {return true;
      } else return false;
    }
    void randPush(std::list<ALE::Mesh::point_type> * p_list, ALE::Mesh::point_type p) {
      int r = std::rand();
      if (r > (RAND_MAX)/2) { p_list->push_front(p);
      } else p_list->push_back(p);
      return;
    }
  } //ending Coarsener
}  //ending ALE
