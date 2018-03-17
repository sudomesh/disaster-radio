kicad-ESP8266 - a library for KiCAD
=====================================
This project has both schematic symbols, footprints, and 3d models for
ESP8266 based modules. It is a work-in-progress. Additions and
corrections are welcome. Enter a pull request or file an issue on
GitHub.

The present modules are:

  * ESP-01  - Note that there are two versions of this module available on
              open market. The schematic symbol is for the `v090` version
              which is probably the same as the `v080`. There is an older
              version that does not include any GPIO signals. The footprint
              is the same for all of the ESP-01 modules.

  * ESP-12  - This is includes an alias part, ESP-07v2, that is suitable
              for the 16 pin variant of the ESP-07. Note that there is a 14
              pin version of the ESP-07 that has different signals and
              pinout. The footprint for the ESP-12 (and ESP-07v2)
              elongated through-hole pads to match those modules.

  * ESP-201 - This is a 26 pin module with pins on 0.1 inch spacing. I
              could not find any official measurements for the pin
              locations so the footprint was created from empirical
              measurements on the unit I have.

  * ESP-12E - This is for the 22 pin variant of the ESP-12. ~~Though all the 
              esp-12 signals are in the same place on ESP-12E, pin numbers
              have changed.~~ As of 2016/12/06 the pin numbers on the
              ESP-12E have been changed to be compatible with the
              ESP-12. If you need the old pin numbers they are still
              available on the `oldNumbers` branch.

  * ESP-07v2 - ESP-07 that is pin compatible with the ESP-12.

  * ESP-13-WROOM-02

  * There is a footprint for the ESP-06, but no supporting
      schematic symbol.

There are 3d models for the ESP-07v2, ESP-13, ESP-12 and ESP-12E.

WARNINGS
---------
  1. The symbols, footprints, and 3d models have not been tested. They are
     based on published images and measurements on individual samples.

  2. The `.dcm` file contains HTML links to information on sites I used
     when creating this library, however I do not have any control over
     the sites themselves. Follow the links with caution.

  3. This library distributed in the hope that it will be useful,
     but __WITHOUT ANY WARRANTY; without even the implied warranty of__
     __MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE__.


Use
---
 1. Clone this archive to a convenient location on your computer. These are
    the commands I use on Linux to create a copy in
    `~/kicad/libraries/kicad-ESP8266`:

        mkdir -p ~/kicad/libraries/
        cd ~/kicad/libraries/
        git clone https://github.com/jdunmire/kicad-ESP8266

 2. Add `ESP8266.lib` to the Component Libraries:

        eeschema:Preferences->Component Libraries->Add

 3. Add `ESP8266.pretty` to the Footprint Libraries:

        pcbnew:Preferences->Footprint Libraries Wizard

 4. To make the 3d models usable, add the location from step #1 to list
    of configured paths as `ESPLIB`. Use a full path. Continuing with
    the example step #1:

        kicad:Preferences->Configure Paths->Add

            Name: ESPLIB
            Path: /home/<your_login_name>/kicad/libraries/kicad-ESP8266


3D Models
---------
The 3D models were built using [OpenSCAD](http://www.openscad.org/) and
then colored in [Wings3D](http://www.wings3d.com/) to produce the VRML
(`.wrl`) format for KiCAD. Wings3D requires a binary STEP format, but
OpenSCAD writes an ASCII version.
[meshconv](http://www.patrickmin.com/meshconv/) will convert the
ASCII format to a binary format.

Here is the workflow I used:

 1. Use OpenScad to build the model. Export the model as STL.
    * Build using F6
    * File->Export...->Export as STL...

 2. Use `meshconv` to translate the file to binary.

        $ meshconv -c stl <STL-FILE-FROM-OpenSCAD> -o <FILENAME>

 3. Now open Wings3d and import the the `FILENAME.stl` as a
 _StereoLithography Binary File (*.stl)_.

 4. Assign colors.

 5. Export as _VRML 2.0 File (*.wrl)_

 6. The 3D model probably needs to be scaled to match the footprint. Use
    the `kicad:Footprint Editor:Edit->Edit Properties->3D Settings`
    dialog. For the ESP-12, the `Shape Scale` X, Y, and Z values are
    0.393700.

This repository includes all the intermediate files:

  * `.scad` - OpenSCAD model
  * `.stl` - OpenSCAD STL
  * `_4wings.stl` - STL file converted by `meshconv`
  * `.wings` - Wings3D model
  * `.wrl` - VRML model for KiCAD


License
-------
Copyright 2015,2016 J.Dunmire

Contact: jedunmire PLUS kicad-ESP8266 AT gmail DOT com

This file is part of kicad-ESP8266. 

kicad-ESP8266 is licensed under the Creative Commons Attribution-ShareAlike
4.0 International License. To view a copy of this license visit
http://creativecommons.org/licenses/by-sa/4.0/.
