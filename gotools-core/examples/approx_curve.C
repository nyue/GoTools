/*
 * Copyright (C) 1998, 2000-2007, 2010, 2011, 2012, 2013 SINTEF ICT,
 * Applied Mathematics, Norway.
 *
 * Contact information: E-mail: tor.dokken@sintef.no                      
 * SINTEF ICT, Department of Applied Mathematics,                         
 * P.O. Box 124 Blindern,                                                 
 * 0314 Oslo, Norway.                                                     
 *
 * This file is part of GoTools.
 *
 * GoTools is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version. 
 *
 * GoTools is distributed in the hope that it will be useful,        
 * but WITHOUT ANY WARRANTY; without even the implied warranty of         
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with GoTools. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * In accordance with Section 7(b) of the GNU Affero General Public
 * License, a covered work must retain the producer line in every data
 * file that is created or manipulated using GoTools.
 *
 * Other Usage
 * You can be released from the requirements of the license by purchasing
 * a commercial license. Buying such a license is mandatory as soon as you
 * develop commercial activities involving the GoTools library without
 * disclosing the source code of your own applications.
 *
 * This file may be used in accordance with the terms contained in a
 * written agreement between you and SINTEF ICT. 
 */

#include "GoTools/creators/ApproxCurve.h"
#include "GoTools/geometry/SplineCurve.h"
#include "GoTools/utils/Point.h"
#include <vector>
#include <fstream>

using std::cout;
using std::endl;
using std::vector;
using std::ofstream;
using namespace Go;

//===========================================================================
//                                                                           
// File: approx_curve.C                                                       
//                                                                           
/// Description:
///
/// This program demonstrates the use of the class ApproxCurve.
/// The class can generate a B-spline curve that approximates a set
/// of parametrized points for a given accuracy by inserting new knots in
/// the curve until the required accuracy is reached.
/// The point and parameter sets are generated by this program. 
///
/// Input to this program from the command line is the accuracy. (The maximum
/// allowed distance from one of the points to the curve.)
///
/// Output is a file in Go-format for plotting the points and the curves.
/// The file name is hard-coded to "approx_curve.g2"

//===========================================================================

