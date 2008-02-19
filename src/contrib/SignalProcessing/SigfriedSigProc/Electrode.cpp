#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "Electrode.h"

///////////////////////////////////////////////////////////////////////////////
/// Makes a new instance of the class and stores the paramters.
/// @param #xpos x-position.
/// @param #ypos y-position.
/// @param #zpos z-position.
/// @param #label label that describes the electrode.
/// @param #color color in RGB of the electrode.
/// @param #marked <B>true</B> if the electrode should be marked else <B>false</B>.
/// @param #colormarker color in RGB of the marker.
/// @param #brunningaverage defines filter to be a running average filter,
///                        overwrite all other filter settings.
///////////////////////////////////////////////////////////////////////////////
CElectrode::CElectrode(float xpos, float ypos, float zpos, string label, TColor color, bool marked, TColor colormarker, float learningratehist, float learningrateaverage, float baselineresolution, bool brunningaverage, int statisticdisplaytype)
{
  this->xpos                = xpos;
  this->ypos                = ypos;
  this->zpos                = zpos;
  this->label               = label;
  this->color               = color;
  this->colormarker         = colormarker;
  this->marked              = marked;
  this->value               = 0;
  this->valuereference      = 0;

  this->brunningaverage     = brunningaverage;
  this->learningratehist    = learningratehist;
  this->learningrateaverage = learningrateaverage;
  this->baselineresolution  = baselineresolution;
  this->baselinemin         = 0;
  this->baselinemax         = 1;
  this->blearnhistograminit = false;
  this->blearnbaselineinit  = false;
  this->baseline            = this->baselinemin;

  this->statisticdisplaytype=statisticdisplaytype;

  if (fabs(this->learningratehist) > 1e-16 && fabs(this->learningrateaverage) > 1e-16 && fabs(this->baselineresolution) > 1e-16) {
    this->histogram.resize((int)((this->baselinemax-this->baselinemin) / this->baselineresolution));

    for (unsigned int i=0; i<histogram.size(); i++) {
      histogram[i] = 0;
    }
  }


  this->counterrunningaverage                  = 0;
  this->counterrunningaveragereference         = 0;
  this->valuesumrunningaverage                 = 0;
  this->valuesumrunningaveragesquared          = 0;
  this->valuesumrunningaveragereference        = 0;
  this->valuesumrunningaveragereferencesquared = 0;


  this->colormarkerpool.push_back(clBlack);
  this->colormarkerpool.push_back(clBlue);
  this->colormarkerpool.push_back(clGreen);
  this->colormarkerpool.push_back(clPurple);
  this->colormarkerpool.push_back(clRed);
  this->colormarkerpool.push_back(clYellow);

  this->indexcolormarker = 0;
  this->maxcolormarker   = this->colormarkerpool.size();

}

///////////////////////////////////////////////////////////////////////////////
/// Destroys the instance of this class.
///////////////////////////////////////////////////////////////////////////////
CElectrode::~CElectrode()
{
}

///////////////////////////////////////////////////////////////////////////////
/// Stores the value of this electrode in #value and:
/// - learns the baseline and histogram.
/// - calculates the running average of all values set so far.
/// .
///////////////////////////////////////////////////////////////////////////////
void CElectrode::SetValue(float value)
{

  // set value
  this->value = value;

  // if learning of the baseline and histogram is enabled
  if (fabs(learningratehist) > 1e-16 && fabs(learningrateaverage) > 1e-16 && fabs(baselineresolution) > 1e-16 && value >= this->baselinemin) {
    // learn the baseline and histogram
    Learn(value);
  } else {

    // if the running average filter is enabled
    if (brunningaverage) {
      // calculate the running average of all values
      counterrunningaverage++;
      valuesumrunningaverage+=value;
      valuesumrunningaveragesquared+=value*value;
    }
  }
}

///////////////////////////////////////////////////////////////////////////////
/// Stores the reference value of this electrode in #valuereference and:
/// - calculates the running average of all values set so far.
/// .
///////////////////////////////////////////////////////////////////////////////
void CElectrode::SetValueReference(float valuereference)
{
  this->valuereference = valuereference;

  if (brunningaverage) {
    counterrunningaveragereference++;
    valuesumrunningaveragereference+=valuereference;
    valuesumrunningaveragereferencesquared+=valuereference*valuereference;
  }
}

