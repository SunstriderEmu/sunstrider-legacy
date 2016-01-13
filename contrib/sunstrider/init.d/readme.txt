(MediaWiki syntax)

Sunstrider is now shipped with some restarters scripts/services.
This system uses 4 differents files:

<blockquote>
'''sunrestartloop ''' :
   Not a service, this script loops and exec the following commands every 5 seconds IFF maintenance file is set to 0 (or does not exists).
     sunServer start
     sunServer clean_dumps
   Just make sure it's running and starting with the system.
    
'''sunserver '''start|stop|kill|cleandumps|status
   Main script, handle the servers processes. Keep in mind that starting or stopping the server does NOT affect the restarter. This scripts has some logs written in the restarter.log file.
   The possible arguments are :
     start : Start the world and the auth if they're not already started. Also backup the old logs in a new folder with the current time.
     stop : Gently stop the world and the auth.
     kill : Kill the world and the auth (avoid using this since it can result in dataloss)
     cleandumps : Force dump cleaning, never use this
     status : Print whether world and auth server are running
   
'''sunrestarter '''start|stop
   Enable or disable the restarter loop. (This just put 0 or 1 in the maintenance.conf file used by sunServer)
   The possible arguments are :
     start : Enable the restarter
     stop : Disable the restarter (does not stop the server)

'''maintenance.conf''' (created by scripts)
   File used to determine if the restarter should be running or not. Contains '0' or '1'.
</blockquote>

== Installation ==

Put the 3 scripts/etc/init.d/. Those can be found in the contrib/sunstrider folder of the repository.
To install the services : [https://www.debian-administration.org/article/28/Making_scripts_run_at_boot_time_with_Debian HowTo install a service]
* update-rc.d sunrestartloop defaults
* update-rc.d sunrestarter defaults
* update-rc.d sunserver defaults

Also update your server install path in the constant in the top of the scripts. The maintenance file can be anywhere but must be the same in each script. Also make sure it is writtable.

== Utilisation ===

Only the sunrestarter and the sunserver are services accepting commands.

The command list for the sunrestarter and sunserver services are listed up in this page. You can also be reminded of these if you type "service <scriptname>" without command.

Examples :
* To stop the restarter : '''service sunrestarter stop'''
* To start the server : '''service sunserver start''' (without effect if already running)
* To restart the server, if the restarted is activated, just do a : '''service sunserver stop'''
* If the server is frozen and won't stop: '''service sunserv kill'''
* To check if the server is running: '''service sunserv status'''
