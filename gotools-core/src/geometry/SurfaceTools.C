#include "GoTools/geometry/SurfaceTools.h"
#include "GoTools/geometry/ElementarySurface.h"
#include "GoTools/geometry/BoundedSurface.h"

using std::vector;
using std::setprecision;
using std::endl;
using std::pair;
using std::make_pair;
using std::shared_ptr;
using std::dynamic_pointer_cast;

namespace Go
{

//===========================================================================
CurveLoop outerBoundarySfLoop(std::shared_ptr<ParamSurface> surf,
			      double degenerate_epsilon) 
//===========================================================================
{
  // It is convenient to let boundary loops be described as CurveOnSurface
  // to store as much information as possible. Due to problems with shared_ptr, 
  // this is not possible from within SplineSurface.
  // This function is implemented to get around this problem

  std::shared_ptr<SplineSurface> spline_sf = 
    std::dynamic_pointer_cast<SplineSurface, ParamSurface>(surf);

  std::shared_ptr<BoundedSurface> bd_sf = 
    std::dynamic_pointer_cast<BoundedSurface, ParamSurface>(surf);

  if (bd_sf.get())
    return bd_sf->outerBoundaryLoop(degenerate_epsilon);
  else 
    {
      // Test for degeneracy.
      bool deg[4];
      if (degenerate_epsilon < 0.0)
	deg[0] = deg[1] = deg[2] = deg[3] = false;
      else
	surf->isDegenerate(deg[0], deg[1], deg[2], deg[3], degenerate_epsilon);

      RectDomain dom = surf->containingDomain();
      vector< shared_ptr< ParamCurve > >  vec;
      int pardir[] = {2, 1, 2, 1};
      int boundary[] = {2, 1, 3, 0};
      double parval[] = {dom.vmin(), dom.umax(), dom.vmax(), dom.umin()};
      if (spline_sf.get())
	{
	  // Spline surface
	  for (int edgenum = 0; edgenum < 4; ++edgenum) {
	    if (!deg[edgenum])
	      {
		// Fetch geometry curve
		shared_ptr<ParamCurve> edgecurve(spline_sf->edgeCurve(edgenum));

		// Construct curve on surface with knowledge about what it is
		shared_ptr<ParamCurve> sfcv = 
		  shared_ptr<ParamCurve>(new CurveOnSurface(surf, edgecurve, 
							    pardir[edgenum], 
							    parval[edgenum], 
							    boundary[edgenum]));
		if (edgenum == 2 || edgenum == 3)
		  sfcv->reverseParameterDirection();
		vec.push_back(sfcv);
	      }
	  }
	}
      else
	{
	  // The boundary loop of non bounded surfaces misses information
	  // about the surface and parameter curves. This information must
	  // be added
	  CurveLoop cv_loop = surf->outerBoundaryLoop(degenerate_epsilon);
	  int nmb_cvs = cv_loop.size();
	  if (nmb_cvs == 0)
	    return cv_loop;

	  shared_ptr<CurveOnSurface> cv = 
	    dynamic_pointer_cast<CurveOnSurface,ParamCurve>(cv_loop[0]);
	  if (cv.get())
	    return cv_loop; // Already curve on surface curves
	  
	  // Make new loop with curve-on-surface curves 
	  for (int ki=0; ki<nmb_cvs; ++ki)
	    {
	      shared_ptr<ParamCurve> sfcv = 
		shared_ptr<ParamCurve>(new CurveOnSurface(surf, cv_loop[ki], 
							  pardir[ki],
							  parval[ki],
							  boundary[ki]));
	      vec.push_back(sfcv);
	    }
	}

      return CurveLoop(vec, (degenerate_epsilon < 0.0) ? DEFAULT_SPACE_EPSILON :
		       degenerate_epsilon);
    }
}

//===========================================================================
std::vector<CurveLoop> allBoundarySfLoops(std::shared_ptr<ParamSurface> surf,
					  double degenerate_epsilon) 
//===========================================================================
{
  std::shared_ptr<BoundedSurface> bd_sf = 
    std::dynamic_pointer_cast<BoundedSurface, ParamSurface>(surf);

  if (bd_sf.get())
    {
      return bd_sf->absolutelyAllBoundaryLoops();
    }
  else
    {
      // There is only one boundary loop...
      std::vector<CurveLoop> cvloopvec;
      cvloopvec.push_back(outerBoundarySfLoop(surf, degenerate_epsilon));
      return cvloopvec;
    }
}

//===========================================================================
std::vector<CurveLoop> 
absolutelyAllBoundarySfLoops(std::shared_ptr<ParamSurface> surf,
			     double degenerate_epsilon) 
//===========================================================================
{
  std::shared_ptr<BoundedSurface> bd_sf = 
    std::dynamic_pointer_cast<BoundedSurface, ParamSurface>(surf);

  if (bd_sf.get())
    {
      return bd_sf->absolutelyAllBoundaryLoops();
    }
  else
    {
      // There is only one boundary loop...
      // Use a negative degeneracy tolarance to tell that also degenerate
      // boundaries must be included in the loop
      std::vector<CurveLoop> cvloopvec;
      cvloopvec.push_back(outerBoundarySfLoop(surf, -1.0));
      return cvloopvec;
    }
}

//===========================================================================
void 
iterateCornerPos(Point& vertex, 
		 vector<pair<shared_ptr<ParamSurface>, Point> > sfs,
		 double tol)
//===========================================================================
{
  Point prev, curr;
  curr = vertex;
  double wgt_fac = 10.0;
  double wgt = 1.0;
  double wgt_sum;
  int max_iter = 10;
  int kr = 0;

  // Iterate until the vertex point doesn't move
  do
    {
      prev = curr;
      curr.setValue(0.0);
      wgt_sum = 0.0;

      for (size_t ki=0; ki<sfs.size(); ++ki)
	{
	  double seed[2];
	  double clo_u, clo_v, clo_dist;
	  Point clo_pt;
	  seed[0] = sfs[ki].second[0];
	  seed[1] = sfs[ki].second[1];
	  sfs[ki].first->closestPoint(prev, clo_u, clo_v, clo_pt,
				      clo_dist, 0.001*tol, NULL, seed);

	  // Check if the surface is elementary
	  shared_ptr<ElementarySurface> elemsf = 
	    dynamic_pointer_cast<ElementarySurface, ParamSurface>(sfs[ki].first);
	  double curr_wgt = (elemsf.get()) ? wgt*wgt_fac : wgt;
	  curr += curr_wgt*clo_pt;
	  wgt_sum += curr_wgt;

	  sfs[ki].second = Point(clo_u, clo_v);
	}
      curr /= wgt_sum;
      kr++;
      if (kr > max_iter)
	break;
    }
  while (prev.dist(curr) > tol);

  vertex = curr;
}

//===========================================================================
  bool cornerToCornerSfs(shared_ptr<ParamSurface> sf1,
			 shared_ptr<CurveOnSurface> sf_cv1,
			 shared_ptr<ParamSurface> sf2,
			 shared_ptr<CurveOnSurface> sf_cv2,
			 double tol)
//===========================================================================
  {
  int bd1, bd2;  // Specifies the surface boundaries corresponding to 
  // the current edges
  // 0 = umin, 1 = umax, 2 = vmin,  3 = vmax
  bool same_orient1, same_orient2;
  bd1 = sf_cv1->whichBoundary(tol, same_orient1);
  bd2 = sf_cv2->whichBoundary(tol, same_orient2);
  if (bd1 < 0 || bd2 < 0)
    return false;  // Adjacency not along boundary

  // Get surface parameters at corners
  RectDomain dom1 = sf1->containingDomain();
  RectDomain dom2 = sf2->containingDomain();
  double corn1_1[2], corn1_2[2], corn2_1[2], corn2_2[2];
  if (bd1 == 0 || bd1 == 1)
    {
      if (bd1 == 0)
	corn1_1[0] = corn1_2[0] = dom1.umin();
      else
	corn1_1[0] = corn1_2[0] = dom1.umax();
      corn1_1[1] = dom1.vmin();
      corn1_2[1] = dom1.vmax();
    }
  else if (bd1 == 2 || bd1 == 3)
    {
      if (bd1 == 2)
	corn1_1[1] = corn1_2[1] = dom1.vmin();
      else
	corn1_1[1] = corn1_2[1] = dom1.vmax();
      corn1_1[0] = dom1.umin();
      corn1_2[0] = dom1.umax();
    }
  if (bd2 == 0 || bd2 == 1)
    {
      if (bd2 == 0)
	corn2_1[0] = corn2_2[0] = dom2.umin();
      else
	corn2_1[0] = corn2_2[0] = dom2.umax();
      corn2_1[1] = dom2.vmin();
      corn2_2[1] = dom2.vmax();
    }
  else if (bd2 == 2 || bd2 == 3)
    {
      if (bd2 == 2)
	corn2_1[1] = corn2_2[1] = dom2.vmin();
      else
	corn2_1[1] = corn2_2[1] = dom2.vmax();
      corn2_1[0] = dom2.umin();
      corn2_2[0] = dom2.umax();
    }
   
  // Evaluate surface corners
  Point pt1 = sf1->point(corn1_1[0], corn1_1[1]);
  Point pt2 = sf1->point(corn1_2[0], corn1_2[1]);
  Point pt3 = sf2->point(corn2_1[0], corn2_1[1]);
  Point pt4 = sf2->point(corn2_2[0], corn2_2[1]);
  
  if (pt1.dist(pt3) > tol && pt1.dist(pt4) > tol)
    return false;
  if (pt2.dist(pt3) > tol && pt2.dist(pt4) > tol)
    return false;
  if (pt3.dist(pt1) > tol && pt3.dist(pt2) > tol)
    return false;
  if (pt4.dist(pt1) > tol && pt4.dist(pt2) > tol)
    return false;
  
  return true;
  }

//===========================================================================
  bool getSfAdjacencyInfo(shared_ptr<ParamSurface> sf1,
			  shared_ptr<CurveOnSurface> sf_cv1,
			  shared_ptr<ParamSurface> sf2,
			  shared_ptr<CurveOnSurface> sf_cv2,
			  double tol,
			  int& bd1, int& bd2, bool& same_orient)
//===========================================================================
  {
    // bd1, bd2:
    // 0 = umin, 1 = umax, 2 = vmin,  3 = vmax
    bool same_orient1, same_orient2;
    bd1 = sf_cv1->whichBoundary(tol, same_orient1);
    bd2 = sf_cv2->whichBoundary(tol, same_orient2);
    if (bd1 < 0 || bd2 < 0)
      return false;  // Adjacency not along boundary

    Point f1_p1 = sf_cv1->faceParameter(sf_cv1->startparam());
    Point f1_p2 = sf_cv1->faceParameter(sf_cv1->endparam());
    Point f2_p1 = sf_cv2->faceParameter(sf_cv2->startparam());
    Point f2_p2 = sf_cv2->faceParameter(sf_cv2->endparam());
    bool opposite = false;
    Point p1 = sf1->ParamSurface::point(f1_p1[0], f1_p1[1]);
    Point p2 = sf1->ParamSurface::point(f1_p2[0], f1_p2[1]);
    Point p3 = sf2->ParamSurface::point(f2_p1[0], f2_p1[1]);
    Point p4 = sf2->ParamSurface::point(f2_p2[0], f2_p2[1]);
    if ((p2 - p1)*(p4 -p3) < 0.0)
      opposite = true;
    if ((same_orient1 && !same_orient2) || (!same_orient1 && same_orient2))
      opposite = !opposite;
    same_orient = !opposite;
    return true;
   }

//===========================================================================
  bool getCorrCoefEnum(shared_ptr<SplineSurface> sf1,
		       shared_ptr<SplineSurface> sf2,
		       int bd1, int bd2, bool same_orient,
		       vector<pair<int,int> >& enumeration)
//===========================================================================
  {
    int kn1 = sf1->numCoefs_u();
    int kn2 = sf1->numCoefs_v();
    int kn3 = sf2->numCoefs_u();
    int kn4 = sf2->numCoefs_v();

    int nmb1 = (bd1 == 0 || bd1 == 1) ? kn2 : kn1;
    int nmb2 = (bd2 == 0 || bd2 == 1) ? kn4 : kn3;
    if (nmb1 != nmb2)
      return false;  // No correspondence

    enumeration.resize(nmb1);
    int start1 = (bd1 == 0 || bd1 == 2) ? 0 :
      ((bd1 == 1) ? kn1-1 : kn1*(kn2-1));
    int del1 = (bd1 == 0 || bd1 == 1) ? kn1 : 1;

    int start2 = (bd2 == 0 || bd2 == 2) ? 0 :
      ((bd2 == 1) ? kn3-1 : kn3*(kn4-1));
    int del2 = (bd2 == 0 || bd2 == 1) ? kn3 : 1;
    if (!same_orient)
      {
	start2 += (nmb2-1)*del2;
	del2 *= -1;
      }

    int ki, idx1, idx2;
    for (ki=0, idx1=start1, idx2=start2; ki<nmb1; ++ki, idx1+=del1, idx2+=del2)
      enumeration[ki] = make_pair(idx1, idx2);
 
    return true;
 }

//===========================================================================
  bool getCoefEnumeration(std::shared_ptr<SplineSurface> sf, int bd,
			  std::vector<int>& enumeration)
//===========================================================================
  {
    if (bd < 0 || bd > 3)
      return false;

    int kn1 = sf->numCoefs_u();
    int kn2 = sf->numCoefs_v();

    int nmb = (bd == 0 || bd == 1) ? kn2 : kn1;
    enumeration.resize(nmb);
    int start = (bd == 0 || bd == 2) ? 0 :
      ((bd == 1) ? kn1-1 : kn1*(kn2-1));
    int del = (bd == 0 || bd == 1) ? kn1 : 1;

    int ki, idx;
    for (ki=0, idx=start; ki<nmb; ++ki, idx+=del)
      enumeration[ki] = idx;

    return true;
  }

//===========================================================================
// find a good seed for closest point computation
void surface_seedfind(const Point& pt, 
		      const ParamSurface& sf, 
		      const RectDomain* rd,
		      double& u,
		      double& v)
//===========================================================================
{
  // Evaluate a number of points in a rectangular grid and find the
  // closest one
  int nmb_sample = 7;
  vector<Point> samples(7*7);

  // Domain
  RectDomain dom;
  if (rd)
    dom = *rd;
  else
    dom = sf.containingDomain();

  int ki, kj, idx;
  double udel = (dom.umax() - dom.umin())/(int)(nmb_sample-1);
  double vdel = (dom.vmax() - dom.vmin())/(int)(nmb_sample-1);
  double upar, vpar;
  for (idx=0, kj=0, vpar=dom.vmin(); kj<nmb_sample; ++kj, vpar+=vdel)
    for (ki=0, upar=dom.umin(); ki<nmb_sample; ++idx, ++ki, upar+=udel)
	samples[idx] = sf.point(upar, vpar);

  // Find closest sampling point
  int min_idx=0;
  double min_dist = pt.dist(samples[0]);
  double dist;
  for (idx=1; idx<(int)samples.size(); ++idx)
    {
      dist = pt.dist(samples[idx]);
      if (dist < min_dist)
	{
	  min_dist = dist;
	  min_idx = idx;
	}
    }

  // Could check distance to neighbouring sample points and make the
  // seed more precise, but return only the corresponding parameter 
  // value for the time being
  kj = min_idx/nmb_sample;
  ki = (min_idx % nmb_sample);
  u = dom.umin() + ki*udel;
  v = dom.vmin() + kj*vdel;
}

//===========================================================================
  double estimateTangentLength(SplineSurface *surf, int pardir, 
			       bool at_start)
//===========================================================================
  {
    int nmb_sample = 5;
    vector<Point> pts(3);
    double len = 0.0;
    if (pardir == 1)
      {
	double upar = (at_start) ? surf->startparam_u() : surf->endparam_u();
	double vpar = surf->startparam_v();
	double del = (surf->endparam_v() - vpar)/(double)(nmb_sample-1);
	for (int ki=0; ki<nmb_sample; ++ki, vpar+=del)
	  {
	    surf->point(pts, upar, vpar, 1);
	    len += pts[1].length();
	  }
      }
    else
      {
	double upar = surf->startparam_u();
	double vpar = (at_start) ? surf->startparam_v() : surf->endparam_v();
	double del = (surf->endparam_u() - upar)/(double)(nmb_sample-1);
	for (int ki=0; ki<nmb_sample; ++ki, upar+=del)
	  {
	    surf->point(pts, upar, vpar, 1);
	    len += pts[2].length();
	  }
      }
    len /= (double)nmb_sample;
    return len;
  }

} // end namespace Go