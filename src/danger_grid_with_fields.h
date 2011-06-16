//
// danger_grid.h
// AU_UAV_ROS
//
// Created by Tyler Young on 5/23/11.
// Added to by Thomas Crescinzi.
//
// A class to generate a grid with a "danger" rating associated with
// each block in the flyable area (which is of our "map" type).
//
// The danger grid is three dimensional: it has x and y dimensions,
// corresponding to the size of the flyable physical area. Additionally, it has a
// time dimension.
//
// Thus, to find the danger associated with the (x, y) position (10, 7) at 4 seconds
// in the future, you would call your_danger_grid_variable( 10, 7, 4 ) (which works 
// in virtue of the overloaded parenthesis operator), which would return a double 
// with the "danger rating" at that square.

#ifndef DANGER_GRID
#define DANGER_GRID

#include <vector>
#include "map.h"
#include "estimate.h"
#include "Plane.h"
#include <math.h>
#include <climits>

#ifndef DEFAULT_LOOK_AHEAD
// The default amount of time in the future to "look ahead" when generating the grid;
// If the aircraft that you're working with haven't hit their goal by this time, the
// calculation stops anyway
#define DEFAULT_LOOK_AHEAD 20
#endif

#ifndef PLANE_DANGER
#define PLANE_DANGER 0.98
#endif

using namespace std;

const double PI = 2*acos(0.0);// pi
const double RADtoDEGREES = 180/PI;//Conversion factor from Radians to Degrees

// the amount we'll multiply danger values by when adding the "fuzziness" (the 
// danger around the predicted squares, to keep other aircraft from coming too
// close)
static const double field_weight = 0.5;

class danger_grid
{
public:
  /**
   * The constructor for the danger grid. It will automatically calculate the
   * danger associated with each square in the map up to the default number of
   * seconds in the future.
   * @param set_of_aircraft A vector array containing the aircraft that need to
   * be considered
   * @param the_map A map which is already set up with a width, height, resolution,
   * etc.
   */
  danger_grid( vector< Plane > & set_of_aircraft, const map the_map );
  
  /**
   * The constructor for the danger grid. It will set up a map per the parameters
   * given and then automatically calculate the danger associated with each square
   * in the map up to the default number of seconds in the future.
   *
   * Note that the width, height, and resolution may be in any units, but the units
   * must be consistent across all measurements.
   * @param set_of_aircraft A vector array containing the aircraft that need to
   * be considered
   * @param width The width of the airspace (our x dimension)
   * @param height The height of the airspace (our y dimension)
   * @param resolution The resolution to be used in the map
   */
  danger_grid( vector< Plane > * set_of_aircraft, const double width,
               const double height, const double resolution );
  
  /**
   * The copy constructor; takes a reference to a danger grid and makes this object
   * a duplicate.
   * @param mc A reference to another danger grid
   */
  danger_grid( const danger_grid * dg);
  
  /**
   * Return the danger rating of a square
   * @param x_pos the x position of the square in question
   * @param y_pos the y position of the square in question
   * @param seconds The number of seconds in the future
   * @return a double containing the square's "danger" rating
   */
  double get_danger_at( unsigned int x_pos, unsigned int y_pos, int seconds ) const;
  
  /**
   * Adds to the danger rating of a square
   * @param x_pos the x position of the square in question
   * @param y_pos the y position of the square in question
   * @param seconds The number of seconds in the future
   * @param danger The danger rating to be assigned
   */
  void add_danger_at( unsigned int x_pos, unsigned int y_pos, int seconds,
                      double danger );
  
  /**
   * Sets the danger rating of a square
   * @param x_pos the x position of the square in question
   * @param y_pos the y position of the square in question
   * @param seconds The number of seconds in the future
   * @param danger The danger rating to be assigned
   */
  void set_danger_at( unsigned int x_pos, unsigned int y_pos, int seconds,
                      double danger );
  
