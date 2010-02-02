::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: $Id$
:: Author:      juergen.mellinger@uni-tuebingen.de
:: Description: A command-line script to apply the P3SignalProcessing
::              filter chain to a BCI2000 dat file.
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
pushd "%~dp0"
@bci_dat2stream < "%1" |(
  TransmissionFilter   |(
  SpatialFilter        |(
  P3TemporalFilter     |(
  LinearClassifier     |(
  Normalizer           |(
bci_stream2table       )))))))
popd
