"""
(C) 2013 by the GRASS Development Team
This program is free software under the GNU General Public
License (>=v2). Read the file COPYING that comes with GRASS
for details.

:authors: Soeren Gebbert
"""

import datetime
import os
import grass.script
import grass.temporal as tgis
import grass.gunittest as gunittest

class TestTRastAlgebraFails(gunittest.TestCase):

    @classmethod
    def setUpClass(cls):
        """Initiate the temporal GIS and set the region
        """
        cls.use_temp_region()
        cls.runModule("g.gisenv",  set="TGIS_USE_CURRENT_MAPSET=1")
        cls.runModule("g.region",  s=0,  n=80,  w=0,  e=120,  b=0,  t=50,  res=10,  res3=10)

    @classmethod
    def tearDownClass(cls):
        """Remove the temporary region
        """
        cls.del_temp_region()

    def test_error_handling(self):        
        # Syntax error
        self.assertModuleFail("t.rast.algebra",  expression="R == A {+,equal| precedes| follows,l} B", basename="r")
        # No STRDS
        self.assertModuleFail("t.rast.algebra",  expression="R = NoSTRDS + NoSTRDS", basename="r")
        # No basename
        self.assertModuleFail("t.rast.algebra",  expression="R = A + B")


if __name__ == '__main__':
    gunittest.test()