///////////////////////////////////////////////////////////////////////////////
/// Returns depending on the #statisticdisplaytype and configuration:
/// - r_square value
/// - z_score value
/// - difference of means value
/// - running average value
/// - current value
/// .
///////////////////////////////////////////////////////////////////////////////
float CElectrode::GetValue()
{

  if (brunningaverage) {
    if (counterrunningaverage > 0) {
      if (counterrunningaveragereference > 0) {

        // r-square
        if (statisticdisplaytype == r_square) {

          double G = pow((valuesumrunningaverage + valuesumrunningaveragereference),2) / (double)(counterrunningaverage+counterrunningaveragereference);

          if (fabs(G) < 1e-20) {
            return(0);
          } else {
            double numerator = pow(valuesumrunningaverage,2)                                   / (double)(counterrunningaverage)
                             + pow(valuesumrunningaveragereference,2)                          / (double)(counterrunningaveragereference)
                             - pow(valuesumrunningaverage+valuesumrunningaveragereference,2)   / (double)(counterrunningaverage+counterrunningaveragereference);

            double denominator = valuesumrunningaveragesquared + valuesumrunningaveragereferencesquared - G;

            return(numerator / denominator);

          }

        // difference of means
        } else if (statisticdisplaytype == difference_of_means) {

          return fabs(valuesumrunningaverage / (double)counterrunningaverage  - valuesumrunningaveragereference / (double)counterrunningaveragereference);

        // z-score
        } else if (statisticdisplaytype == z_score) {

          double mean_this  = valuesumrunningaverage                 / (double)counterrunningaverage;
          double mean_ref   = valuesumrunningaveragereference        / (double)counterrunningaveragereference;
          double mean_ref2  = valuesumrunningaveragereferencesquared / (double)counterrunningaveragereference;
          double std_ref    = mean_ref2 - pow(mean_ref,2);

          double zscore = 0;

          if (fabs(std_ref) > 1e-16) {
            zscore     = fabs(mean_this-mean_ref) / std_ref;
          } else {
            zscore     = 0;
          }

          return zscore;

        // no valid statistic setting
        } else {
          return 0;
        }

      } else {
        return valuesumrunningaverage/(double)counterrunningaverage;
      } // if (counterrunningaveragereference > 0)
    } else {
      return this->value;
    }

  } else if (fabs(this->learningratehist) > 1e-16 && fabs(this->learningrateaverage) > 1e-16 && fabs(this->baselineresolution) > 1e-16) {
    return this->value - this->baseline;
  } else {
    // if buffer is not filtered up return last value;
    return this->value;
  }


}

///////////////////////////////////////////////////////////////////////////////
/// Returns x-position in stored in #posx
///////////////////////////////////////////////////////////////////////////////
float CElectrode::GetXpos()
{
  return this->xpos;
}

///////////////////////////////////////////////////////////////////////////////
/// Returns y-position in stored in #posy
///////////////////////////////////////////////////////////////////////////////
float CElectrode::GetYpos()
{
  return this->ypos;
}

///////////////////////////////////////////////////////////////////////////////
/// Returns z-position in stored in #posz
///////////////////////////////////////////////////////////////////////////////
float CElectrode::GetZpos()
{
  return this->zpos;
}


///////////////////////////////////////////////////////////////////////////////
/// Returns electrode description in stored in #label.
///////////////////////////////////////////////////////////////////////////////
string CElectrode::GetLabel()
{
  return this->label;
}

///////////////////////////////////////////////////////////////////////////////
/// Sets the marker to be marked or not.
/// @param value <B>true</B> if electrode should be marked else <B>false</B>
///////////////////////////////////////////////////////////////////////////////
void CElectrode::SetMarker(bool value)
{
  this->marked = value;
}

///////////////////////////////////////////////////////////////////////////////
/// Return the current status of the makrer that is stored in {@link #marked}.
/// @returns <B>true</B> if marked else <B>false</B>.
///////////////////////////////////////////////////////////////////////////////
bool CElectrode::GetMarker()
{
  return this->marked;
}

