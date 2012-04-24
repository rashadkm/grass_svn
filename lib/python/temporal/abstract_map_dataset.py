# -*- coding: utf-8 -*-
"""!@package grass.temporal

@brief GRASS Python scripting module (temporal GIS functions)

Temporal GIS related functions to be used in temporal GIS Python library package.

Usage:

@code
import grass.temporal as tgis

...
@endcode

(C) 2008-2011 by the GRASS Development Team
This program is free software under the GNU General Public
License (>=v2). Read the file COPYING that comes with GRASS
for details.

@author Soeren Gebbert
"""
from abstract_dataset import *
from datetime_math import *

class temporal_map_relations(abstract_dataset):
    """!This class implements a temporal topology access structure
    
       This object will be set up by temporal topology creation methods.
       
       If correctly initialize the calls get_next() and get_prev() let the user walk temporally forward
       and backward in time.
       
       The following temporal relations with access methods are supported:
       * equal
       * follows
       * precedes
       * overlaps
       * overlapped
       * during (including starts, finishes)
       * contains (including started, finished)
       
       
       Code:
       # We have build the temporal topology and we know the first map
       start = first
       while start:
       
	   # Print all maps this map temporally contains
           dlist = start.get_contains()
           for _map in dlist:
               _map.print_info()
               
           start = start.get_next()       
    """

    def __init__(self):
        self.reset_temporal_topology()
        
    def reset_temporal_topology(self):
	"""!Reset any information about temporal topology"""
        self._temporal_topology = {}
        self._has_temporal_topology = False
        
    def set_temporal_topology_build_true(self):
	"""!Same as name"""
	self._has_temporal_topology = True
	
    def set_temporal_topology_build_false(self):
	"""!Same as name"""
	self._has_temporal_topology = False

    def is_temporal_topology_build(self):
	"""!Check if the temporal topology was build"""
	return self._has_temporal_topology

    def set_next(self, _map):
	"""!Set the map that is temporally as closest located after this map.
	
	   Temporally located means that the start time of the "next" map is 
	   temporally located AFTER the start time of this map, but temporally 
	   near than other maps of the same dataset.
	   
	   @param _map This object should be of type abstract_map_dataset or derived classes
	"""
        self._temporal_topology["NEXT"] = _map

    def set_prev(self, _map):
	"""!Set the map that is temporally as closest located before this map.
	
	   Temporally located means that the start time of the "previous" map is 
	   temporally located BEFORE the start time of this map, but temporally 
	   near than other maps of the same dataset.
	   
	   @param _map This object should be of type abstract_map_dataset or derived classes
	"""
        self._temporal_topology["PREV"] = _map

    def next(self):
	"""!Return the map with a start time temporally located after
	   the start time of this map, but temporal closer than other maps
	   
	   @return A map object or None
	"""
        if not self._temporal_topology.has_key("NEXT"):
            return None
        return self._temporal_topology["NEXT"]

    def prev(self):
	"""!Return the map with a start time temporally located before
	   the start time of this map, but temporal closer than other maps
	   
	   @return A map object or None
	"""
        if not self._temporal_topology.has_key("PREV"):
            return None
        return self._temporal_topology["PREV"]

    def append_equivalent(self, _map): 
	"""!Append a map with equivalent temporal extent as this map
	
	   @param _map This object should be of type abstract_map_dataset or derived classes
	"""
        if not self._temporal_topology.has_key("EQUAL"):
            self._temporal_topology["EQUAL"] = []
        self._temporal_topology["EQUAL"].append(_map)

    def get_equivalent(self):
	"""!Return a list of map objects with equivalent temporal extent as this map
	
	   @return A list of map objects or None
	"""
        if not self._temporal_topology.has_key("EQUAL"):
            return None
        return self._temporal_topology["EQUAL"]

    def append_overlaps(self, _map): 
	"""!Append a map that this map temporally overlaps
	
	   @param _map This object should be of type abstract_map_dataset or derived classes
	"""
        if not self._temporal_topology.has_key("OVERLAPS"):
            self._temporal_topology["OVERLAPS"] = []
        self._temporal_topology["OVERLAPS"].append(_map)

    def get_overlaps(self): 
	"""!Return a list of map objects that this map temporally overlaps
	
	   @return A list of map objects or None
	"""
        if not self._temporal_topology.has_key("OVERLAPS"):
            return None
        return self._temporal_topology["OVERLAPS"]

    def append_overlapped(self, _map): 
	"""!Append a map that this map temporally overlapped
	
	   @param _map This object should be of type abstract_map_dataset or derived classes
	""" 
        if not self._temporal_topology.has_key("OVERLAPPED"):
            self._temporal_topology["OVERLAPPED"] = []
        self._temporal_topology["OVERLAPPED"].append(_map)

    def get_overlapped(self): 
	"""!Return a list of map objects that this map temporally overlapped
	
	   @return A list of map objects or None
	"""
        if not self._temporal_topology.has_key("OVERLAPPED"):
            return None
        return self._temporal_topology["OVERLAPPED"]
    
    def append_follows(self, _map): 
	"""!Append a map that this map temporally follows
	
	   @param _map This object should be of type abstract_map_dataset or derived classes
	""" 
        if not self._temporal_topology.has_key("FOLLOWS"):
            self._temporal_topology["FOLLOWS"] = []
        self._temporal_topology["FOLLOWS"].append(_map)

    def get_follows(self): 
	"""!Return a list of map objects that this map temporally follows
	
	   @return A list of map objects or None
	"""
        if not self._temporal_topology.has_key("FOLLOWS"):
            return None
        return self._temporal_topology["FOLLOWS"]
    
    def append_precedes(self, _map): 
	"""!Append a map that this map temporally precedes
	
	   @param _map This object should be of type abstract_map_dataset or derived classes
	""" 
        if not self._temporal_topology.has_key("PRECEDES"):
            self._temporal_topology["PRECEDES"] = []
        self._temporal_topology["PRECEDES"].append(_map)

    def get_precedes(self): 
	"""!Return a list of map objects that this map temporally precedes
	
	   @return A list of map objects or None
	"""
        if not self._temporal_topology.has_key("PRECEDES"):
            return None
        return self._temporal_topology["PRECEDES"]
    
    def append_during(self, _map):  
	"""!Append a map that this map is temporally located during
	   This includes temporal relationships starts and finishes
	
	   @param _map This object should be of type abstract_map_dataset or derived classes
	""" 
        if not self._temporal_topology.has_key("DURING"):
            self._temporal_topology["DURING"] = []
        self._temporal_topology["DURING"].append(_map)

    def get_during(self): 
	"""!Return a list of map objects that this map is temporally located during
	   This includes temporally relationships starts and finishes
	
	   @return A list of map objects or None
	"""
        if not self._temporal_topology.has_key("DURING"):
            return None
        return self._temporal_topology["DURING"]
    
    def append_contains(self, _map):   
	"""!Append a map that this map temporally contains
	   This includes temporal relationships started and finished
	
	   @param _map This object should be of type abstract_map_dataset or derived classes
	""" 
        if not self._temporal_topology.has_key("CONTAINS"):
            self._temporal_topology["CONTAINS"] = []
        self._temporal_topology["CONTAINS"].append(_map)

    def get_contains(self): 
	"""!Return a list of map objects that this map temporally contains
	   This includes temporal relationships started and finished
	
	   @return A list of map objects or None
	"""
        if not self._temporal_topology.has_key("CONTAINS"):
            return None
        return self._temporal_topology["CONTAINS"]

    def _generate_map_list_string(self, map_list, line_wrap = True):
	count = 0
	string = ""
	for _map in map_list:
	    if line_wrap and count > 0 and count % 3 == 0:
		string += "\n | ............................ "
		count = 0
	    if count == 0:
		string += _map.get_id()
	    else:
		string += ",%s" % _map.get_id()
	    count += 1
	    
	return string
                
    def print_temporal_topology_info(self):
        """!Print information about this class in human shell style"""
        _next = self.next()
        _prev = self.prev()
        _equal = self.get_equivalent()
        _follows = self.get_follows()
        _precedes = self.get_precedes()
        _overlaps = self.get_overlaps()
        _overlapped = self.get_overlapped()
        _during = self.get_during()
        _contains = self.get_contains()
        
        print " +-------------------- Temporal Topology -------------------------------------+"
        #          0123456789012345678901234567890
        if _next:
	    print " | Next: ...................... " + str(_next.get_id())
        if _prev:
	    print " | Previous: .................. " + str(_prev.get_id())
        if _equal:
	    print " | Equivalent: ................ " + self._generate_map_list_string(_equal)
        if _follows:
	    print " | Follows: ................... " + self._generate_map_list_string(_follows)
        if _precedes:
	    print " | Precedes: .................. " + self._generate_map_list_string(_precedes)
        if _overlaps:
	    print " | Overlaps: .................. " + self._generate_map_list_string(_overlaps)
        if _overlapped:
	    print " | Overlapped: ................ " + self._generate_map_list_string(_overlapped)
        if _during:
	    print " | During: .................... " + self._generate_map_list_string(_during)
        if _contains:
	    print " | Contains: .................. " + self._generate_map_list_string(_contains)
         
    def print_temporal_topology_shell_info(self):
        """!Print information about this class in human readable style"""
        
        _next = self.next()
        _prev = self.prev()
        _equal = self.get_equivalent()
        _follows = self.get_follows()
        _precedes = self.get_precedes()
        _overlaps = self.get_overlaps()
        _overlapped = self.get_overlapped()
        _during = self.get_during()
        _contains = self.get_contains()
        
        if _next:
	    print "next=" + _next.get_id()
        if _prev:
	    print "prev=" + _prev.get_id()
        if _equal:
	    print "equivalent=" +self._generate_map_list_string(_equal, False)
        if _follows:
	    print "follows=" +self._generate_map_list_string(_follows, False)
        if _precedes:
	    print "precedes=" +self._generate_map_list_string(_precedes, False)
        if _overlaps:
	    print "overlaps=" +self._generate_map_list_string(_overlaps, False)
        if _overlapped:
	    print "overlapped=" +self._generate_map_list_string(_overlapped, False)
        if _during:
	    print "during=" +self._generate_map_list_string(_during, False)
        if _contains:
	    print "contains=" +self._generate_map_list_string(_contains, False)
                