  /**
   * The overloaded ( ) operator. Allows simple access to the danger rating of a
   * given square at a specified number of seconds in the future.
   * This performs the same function as get_danger_at()
   * @param x The x location of the square in question
   * @param y The y location of the square in question
   * @param time The number of seconds in the future for which we need the danger
   * rating.
   * @return a double containing the square's "danger" rating
   */
  double operator()( unsigned int x, unsigned int y, int time ) const;
  
  unsigned int get_width_in_squares() const;
  unsigned int get_height_in_squares() const;
  unsigned int get_time_in_secs() const;
  vector< map > get_danger_space() const;
  
  
  /**
   * Modifies the map to store the cost of the path which begins at each square and
   * takes a straight line to the goal, effectively creating a simplified version of
   * a best cost grid.
   * Tyler is adding this here to avoid using a "wrapper" for the straight-line
   * heuristic.
   * @param goal_x The x coordinate for the goal
   * @param goal_y The y coordinate for the goal
   */
  void calculate_distance_costs( unsigned int goal_x, unsigned int goal_y );
  
  /**
   * Output the map at a given time; for troubleshooting only
   * @param time The time, in seconds, whose map should be output
   */
  void dump( int time ) const;
  
  /**
   * Output the map at a given time, scaling all numbers down significantly;
   * for troubleshooting the "bester cost grid"
   * @param time The time, in seconds, whose map should be output
   */
  void dump_big_numbers( int time ) const;
  
private:
  enum bearing_t { N, NE, E, SE, S, SW, W, NW };

  /**
   * The method that does virtually all the important work for the class.
   * Calculates danger ratings for all squares in all maps of the danger_space
   * (where the danger_space is the set of maps corresponding to each second we are
   * looking ahead and behind).
   */
  void fill_danger_space();
  
  /**
   * Set up the weighting scheme for danger ratings in the future.
   * At the moment, this simply decreases the danger linearly as you go farther
   * into the future.
   */
  void set_danger_scale( );
  
  /**
   * Return the danger constant for a given time in the future; multiply a "raw"
   * danger rating by this to scale the rating down based on the uncertainty
   * inherent in predicting the other aircraft's future paths.
   * @param seconds The number of seconds in the future
   * @return a double containing the time's danger constant
   */
  double adjust_danger( int seconds ) const;
  
  /**
   * A function to calculate an aircraft's future positions. The plane's position is
   * extrapolated until it is estimated to reach its goal waypoint.
   * Each second's-worth of estimate is separated by an estimate with danger = -1.
   * @param the_plane The plane object whose future position is being calculated
   * @return A vector of estimates, which are (x, y, danger) triples; these
   * estimates represent a "spread" of probable locations where the
   * aircraft may be at a given second in the future.
   */
  vector< estimate > calculate_future_pos( Plane & the_plane );
  
  /**
   * Converts a bearing in degrees to a "named" version, for use in deciding which
   * nearby squares are in the path of the aircraft
   * @param the_bearing Bearing of the aircraft in degrees (0 is due north, 
   *                    90 due east, and so on)
   * @return A named version of the direction (N for bearings -22.5 to 22.5 deg,
   *         NE for bearings 22.5 to 67.5 deg, and so on)
   */
  bearing_t name_bearing( double the_bearing );
  
  /**
   * Outputs the contents of an "estimate" vector array
   * Useful only for troubleshooting
   * @param dump_me The vector of estimate to be output
   */
  void dump_est( vector< estimate > dump_me );
  
  // Functions by Thomas
  void neighoboringAngles(double angle, double &first, double &second);
  void placeDanger(double angle, vector<estimate> &e, double closest, double other,
                   int x, int y, double danger);//places the data into the estimate struct
  void dangerRecurse(estimate e, int destination[], vector<estimate> &theFuture);
    
  unsigned int look_ahead;
  unsigned int look_behind; // the number of seconds to consider in the past
  
  // the weighting applied to danger estimates in the future
  vector< double > danger_ratings;
  
