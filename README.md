# Tidy GUI2

This project uses Qt to create a cross-platform GUI application, and uses an external libtidy so can be kept up-to-date. Additionally, uses a perl script to generate a large chunk of the tab dialogs code, so can be easily kept up-to-date as features are added or removed from libtidy source.

See `History` below for a considerable list of attempts at this. While this is a serious working attempt at a Tidy GUI, it was also done to remind myself Qt4 coding, with some perl scripting to be able to auto-generate a big chunk of code from the latest source.

It was also used to develop and test my FindTidy.cmake cmake `find_package` module.

The important issue is that using Qt as the GUI builder means it should be full cross-platform.

#### Dependencies

##### Qt

Qt can be downloaded from https://www.qt.io/download/. As well as offering 'enterprise' versions, there is also a 'free' community version. This was built against Qt4, but should not be too much effort to use later versions.

##### CMake

While Qt has is own 'qmake' system, and a Qt Designer GUI IDE, this project is built using CMake, from Kitware - http://www.cmake.org/ - which is also a robust cross-platform build file generator. In each platform it supports a range of 'native' build systems. In unix this includes the 'standard' Unix Makefiles, Ninja, etc... in windows this includes many versions of MSVC, plus MinGW, MSys, etc... and others in the Apple MAC...

##### LibTidy

And of course is dependant on library Tidy. This can be built and installed from source - https://github.com/htacg/tidy-html5 - or installed from release binaries - http://www.htacg.org/binaries/ - or in unix it may be available from distribution maintainers. 

It is important to make sure the API version is compatible. Thankfully so far this API has NOT changed much, so any version later than 5.0.0 should be ok. It will NOT work with the older libtidy-0.99.so that is still available in some distributions.

And as indicated above, if and when there are API version changes, then the output from `tidy -xml-help` needs to be saved, and the perl script tidyxmlcfg.pl run with paths to the xml-help output, AND the paths to the tidy-gui2 source files tg-dialog.cpp and tg-dialog.h so these can be updated for a re-compile of this tidy-gui2.

In this way the several pages of libtidy configuration parameters can be dynamically updated. Should be like -

```
$ cd tidy-gui2   # into the git clone source
$ cd data
$ tidy -xml-config > tidycfg.xml # get current XML config
$ perl tidyxmlcfg.pl tidycfg.xml # warning! WIP - hardcoded paths, etc...
```

It re-writes the tg-dialog.[cpp|h] with the new code, but the script needs some work, like add the dialog files to the UI, get $debug_on zerored... At present it has **hard coded** paths...

#### Building

##### Unix

Using the native make build tool, this can be as simple as -

 1. cd tidy-gui2
 2. cmake .
 3. make
 4. [sudo] make install
 
##### Windows
 
If a version of MSVC is installed, this again this can be a simple -
  
  1. cd tidy-gui2
  2. cmake .
  3. cmake --build . --config Release
  4. copy the Release\tidy-gui2.exe to a folder in your path
  5. if by chance you are using the DLL version of libtidy, then the DLL must also be copied.

There is also a CMake GUI which can be used instead of the above CLI.

#### History

There have been some previous attempts at a Tidy GUI. Some had used the Tidy binary as an external app, which is ok, but not totally satisfying. Others used a built-in tidy library code, which of course ages, and others were not cross-platform.

Some found, in no particular order -

 1. Andre Blavier's TidyGUI - http://ablavier.pagesperso-orange.fr/TidyGUI/ - circa 2001 0 This seems a MS Windows ONLY application, and a quite old built-in version of libtidy source. 
 2. Another was F0rud A GTidy-GUI for tidy - https://sourceforge.net/projects/gtidy/ - circa 2004 - This seems a Delphi pascal source, and runs the tidy binary externally, I think. Not tried. 
 3. Charles Creitzel's Tidy UI - http://users.rcn.com/creitzel/tidy.html#tidyui - circa 2003 - but seem unable to find the  source... 
 4. Dirk Paehl's GUITIDY - http://www.paehl.de/tidy/ - circa 2005 - appears another Delphi pascal source. Again seems to run tidy exe externally, but not sure Not tried.
 5. Balthisar's Tidy for Work - http://www.balthisar.com/software/tidy/ - circa 2015 - Seems for Mac OS X only. Could not find source. Not tried.
 6. PC-WELT - http://www.pcwelt.de/downloads/HTML-Tidy-GUI-1218683.html - circa 2002 - Seems Windows only. Page in German. Source not found. Not tried.
 7. DEW Associates Corp - http://www.dewassoc.com/support/useful/tidygui.htm - circa 2000 - Seems a repeat of Andre Blavier's TidyGUI!
 8. Arnaud BERCEGEAY's GEMTidy - http://gemtidy.free.fr/ - circa 2003 - Not sure what this is, or how to use it. Could not find source. Not tried.
 9. HTML Tidy GUI Front-End for Ubuntu linux - http://xmodx.com/html-tidy-gui-front-end-for-ubuntu-linux/ - circa 2008 - Not tried
 10. Dedoimedo - http://www.dedoimedo.com/computers/html-tidy.html - circa 2009 - Not tried
 11. Dave Brotherstone (bruderstein) - https://github.com/bruderstein/NppTidy2 - circa 2012 - Plugin for Notepadd++. While this was a relativley recent version of libtidy, I forked it to - https://github.com/geoffmcl/NppTidy2 - and brought it up to 4.9.30. AND added a convenient CMakeLists.txt to build it with the latest libtidy in a git submdoule. This is a very effective plugin for notepad++, a very full featured powerful editor.
 12. Tidy for Mac OS - http://www.oocities.org/terry_teague/tidy.html - circa 2004 - This looks like a mirrored page, and states it may be out of date, but it also lists some other Tidy GUI projects. Not fully explored.

And probably MANY others that I missed ;=)) Back to **THIS** Tidy GUI2 ...
 
As indicated above, this Tidy GUI2 can be compiled against an INSTALLED libtidy with a matching API. It is still neccessary to re-compile this GUI after any substantial API change, to add or remove the MANY parameters supported by the library. In this way it should be possible to easily keep it UP-TO-DATE ;=)) But that is no guarantee that it will be...

Enjoy.

v5.1.11 - 20150919 - 20150530

; eof
