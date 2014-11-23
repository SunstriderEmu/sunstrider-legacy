<!----------------------------------------------------------------------------->
# Install on Windows

## Compiling The Server

The server currently only builds as x86 Release (or RealeaseWithDebInfo) on
Windows.

1. Get mercurial, and clone the repository
   (ssh://hg@bitbucket.org/Kelno/windrunner).

2. Install [CMake][cmake]

3. Install [OpenSSL][openssl] 32-bit version and note the folder to which you
   install it.

4. Extract [bzip2][bzip2] to a directory of your choice.

5. Copy `install/bzip2_cmake_editme.reg` outside of the repository, edit it
   to put in the directory where you extracted bzip2, then run it.

6. Install [zlib][zlib] to a directory of your choice.

7. Install [Boost 1.55 32-bits for Visual Studio 2013][boost] to a directory of
   your choice. Set the **environment** variable (not a CMake variable)
   `BOOST_ROOT` to wherever you installed Boost.

8. Copy `install/zlib_cmake_editme.reg` outside of the repository, edit it
   to put in the directory where you installed zlib, then run it.

9. Run `cmake-gui` and indicate the repository as the source folder, and a
   folder of your choice to hold the compiled artifacts.

10. Click Configure, then tweak the variables that appear, then re-click
    Configure. In particular, you should make sure that `ACE_LIBRARY` points to
    the release version of the library (which is the only one included,
    currently). Also verify that the `SSL_EAY_*` and `LIB_EAY_*` variables point
    to the right version of OpenSSL (usually the one you installed at step 3).
    Set `CMAKE_INSTALL_PREFIX` to the location where you want the server to be
    installed. Also, be sure to check the `TOOLS` option.

11. Click Generate, and select your compiler. We use Visual Studio 12 (2013). Be
    sure not the select the 64 bit version!

12. Open the `WindRunner.sln` solution generated in the build directory.

13. Right-click the `INSTALL` project, then click `build`.

[cmake]:
http://www.cmake.org/cmake/resources/software.html

[openssl]:
http://slproweb.com/products/Win32OpenSSL.html

[bzip2]:
http://gnuwin32.sourceforge.net/packages/bzip2.htm

[zlib]:
http://sourceforge.net/projects/gnuwin32/files/zlib/

[boost]:
http://sourceforge.net/projects/boost/files/boost-binaries/1.55.0/boost_1_55_0-msvc-12.0-32.exe/download

<!----------------------------------------------------------------------------->
## Setting Up The DB

1. Download [MariaDB][maria_db] (5.5 series) and install it. Be sure to note
   down your root password, and the port you select if it not the default. If
   you don't already have an SQL client, be sure to install HeidiSQL when
   proposed to do so.

2. Download a world database, e.g [this one][world_db], copy it to the `sql`
   folder in the repository then rename it to `world.sql` (so that it is ignored
   by Mercurial and taken into account by the script at step 3).

3. If this is a first-time setup, switch to the `sql` sub-directory of the
   repository, then run `mysql < create_mysql.sql` then `mysql <
   populate_mysql.sql`.

[maria_db]:
https://downloads.mariadb.org/

[world_db]:
http://www.sunstrider.cf/files/world_20141123_r6730.sql.bz2

<!----------------------------------------------------------------------------->
## Extracting maps, vmaps and dbc.

Follow these steps after compiling the core.

- Copy `dep/lib/win32_release/ACE.dll`, ` from the **source** directory to your
  burning crusade install directory.

- Copy `bin/zlib1.dll` from the zlib install directory to your burning crusade
  install directory.

- Copy `bin/bzip2.dll` from the bzip2 install directory to your burning crusade
  install directory.

- Copy `contrib\map_extractor\Release\mapextractor.exe`from the build
  directory to your burning crusade install directory.

- Copy `contrib/mmaps_generator/Release/mmaps_generator.exe` from the build
  directory to your burning crusade install directory.

- Copy `contrib/vmap4_extractor/Release/vmap4extractor.exe` from the build
  directory to your burning crusade install directory.

- Copy `contrib/vmap4_assembler/Release/vmap4assembler.exe` from the build
  directory to your burning crusade install directory.

- Run `mapextractor.exe` in the game directory. This will create two directories
  named `maps` and `dbc`.

- Run `vmap4extractor.exe` in the game directory. This will create a directory
  named `Buildings`.

- Create a directory named `vmaps`, then run `vmap4assembler.exe Buildings
  vmaps` in the game directory.

- Create a directory named `mmaps`, then run `mmaps_generator.exe` in the game
  directory.

- Move the directories `maps`, `dbc`, `vmaps` and `mmaps` from your game
  directory to your server install location. This was the value of the
  `CMAKE_INSTALL_PREFIX` variable inside CMake. If you're not sure, you can
  re-run `cmake-gui` to check. You can delete the `Buildings` directory.

[maps]:
http://www.mediafire.com/download/rb0br7hcw5weoml/maps.rar

[dbc]:
http://canardwc.free-h.net/backup/dbc.tar.bz2

<!----------------------------------------------------------------------------->
## Install and Configure the Server

- Copy `ACE.dll`, `zlib1.dll` and `bzip2.dll` from your game install directory
  (or from the original location, see last section) to your server install
  directory.

- Rename `authserver.conf.dist` from your server install directory to
  `authserver.conf`.

- Rename `worldserver.conf.dist` from your server install directory to
  `worldserver.conf`.

- Modify `authserver.conf` to put in your preferences. In particular, you need
  to tweak `LoginDatabaseInfo` to be able to access the database.

Local realmlist entries are already configured for BC and LK, so you don't need
to add one yourself for development. If you need to, just look at the structure
of the `realmlist` table in the `wrauth` database.