  // the set of aircraft with which we are concerned
  vector< Plane > * aircraft;
  
  // The danger space is a bit strange due to the fact that it's an array of maps,
  // where each position in the array corresponds to a time.
  vector< map > danger_space;
  
#ifdef OVERLAYED
  vector< map > overlayed; // Used only when dumping output
#endif
};

danger_grid::danger_grid( vector< Plane > & set_of_aircraft, const map the_map )
{
  look_ahead = DEFAULT_LOOK_AHEAD;
  look_behind = 2;
  aircraft = &set_of_aircraft;
  
  // Set up the danger_space
  map set_up( the_map.get_width_in_meters(), the_map.get_height_in_meters(),
              the_map.get_resolution() ); // empty map to fill danger_space with
  
#ifdef OVERLAYED
  overlayed.push_back( map( the_map ) );
#endif
  for( unsigned int i = 0; i <= (look_ahead + look_behind); i++ )
  {
    danger_space.push_back( set_up );
  } // danger_space is now a set of maps, with one map for each second in time that
  // we will work with.
  
  // Set up the danger ratings
  set_danger_scale( );
  
  // Do all the work -- calculate the danger rating for all squares at all times
  fill_danger_space();
}

danger_grid::danger_grid( vector< Plane > * set_of_aircraft, const double width,
                          const double height, const double resolution )
{
  look_ahead = DEFAULT_LOOK_AHEAD;
  look_behind = 2;
  aircraft = set_of_aircraft;
#ifdef DEBUG
  assert( set_of_aircraft->size() != 0 );
  assert( resolution > EPSILON );
  assert( resolution < height && resolution < width );
  assert( height / resolution < 1000000 );
  assert( width / resolution < 1000000 );
#endif
  
#ifdef OVERLAYED
  overlayed.push_back( map( width, height, resolution ) );
#endif
 
  for( unsigned int i = 0; i <= (look_ahead + look_behind); i++ )
  {
    map set_up( width, height, resolution );
    danger_space.push_back( set_up );
  } // danger_space is now a set of maps, with one map for each second in time that
   // we will work with.

  // Set up the danger ratings
  set_danger_scale( );
  
  // Do all the work -- calculate the danger rating for all squares at all times
  fill_danger_space();
}

danger_grid::danger_grid( const danger_grid * dg)
{
  look_ahead = dg->get_time_in_secs();
  look_behind = 2;
  
  danger_space = dg->get_danger_space();
}

