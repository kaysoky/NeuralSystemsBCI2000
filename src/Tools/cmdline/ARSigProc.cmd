::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: File:        ARSigProc.cmd
:: Date:        Jan 10, 2005
:: Author:      juergen.mellinger@uni-tuebingen.de
:: Description: A command-line script to apply the ARSignalProcessing
::              filter chain to a BCI2000 dat file.
::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
@bci_dat2stream < "%1" |(
  TransmissionFilter   |(
  CalibrationFilter    |(
  SpatialFilter        |(
  ARFilter             |(
  ClassFilter          |(
  NormalFilter         |(
bci_stream2table       )))))))