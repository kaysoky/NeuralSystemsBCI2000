#include <stdio.h>
#include <math.h>
#include <conio.h>
#include <vcl.h>
#include "getmem.h"

MEM::MEM( void )
{
        parms= 0;
}

static float sqrarg;

#define SQR(a) (sqrarg=(a),sqrarg*sqrarg)

/***********************************************************************
        data = real vector of data
        n    = length of data vector
        m    = model order
        cof  = array of autoregressive coefficients
        pm   = scalar factor for cof - white noise driving variance
***********************************************************************/

int MEM::memcof()
{
        int k,j,i;
        float p=0.0;
        float num,denom;
		  float wk1[MAXDATA],wk2[MAXDATA],wkm[MAXDATA];

		  for (j=0;j<sp_lth;j++) p += SQR(data[j]);

        pm= p/sp_lth;       /* this statement hangs up !! */

        wk1[1]=data[0];
        wk2[sp_lth-1]=data[sp_lth-1];
        for (j=2;j<=sp_lth-1;j++)
        {
                wk1[j]=data[j-1];
                wk2[j-1]=data[j-1];
        }
        for (k=1;k<=model_order;k++)
        {
                num=0.0;
                denom=0.0;
                for (j=1;j<=(sp_lth-k);j++)
                {
                        num += wk1[j]*wk2[j];
                        denom += SQR(wk1[j])+SQR(wk2[j]);
                }

                if(denom<0.0000001) return(-1);

                cof[k]=2.0*num/denom;
                pm *= (1.0-SQR(cof[k]));
                if (k != 1)
                        for (i=1;i<=(k-1);i++)
                                cof[i]=wkm[i]-cof[k]*wkm[k-i];

                if (k == model_order)  return(1);

                for (i=1;i<=k;i++) wkm[i]=cof[i];
                for (j=1;j<=(sp_lth-k-1);j++)
                {
                        wk1[j] -= wkm[k]*wk2[j];
                        wk2[j]=wk2[j+1]-wkm[k]*wk1[j+1];
                }
        }
        return(1);
}

#undef SQR

int MEM::init_elv( )
     /* float start;             start of spectrum in % Nyquist rate */
     /* float stop;              end of spectrum- % Nyquist rate     */
     /* float delta;             resolution required- % Nyquist rate */
     /* float wpr[];             cosine lookup table                 */
     /* float wpi[];             sine lookup table                   */
{
        int i,n;
        double index,theta;

        theta= 6.28318530717959;
        n= (int)( (stop - start) / delta );
        
        if(n > NLINES) return(-1);               /* error if too large */

        for(i=0;i<n;i++)
        {
                index= start + (i  * delta);
                wpr[i]= (float) ( cos( index * theta ) ); 
                wpi[i]= (float) ( sin( index * theta ) );
        }
        return( n );
 }
        

int MEM::evlmem()
        /* float cof[];      array of autoregressive coefficients    */
        /* float pm;         power of driving white noise            */
        /* float pr[];       address of cosine lookup table          */
        /* float pi[];       address of sine lookup table            */
        /* float pwr[];      output- power spectra                   */
        /* int npts;         number of points in spectra             */
        /* int bndw;         spectral band width- # spectral points  */
        /* int m;            model order                             */
{
        int i,j,index;
        float sumr,sumi;
        float wr,wi,fwpr,fwpi,wtemp;
        float denom;

        index= ( n_bins / f_width ) + 1;
        for(j=0;j<index;j++)    pwr[j]= 0;

        for(j=0;j<n_bins;j++)
        {
                sumr= 1.0;
                sumi= 0.0;
                wr= 1.0;
                wi= 0.0;
                fwpr= wpr[j];
                fwpi= wpi[j];

                for (i=1;i<=model_order;i++)
                {
                        wr=(wtemp=wr)*fwpr-wi*fwpi;
                        wi=wi*fwpr+wtemp*fwpi;
                        sumr -= cof[i]*wr;
                        sumi -= cof[i]*wi;
                }

          index= j / f_width;
          denom= (sumr*sumr+sumi*sumi);
          if(denom>0.0000001) pwr[index]+= pm/denom;
          else return(-1);
        }
        return(index);
}