void danger_grid::fill_danger_space()
{
  for( vector< Plane >::iterator current_plane = aircraft->begin();
      current_plane != aircraft->end(); ++current_plane )
  {
    danger_space[0 + look_behind].add_danger_at((*current_plane).getLocation().getX(),
                                                (*current_plane).getLocation().getY(), 1.0);
    
#ifdef OVERLAYED
    overlayed[0].add_danger_at((*current_plane).getLocation().getX(),
                               (*current_plane).getLocation().getY(), 1.0);
#endif
  }
  
  vector< estimate > est;
  danger_grid::bearing_t bearing;
  
  // Fill the danger_space for times in the past
  // For each plane . . .
  for( vector< Plane >::iterator current_plane = aircraft->begin();
      current_plane != aircraft->end(); ++current_plane )
  {
    // Get the estimated danger for relevant squares in the map at this time
    est = calculate_future_pos( *current_plane );
    
    bearing = name_bearing( (*current_plane).getBearing() );
    
    int t = 1; // initialize the counter for steps in time (seconds)

    // For each estimated (x, y, danger) triple . . .
    for( vector< estimate >::iterator current_est = est.begin();
        current_est != est.end(); ++current_est )
    {
      if( t <= (int)look_ahead ) // if this estimate is close enough to plan for it . . .
      {
        // If these are legal xs and ys, and if the danger is not a "timestamp" divider
        if( (*current_est).x >= 0 && (*current_est).x <
            (int)danger_space[0].get_width_in_squares() &&
            (*current_est).y >= 0 &&
            (*current_est).y < (int)danger_space[0].get_height_in_squares() &&
            (*current_est).danger > -(EPSILON) )
        {
          // Set the danger of the square based on what
          // calculate_future_pos() found, but scale it according to how
          // far back in time we're predicting
          natural time = t + look_behind;
          natural x = (*current_est).x;
          natural y = (*current_est).y;
          double d = (*current_est).danger * adjust_danger(t);
          
          danger_space[ time ].add_danger_at(x, y, d );
          
          // . . . and then add a bit of "fuzziness" (danger around the predicted
          // square, so that other planes don't come too close)
          d *= field_weight; // adjust the danger for these fields
          switch( bearing )
          {
            case N:
              // straight left
              danger_space[ time ].safely_add_danger_at( x - 1,   y  , d );
              // dag left+up
              danger_space[ time ].safely_add_danger_at( x - 1, y - 1, d );
              // straight up
              danger_space[ time ].safely_add_danger_at(   x  , y - 1, d );
              // dag right+up
              danger_space[ time ].safely_add_danger_at( x + 1, y - 1, d );
              // straight right
              danger_space[ time ].safely_add_danger_at( x + 1,   y  , d );
              break;
            case NE:
              // dag left+up
              danger_space[ time ].safely_add_danger_at( x - 1, y - 1, d );
              // straight up
              danger_space[ time ].safely_add_danger_at(   x  , y - 1, d );
              // dag right+up
              danger_space[ time ].safely_add_danger_at( x + 1, y - 1, d );
              // straight right
              danger_space[ time ].safely_add_danger_at( x + 1,   y  , d );
              // dag right+down
              danger_space[ time ].safely_add_danger_at( x + 1, y + 1, d );
              break;
            case E:
              // straight up
              danger_space[ time ].safely_add_danger_at(   x  , y - 1, d );
              // dag right+up
              danger_space[ time ].safely_add_danger_at( x + 1, y - 1, d );
              // straight right
              danger_space[ time ].safely_add_danger_at( x + 1,   y  , d );
              // dag right+down
              danger_space[ time ].safely_add_danger_at( x + 1, y + 1, d );
              // straight down
              danger_space[ time ].safely_add_danger_at(   x  , y + 1, d );
              break;
            case SE:
              // dag right+up
              danger_space[ time ].safely_add_danger_at( x + 1, y - 1, d );
              // straight right
              danger_space[ time ].safely_add_danger_at( x + 1,   y  , d );
              // dag right+down
              danger_space[ time ].safely_add_danger_at( x + 1, y + 1, d );
              // straight down
              danger_space[ time ].safely_add_danger_at(   x  , y + 1, d );
              // dag left+down
              danger_space[ time ].safely_add_danger_at( x - 1, y + 1, d );
              break;
            case S:
              // straight right
              danger_space[ time ].safely_add_danger_at( x + 1,   y  , d );
              // dag right+down
              danger_space[ time ].safely_add_danger_at( x + 1, y + 1, d );
              // straight down
              danger_space[ time ].safely_add_danger_at(   x  , y + 1, d );
              // dag left+down
              danger_space[ time ].safely_add_danger_at( x - 1, y + 1, d );
              // straight left
              danger_space[ time ].safely_add_danger_at( x - 1,   y  , d );
              break;
            case SW:
              // dag right+down
              danger_space[ time ].safely_add_danger_at( x + 1, y + 1, d );
              // straight down
              danger_space[ time ].safely_add_danger_at(   x  , y + 1, d );
              // dag left+down
              danger_space[ time ].safely_add_danger_at( x - 1, y + 1, d );
              // straight left
              danger_space[ time ].safely_add_danger_at( x - 1,   y  , d );
              // dag left+up
              danger_space[ time ].safely_add_danger_at( x - 1, y - 1, d );
              break;
            case W:
              // straight down
              danger_space[ time ].safely_add_danger_at(   x  , y + 1, d );
              // dag left+down
              danger_space[ time ].safely_add_danger_at( x - 1, y + 1, d );
              // straight left
              danger_space[ time ].safely_add_danger_at( x - 1,   y  , d );
              // dag left+up
              danger_space[ time ].safely_add_danger_at( x - 1, y - 1, d );
              // straight up
              danger_space[ time ].safely_add_danger_at(   x  , y - 1, d );
              break;
            case NW:
              // dag left+down
              danger_space[ time ].safely_add_danger_at( x - 1, y + 1, d );
              // straight left
              danger_space[ time ].safely_add_danger_at( x - 1,   y  , d );
              // dag left+up
              danger_space[ time ].safely_add_danger_at( x - 1, y - 1, d );
              // straight up
              danger_space[ time ].safely_add_danger_at(   x  , y - 1, d );
              // dag right+up
              danger_space[ time ].safely_add_danger_at( x + 1, y - 1, d );
              break;
          }
#ifdef OVERLAYED
          overlayed[0].add_danger_at((*current_est).x,
                                     (*current_est).y,
                                     (*current_est).danger * adjust_danger(t) );
#endif
        }
        else // this estimate is only a timestamp marker
        {
          ++t;
        }
      }
    }
  }
}

