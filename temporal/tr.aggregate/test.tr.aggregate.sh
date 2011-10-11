# Test the extraction of a subset of a space time raster dataset

# We need to set a specific region in the
# @preprocess step of this test. We generate
# raster with r.mapcalc and create two space time raster datasets
# with relative and absolute time
# The region setting should work for UTM and LL test locations
g.region s=0 n=80 w=0 e=120 b=0 t=50 res=10 res3=10 -p3

r.mapcalc --o expr="prec_1 = rand(0, 550)"
r.mapcalc --o expr="prec_2 = rand(0, 450)"
r.mapcalc --o expr="prec_3 = rand(0, 320)"
r.mapcalc --o expr="prec_4 = rand(0, 510)"
r.mapcalc --o expr="prec_5 = rand(0, 300)"
r.mapcalc --o expr="prec_6 = rand(0, 650)"

t.create --o type=strds temporaltype=absolute dataset=precip_abs1 gran="3 months" title="A test" descr="A test"
tr.register --v -i dataset=precip_abs1 maps=prec_1,prec_2,prec_3,prec_4,prec_5,prec_6 start="2001-01-15 12:05:45" increment="14 days"

# The first @test
# We create the space time raster datasets and register the raster maps with absolute time interval

t.info type=strds dataset=precip_abs1

tr.aggregate --o --v input=precip_abs1 output=precip_abs2 base=prec_sum granularity="2 days" method=average
t.info type=strds dataset=precip_abs2
tr.aggregate --o --v input=precip_abs1 output=precip_abs2 base=prec_sum granularity="1 months" method=maximum
t.info type=strds dataset=precip_abs2
tr.aggregate --o --v input=precip_abs1 output=precip_abs2 base=prec_sum granularity="2 months" method=minimum
t.info type=strds dataset=precip_abs2
tr.aggregate --o --v input=precip_abs1 output=precip_abs2 base=prec_sum granularity="3 months" method=sum
t.info type=strds dataset=precip_abs2


t.remove type=raster dataset=prec_1,prec_2,prec_3,prec_4,prec_5,prec_6
t.remove type=strds dataset=precip_abs1,precip_abs2
