//
//  map_fixed.h
//  AU_UAV_ROS
//
//  Created by Tyler Young on 5/21/11.
//  Copyright 2011 Noteflow Innovations. All rights reserved.
//

#ifndef MAP
#define MAP

// A class that stores a two-dimensional representation of the world;
// Each square in this grid has a danger associated with it, and it may have
// one or more aircraft present in it.

#include <vector>
#include <cassert>

#ifndef PLANE_DANGER
#define PLANE_DANGER 0.98
#endif

#ifndef EPSILON
#define EPSILON 0.000001
#endif

using namespace std;

struct grid_square
{
    vector< unsigned int > planes;
    double danger;
    
    /**
     * The constructor for a grid square; sets danger to 0 initially
     */
    grid_square( )
    {
        danger = 0.0;
    }
};

// A map is composed of grid squares
// It has a width (in meters), a height (in meters), and a resolution (the size
// of a single grid square, in meters)
// Don't forget: latitude is horizontal, longitude is vertical
class map
{
public:
    /**
     * The constructor for a map object
     * @param width_of_field The width of the flyable area, in meters
     * @param height_of_field The height of the flyable area, in meters
     * @param map_resolution The resolution (width and height of a given square),
     *                       in meters
     */
    map( double width_of_field, double height_of_field, double map_resolution );
    
    // Destructor
    ~map( );
    
    /**
     * Return IDs of all aircraft in a given (x,y) square
     * @param x_pos the x position of the square in question
     * @param y_pos the y position of the square in question
     * @return a vector of unsigned ints corresponding to the unique IDs of any
     *         aircraft in this square 
     */
    vector< unsigned int > get_planes_at( unsigned int x_pos, unsigned int y_pos ) const;
    
    /**
     * Add an aircraft (i.e., its unique ID) to a given (x,y) square
     * NOTE: Adding a plane automatically sets the danger for this square
     *       to near-maximum
     * @param x_pos the x position of the square in question
     * @param y_pos the y position of the square in question
     * @param id the unique integer identifying the aircraft
     */
    void add_plane_at( unsigned int x_pos, unsigned int y_pos, unsigned int id );
    
    /**
     * Return the danger rating of a square
     * @param x_pos the x position of the square in question
     * @param y_pos the y position of the square in question
     * @return a double containing the square's "danger" rating
     */
    double get_danger_at( unsigned int x_pos, unsigned int y_pos );
    
    /**
     * Set the danger rating of a square
     * @param x_pos the x position of the square to set
     * @param y_pos the y position of the square to set
     * @param danger the danger to be assigned to this square
     */
    void set_danger_at( unsigned int x_pos, unsigned int y_pos, double danger );
    
    unsigned int get_width_in_squares( ) const;
    double get_width_in_meters( ) const;
    unsigned int get_height_in_squares( ) const;
    double get_height_in_meters( ) const;
    
    unsigned int get_resolution( ) const; // (in meters)
    
    // Prints the contents of the map, once with the aircraft and their locations,
    // and once with the danger values. Used for testing.
    void dump( ) const;
    
private:    
    vector< vector< grid_square > > the_map; // a 2-D vector array of grid squares
    double width;
    double height;
    double resolution;
    unsigned int squares_wide;
    unsigned int squares_high;
};

map::map( double width_of_field, double height_of_field, double map_resolution )
{
    assert( width_of_field > EPSILON );
    assert( height_of_field > EPSILON );
    assert( map_resolution > EPSILON );
    
    width = width_of_field;
    height = height_of_field;
    resolution = map_resolution;
    squares_wide = ( width_of_field / resolution ); // NOTE: is it a problem that
                                                    //       this rounds down??
    squares_high = ( height_of_field / resolution );
    
    // This *should* create a 2-d array accessed in [x][y] order
    the_map.resize( squares_wide );
    
    for( unsigned int i = 0; i < the_map.size(); ++i )
        the_map[ i ].resize( squares_high );
    
    for( vector< vector< grid_square > >::iterator i = the_map.begin(); 
        i != the_map.end(); ++i )
    {
        for( vector< grid_square >::iterator j = i->begin(); j != i->end(); ++j )
        {
            j->danger = 0;
        }
    }
    
    cout << "The map constructor went ok" << endl; //////////////////////////////////////////////////////////////////
    cout << "Size of x-direction is: " << the_map.size() << endl;
    cout << "Size of y-direction is: " << the_map[1].size() << endl;
}

map::~map( ) { }

vector< unsigned int > map::get_planes_at( unsigned int x_pos, unsigned int y_pos ) const
{
    return the_map[ x_pos ][ y_pos ].planes;
}

void map::add_plane_at( unsigned int x_pos, unsigned int y_pos, unsigned int id )
{
    //cout << " Trying to add a plane at (" << x_pos << ", "<< y_pos << ")" << endl; //////////////////////////////////////
    the_map[ x_pos ][ y_pos ].planes.push_back( id );
    //cout << "setting danger" << endl; //////////////////////////////////////////////////////////////////////////////////
    the_map[ x_pos ][ y_pos ].danger = PLANE_DANGER;
    
}

double map::get_danger_at( unsigned int x_pos, unsigned int y_pos )
{
    return the_map[ x_pos ][ y_pos ].danger;
}

void map::set_danger_at( unsigned int x_pos, unsigned int y_pos, double new_danger )
{
    the_map[ x_pos ][ y_pos ].danger = new_danger;
}

unsigned int map::get_width_in_squares( ) const
{
    return squares_wide;
}

double map::get_width_in_meters( ) const
{
    return width;
}

unsigned int map::get_height_in_squares( ) const
{
    return squares_high;
}

double map::get_height_in_meters( ) const
{
    return height;
}

unsigned int map::get_resolution( ) const
{
    return resolution;
}

void map::dump( ) const
{
    /*for( vector< vector< grid_square > >::const_iterator i = the_map.begin(); 
         i != the_map.end(); ++i )
    {
        for( vector< grid_square >::const_iterator j = i->begin(); j != i->end(); ++j )
        {
            if( j->danger < EPSILON && j->danger > -EPSILON )
            {
                cout << " --  ";
            }
            else
            {
                printf( "%1.2f ", j->danger );
                //cout << j->danger;
            }
        }
        cout << endl;
    }*/
    for( int right_index = the_map[0].size() - 1; right_index >= 0; --right_index )
    {
        for( unsigned int left_index = 0; left_index < the_map.size(); ++left_index )
        {
            if( the_map[ left_index ][ right_index ].danger < EPSILON && 
                the_map[ left_index ][ right_index ].danger > -EPSILON )
            {
                cout << "-  ";
            }
            else
            {
                printf( "%2.0f ", (the_map[ left_index ][ right_index ].danger)*100 );
                //cout << j->danger;
            }
        }
        cout << endl;
    }
}

#endif