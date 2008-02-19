#ifndef ElectrodeH
#define ElectrodeH


#include <stdlib.h>
#include <stdio.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <Graphics.hpp>

using namespace std;

///////////////////////////////////////////////////////////////////////////////
/// CElectrode class
/// @author <a href="mailto:pbrunner@wadsworth.org">Brunner Peter</a>
/// @version 1.0
/// @brief This class represents the basic properties of an electrode
/// - position ({@link #xpos},{@link #ypos},{@link #zpos})
/// - {@link #label}
/// - {@link #color}
/// - marker ({@link #marked},{@link #colormarker})
/// - filter ({@link #orderlowpass},{@link #orderlowpass})
/// .
/// as well as methods to access these properties in an ecapsulated way.
///////////////////////////////////////////////////////////////////////////////
class CElectrode
{
  public:

    CElectrode(float xpos, float ypos, float zpos, string label, TColor color, bool marked=false, TColor colormarker=clBlack , float learningratehist=0, float learningrateaverage=0, float baselineresolution=0, bool brunningaverage=false, int statisticdisplaytype=0);
    ~CElectrode();


    void        SetValue          (float value);
    void        SetValueReference (float value);
    float       GetValue          ();
    float       GetXpos           ();
    float       GetYpos           ();
    float       GetZpos           ();
    string      GetLabel          ();
    void        SetMarker         (bool value);
    bool        GetMarker         ();
    TColor      GetColor          ();
    TColor      GetColorMarker    ();
    void        Learn             (float value);
    void        LearnHistogram    (float value);
    void        LearnBaseline     ();

    virtual void ToggleMarker     ();
    virtual void ToggleMarkerColor();

    enum TDisplayType
    {
        z_score              = 0,
        difference_of_means  = 1,
        r_square             = 2,
    };

  private:

    /// Stores the current value which is set by {@link #SetValue}.
    float       value;

    float       valuereference;

    /// x-position set by constructor {@link #CElectrode}.
    float       xpos;

    /// x-position set by constructor {@link #CElectrode}.
    float       ypos;

    /// z-position set by constructor {@link #CElectrode}.
    float       zpos;

    /// Label set by constructor {@link #CElectrode}.
    string      label;

    /// Marker set <B>true</B> if marked else <B>false</B>
    /// by constructor {@link #CElectrode} and can be toggled by
    /// {@link #ToggleMarkerColor}.
    bool        marked;

    /// Color of electrode set by constructor {@link #CElectrode}.
    TColor      color;

    /// Color of marker set by constructor {@link #CElectrode} and can
    /// be switched by {@link #ToggleMarkerColor}.
    TColor      colormarker;

    /// Pool of markers that is filled up in constructor {@link #CElectrode}
    /// and used in {@link #ToggleMarkerColor}.
    vector<TColor> colormarkerpool;

    /// Tracks the current used {@link #colormarker} from the possible values
    /// of {@link #colormarkerpool}.
    int            indexcolormarker;

    /// Number of colors in {@link #colormarkerpool} is set in constructor
    /// {@link #CElectrode}
    int            maxcolormarker;

    /// Sum of the all values that have been stored using using #SetValue
    /// in case #runningaverage was set <B>true</B>.
    double        valuesumrunningaverage;

    /// Sum of the all squared values that have been stored using using #SetValue
    /// in case #runningaverage was set <B>true</B>.
    double        valuesumrunningaveragesquared;

    /// Sum of the all values that have been stored using using #SetValueReference.
    double        valuesumrunningaveragereference;

    /// Sum of the all squared values that have been stored using using #SetValueReference.
    double        valuesumrunningaveragereferencesquared;

    /// Number of all all values that have been stored using using #SetValue
    /// in case #runningaverage was set <B>true</B>.
    unsigned long counterrunningaverage;

    /// Number of all all values that have been stored using using #SetValueReference
    /// in case #runningaverage was set <B>true</B>.
    unsigned long counterrunningaveragereference;

    /// Defines the filter to be a running average.
    /// Overwrites all other filters.
    bool          brunningaverage;

    /// Defines the coefficient of the 1st-order IIR filter that learns the
    /// histogram of the value over the time.
    float         learningratehist;

    /// Defines the coefficient of the 1st-order IIR filter that learns the
    /// average of the electrode over the time.
    float         learningrateaverage;

    /// Defines the resolution of the #histogram.
    float         baselineresolution;

    /// Defines the current min range of the #histogram.
    float         baselinemin;

    /// Defines the current max range of the #histogram.
    float         baselinemax;

    /// Defines the statistic type of the display according to #TDisplayType.
    int           statisticdisplaytype;

    /// Stores the histogram of all values set by #SetValue.
    vector<float> histogram;

    /// Stores the baseline of all values set by #SetValue.
    float         baseline;

    /// Set true after the first call of #LearnHistogram.
    bool          blearnhistograminit;

    /// Set true after the first call of #LearnBaseline.
    bool          blearnbaselineinit;

};

#endif
