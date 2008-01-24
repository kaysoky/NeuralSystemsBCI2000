BCI2000Export -- create ASCII or BrainVisionAnalyzer import files from BCI2000 files.

BCI2000Export is a drag-and-drop-based program that allows to 
- import BCI2000 *.dat files into BrainProduct's VisionAnalyzer program,
- convert BCI2000 *.dat files into ASCII files.

General Usage
=============
You may learn about the program's options by starting it and using the help hints that appear when stopping the mouse inside one of its four main areas.

Output Format -- Choose between space-delimited, tabular ASCII output and BrainVisionAnalyzer Import formats.

Importing Files -- To convert files, drop them onto the "drop area" located inside its main window, or onto its application icon. Previous settings apply; BCI2000 states not found in the "import states" list will be added to the list and imported.

Excluding States -- To populate the list of import states, drop BCI2000 files onto the "import states" list. You can suppress the import of a state by unchecking its name in the list.

Channel Names -- Channel names are given as a list of strings in the order in which the respective channels appear in the file.

ASCII Export
============
An ASCII file is a space-delimited matrix with columns corresponding to channels and states, and rows corresponding to samples. The first row is a list of column headers.

BrainVisionAnalyzer Import
==========================
To use BCI2000Export most conveniently with BrainVisionAnalyzer, use the VisionAnalyzer's "Edit Workspace..." command to set the raw files folder to your BCI2000 data file folder.

BCI2000 states are mapped to the somewhat different concept of "Markers" used by the BrainVision software.
Unlike states, which are written once for each sample, markers are given as ranges in the time domain, extending over arbitrary time intervals.
In the importer program, the basic idea is to create one marker object for each state run except those runs during which the state is zero.
For multi-bit states, the state's value enters into the marker's name, e.g. "Target Code 2".