void danger_grid::set_danger_scale( )
{
  // For now, we aren't scaling anything down
  for( int i = 0; i <= (int)(look_behind + look_ahead + 1); i++ )
    danger_ratings.push_back( PLANE_DANGER );
}

double danger_grid::get_danger_at( unsigned int x_pos, unsigned int y_pos, int seconds ) const
{
#ifdef DEBUG
  assert( seconds < (int)( danger_space.size() - look_behind ) );
  assert( seconds >= -(int)look_behind );
  assert( x_pos < UINT_MAX && y_pos < UINT_MAX );
#endif
  return danger_space[ seconds + look_behind ].get_danger_at( x_pos, y_pos );
}

void danger_grid::add_danger_at( unsigned int x_pos, unsigned int y_pos, int seconds,
                                 double danger )
{
#ifdef DEBUG
  assert( seconds < (int)( danger_space.size() - look_behind ) );
  assert( seconds >= -(int)look_behind );
  assert( x_pos < UINT_MAX && y_pos < UINT_MAX );
  assert( danger > -1.0 );
#endif
  danger_space[ seconds + look_behind ].add_danger_at( x_pos, y_pos, danger );
}

void danger_grid::set_danger_at( unsigned int x_pos, unsigned int y_pos, int seconds,
                                 double danger )
{
#ifdef DEBUG
  assert( seconds < (int)( danger_space.size() - look_behind ) );
  assert( seconds >= -(int)look_behind );
  assert( x_pos < UINT_MAX && y_pos < UINT_MAX );
  assert( danger > -1.0 );
#endif
  danger_space[ seconds + look_behind ].set_danger_at( x_pos, y_pos, danger );
}

double danger_grid::adjust_danger( int seconds ) const
{
  return danger_ratings[ seconds + look_behind ];
}

double danger_grid::operator()( unsigned int x, unsigned int y, int time ) const
{
  return get_danger_at( x, y, time );
}

unsigned int danger_grid::get_width_in_squares() const
{
  return danger_space[ 0 ].get_width_in_squares();
}

unsigned int danger_grid::get_height_in_squares() const
{
  return danger_space[ 0 ].get_height_in_squares();
}

unsigned int danger_grid::get_time_in_secs() const
{
  return look_ahead;
}

vector< map > danger_grid::get_danger_space() const
{
  return danger_space;
}

