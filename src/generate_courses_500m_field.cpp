//
//  generate_courses.cpp
//  AU_UAV_ROS
//
//  Created by Tyler Young on 6/11/11.
//
// Used to create a course file for use with ROS.


#include <cstdlib>
#include <cassert>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>

std::string double_to_text(const double & d)
{
  std::stringstream ss;
  ss << std::setprecision( std::numeric_limits<double>::digits10+2);
  ss << d;
  return ss.str();
}

#ifndef TO_STRING
#define TO_STRING
template <class T>
inline std::string to_string( const T& t )
{
  std::stringstream ss;
  ss << t;
  return ss.str();
}
#endif

using namespace std;

#ifndef natural
#define natural unsigned int
#endif

// The following vars are required to define a position
// These are const because they remain constant for our airfield.
const double upper_left_longitude = -115.808173;
const double upper_left_latitude = 37.244956;
const double width_in_degrees_longitude = 0.005653;
const double height_in_degrees_latitude = -0.004516;

int main()
{
  int seed = 803;
  natural num_planes = 32;
  natural num_waypts = 20;
  natural min_alt = 1400; // These two vars define the altitude range we'll give to
  natural max_alt = 1401; // our aircraft.
  
  // The directory in which to save the courses
  string course_dir = "/Volumes/DATA/Dropbox/school/Auburn/Code/AU_UAV_stack/AU_UAV_ROS/courses/";
  //string course_dir = "/mnt/hgfs/Dropbox/school/Auburn/Code/AU_UAV_stack/AU_UAV_ROS/courses/";
  //string course_dir = "/home/trescenzi/Dropbox/Auburn/Code/AU_UAV_stack/AU_UAV_ROS/courses/";
  string name = "final_" + to_string(num_planes) + "_500m_mormon";
  char defaults;
  
  cout << "You're creating a course on the 500 m by 500 m field." << endl << endl;
  cout << "Defaults are a random seed of " << seed << ", using " << num_planes << " planes," << endl;
  cout << num_waypts << " waypoints, with heights between " << min_alt << " and " << max_alt << "," << endl;
  cout << "and a file name and path of:"<< endl;
  cout << "   " << course_dir << name << ".course" << endl << endl;
  cout << "Use defaults (y/n)?  ";
  cin >> defaults;
  if( defaults == 'n' || defaults == 'N')
  {
    cout << "Random seed: ";
    cin >> seed;
    
    cout << "Number of planes to generate (start counting at 1):  ";
    cin >> num_planes;
    assert( num_planes < 10000 && num_planes >= 1 );
    
    cout << "Number of waypoints per plane:  ";
    cin >> num_waypts;
    assert( num_waypts >= 1 && num_waypts < 1000000000 );
    
    cout << "Minimum assigned altitude:  ";
    cin >> min_alt;
    assert( min_alt < 1000000 );
    
    cout << "Maximum assigned altitude:  ";
    cin >> max_alt;
    assert( max_alt < 1000000 );
    
    cout << "File path (e.g., /home/UserName/Desktop/):  ";
    cin >> course_dir;
    
    cout << "File name (excluding path and excluding .course suffix):  ";
    cin >> name;
  }
  srand ( seed );


  // Build the filename string
  stringstream ss( stringstream::out );
  ss << course_dir;
  ss << ( name.empty() ? "a" : name.c_str() ) << ".course";
  string name_with_path = ss.str();
  
  // Open the file
  ofstream course_file;
  course_file.open( name_with_path.c_str() );
  assert( course_file.is_open() );
  
  // Write the header
  course_file << "# A course file generated by Tyler Young's autogenerator" << "\n";
  course_file << "# By default, all points are located in the \"FINAL\" test field." << "\n";
  course_file << "# Data format:" << "\n";
  course_file << "#     planeID latitude longitude altitude" << "\n";
  course_file << "# " << "\n";
  course_file << "# (As you may have noticed, a '#' begins a commented line)" << "\n";
  course_file << "# (Also, blank lines are allowed in these files)" << "\n";
  course_file << "# " << "\n";
  course_file << "# Since files are parsed linearly, you should start out by" << "\n";
  course_file << "# initializing all aircraft's positions. Then you can cluster" << "\n";
  course_file << "# points by aircraft ID, as seen below." << "\n\n";
  course_file << "# Note: This file was created with the following settings:\n";
  course_file << "#     Random seed " << seed << "\n";
  course_file << "#     Number of planes " << num_planes << "\n";
  course_file << "#     Number of waypoints " << num_waypts << "\n";
  course_file << "#     Altitude between " << min_alt << " and " << max_alt << "\n";
  course_file << "#  \n";
  course_file << "#     Latitude between " << double_to_text(upper_left_latitude + height_in_degrees_latitude) << " and " << double_to_text(upper_left_latitude) << "\n";
  course_file << "#     Longitude between " << double_to_text(upper_left_longitude) << " and " << double_to_text(upper_left_longitude + width_in_degrees_longitude) << "\n\n";

  
  // Vars to use in randomizing
  double longitude;
  double latitude;
  double altitude;
  
  // Write the starting positions
  course_file << "# Starting positions:" << "\n";
  course_file << "#ID\t\tLat\t\t\tLong\t\t\tAlt\n";
  for( natural id = 0; id < num_planes; id++ )
  {
    longitude = upper_left_longitude + ( (double)( rand() % 5652999 ) / 1000000000 );
    latitude = upper_left_latitude - ( (double)( rand() % 4515999 ) / 1000000000 );
    altitude = min_alt + rand() % (max_alt - min_alt);
    course_file << id << "\t\t" << double_to_text(latitude) << "\t" <<
    double_to_text(longitude) << "\t" << altitude << "\n";
  }
  
  // Write the rest of the paths
  for( natural id = 0; id < num_planes; id++ )
  {
    course_file << "\n# Plane ID == " << id << "\n";
    course_file << "#ID\t\tLat\t\t\tLong\t\t\tAlt\n";
    for( natural crnt_waypt = 0; crnt_waypt < num_waypts; crnt_waypt++ )
    {
      longitude = upper_left_longitude + ( (double)( rand() % 5652999 ) / 1000000000 );
      latitude = upper_left_latitude - ( (double)( rand() % 4515999 ) / 1000000000 );
      altitude = min_alt + rand() % (max_alt - min_alt);
      course_file << id << "\t\t" << double_to_text(latitude) << "\t" <<
      double_to_text(longitude) << "\t" << altitude << "\n";
    }
  }
  
  course_file << "\n"; // *nix text files need to end with a newline
  
  course_file.close();
  
  cout << endl << "File created!" << endl;
  cout << "Check the following location for your course file:" << endl;
  cout << name_with_path;
}
