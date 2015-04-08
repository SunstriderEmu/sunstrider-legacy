<!----------------------------------------------------------------------------->
# Install on Windows

## Compiling The Server

The server currently only builds as x86 on
Windows.

1. Get Git, and clone the repository
   (https://github.com/Nqsty/sunstrider.git).

2. Install [CMake][cmake]

3. Install [OpenSSL][openssl] 32-bit version and note the folder to which you
   install it. Caution, do not use the 'Light' version.

4. Install [Boost 1.55 32-bits for Visual Studio 2013][boost] or newer to a directory of
   your choice. Set the **environment** variable (not a CMake variable)
   `BOOST_ROOT` to wherever you installed Boost.

5. Run `cmake-gui` and indicate the repository as the source folder, and a folder of your choice to hold the compiled artifacts.

6. Click Configure, then tweak the variables that appear. In particular, you should make sure that `ACE_LIBRARY` points to the release version of the library (which is the only one included, currently). Also verify that the `SSL_EAY_*` and `LIB_EAY_*` variables point to the right version of OpenSSL (usually the one you installed at step 3). Set `CMAKE_INSTALL_PREFIX` to the location where you want the server to be installed. Also, be sure to check the `USE_EMBEDDED_MYSQL_LIB` option if you're on a 64 bits system. (*)

7. Click Generate, and select your compiler. We use Visual Studio 12 (2013). Be sure not the select the 64 bit version!

8. Open the `Sunstrider.sln` solution generated in the build directory.

9. Right-click the `INSTALL` project, then click `build`.

(*) This option is necessary because currently we do x86 builds only on Windows, but the library bundled with a x64 installation of MariaDB is a x64 one, so we bundled the 32-bits version of the library in the repo. You can remove this option if you've installed MariaDB x86. Note that using a 64-bits server with a 32-bits client does not pose any problem.

[cmake]:
http://www.cmake.org/cmake/resources/software.html

[openssl]:
http://slproweb.com/products/Win32OpenSSL.html

[boost]:
http://sourceforge.net/projects/boost/files/boost-binaries/1.55.0/boost_1_55_0-msvc-12.0-32.exe/download

<!----------------------------------------------------------------------------->
## Setting Up The DB

1. Download [MariaDB][maria_db] (5.5 series) and install it. Be sure to note down your root password, and the port you select if it not the default. If you don't already have an SQL client, you can install HeidiSQL when    proposed to do so.

2. Download a world database, e.g [this one][world_db], copy it to the `sql` folder in the repository then rename it to `world.sql` (so that it is ignored by Git and taken into account by the script at step 3).

3. If this is a first-time setup, switch to the `sql` sub-directory of the repository, then run `mysql < create_mysql.sql` then `mysql < populate_mysql.sql`.

[maria_db]:
https://downloads.mariadb.org/

[world_db]:
http://www.sunstrider.cf/files/world_2015_04_08_r6592.sql.bz2

<!----------------------------------------------------------------------------->
## Extracting maps, vmaps and dbc.

Follow these steps after compiling the core.

- Copy `dep/lib/win32_release/ACE.dll` from the **source** directory to your burning crusade install directory.

- Copy `bin/zlib1.dll` from the zlib install directory to your burning crusade install directory.

- Copy `bin/bzip2.dll` from the bzip2 install directory to your burning crusade install directory.

- Copy `src/tools/map_extractor/Release/mapextractor.exe`from the build
  directory to your burning crusade install directory.

- Copy `src/tools/mmaps_generator/Release/mmaps_generator.exe` from the build
  directory to your burning crusade install directory.

- Copy `src/tools/vmap4_extractor/Release/vmap4extractor.exe` from the build
  directory to your burning crusade install directory.

- Copy `src/tools/vmap4_assembler/Release/vmap4assembler.exe` from the build
  directory to your burning crusade install directory.

- Run `mapextractor.exe` in the game directory. This will create two directories
  named `maps` and `dbc`.

- Run `vmap4extractor.exe` in the game directory. This will create a directory
  named `Buildings`.

- Create a directory named `vmaps`, then run `vmap4assembler.exe Buildings
  vmaps` in the game directory.

- Create a directory named `mmaps`, then run `mmaps_generator.exe` in the game
  directory.

- Move the directories `maps`, `dbc`, `vmaps` and `mmaps` from your game directory to your server install location. This was the value of the `CMAKE_INSTALL_PREFIX` variable inside CMake. If you're not sure, you can re-run `cmake-gui` to check. You can delete the `Buildings` directory.

<!----------------------------------------------------------------------------->
## Install and Configure the Server  
  
- Copy to your server install directory : 
	- `ACE.dll` from your game install directory (or from the original location, see last section)  
	- `libeay32.dll` from the OpenSSL install directory.        
	- `dep/lib/win32_release/libmySQL.dll` (from there or from your MariaDB folder)
	- `dep/lib/win32_release/libircclient.dll`  
  
- Rename `authserver.conf.dist` from your server install directory to `authserver.conf`.
- Rename `worldserver.conf.dist` from your server install directory to `worldserver.conf`.

- Modify `authserver.conf` and `worldserver.conf` to put in your preferences. In particular, you need to tweak the `***DatabaseInfo` options to be able to access the database.

Local realmlist entries are already configured for BC and LK, so you don't need to add one yourself for development. If you need to, just look at the structure of the `realmlist` table in the `auth` database.

Some dummy accounts are already in the database, from `test1` to `test5`, the password are the usernames.

<!----------------------------------------------------------------------------->
## Running the Server

### Running From Visual Studio

Right click the worldserver and set as startup project.

You also need to change the output directory of the executables so that they get put somewhere where the config files and DLLs are available (typically, the
"install directory").

To change the output directories, right click the `worldserver` project, then edit Properties > Configuration Properties > Output Directory. Same for the
`authserver` project. Beware that this change will reset each time you regenerate the Visual Studio Project files using CMake!

Finally, you need to set the working directory to the folder where you put all those dll and config files, else the server won't be able to find them.

### Running From Windows

By default, the executables and debug informations will be put into
`<build_dir>/src/worldserver/<vs_config>` and
`<build_dir>/src/authserver/<vs_config>`. So for instance
`<build_dir>/src/worldserver/RelWithDebInfo`. Follow the instruction from the previous section to change that to your install directory.

Alternatively, you can copy them manually to your install directory. Normally, the INSTALL job should take care of this, but it is currently broken. You need to change the directory output path to run from VS anyway, so it's not a big annoyance.