vector< estimate > danger_grid::calculate_future_pos( Plane & plane )
{
  
  vector< estimate > theFuture;
  
  Position current=plane.getLocation();
  Position destination=plane.getFinalDestination();
  
  //distance formula: line to destination
  int x1=current.getX(),x2=destination.getX(),y1=current.getY(),y2=destination.getY();
  double xDistance=( fabs((double)x2-x1) ),yDistance=( fabs((double)y2-y1) );
  double distance = sqrt((double)(xDistance*xDistance)+(yDistance*yDistance));
  if(xDistance==0&&yDistance==0)//your there!!!!!!!(hopefully)
  {cout<<"There was totally an error"; return theFuture;}
  //find the angle to the waypoint
  double angle=(180-RADtoDEGREES*(asin((double)xDistance/(double)distance)));
  if(y2<y1)
    angle=(RADtoDEGREES*(asin((double)xDistance/(double)distance)));
  
  if((x2-x1)<0)//positive means that the plane is headed to the left aka west
    angle=(-1)*angle;//the plane goes from -180 to +180
  
  //find closest straight line
  double neighbors[2];
  neighoboringAngles(angle, neighbors[0], neighbors[1]);
  double closestAngle=0,otherAngle=0;
  if(fabs(angle-neighbors[0])>=fabs(angle-neighbors[1]))//distance
  {closestAngle=neighbors[1]; otherAngle=neighbors[0];}
  else
  {closestAngle=neighbors[0]; otherAngle=neighbors[1];}
  
  //find displacement percentage
  double danger;
  if((fabs(angle)>fabs(closestAngle))&&closestAngle!=0)
    danger=(closestAngle/angle);
  else if(closestAngle!=0)
    danger=(angle/closestAngle);
  else//i hate 0
    danger=1-(angle/otherAngle);//because you can't use 0 find the inverse of the displacement to the other angel.
  
  //place displacement percentage in closest square and then place the remainder in the other square
  placeDanger(angle, theFuture, closestAngle, otherAngle, x1, y1, danger);
  //start the branching
  int dest[2]={x2,y2};//can't pass it without a name :(
  if(theFuture.back().danger>.3)
    dangerRecurse(theFuture.back(), dest, theFuture);
  theFuture.push_back(estimate(0,0,-1));
  dangerRecurse(theFuture[theFuture.size()-3],dest,theFuture);
  
  return theFuture;
}

void danger_grid::dangerRecurse(estimate e, int destination[], vector<estimate> &theFuture)
{
  
  int x1=e.x;
  int y1=e.y;
  int x2=destination[0];
  int y2=destination[1];
  int dest[2]={x2,y2};
  
  double xDistance=( fabs((double)x2-x1) ), yDistance=( fabs((double)y2-y1) );
  if(xDistance==0&&yDistance==0)//your there!!!!!!!(hopefully)
  {return;}
  double distance = sqrt((double)(xDistance*xDistance)+(yDistance*yDistance));
  
  //find the angle to the waypoint
  double angle=(180-RADtoDEGREES*(asin((double)xDistance/(double)distance)));
  if(y2<y1)
    angle=(RADtoDEGREES*(asin((double)xDistance/(double)distance)));
  if((x2-x1)<0)//negative means that the plane is headed to the left aka west
    angle=(-1)*angle;//the plane goes from -180 to +180
  
  //find closest straight line
  double neighbors[2];
  neighoboringAngles(angle, neighbors[0], neighbors[1]);
  double closestAngle=0,otherAngle=0;
  if(fabs(angle-neighbors[0])>=fabs(angle-neighbors[1]))//distance
  {closestAngle=neighbors[1]; otherAngle=neighbors[0];}
  else
  {closestAngle=neighbors[0]; otherAngle=neighbors[1];}
  
  //find displacement percentage
  double danger;
  if((fabs(angle)>fabs(closestAngle))&&closestAngle!=0)
    danger=(closestAngle/angle);
  else if(closestAngle!=0)
    danger=(angle/closestAngle);
  else//i hate 0
    danger=1-(angle/otherAngle);//because you can't use 0 find the inverse of the displacement to the other angle.
  
  //now add the new danger to theFuture
  placeDanger(angle, theFuture, closestAngle, otherAngle, x1, y1, danger);
  
  //brance it up now
  if(theFuture.back().danger>.3)
    dangerRecurse(theFuture.back(), dest, theFuture);
  //add time--still a slight issue as the branches don't exist in time but should not be too major--
  theFuture.push_back(estimate(0,0,-1));
  //default branch
  dangerRecurse(theFuture[theFuture.size()-3],dest,theFuture);
  
  
}