###############################################################################

class abstract_map_dataset(temporal_map_relations):
    """!This is the base class for all maps (raster, vector, raster3d) 
       providing additional function to set the valid time and the spatial extent.
    """
    def __init__(self):
	temporal_map_relations.__init__(self)
	
    def get_new_stds_instance(self, ident):
        """!Return a new space time dataset instance in which maps are stored with the type of this class

           @param ident: The identifier of the dataset
        """
        raise IOError("This method must be implemented in the subclasses")
    
    def get_stds_register(self):
        """!Return the space time dataset register table name in which stds are listed in which this map is registered"""
        raise IOError("This method must be implemented in the subclasses")

    def set_stds_register(self, name):
        """!Set the space time dataset register table name.
        
           This table stores all space time datasets in which this map is registered.

           @param ident: The name of the register table
        """
        raise IOError("This method must be implemented in the subclasses")
 
    def check_resolution_with_current_region(self):
        """!Check if the raster or voxel resolution is finer than the current resolution
           Return "finer" in case the raster/voxel resolution is finer than the current region
           Return "coarser" in case the raster/voxel resolution is coarser than the current region

           Vector maps are alwyas finer than the current region
        """
        raise IOError("This method must be implemented in the subclasses")
	
    def has_grass_timestamp(self):
        """!Check if a grass file bsased time stamp exists for this map. 
        """
        raise IOError("This method must be implemented in the subclasses")
    
    def write_timestamp_to_grass(self):
        """!Write the timestamp of this map into the map metadata in the grass file system based spatial
           database. 
        """
        raise IOError("This method must be implemented in the subclasses")
    
    def remove_timestamp_from_grass(self):
        """!Remove the timestamp from the grass file system based spatial database
        """
        raise IOError("This method must be implemented in the subclasses")
	
    def map_exists(self):
        """!Return True in case the map exists in the grass spatial database
        
           @return True if map exists, False otherwise
        """        
        raise IOError("This method must be implemented in the subclasses")
	        
    def read_info(self):
        """!Read the map info from the grass file system based database and store the content 
           into a dictionary
        """
        raise IOError("This method must be implemented in the subclasses")

    def load(self):
        """!Load the content of this object from the grass file system based database"""
        raise IOError("This method must be implemented in the subclasses")
	
    def _convert_timestamp(self):
	"""!Convert the valid time into a grass datetime library compatible timestamp string
	    
	    This methods works for reltaive and absolute time
	    
	    @return the grass timestamp string
	"""
	start = ""
        
        if self.is_time_absolute():
	    start_time, end_time, tz = self.get_absolute_time()
	    start = datetime_to_grass_datetime_string(start_time)
	    if end_time:
		end = datetime_to_grass_datetime_string(end_time)
		start += " / %s"%(end)
	else:
	    start_time, end_time, unit = self.get_relative_time()
	    start = "%i %s"%(int(start_time), unit)
	    if end_time != None:
		end = "%i %s"%(int(end_time), unit)
		start += " / %s"%(end)

	return start
		        
    def get_map_id(self):
	"""!Return the map id. The map id is the unique map identifier in grass and must not be equal to the 
	   primary key identifier (id) of the map in the database. Since vector maps may have layer information,
	   the unique id is a combination of name, layer and mapset.
	   
	   Use get_map_id() every time your need to access the grass map in the file system but not to identify
	   map information in the temporal database.
	
	"""
        return self.base.get_map_id()

    def build_id(self, name, mapset, layer=None):
	"""!Convenient method to build the unique identifier
	
	    Existing layer and mapset definitions in the name string will be reused

           @param return the id of the vector map as name(:layer)@mapset while layer is optional
        """
        
        # Check if the name includes any mapset
	if name.find("@") >= 0:
	    print name
	    name, mapset = name.split("@")

        # Check for layer number in map name
	if name.find(":") >= 0:
	    name, layer = name.split(":")
	    
        if layer:	    
	    return "%s:%s@%s"%(name, layer, mapset)
	else:
	    return "%s@%s"%(name, mapset)
	    
    def get_layer(self):
	"""!Return the layer of the map or None in case no layer is defined"""
	return self.base.get_layer()
        
    def print_info(self):
        """!Print information about this class in human readable style"""
        
        if self.get_type() == "raster":
            #                1         2         3         4         5         6         7
            #      0123456789012345678901234567890123456789012345678901234567890123456789012345678
            print ""
            print " +-------------------- Raster Dataset ----------------------------------------+"
        if self.get_type() == "raster3d":
            #                1         2         3         4         5         6         7
            #      0123456789012345678901234567890123456789012345678901234567890123456789012345678
            print ""
            print " +-------------------- Raster3d Dataset --------------------------------------+"
        if self.get_type() == "vector":
            #                1         2         3         4         5         6         7
            #      0123456789012345678901234567890123456789012345678901234567890123456789012345678
            print ""
            print " +-------------------- Vector Dataset ----------------------------------------+"
        print " |                                                                            |"
	self.base.print_info()
	if self.is_time_absolute():
	    self.absolute_time.print_info()
        if self.is_time_relative():
	    self.relative_time.print_info()
	self.spatial_extent.print_info()
	self.metadata.print_info()
        datasets = self.get_registered_datasets()
        count = 0
        string = ""
        if datasets:
            for ds in datasets:
                if count > 0 and count % 3 == 0:
                    string += "\n | ............................ "
                    count = 0
                if count == 0:
                    string += ds["id"]
                else:
                    string += ",%s" % ds["id"]
                count += 1
        print " | Registered datasets ........ " + string
        if self.is_temporal_topology_build():
	    self.print_temporal_topology_info()
        print " +----------------------------------------------------------------------------+"

    def print_shell_info(self):
        """!Print information about this class in shell style"""
	self.base.print_shell_info()
	if self.is_time_absolute():
	    self.absolute_time.print_shell_info()
        if self.is_time_relative():
	    self.relative_time.print_shell_info()
	self.spatial_extent.print_shell_info()
	self.metadata.print_shell_info()
        datasets = self.get_registered_datasets()
        count = 0
        string = ""
        if datasets:
	    for ds in datasets:
		if count == 0:
		    string += ds["id"]
		else:
		    string += ",%s" % ds["id"]
		count += 1
	    print "registered_datasets=" + string

        if self.is_temporal_topology_build():
	    self.print_temporal_topology_shell_info()
	    
    def insert(self, dbif=None, execute=True):
        """!Insert temporal dataset entry into database from the internal structure

	   This functions assures that the timetsamp is written to the grass file system based database
	    
           @param dbif: The database interface to be used
           @param execute: If True the SQL statements will be executed.
                           If False the prepared SQL statements are returned and must be executed by the caller.
        """
        self.write_timestamp_to_grass()
        return abstract_dataset.insert(self, dbif, execute)

    def update(self, dbif=None, execute=True):
	"""!Update temporal dataset entry of database from the internal structure
	   excluding None variables
	   
	   This functions assures that the timetsamp is written to the grass file system based database

           @param dbif: The database interface to be used
           @param execute: If True the SQL statements will be executed.
                           If False the prepared SQL statements are returned and must be executed by the caller.
	"""
        self.write_timestamp_to_grass()
        return abstract_dataset.update(self, dbif, execute)

    def update_all(self, dbif=None, execute=True):
	"""!Update temporal dataset entry of database from the internal structure
	   and include None varuables.
	   
	   This functions assures that the timetsamp is written to the grass file system based database

           @param dbif: The database interface to be used
           @param execute: If True the SQL statements will be executed.
                           If False the prepared SQL statements are returned and must be executed by the caller.
	"""
        self.write_timestamp_to_grass()
        return abstract_dataset.update_all(self, dbif, execute)
        
    def set_absolute_time(self, start_time, end_time=None, timezone=None):
        """!Set the absolute time interval with start time and end time
        
           @param start_time: a datetime object specifying the start time of the map
           @param end_time: a datetime object specifying the end time of the map
           @param timezone: Thee timezone of the map
        
        """
        if start_time and not isinstance(start_time, datetime) :
	    if self.get_layer():
		core.fatal(_("Start time must be of type datetime for %s map <%s> with layer: %s") % (self.get_type(), self.get_map_id(), self.get_layer()))
	    else:
		core.fatal(_("Start time must be of type datetime for %s map <%s>") % (self.get_type(), self.get_map_id()))

        if end_time and not isinstance(end_time, datetime) :
	    if self.get_layer():
		core.fatal(_("End time must be of type datetime for %s map <%s> with layer: %s") % (self.get_type(), self.get_map_id(), self.get_layer()))
	    else:
		core.fatal(_("End time must be of type datetime for %s map <%s>") % (self.get_type(), self.get_map_id()))

        if start_time and end_time:
            if start_time > end_time:
		if self.get_layer():
		    core.fatal(_("End time must be greater than start time for %s map <%s> with layer: %s") % (self.get_type(), self.get_map_id(), self.get_layer()))
		else:
		    core.fatal(_("End time must be greater than start time for %s map <%s>") % (self.get_type(), self.get_map_id()))
            else:
                # Do not create an interval in case start and end time are equal
                if start_time == end_time:
                    end_time = None

        self.base.set_ttype("absolute")
        
        self.absolute_time.set_start_time(start_time)
        self.absolute_time.set_end_time(end_time)
        self.absolute_time.set_timezone(timezone)

    def update_absolute_time(self, start_time, end_time=None, timezone=None, dbif = None):
        """!Update the absolute time

	   This functions assures that the timetsamp is written to the grass file system based database

           @param start_time: a datetime object specifying the start time of the map
           @param end_time: a datetime object specifying the end time of the map
           @param timezone: Thee timezone of the map
        """
        dbif, connect = init_dbif(dbif)

        self.set_absolute_time(start_time, end_time, timezone)
        self.absolute_time.update_all(dbif)
        self.base.update(dbif)

        if connect == True:
            dbif.close()

        self.write_timestamp_to_grass()
        
    def set_relative_time(self, start_time, end_time, unit):
        """!Set the relative time interval 
        
           @param start_time: A double value 
           @param end_time: A double value 
           @param unit: The unit of the relative time. Supported units: years, months, days, hours, minutes, seconds

           Return True for success and False otherwise

        """

        if not self.check_relative_time_unit(unit):
	    if self.get_layer():
		core.error(_("Unsupported relative time unit type for %s map <%s> with layer %s: %s") % (self.get_type(), self.get_id(), self.get_layer(), unit))
	    else:
		core.error(_("Unsupported relative time unit type for %s map <%s>: %s") % (self.get_type(), self.get_id(), unit))
            return False
        

        if start_time != None and end_time != None:
            if int(start_time) > int(end_time):
		if self.get_layer():
		    core.error(_("End time must be greater than start time for %s map <%s> with layer %s") % (self.get_type(), self.get_id(), self.get_layer()))
		else:
		    core.error(_("End time must be greater than start time for %s map <%s>") % (self.get_type(), self.get_id()))
                return False
            else:
                # Do not create an interval in case start and end time are equal
                if start_time == end_time:
                    end_time = None

        self.base.set_ttype("relative")
        
        self.relative_time.set_unit(unit)
        self.relative_time.set_start_time(int(start_time))
        if end_time != None:
            self.relative_time.set_end_time(int(end_time))
        else:
            self.relative_time.set_end_time(None)

        return True

    def update_relative_time(self, start_time, end_time, unit, dbif = None):
        """!Update the relative time interval

	   This functions assures that the timetsamp is written to the grass file system based database
	    
           @param start_time: A double value 
           @param end_time: A double value 
           @param dbif: The database interface to be used
        """
        dbif, connect = init_dbif(dbif)

        if self.set_relative_time(start_time, end_time, unit):
            self.relative_time.update_all(dbif)
            self.base.update(dbif)

        if connect == True:
            dbif.close()

        self.write_timestamp_to_grass()

    def set_spatial_extent(self, north, south, east, west, top=0, bottom=0):
        """!Set the spatial extent of the map

           @param north: The northern edge
           @param south: The southern edge
           @param east: The eastern edge
           @param west: The western edge
           @param top: The top edge
           @param bottom: The bottom edge
        """
        self.spatial_extent.set_spatial_extent(north, south, east, west, top, bottom)
        
    def check_valid_time(self):
        """!Check for correct valid time"""
        if self.is_time_absolute():
            start, end, tz = self.get_absolute_time()
        else:
            start, end, unit = self.get_relative_time()

        if start != None:
            if end != None:
                if start >= end:
		    if self.get_layer():
			core.error(_("Map <%s> with layer %s has incorrect time interval, start time is greater than end time") % (self.get_map_id(), self.get_layer()))
		    else:
			core.error(_("Map <%s> has incorrect time interval, start time is greater than end time") % (self.get_map_id()))
                    return False
        else:
            core.error(_("Map <%s> has incorrect start time") % (self.get_map_id()))
            return False

        return True

    def delete(self, dbif=None, update=True, execute = True):
	"""!Delete a map entry from database if it exists
        
            Remove dependent entries:
            * Remove the map entry in each space time dataset in which this map is registered
            * Remove the space time dataset register table
            
           @param dbif: The database interface to be used
           @param update: Call for each unregister statement the update from registered maps 
                          of the space time dataset. This can slow down the un-registration process significantly.
           @param execute: If True the SQL DELETE and DROP table statements will be executed.
                           If False the prepared SQL statements are returned and must be executed by the caller.

           @return The SQL statements if execute == False, else an empty string, None in case of a failure
        """

        dbif, connect = init_dbif(dbif)
        statement = ""

        if self.is_in_db(dbif):
 
            # SELECT all needed information from the database
            self.metadata.select(dbif)

            # First we unregister from all dependent space time datasets
            statement += self.unregister(dbif=dbif, update=update, execute=False)

            # Remove the strds register table
            if self.get_stds_register():
                statement += "DROP TABLE " + self.get_stds_register() + ";\n"

            core.verbose(_("Delete %s dataset <%s> from temporal database") % (self.get_type(), self.get_id()))

            # Delete yourself from the database, trigger functions will take care of dependencies
            statement += self.base.get_delete_statement()

        if execute == True:
            dbif.execute_transaction(statement)

        # Remove the timestamp from the file system
        self.remove_timestamp_from_grass()
        
        self.reset(None)

        if connect == True:
            dbif.close()
 
        if execute:
            return ""

        return statement

    def unregister(self, dbif=None, update=True, execute=True):
	"""! Remove the map entry in each space time dataset in which this map is registered

           @param dbif: The database interface to be used
           @param update: Call for each unregister statement the update from registered maps 
                          of the space time dataset. This can slow down the un-registration process significantly.
           @param execute: If True the SQL DELETE and DROP table statements will be executed.
                           If False the prepared SQL statements are returned and must be executed by the caller.

           @return The SQL statements if execute == False, else an empty string
        """

	if self.get_layer():
	    core.verbose(_("Unregister %s map <%s> with layer %s from space time datasets") % \
	                   (self.get_type(), self.get_map_id(), self.get_layer()))
	else:
	    core.verbose(_("Unregister %s map <%s> from space time datasets") % (self.get_type(), self.get_map_id()))
        
        statement = ""
        dbif, connect = init_dbif(dbif)
        
        # Get all datasets in which this map is registered
        rows = self.get_registered_datasets(dbif)

        # For each stds in which the map is registered
        if rows:
            count = 0
            num_sps = len(rows)
            for row in rows:
                core.percent(count, num_sps, 1)
                count += 1
                # Create a space time dataset object to remove the map
                # from its register
                stds = self.get_new_stds_instance(row["id"])
                stds.metadata.select(dbif)
                statement += stds.unregister_map(self, dbif, False)
                # Take care to update the space time dataset after
                # the map has been unregistered
                if update == True and execute == True:
                    stds.update_from_registered_maps(dbif)

            core.percent(1, 1, 1)
            
        if execute == True:
            dbif.execute_transaction(statement)

        if connect == True:
            dbif.close()

        if execute:
            return ""

        return statement
            
    def get_registered_datasets(self, dbif=None):
        """!Return all space time dataset ids in which this map is registered as
           dictionary like rows with column "id" or None if this map is not registered in any
           space time dataset.

           @param dbif: The database interface to be used
        """
        dbif, connect = init_dbif(dbif)

        rows = None

        try:
            if self.get_stds_register() != None:
                # Select all stds tables in which this map is registered
                sql = "SELECT id FROM " + self.get_stds_register()
                dbif.cursor.execute(sql)
                rows = dbif.cursor.fetchall()
        except:
            core.error(_("Unable to select space time dataset register table <%s>") % (self.get_stds_register()))

        if connect == True:
            dbif.close()
            
        return rows

