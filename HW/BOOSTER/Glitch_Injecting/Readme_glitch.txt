The Glitch injecting booster is based on the CVP Booster 3.
A schematic for the previous version of the booster 3 was
shown in the March 1997 issue of Model Railroader.  This booster
is unique in that it shuts off track power very briefly each
transition from negative to positive.  This shutoff can cause
a ringing glitch when the booster is attached to a layour with
capacitance and inductance.

You must make these changes to
the booster 3 to build a glitch injecting booster:

1. Add an additional .1 uFd capacitor to C8 for a total of .2 uFd at C8.
   This is necessary because the Booster 3 cuts off power too soon (about
   6 msec.) for the long stretched zero tests which run 10 msec.  See
   EASY_DCC_MOD.JPG for a picture of the added capacitor on the booster
   3 circuit board.

2. Build the layout impedance simulator network shown in the Orcad
   files GLITCHER1.DBK and GLITCHER1.DSN or the PDF file GLITCHER.PDF.
   This network simulates the complex impedance of a layour and induces
   this crossover ringing glitch.  Attach the network to the booster output
   as shown in the schematic.

3. Connect the booster and verify that the glitch is present at each signal
   transition.  An oscilloscope should show a signal similar to CVP_NET.PDF.