void danger_grid::neighoboringAngles(double angle, double &first, double &second)
{
  if(angle>0)
  {
    if(angle<45)
    {first=0; second = 45;}
    else if(angle<90)
    {first=45; second = 90;}
    else if(angle<135)
    {first=90; second = 135;}
    else if(angle<=180)
    {first=135; second = 180;}
  }
  
  else
  {
    if(angle>-45)
    {first=0; second = -45;}
    else if(angle>-90)
    {first=-45; second = -90;}
    else if(angle>-135)
    {first=-90; second = -135;}
    else if(angle>=-180)
    {first=-135; second = -180;}
  }
}

void danger_grid::placeDanger(double angle, vector<estimate> &e, double closest, double other, int x, int y, double danger)
{
  if(angle>0)//to the right
  {
    if(closest==0)//north && northeast
    {
      e.push_back(estimate(x,y-1,danger));//majority in N
      e.push_back(estimate(x+1,y-1,1-danger));//remainder in NE
    }
    else if(closest == 45 && other == 0)//northeast && north
    {
      e.push_back(estimate(x+1,y-1,danger));//majority in NE
      e.push_back(estimate(x,y-1,1-danger));//remainder in N
    }
    
    else if(closest == 45)//northeast && east
    {
      e.push_back(estimate(x+1,y-1,danger));//majority in NE
      e.push_back(estimate(x+1,y,1-danger));//remainder in east
    }
    
    else if(closest == 90 && other == 45)//east && northeast
    {
      e.push_back(estimate(x+1,y,danger));//majority in east
      e.push_back(estimate(x+1,y-1,1-danger));//remainder in NE
    }
    
    else if(closest == 90)//east && southeast
    {
      e.push_back(estimate(x+1,y,danger));//majority in east
      e.push_back(estimate(x+1,y+1,1-danger));//remainder in SE
    }
    
    else if(closest == 135 && other == 90)//southeast && east
    {
      e.push_back(estimate(x+1,y+1,danger));//majority in SE
      e.push_back(estimate(x+1,y,1-danger));//remainder in east
    }
    
    else if(closest == 135)//southeast && south
    {
      e.push_back(estimate(x+1,y+1,danger));//majority in SE
      e.push_back(estimate(x,y+1,1-danger));//remainder in south
    }
    
    else//south && southeast
    {
      e.push_back(estimate(x,y+1,danger));//majority in south
      e.push_back(estimate(x+1,y+1,1-danger));//majority in SE
    }
  }
  else//to the left
  {
    if(closest==0)//north && northwest
    {
      e.push_back(estimate(x,y-1,danger));//majority in N
      e.push_back(estimate(x-1,y-1,1-danger));//remainder in NW
    }
    else if(closest == -45 && other == 0)//northwest && north
    {
      e.push_back(estimate(x-1,y-1,danger));//majority in NW
      e.push_back(estimate(x,y-1,1-danger));//remainder in N
    }
    
    else if(closest == -45)//northwest && west
    {
      e.push_back(estimate(x-1,y-1,danger));//majority in NW
      e.push_back(estimate(x-1,y,1-danger));//remainder in west
    }
    
    else if(closest == -90 && other == -45)//west && northwest
    {
      e.push_back(estimate(x-1,y,danger));//majority in west
      e.push_back(estimate(x-1,y-1,1-danger));//remainder in NE
    }
    
    else if(closest == -90)//west && southwest
    {
      e.push_back(estimate(x-1,y,danger));//majority in west
      e.push_back(estimate(x-1,y+1,1-danger));//remainder in SW
    }
    
    else if(closest == -135 && other == -90)//southwest && west
    {
      e.push_back(estimate(x-1,y+1,danger));//majority in SW
      e.push_back(estimate(x-1,y,1-danger));//remainder in west
    }
    
    else if(closest == -135)//southwest && south
    {
      e.push_back(estimate(x-1,y+1,danger));//majority in SW
      e.push_back(estimate(x,y+1,1-danger));//remainder in south
    }
    
    else//south && southwest
    {
      e.push_back(estimate(x,y+1,danger));//majority in south
      e.push_back(estimate(x-1,y+1,1-danger));//majority in SW
    }
  }
}

