# Install on Linux

<!----------------------------------------------------------------------------->
## Requirements

1. Get the right mariadb repository [here][mariadbrepository] (version 5.x). Mysql is also supported but you may encounter problems when importing the database.

2. sudo apt-get install g++ gcc make cmake libmysql++-dev openssl libssl-dev zlib1g-dev libtool libmariadbclient-dev patch build-essential mariadb-server mariadb-client git libreadline-dev libbz2-1.0 libbz2-dev libboost-dev libboost-thread-dev libboost-system-dev libboost-filesystem-dev libboost-program-options-dev libboost-regex-dev

[mariadbrepository]:
https://downloads.mariadb.org/mariadb/repositories/

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
`make install -j <x>`   <sub>where <x\> is a number of processors to use for the compilation.</sub>   
	This will install the server binaries in `<root_install_folder>/bin`, and the configurations files in `<root_install_folder>/etc`


<!----------------------------------------------------------------------------->
## Setting Up The DB

The server use 4 differents databases. 
 
1. If this is a first-time setup, switch to the `sql` sub-directory of the
   repository, then run `mysql -u root -p < create_mysql.sql`. This will create the four empty databases: `auth`, `characters`, `logs`, `world`.
2. From the `sql` directory, import 'auth.sql', 'characters.sql' and 'logs.sql' into their corresponding databases.
3. Download the last [World Database][worlddatabase], and import it into the `world` database.
4. Apply every 'world', 'char', 'auth' updates in the `sql/updates` directory,
   applying them from the lowest date to the highest.

   You can do it by hand, or under bash, this can be achieved by running the `apply_updates.sh` script that is in the `sql` directory.

   <sub>*When the database creation scripts are updated, those updates are moved to the `sql/updates/old` directory and are not needed anymore.*</sub>

As an aside, you can dispense yourself from typing your mysql password if you [configure the
user and password in the `.my.cnf` file][autologin].

[worlddatabase]:
https://github.com/kelno/sunstrider/releases
[autologin]:
http://stackoverflow.com/questions/19171021/auto-authenticate-password-in-mysql

<!----------------------------------------------------------------------------->
## Extracting maps, vmaps and dbc

You can skip this step by downloading the already extracted folders [dbc][dbc], [maps][maps], [vmaps][vmaps], [mmaps][mmaps], and put them into the `<root_install_folder>/data` folder.

Otherwise, if those files were missing, outdated, if if you want to do it by yourself, here are how to make them.

1. If everything compiled correctly, the following binaries should have been created in your `<root_install_folder>/bin` folder:  

	- **mapextractor**
	- **vmap4extractor**
	- **vmap4assembler**
	- **mmaps_generator**
	
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
  directory to your `<root_install_folder>`. You can delete the `Buildings` directory.

[dbc]:
http://www.sunstrider.cf/files/dbc.tar.bz2
[maps]:
http://www.sunstrider.cf/files/maps.tar.bz2
[vmaps]:
http://www.sunstrider.cf/files/vmaps.tar.bz2
[mmaps]:
http://www.sunstrider.cf/files/mmaps.tar.bz2

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
to add one yourself for development. If you need to, just look at the structure
of the `realmlist` table in the `auth` database.

Some dummy accounts are already in the database, from `test1` to `test5`. The
password are the usernames.

<!----------------------------------------------------------------------------->
## Running the Server

Just run both the **worldserver** and the **authserver** in the `<root_install_folder>/bin` folder.  

Sunstrider now also has some auto restarter scripts that you can use, see [this wiki page][restarterscripts].
[restarterscripts]:
http://www.sunstrider.cf/wiki/Restarter_scripts