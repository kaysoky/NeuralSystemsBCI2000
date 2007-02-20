/* (C) 2000-2007, BCI2000 Project
/* http://www.bci2000.org
/*/
#include "PCHIncludes.h"
#pragma hdrstop

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "getfir.h"

// FILE *firfile;

FIR::FIR( void )
{
      //  firfile= fopen("Firfille.asc","w+");
      //  fprintf(firfile,"Construction \n");
}
FIR::~FIR( void )
{
     //   fprintf(firfile,"On the Eve of Destruction \n");
     //   fclose( firfile );
}

void FIR::fircof( double f_ctr, double f_sample, double width )
{
	int i;
	double norm;
	double pi=M_PI;
      	double freq_center;
      	double freq_sample;
	double fh;
	double fl;
	double h[MAXORDER];
	double re;
	double im;
	double wdth;

	freq_center= f_ctr;
	freq_sample= f_sample;
	wdth= width/2.0;

	fh= freq_center/freq_sample+(wdth/freq_sample);
	fl= freq_center/freq_sample-(wdth/freq_sample);

	for(i=1; i<= (order/2); i++)
		h[order/2-i]= 2*(sin((fh-fl)*pi*i)*cos((fh+fl)*pi*i))/(pi*i);
	h[order/2]= 2*(fh-fl);
	for(i=0;i<(order/2);i++)
		h[order-i]= h[i];

	// Hanning window

	for(i=0;i<=order;i++)
		h[i]= h[i]*0.5*(1.0-cos(2.0*pi*(i+1.0)/(order+2.0)));
	//Normalize
	re= 0.0;
	im= 0.0;

	for(i=0;i<=order;i++)
	{
		re= re+ h[i]*cos(pi*(fh+fl)*(order-i));
		im= im+ h[i]*sin(pi*(fh+fl)*(order-i));
	}
	norm= sqrt(re*re+im*im);

	for(i=0;i<=order;i++)
	{
		hc[i]= h[i]/norm;
	}
}


void FIR::setFIR( int chan, int n_coef, float *coeff )
{
        int j;

        for(j=0;j<n_coef;j++)
        {
                coffs[chan][j]= coeff[j];
        }
        order= n_coef;
        chans= chan + 1;
}

void FIR::convolve( int chan, int count, float *data, float *result )
{
        int i,j;
        int index;
        int n;

        n= 0;
        for(i=order-1;i<count;i++)
        {
                result[n]= 0;
                for(j=0;j<order;j++)
                {
                        index= i-j;
                        result[n]+= coffs[chan][j] * data[index];
                }
                n++;
        }

}

float FIR::mean( int count, float *data )
{
        int i;
        double x;
        double mean;
        double n;
        double num;

        n= (double)(count - order );
        x= 0;

        for(i=0;i<count;i++)
        {
                x+= data[i];
        }

        if( n > 0 )
        {
                mean= x / n;
        }
        else
                mean= x;

                return( (float)mean );

}

float FIR::rms( int count, float *data )
{
        int i;
        double x;
        double x2;
        double rms;
        double n;
        double num;

        n= 0;
        x= 0;
        x2= 0;

        for(i=0;i<count;i++)
        {
                x+= data[i];
                x2+= data[i] * data[i];
                n++;
        }

        if( n > 0 )
        {
                 rms= ( x2 - ( x * x / n ) ) / ( n - 1 ) ;
                 if( rms > 0.0 ) rms= sqrt( rms );
                 else            rms= 0.0;
        }
        else
                rms= -1.0;

                return( (float)rms );

}

float FIR::max( int count, float *data )
{
        int i;
        float max;

        max= 0;

        for(i=0;i<count;i++)
                if( data[i] > max ) max= data[i];
        return( max );
}




