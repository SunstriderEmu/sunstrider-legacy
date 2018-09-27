# Install on Linux

<!----------------------------------------------------------------------------->
## Requirements

1. Get the right mariadb repository [here][mariadbrepository] (version 10.x). MySQL should also work but is not supported, and you may encounter problems when importing the databases.

2. Get the right LLVM repository [here][llvmrepository]. Current supported clang version is 7.

3. Add LLVM key `wget -O - http://apt.llvm.org/llvm-snapshot.gpg.key|sudo apt-key add -`

4. sudo apt-get install clang-7 make cmake libmysql++-dev openssl libssl-dev zlib1g-dev libtool libncurses5-dev libmariadbclient-dev patch build-essential mariadb-server mariadb-client git libreadline-dev libbz2-1.0 libbz2-dev apt-get install libboost1.67-dev libboost-thread1.67-dev libboost-system1.67-dev libboost-filesystem1.67-dev libboost-program-options1.67-dev libboost-regex1.67-dev libboost-iostreams1.67-dev 

Clang is now the main supported compiler, but gcc should work too.

[mariadbrepository]:
https://downloads.mariadb.org/mariadb/repositories/
[llvmrepository]:
http://apt.llvm.org/

<!----------------------------------------------------------------------------->
## Compiling The Server

<sub>*Note that the folder structure I use here is merely a suggestion, but I will assume you're using it to keep these instructions simple*</sub>

1. Create a root folder for your emulator installation. I will refer to this folder as `<root_install_folder>`.  
Inside, create the following folders :  
	- **build**  
	- **data** 
	
2. Move this repository in the `<root_install_folder>` as a `sources` folder, so that the file you're currently reading should be in ` <root_install_folder>/sources/doc`.  
	In the `<root_install_folder>/build` folder, create a script containing :  

		cmake -DCMAKE_INSTALL_PREFIX="<root_install_folder>"  ../sources

	Then execute it (also from the build folder). This will create the make files into your build folder.  

3. The server is now ready to be compiled, stay in the same folder and type:  
`make install -j <x>`   <sub>where x is a number of processors to use for the compilation.</sub>   
	This will install the server binaries in `<root_install_folder>/bin`, and the configurations files in `<root_install_folder>/etc`

<!----------------------------------------------------------------------------->
## Setting Up The DB

1. Download [MariaDB][maria_db] (10.1 series) and install it. Be sure to note
   down your root password, and the port you select if it not the default. If
   you don't already have an SQL client, you can install HeidiSQL when proposed
   to do so.  
   /!\ Only 10.1 series or lower are supported at the moment.

2. Download the last world database [here][world_db], extract it to your CMAKE_INSTALL_PREFIX directory. 

3. You may follow the instructions [here][trinity_db_instructions]. Just use the world you just downloaded instead.

[maria_db]:
https://downloads.mariadb.org/

[world_db]:
https://github.com/ValorenWoW/sunstrider-core/releases

[my_ini]:
http://www.avajava.com/tutorials/lessons/how-do-i-log-on-to-mysql-automatically.html

[trinity_db_instructions]:
https://trinitycore.atlassian.net/wiki/spaces/tc/pages/2130092/Databases+Installation

<!----------------------------------------------------------------------------->
## Extracting maps, vmaps and dbc

You can skip this step by downloading [the already extracted data folders][githubdata] and extracting them into your `<root_install_folder>/data` folder.

Otherwise, if those files were missing, outdated, if if you want to do it by yourself, here are instructions to extract them:

1. If everything compiled correctly, the following binaries should have been created in your `<root_install_folder>/bin` folder:  

	- **mapextractor**
	- **vmap4extractor**
	- **vmap4assembler**
	- **mmaps_generator**
	
	If not, make sure the TOOLS cmake option was enabled when building.  
	You'll need a WoW client to extract to use them. Copy all those binaries into the WoW 2.4.3 client folder.
2. Run `mapextractor.exe` in the game directory. This will create two directories
  named `maps` and `dbc`.

3. Run `vmap4extractor.exe` in the game directory. This will create a directory
  named `Buildings`.

4. Create a directory named `vmaps`, then run `vmap4assembler.exe Buildings
  vmaps` in the game directory.

5. Create a directory named `mmaps`, then run `mmaps_generator.exe` in the game
  directory.

6. Move the directories `maps`, `dbc`, `vmaps` and `mmaps` from your game
  directory to `<root_install_folder>/data`. You can delete the `Buildings` directory generated at step 3.

[githubdata]:
https://github.com/ValorenWoW/sunstrider-core/releases

<!----------------------------------------------------------------------------->

## Install and Configure the Server

1. The following files have been installed to your `<root_install_folder>/etc` folder :
	- **authserver.conf.dist**
	- **worldserver.conf.dit**
	
	Rename those files as `authserver.conf` and `worldserver.conf`.
2. Update `authserver.conf` options:  
	- **LoginDatabaseInfo**: See info in conf file.
3. Update `worldserver.conf` options:  
	- **LoginDatabaseInfo**: See info in conf file.
	- **WorldDatabaseInfo**: See info in conf file.
	- **CharacterDatabaseInfo**: See info in conf file.
	- **LogsDatabaseInfo**: See info in conf file.
	- **DataDir**: Set this your `<root_install_folder>/data` folder.
	- **LogsDir***: Because you'll probably want logs. See info in conf file. I suggest setting this `<root_install_folder>/logs` (create the folder yourself).

Local realmlist entries are already configured in database for BC and LK, so you don't need
to add one yourself for development. 

Some dummy accounts are already in the database, from `test1` to `test5`. The
password are the usernames.

<!----------------------------------------------------------------------------->
## Running the Server

Just run both the **worldserver** and the **authserver** in the `<root_install_folder>/bin` folder.  

Sunstrider also has some generic systemd scripts that you can use, see [this directory][restarterscripts].
[restarterscripts]:
https://github.com/ValorenWoW/sunstrider-core/tree/master/contrib/sunstrider
