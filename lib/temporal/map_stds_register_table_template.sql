--#############################################################################
-- This SQL script generates the table listing all STDS in
-- which the GRASS_MAP map is registered
--
-- This table is map specific and created for each GRASS_MAP map which is registered 
-- in a STDS. MAP_NAME is a placeholder for the dataset name.
--
-- Author: Soeren Gebbert soerengebbert <at> googlemail <dot> com
--#############################################################################

PRAGMA foreign_keys = ON;

-- MAP_NAME is a placeholder for specific map name (SQL compliant): name_mapset
-- MAP_ID is a placeholder for specific map id: name@mapset
-- GRASS_MAP is a placeholder for specific map type: raster, raster3d or vector
-- STDS is a placeholder for specific space-time dataset type: strds, str3ds, stvds

-- This table stores the names of the space-time datasets in which this map is registered 
CREATE TABLE  MAP_NAME_STDS_register (
  id VARCHAR NOT NULL, -- This column is a primary foreign key storing the STDS names
  PRIMARY KEY (id),
  FOREIGN KEY (id) REFERENCES  STDS_base (id) ON DELETE CASCADE
);

CREATE TRIGGER MAP_NAME_STDS_register_insert AFTER INSERT ON MAP_NAME_STDS_register 
  BEGIN
    UPDATE GRASS_MAP_base SET modification_time = datetime("NOW") WHERE GRASS_MAP_base.id = "MAP_ID";
    UPDATE GRASS_MAP_base SET revision = (revision + 1) WHERE GRASS_MAP_base.id = "MAP_ID";
  END;

CREATE TRIGGER MAP_NAME_STDS_register_delete AFTER DELETE ON MAP_NAME_STDS_register 
  BEGIN
    UPDATE GRASS_MAP_base SET modification_time = datetime("NOW") WHERE GRASS_MAP_base.id = "MAP_ID";
    UPDATE GRASS_MAP_base SET revision = (revision + 1) WHERE GRASS_MAP_base.id = "MAP_ID";
  END;


