#include "stepwisefit.h"
#include <iostream>

#define TRUE 1
#define FALSE 0

using namespace std;
///////////////////////////////////////////////////////////////////
/// Apply the Stepwise Linear Discriminant Analysis (SWLDA) classifier
/// to a given data. SWLDA models a response variable as a function of
/// the predictor variables represented by the columns of the input data.
/// The provided data must have ROWS > COLUMS.
/// @param [in] X         Given data
/// @param [in] y         Response variable
/// @param [in] penter    parameter penter
///	@param [in] premove   parameter premove
/// @param [in] maxiter   parameter Maximum number of features
/// @param [out] B        Vector of estimated coefficient values for all columns of X
/// @param [out] SE       Vector os standard errors of B
/// @param [out] PVAL     Vector of p-values for testing if B is 0
/// @param [out] in       Logical vector indicating which predictors are in the final model
/// \author Cristhian Potes
/// \date May 30, 2009
/// Reference: Draper, N. R., and H. Smith. Applied Regression Analysis, John Wiley & Sons, 1966. pp. 173-216

void stepwisefit(const ap::real_2d_array& X, const ap::real_1d_array& y, const double penter, const double premove,
                 const int max_iter, ap::real_1d_array& B, ap::real_1d_array& SE, ap::real_1d_array& PVAL,
                 ap::boolean_1d_array& in, CALLBACK_STATUS callback_status)
{
  ///////////////////////////////////////////////////////////////////
  // Section: Define variables
  bool FLAG = TRUE;
  int swap;
  int iter=1;
  ostringstream oss;
  ///////////////////////////////////////////////////////////////////
  // Section: Compute B, SE, PVAL, and the next step
  while(FLAG==TRUE)
  {
    if (callback_status != NULL)
    {
      oss << "Added feature... " << "[" << iter << "/" << max_iter << "]";
      callback_status(oss.str());
      oss.str("");
    }
    else
      printf("Added feature... %d\n", iter);

    stepcalc(X, y, in, B, SE, PVAL);
    swap = stepnext(in, PVAL, penter, premove);
    if (swap==-1)
      FLAG=FALSE;
    else
      in(swap)=(!(in(swap)));

    iter++;
    if (iter>max_iter)
      FLAG = FALSE;
  }
  printf("\n");
}