danger_grid::bearing_t danger_grid::name_bearing( double the_bearing )
{
  the_bearing = fmod(the_bearing, 360); // modular division for floats
  
  if( the_bearing > -22.5 && the_bearing <= 22.5 )
    return N;
  else if( the_bearing > 22.5 && the_bearing <= 67.5 )
    return NE;
  else if( the_bearing > 67.5 && the_bearing <= 112.5 )
    return E;
  else if( the_bearing > 112.5 && the_bearing <= 157.5 )
    return SE;
  else if( the_bearing > 157.5 && the_bearing <= 202.5 )
    return S;
  else if( the_bearing > 202.5 && the_bearing <= 247.5 )
    return SW;
  else if( the_bearing > 247.5 && the_bearing <= 292.5 )
    return W;
  else if( the_bearing > 292.5 && the_bearing <= 337.5 )
    return NW;
  else
    return N;
}


void danger_grid::dump_est( vector< estimate > dump_me )
{
  unsigned int i = 0;
  for( vector< estimate >::iterator crnt_est = dump_me.begin();
      crnt_est != dump_me.end(); ++crnt_est )
  {
    cout << endl << " Estimate " << i << "'s data: " << endl;
    cout << " x = " << (*crnt_est).x << endl;
    cout << " y = " << (*crnt_est).y << endl;
    cout << " d = " << (*crnt_est).danger << endl;
    ++i;
  }
}

void danger_grid::calculate_distance_costs( unsigned int goal_x, unsigned int goal_y )
{
  unsigned int width = get_width_in_squares();
  unsigned int height = get_height_in_squares();
  double danger_adjust = ( width + height ) / 4;
  
  for( unsigned int crnt_x = 0; crnt_x <  width; crnt_x++ )
  {
    for( unsigned int crnt_y = 0; crnt_y < height; crnt_y++ )
    {
      for( unsigned int crnt_t = 0; crnt_t < look_ahead; crnt_t++ )
      {
        double crnt_danger = danger_space[ crnt_t ].get_danger_at( crnt_x, crnt_y );
        double dist = sqrt( (crnt_x - goal_x)*(crnt_x - goal_x) + 
                           (crnt_y - goal_y)*(crnt_y - goal_y) );
        danger_space[crnt_t].set_danger_at( crnt_x, crnt_y,
                                 danger_adjust * crnt_danger + dist );
      }
    }
  }
}

void danger_grid::dump( int time ) const
{
#ifdef DEBUG
  assert( time + (int)look_behind < (int)( danger_space.size() ) || time == 10000 );
#endif
  
  if( time == 10000 )
  {
#ifdef OVERLAYED
    overlayed[0].dump();
#endif
  }
  else
  {
    // the meat of the dump is performed by the map class
    danger_space[ time + look_behind ].dump();
  }
}

void danger_grid::dump_big_numbers( int time ) const
{
#ifdef DEBUG
  assert( time + (int)look_behind < (int)( danger_space.size() ) || time == 10000 );
#endif
  
  if( time == 10000 )
  {
#ifdef OVERLAYED
    overlayed[0].dump_big_numbers();
#endif
  }
  else
  {
    // the meat of the dump is performed by the map class
    danger_space[ time + look_behind ].dump_big_numbers();
  }
}

#endif

