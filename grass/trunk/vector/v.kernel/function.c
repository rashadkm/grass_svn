#include <math.h>
#include "gis.h"
#include "global.h"

double gaussian2dBySigma(d,sigma)
     /*
       probability for gaussian distribution 
     */
     double sigma, d;
{
  double res;
  res=1/(2.*M_PI*sigma*sigma)*exp(-d*d/(2.*sigma*sigma));
  return(res);
}

double gaussian2dByTerms(d,term1,term2)
     /*
       term1=1./(2.*M_PI*sigma*sigma)
       term2=2.*sigma*sigma;
     */
     double d, term1, term2;
{
  double res;
  res=term1*exp(-d*d/term2);
  return(res);
}


double segno(double x){
  double y; 
  y = (x > 0 ? 1. : 0.) + (x < 0 ? -1. : 0.);
  return y;
}


double kernel1(d,rs,lambda)
     /*
     */
     double d, rs, lambda;
{
  double res,a;
  a=lambda-1.;
  if(lambda==1.){
    res=1./(M_PI*(d*d+rs*rs));
  }else{
    res=segno(a)*(a/M_PI)*(pow(rs,2.*a))*(1/pow(d*d+rs*rs,lambda));
  }
    /*  res=1./(M_PI*(d*d+rs*rs));*/
  return(res);
}

double invGaussian2d(sigma,prob)
     double sigma, prob;
{
  double d;
  d = sqrt(-2*sigma*sigma*log(prob*M_PI*2*sigma*sigma));
  return(d);
}     

double euclidean_distance(x,y,n)
     /*
       euclidean distance between vectors x and y of length n
     */
     double *x, *y;
     int n;
{
  int j;
  double out = 0.0;
  double tmp;
  

  for(j=0;j<n;j++){
    tmp = x[j] - y[j];
    out += tmp * tmp;
  }
  

  return sqrt(out);
}


