**If you want to use 'flviewer' for LINUX, APPLE, MS-WINDOWS:**

* Download the FLTK library from

       http://www.fltk.org

  e.g. fltk-1.3.x-r10813.tar.bz2   
  and expand it.

* For versions < r10813 MANUALLY uncomment in

       FL/Enumerations.H

  the line

       //#define FL_ABI_VERSION 10304

  If you do not 

       #define FL_ABI_VERSION 10304

  the horizontal BAR of the tree window is not activated.

* For versions >= r10813, you can do:

       either: configure --with-abiversion=10304   
       or    : cmake -DFL_ABI_VERSION:string="10304"

  NOTE: this may be changed.

* MS-WINDOWS may need in-source configuration/compilation.

  MS-WINDOWS out-source configuration/compilation may need   
  '-DOPTION_BUILD_EXAMPLES:bool=off'.

* Compile and install the FLTK library.

* For FLVIEWER, CMAKE arguments for the OPENJPEG library may be:

       -DBUILD_SHARED_LIBS:bool=on   
       -DBUILD_FLVIEWER:bool=on   
       -DFLTK_SKIP_OPENGL:bool=on   
       -DBUILD_WITH_GERMAN:bool=off

* Compile the OPENJPEG library.

* Call 'bin/flviewer --version'.

* Call 'bin/flviewer' to test the application.

* Some files with a large number of SOP markers, e.g.

       'openjpeg-data/input/conformance/p0_07.j2k'

  with 256 tiles, may need some time. On my computer about 2 minutes.  
  The SOP markers are therefore ON only on demand (read_jpeg2000.cxx).

* The FLVIEWER menu has some buttons, inputs and outputs:

    **Tracks, Frames, Threads                     :** for MJ2   
    **Stop, Go on, Restart, Close, <-Step, Step-> :** for MJ2, JPX, JPM   
    **Tile, Reduction and Area                    :** for JP2/J2K, JPM, JPX   
    **Section and Browse                          :** for all file types   
    **Layer and Component                         :** for resp. files   

    **To use 'Section':**   
      Push the left mouse button and draw a rectangle   
      Press 'Section'   
      To remove the rectangle, click into the main window   

      In the section window use the right mouse button to select an action.   

      Only one section window can be open.   

	  Use the right mouse button to close the section window.   

      If you handle an animation of type MJ2 you must first press   
      the 'Stop' button to stop the movie before you can get a section.

* Install the OPENJPEG library.

* JPM and JPX files can be found on:   

       http://home.arcor.de/szukw000   

  or (possibly) on   

       https://share.openanalytics.eu/data/public/76cb6a.php   
       https://share.openanalytics.eu/data/public/1e7708.php