void MEM::dtrnd( void )
{
	int i;
        float mean;
	float x;
	float y;
	float xy;
	float x2;
	float y2;
	float n;
	float b;
	float a;
	float fi;

	if( trend==1 )
	{
		y= 0;
		n= 0;

		for(i=0;i<sp_lth;i++)
		{
			y+= data[i];
			n++;
		}

		if( n > 0 ) mean= y/n;

		for(i=0;i<sp_lth;i++)
		{
			data[i]-= mean;
		}
	}
	else if( trend==2 )
	{

		x=  0;
		y=  0;
		xy= 0;
		x2= 0;
		y2= 0;
		n=  0;

		for(i=0;i<sp_lth;i++)
		{
			fi= (float)i;
			xy+= fi * data[i];
			x+= fi;
			x2+= ( fi*fi );
			y+= data[i];
			y2+= ( data[i] * data[i] );
			n++;
		}

		if( n > 0 )
		{
			b= xy - ( (x * y )/n );
			b= b / ( (x2 - ( x * x  / n ) ) );

			a= ( y - (b * x) ) / n;

			for(i=0;i<sp_lth;i++)
			{
				fi= (float)(i);
				data[i]-= ( ( b*fi ) + a );
			}
		}
	}
}



/******************************************************************
        get_mem computes the MEM spectral analysis
*******************************************************************/


int MEM::get_mem( void )
{

        int i;
        int err;
        int points;

        /* initialize if parameters are new */

        if( parms == 0 )
        {
                start  = start   / hz;
                stop   = stop    / hz;
                delta  = delta   / hz;

                n_bins= init_elv();

                if(n_bins < 1)
                {
                        Application->MessageBox("Error for n_bins out of bounds",
                                mtWarning,MB_ICONWARNING|MB_OK);
                                return(-1);
                }
                else
                        parms= 1;

                /* also test for over bounds */


                points=  1 + n_bins/f_width;         /* first value in array is npoints */


                if(points>NLINES)    points= NLINES;
        }

        if( trend > 0 )
        {
                dtrnd();
        }

	if(memcof()<1)
        {
                 err= -1;
                 goto skip;
        }

        if(pm < 0)
        {
                printf("NOISE < 0 ");
                err= -2;
                goto skip;
        }

 	if( ( n_bands= evlmem() ) <1 )
        {

                err= -3;
                goto skip;
        }    

        for(i=0;i< n_bands;i++)
        {
              if(pwr[i]>0)
              {
                 pwr[i]= 2 * sqrt((double)(pwr[i]))/ (float)(f_width);
              }

              else
              {
                 pwr[i]= -4.0;
              }

        }

        return( 1 );

skip:   for(i=0;i<MAXBINS;i++)   pwr[i]= err;
        return(err);
}


void MEM::setStart( float start_val )
{
        start= start_val;
        bstart= start_val;
        parms= 0;
}
void MEM::setStop( float stop_val )
{
        stop= stop_val;
        parms= 0;
}
void MEM::setDelta( float delta_val )
{
        delta= delta_val;
        parms= 0;
}
void MEM::setHz( int Hz_val )
{
        hz= Hz_val;
        parms= 0;
}
void MEM::setBandWidth( float BandWidth )
{
        f_width= ( BandWidth +  (0.5 * delta) ) / delta;
        bandwidth= BandWidth;
        parms= 0;
}        
void MEM::setModelOrder( int vModelOrder )
{
        model_order= vModelOrder;
}
void MEM::setTrend( int vtrend )
{
        trend= vtrend;
}

int MEM::setData( int length, float values[] )
{
        int i;

        if( length > MAXDATA )
        {
                Application->MessageBox("Error: Data Exceeds Max Length",
                        mtWarning,MB_ICONWARNING|MB_OK);
                        return(-1);
        }
                        
        sp_lth= length;

        for(i=0;i<sp_lth;i++)
        {
                data[i]= values[i];
        }
        return( sp_lth );
}
int MEM::get_pwr( float values[] )
{
        int i;

        for(i=0;i<n_bands;i++)
        {
                values[i]= pwr[i];
        }

        return( n_bands );
}                