int main(int argc, char** argv)
{
    if (argc != 2)
    {
      cout << "\nUsage: " << argv[0]
	   << " tolerance\n" << endl;
      exit(-1);
    }

    cout << "\nRunning program " << argv[0] << " with tolerance = " << argv[1]
	 << endl;
    // Define space dimension, points and parameter values.
    int dim   = 3;  // Space dimension. 
    int numpt = 9;  // Number of input points.
    double start_param = 0.0;
    double end_param   = 2*M_PI;
    double dt = (end_param-start_param)/(numpt-1);
    std::vector<double> param(numpt);  // Parameter values
    // Points. Consecutively in "xyzxyzxyz-fashion".
    std::vector<double> points(numpt * dim);
    for (int i = 0; i < numpt; ++i) {
	double t = i*dt;
	param[i] = t;
	points[i*dim + 0] = t;       // x
	points[i*dim + 1] = sin(t);  // y
	points[i*dim + 2] = cos(t);  // z
    }

    // Get geometric tolerance from argument list.    
    double aepsge = atof(argv[1]);

    // -------------------------------------------------------------------------
    // Approximate a curve through the points. Constructor 1.
    // The user specifies a set of parameterized points and a tolerance.
    // The generated curve will have a spline basis of order 4 (cubic).
    // The number of control points is equal to one-sixth of the number of
    // input points, but at least four at the start.
    // The knotvector of the curve's basis will be set to uniform.
    ApproxCurve approx_curve1(points, param, dim, aepsge);

    // Get the spline curve.    
    double maxdist;   // Maximum distance between the generated curve and
                      // the data points.
    double avdist;    // Average distance between the generated curve and
                      // the data points.
    int max_iter = 5; // Maximum number of iterations to use.
    shared_ptr<SplineCurve> curve = 
	approx_curve1.getApproxCurve(maxdist, avdist, max_iter);
    cout << "\nMaximum distance between curve1 and the data points= "
	 << maxdist << ".  Tolerance= " << aepsge << endl;
    cout << "Average distance between curve1 and the data points= "
	 << avdist << endl;
    cout << "Start parameter = " << curve->startparam() << ".  "
	 << "End parameter = "   << curve->endparam() << ".  "
	 << "Number of control points = " <<  curve->numCoefs() << endl;
    // Write curve to file. Colour=red
    ofstream fout("approx_curve.g2");
    // Class_SplineCurve=100 MAJOR_VERSION=1 MINOR_VERSION=0 auxillary_data=4
    // The four auxillary data values defines the colour (r g b alpha)
    //curve->writeStandardHeader(fout); // write header.
    fout << "100 1 0 4 255 0 0 255" << endl; 
    fout << *curve;    // write spline curve data.

    // Write input points to file. Colour=red
    // Class_PointCloud=400 MAJOR_VERSION=1 MINOR_VERSION=0 auxillary data=4
    fout << "400 1 0 4 255 0 0 255" << endl; // Header.
    fout << numpt << endl;
    for (int i = 0; i < numpt; ++i) {
	int ip = i*dim;
	Go::Point inp_point(points[ip], points[ip+1], points[ip+2]);
	fout << inp_point << endl;  // write input point coordinates.
    }


    // The user may specifiy the start and end points and optionally the 
    // tangents of the approximation curve (if the tangents are not set, this
    // information will result from the smoothing equation).
    // Add new end points and directions.
    vector<Point> start_point(2), end_point(2);
    start_point[0] = Point(-1,-1,0); // Start point 
    start_point[1] = Point( 2, 0,0); // Start direction 
    end_point[0] = Point(7.28, 1,0); // End point   
    end_point[1] = Point(   2, 0,0); // End direction
    approx_curve1.setEndPoints(start_point, end_point);

    // Get the spline curve
    curve = approx_curve1.getApproxCurve(maxdist, avdist, max_iter);
    cout << "\nMaximum distance between curve1_enddir and the data points= "
	 << maxdist << endl;
    cout << "Average distance between curve1_enddir and the data points= "
	 << avdist << endl;
    cout << "Start parameter = " << curve->startparam() << ".  "
	 << "End parameter = "   << curve->endparam() <<  ".  "
	 << "Number of control points = " <<  curve->numCoefs() << endl;
    // Write curve file.  Colour=green.
    fout << "100 1 0 4 0 255 0  255" << endl;
    //curve->writeStandardHeader(fout); // write header.
    fout << *curve;    // write spline curve data.

    // -------------------------------------------------------------------------
    // Approximate a curve through the points. Constructor 2.
    // The user specifies a set of parameterized points and a tolerance as well
    // as order of the resulting spline curve and the number of control points
    // at the start of the iteration.
    // The knotvector of the curve's basis will be set to uniform.
    int ik = 5;  // The order of the resulting spline curve (polynom. degree + 1)
    int in = 5;  // Number of control points at the start of the iteration. At
                 // least equal to order.
    ApproxCurve approx_curve2(points, param, dim, aepsge, in, ik);
    curve = approx_curve2.getApproxCurve(maxdist, avdist, max_iter);
    cout << "\nMaximum distance between curve2 and the data points= "
	 << maxdist << endl;
    cout << "Average distance between curve2 and the data points= "
	 << avdist << endl;
    cout << "Start parameter = " << curve->startparam() << ".  "
	 << "End parameter = "   << curve->endparam() << ".  "
	 << "Number of control points = " <<  curve->numCoefs() << endl;

    // Write curve to file. Colour=blue.
    fout << "100 1 0 4 0 0 255  255" << endl;
    fout << *curve;    // write spline curve data.
    fout.close();

    // cout << "Open the file 'approx_curve.g2' in 'goview' to look at the results"
    //      << endl;

    return 0;
}