///////////////////////////////////////////////////////////////////////////////
/// Return the current color of the electrode stored in {@link #color}.
///////////////////////////////////////////////////////////////////////////////
TColor CElectrode::GetColor()
{
  return this->color;
}

///////////////////////////////////////////////////////////////////////////////
/// Return the current color of the marker stored in {@link #colormarker}.
///////////////////////////////////////////////////////////////////////////////
TColor CElectrode::GetColorMarker()
{
  return this->colormarker;
}

///////////////////////////////////////////////////////////////////////////////
/// Toggles the status of the marker stored in {@link #marked}.
///////////////////////////////////////////////////////////////////////////////
void CElectrode::ToggleMarker()
{
  SetMarker(!GetMarker());
}

///////////////////////////////////////////////////////////////////////////////
/// Toggles the status of the color of the marker store in {@link #colormarker}
/// in the following order:
/// - black
/// - blue
/// - green
/// - purple
/// - red
/// - yellow
/// .
///////////////////////////////////////////////////////////////////////////////
void CElectrode::ToggleMarkerColor()
{
  colormarker = colormarkerpool[++indexcolormarker % maxcolormarker];
}


///////////////////////////////////////////////////////////////////////////////
/// Subsequently #LearnHistogram and #LearnBaseline.
///////////////////////////////////////////////////////////////////////////////
void CElectrode::Learn(float value)
{
  LearnHistogram(value);

  LearnBaseline();
}

///////////////////////////////////////////////////////////////////////////////
/// Learns the histogram of all set values.
/// Ages the histogram using an 1-st order IIR filter with the filter
/// coefficient #learningratehist.
/// The histogram is established with the resolution of #baselineresolution
/// and resized if any value exceedes the range of #baselinemin to #baselinemax.
///////////////////////////////////////////////////////////////////////////////
void CElectrode::LearnHistogram(float value)
{
  unsigned int index;
  int size_old;

  // calculates the index of the histogram bin that is being incremented
  index = (unsigned int) (value / this->baselineresolution);

  // if index is out of the range resize histogram
  if (index >= this->histogram.size()) {
    // increase the range twofold until range is sufficient
    do {

      // resize histogram
      size_old = this->histogram.size();
      this->histogram.resize(this->histogram.size()*2);
      this->baselinemax = this->baselinemax * 2;

      // initialize new index bins
      for (unsigned int i=size_old; i<this->histogram.size(); i++) {
        this->histogram[i] = 0;
      }

    } while (index >= this->histogram.size());
  }


  // if this was called the firsttime just initialize the histogram
  if (!this->blearnhistograminit) {
    this->histogram[index] = 1;
    this->blearnhistograminit = true;

  } else {
    // if this was called before age the histogram using an IIR filter
    // with the IIR filter coefficients learningratehist
    for (unsigned int i=0; i<this->histogram.size(); i++) {
      this->histogram[i] = this->histogram[i] * this->learningratehist;
    }

    // IIR filter equation
    this->histogram[index] = this->histogram[index] + 1 * (1-this->learningratehist);

  }
}


///////////////////////////////////////////////////////////////////////////////
/// Learns the baseline as the 5%-percentile of all set values.
/// Ages the baseline using an 1-st order IIR filter with the filter
/// coefficient #learningrateaverage.
/// The baseline is learned using the #histogram and aged using an 1st-order
/// IIR filter.
///////////////////////////////////////////////////////////////////////////////
void CElectrode::LearnBaseline()
{
  float lower_quantile;
  unsigned int   index;
  float sum=0;

  // if this is initialized the first time
  if (!this->blearnbaselineinit) {
    // just set the baseline to the current value
    this->baseline = value;
    this->blearnbaselineinit = true;
  } else {

    index = 0;

    // sum the histogram until the 5%-percentile is reached
    do {
      sum+=histogram[index++];
    } while (sum < 0.05 && index < this->histogram.size());

    // calculates the 5%-percentile
    lower_quantile = (float)index / (float)this->histogram.size() * (this->baselinemax-this->baselinemin);

    // ages the 5%-percentile using an IIR filter with the filter coefficient learningrateaverage
    this->baseline = this->baseline * this->learningrateaverage + lower_quantile * (1-this->learningrateaverage);
  }
}

