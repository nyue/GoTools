#ifndef MESH_2D_H
#define MESH_2D_H

#include <iostream>
#include <vector>
#include <assert.h>
#include "GoTools/geometry/Streamable.h"
#include "GoTools/lrsplines2D/Direction2D.h"
#include "GoTools/lrsplines2D/Mesh2DIterator.h"
#include "GoTools/lrsplines2D/IndexMesh2DIterator.h"

namespace Go
{

// simple structure used for compact encoding of mesh topology.  
struct GPos { 
  // due to Visual Studio 2010 not supporting initializer lists, we have
  // to make an explicit constructor here.
  GPos(int i, int m) : ix(i), mult(m) {}
  GPos() : ix(-1), mult(-1) {}

  int ix; 
  int mult;
};  

// =============================================================================
class Mesh2D : public Streamable 
// =============================================================================
{
public:
  // ---------------------------------------------------------
  // --- CONSTRUCTORS, READING, WRITING AND SWAP FUNCTIONS ---
  // ---------------------------------------------------------
  Mesh2D() {}; // empty mesh
  Mesh2D(std::istream& is); // read mesh from stream

  // Construct a full, 'tensor product' mesh, based on two knotvectors, expressed
  // in the provided ranges [kx_start, kx_end] and [ky_start, ky_end].  Multiplicities > 1
  // are allowed, and expressed by repeated values in the ranges.
  template<typename Iterator> 
  Mesh2D(Iterator kx_start, Iterator kx_end, Iterator ky_start, Iterator ky_end);
 
  // Construct a full, 'tensor product' mesh, based on two knotvectors, expressed
  // in the provided 1-D arrays 'xknots' and 'yknots'.  The arrays must have begin()
  // and end() member methods.  Multiplicities > 1 are allowed, and expressed by repeated
  // values.
  template<typename Array>
  Mesh2D(const Array& xknots, const Array& yknots);
  
  // Read the mesh from a stream
  virtual void read(std::istream& is);        

  // Write the mesh to a stream
  virtual void write(std::ostream& os) const; 

  // Swap two meshes
  void swap(Mesh2D& rhs);             

  // -----------------------
  // --- QUERY FUNCTIONS ---
  // -----------------------

  // The 'nu'-operator (c.f. definition in LR-spline paper).  Expresses, for a given consecutive
  // set of meshrectangles, the lowest multiplicity found therein (the lowest possible is 
  // 0, which can be interpreted as 'no meshrectangle').  The set of meshrectangles to be
  // examined is defined by the parameters 'd', 'ix', 'start' and 'end', and the resulting
  // lowest multiplicity is given as the return value.
  // The parameters mean:
  // d     - direction of the consecutive set of meshrectangles (are they lying on a row [YFIXED] 
  //         or on a column [XFIXED]).
  // ix    - the index of the column [XFIXED] or row [YFIXED] on which the meshrectangles are lying
  // start - the row (or column) index of the first meshrectangle of the consecutive set
  // end   - the one-past-end index of the last meshrectangle of the consecutive set
  int nu(Direction2D d, int ix, int start, int end) const;

  // Get the number of distinct knot valuess in a given direction (rows: YFIXED, columns: XFIXED).
  // Note that this is the number of _distinct_ knots, so multiplicities are not taken into
  // account.
  int numDistinctKnots(Direction2D d) const;
  
  // Return the knot value for the knot with index 'ix' along direction 'd'
  // (rows: YFIXED, columns: XFIXED).
  double kval(Direction2D d, int ix) const;

  // Get the lowest knot value (i.e. the first knot value in the knot vector), along a given
  // direction.
  double minParam(Direction2D d) const;

  // Get the highest knot value (i.e. the last knot value in the knot vector), along a given
  // direction.
  double maxParam(Direction2D d) const;

  // Get a pointer to the start of the knot vector in the given direction.
  const double* const knotsBegin(Direction2D d) const;
  
  // Get a pointer to the one-past-end of the knot vector in the given direction.
  const double* const knotsEnd  (Direction2D d) const;

  // Fetch the knot vector of the curve corresponding to a given row or column
  // Multiplicity is included
  // d  - determine whether to examine a row (YFIXED) or a column (XFIXED)
  // ix - index of row/column from which to fetch the knot vector
  std::vector<double> getKnots(Direction2D d, int ix) const;

  // Determine the length of the longest k-meshrectangle with multiplicity 
  // (at least) 'mult' and with starting point at 'start'.
  int extent(Direction2D d, int ix, int start, int mult) const;

  // Find the largest multiplicity of any of the meshrectangles on a given row or column.
  // d  - determine whether to look at a row (YFIXED) or column (XFIXED)
  // ix - index of the row/column to examine.
  int largestMultInLine(Direction2D d, int ix) const; 

  // Find the minimum multiplicity of any of the meshrectangles on a given row or column.
  // d  - determine whether to look at a row (YFIXED) or column (XFIXED)
  // ix - index of the row/column to examine.
  int minMultInLine(Direction2D d, int ix) const; 

  // For a given row (or column) find all consecutive segments of meshrectangles with multiplicities
  // greater than or equal to a given threshold. Each found segment is represented as an integer pair,
  // representing the start index of the first meshrectangle in the segment and the one-past-end index
  // of the last meshrectangle in the segment.
  // d  - determine whether to examine a row (YFIXED) or a column (XFIXED)
  // ix - index of row/column to examine
  // threshold - the multiplicity threshold.  Default is one, which will give all segments of
  //             consecutive meshrectangles (considering those with multiplicity '0' to be nonexistent)
  std::vector<std::pair<int, int> > segments(Direction2D dir, int ix, int threshold = 1) const;
  
  // Returns Mesh2DIterators referring to the first (begin) and
  // one-past-end (end) elements of the mesh. Mesh2DIterators can be
  // used to loop over elements in a mesh.
  Mesh2DIterator begin() const;
  Mesh2DIterator end() const;  

  // Returns IndexMesh2DIterators referring to the first
  // (indexMeshBegin) and to one-past-end (indexMeshEnd) element of
  // the index mesh (which is basically the mesh when counting the
  // knots with multiplicity). The IndexMesh2DIterator may be used for
  // looping over index elements. This is useful for check of linear
  // independence (where it must be examined which basis functions have
  // support on which index elements).
  IndexMesh2DIterator indexMeshBegin() const;
  IndexMesh2DIterator indexMeshEnd() const;

  // Index of first mesh rectangle in multiplicity vector
  int firstMeshVecIx(Direction2D d) const;
  // Index of last mesh rectangle in multiplicity vector
  int lastMeshVecIx(Direction2D d) const;
  
  // ----------------------
  // --- EDIT FUNCTIONS --- 
  // ----------------------

  // set the multiplicity of a consecutive set of meshrectangles.  
  // The consecutive set is specified by:
  // d  - the direction (a row: YFIXED, a column: XFIXED)
  // ix - the index of the row/column of the meshrectangles
  // start - the index to the start of the first consecutive meshrectangle along the line
  // end   - the index to the one-past-end of the last consecutive meshrectangle along the line
  void setMult(Direction2D d, int ix, int start, int end, int mult);

  // increment multiplicity of a consecutive set of meshrectangles by 'mult'
  // The consecutive set is specified by:
  // d  - the direction (a row: YFIXED, a column: XFIXED)
  // ix - the index of the row/column of the meshrectangles
  // start - the index to the start of the first consecutive meshrectangle along the line
  // end   - the index to the one-past-end of the last consecutive meshrectangle along the line
  void incrementMult(Direction2D d, int ix, int start, int end, int mult);

  // Insert a line with X (or Y) fixed at 'kval', and with the
  // indicated multiplicity.  NB, 'kval' should be different from
  // any knot values already in the vector (otherwise it would not be
  // a new line).
  // Returns the index of the newly inserted line.
  // d    - specify whether the line should be parallel to y-axis (XFIXED) or parallel to 
  //        x-axis (YFIXED).  (A line parallel to the y-axis will yield a new knot in the 
  //        x-knotvector and vice versa).
  // kval - the knot value (i.e. parameter value in the 'fixed' direction).  Should be 
  //        different from any value already in the mesh in the given direction.
  // mult - the multiplicity of the meshrectangles on the new line.  (They will all have 
  //        the same multiplicity after insertion, but this can be changed with the 'setMult()' 
  //        and 'incrementMult()' member functions).
  int insertLine (Direction2D d, double kval, int mult = 0);
  
 private:

  // --------------------
  // --- PRIVATE DATA --- 
  // --------------------

  // store the values of the knots in x and y direction (only unique values - multiplicities
  // are accounted for in the grid itself.
  std::vector<double> knotvals_x_;
  std::vector<double> knotvals_y_;

  std::vector<std::vector<GPos> > mrects_x_; // meshrectangles with x constant (|| with y-axis)
  std::vector<std::vector<GPos> > mrects_y_; // meshrectangles with y constant (|| with x-axis)

  // -----------------------
  // --- PRIVATE METHODS ---
  // -----------------------

  // initializer function - used by all constructors
  template<typename Iterator>
  void init_(Iterator kx_start, Iterator kx_end,
	     Iterator ky_start, Iterator ky_end);

  void consistency_check_() const ;
  const std::vector<GPos>& select_meshvec_(Direction2D d, int ix) const;
  std::vector<GPos>& select_meshvec_(Direction2D d, int ix);

  // Utility function that converts a knotvector (represented by the range [kvec_start, kvec_end) )
  // into a compact form consisting of a vector of all the unique values (return value of the 
  // function), and a vector containing all the corresponding multiplicities (argument 'm').
  template<typename Iterator>
  static std::vector<double> compactify_knotvec_(Iterator kvec_start, Iterator kvec_end, std::vector<int>& m);
  
}; // end class Mesh2D

// =============================================================================
template<typename Iterator>
Mesh2D::Mesh2D(Iterator kx_start, Iterator kx_end, Iterator ky_start, Iterator ky_end)
// =============================================================================
{
  init_(kx_start, kx_end, ky_start, ky_end);
}

// =============================================================================
template<typename Array>
Mesh2D::Mesh2D(const Array& xknots, const Array& yknots)
// =============================================================================
{
  init_(xknots.begin(), xknots.end(), yknots.begin(), yknots.end());
}

// =============================================================================
template<typename Iterator>
void Mesh2D::init_(Iterator kx_start, Iterator kx_end, Iterator ky_start, Iterator ky_end)
// =============================================================================
{
  // saving the knotvals (NB: multiplicities should not be saved here, so sequence
  // should be strictly increasing).
  std::vector<int> mult_x, mult_y; // will be used to store multiplicities
  knotvals_x_ = compactify_knotvec_(kx_start, kx_end, mult_x); // sequence of unique knot values returned,
  knotvals_y_ = compactify_knotvec_(ky_start, ky_end, mult_y); // while multiplicities go to 'mult_x'/'y'.

  mrects_x_ = std::vector<std::vector<GPos> >(knotvals_x_.size(), std::vector<GPos>(1, GPos(0, 1)));
  mrects_y_ = std::vector<std::vector<GPos> >(knotvals_y_.size(), std::vector<GPos>(1, GPos(0, 1)));

  // here we also checks that the grid is nonempty, which permits us to do the adjustments
  // further below.
  consistency_check_();

  // setting correct knot multiplicites
  for (int i = 0; i != numDistinctKnots(XFIXED); ++i) 
    setMult(XFIXED, i, 0, numDistinctKnots(YFIXED) - 1, mult_x[i]);
  for (int i = 0; i != numDistinctKnots(YFIXED); ++i) 
    setMult(YFIXED, i, 0, numDistinctKnots(XFIXED) - 1, mult_y[i]);

}

// =============================================================================
template<typename Iterator>
std::vector<double> Mesh2D::compactify_knotvec_(Iterator kvec_start, Iterator kvec_end, std::vector<int>& mult) 
// =============================================================================
{
  // simple and inefficient implementation.  Optimization should not matter 
  // much here anyway, so the priority is on clear code.

  // Establishing the 'compact' knotvector without any explicit multiplicities
  std::vector<double> result;
  unique_copy(kvec_start, kvec_end, std::back_inserter(result));

  // counting multiplicities and keeping track of them in the 'mult' vector
  mult.clear();  
  for (auto i = result.begin(); i != result.end(); ++i) 
    mult.push_back(count(kvec_start, kvec_end, *i));

  return result;
}

// =============================================================================
inline double Mesh2D::minParam(Direction2D d) const
// =============================================================================
{
  return (d == XFIXED) ? knotvals_x_.front() : knotvals_y_.front();
}

// =============================================================================
inline double Mesh2D::maxParam(Direction2D d) const
// =============================================================================
{
  return (d == XFIXED) ? knotvals_x_.back() : knotvals_y_.back();
}

// =============================================================================
inline int Mesh2D::numDistinctKnots(Direction2D d) const 
// =============================================================================
{
  return (d == XFIXED) ? (int)knotvals_x_.size() : (int)knotvals_y_.size();
};

inline int Mesh2D::firstMeshVecIx(Direction2D d) const 
// =============================================================================
{
  return 0;
};

inline int Mesh2D::lastMeshVecIx(Direction2D d) const 
// =============================================================================
{
  return (d == XFIXED) ? (int)mrects_x_.size()-1 : (int)mrects_y_.size()-1;
};

// =============================================================================
inline double Mesh2D::kval(Direction2D d, int ix) const
// =============================================================================
{
  return (d == XFIXED) ? knotvals_x_[ix] : knotvals_y_[ix];
}

// =============================================================================
inline const std::vector<GPos>& Mesh2D::select_meshvec_(Direction2D d, int ix) const
// =============================================================================
{
  assert( (d == XFIXED) || (d == YFIXED));
  return (d == XFIXED) ? mrects_x_[ix] : mrects_y_[ix];
}

// =============================================================================
inline std::vector<GPos>& Mesh2D::select_meshvec_(Direction2D d, int ix)
// =============================================================================
{
  assert( (d == XFIXED) || (d == YFIXED));
  return (d == XFIXED) ? mrects_x_[ix] : mrects_y_[ix];
}

//==============================================================================
inline const double* const Mesh2D::knotsBegin(Direction2D d) const
//==============================================================================
{
  return (d == XFIXED) ? &knotvals_x_[0] : &knotvals_y_[0];
}

//==============================================================================
inline const double* const Mesh2D::knotsEnd(Direction2D d) const
//==============================================================================
{
  return knotsBegin(d) + numDistinctKnots(d);
}

// =============================================================================
inline Direction2D flip(Direction2D d) 
// =============================================================================
{ 
  return (d == XFIXED) ? YFIXED : XFIXED;
} 

// defining streaming operators
inline std::ostream& operator<<(std::ostream& os, const GPos& g)  { return os << g.ix << " " << g.mult << " ";}
inline std::istream& operator>>(std::istream& is, GPos& g)        { return is >> g.ix >> g.mult;}
inline std::ostream& operator<<(std::ostream& os, const Mesh2D& m){ m.write(os); return os;}
inline std::istream& operator>>(std::istream& is, Mesh2D& m)      { m.read(is); return is;}

}; // end namespace Go

#endif 

