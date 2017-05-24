
                        ###############################
                        #                             #
                        #      PARABOT V0.92 BETA     #
                        #                             #
                        #        by nekonomicon       #
                        #                             #
                        ###############################




********************************************************************************

CONTENTS

********************************************************************************


I.    About

II.   How to...

III.  Version History

IV.   Disclaimer

V.    Credits




********************************************************************************

  I. About

********************************************************************************


The Parabot is an artificial player for Half-Life. The current version 
(0.92 beta) supports the following playmodes/Mods:

-> Half-Life (deathmatch & teamplay)
-> Deathmatch Classic (DMC)
-> Opposing Force (deathmatch & CTF)
-> HolyWars 2.0
-> Bubblemod
-> Severians
-> Adrenaline Gamer 6.6 (All gamemodes)
-> They Hunger Trilogy

You copy the Parabot to your system by extracting the "parabot092-pc.zip" archive.
After that you have to install the bot for the Mods you wish to play (have a
look at the "How to..." section for that).

The Parabot comes without any waypoint-files that you might know from other 
bots. It will learn new maps by just playing them together with humans and store
its experiences in the "addons/parabot/navpoints" folder. To make this learning 
more efficient you should move as much as possible when first starting a new 
map, picking up every item you see on your way. 
After approximately 15 minutes the Parabot should navigate quite well on average
sized maps. Note that the storing of the navigation files might cause small 
delays when changing maps or ending the game.

For recent news and the latest updates have a look at the github page 
https://github.com/nekonomicon/Parabot



********************************************************************************

  II. How to...

********************************************************************************


1.) ...install or uninstall Parabot:

    Copy "addons" folder from archive to Half-Life/<modfolder>.
    If you have installed metamod then add the following lines to
    your plugins.ini:
	"win32 addons/parabot/dlls/parabot.dll"
	"lin32 addons/parabot/dlls/parabot.so"

    If not then edit Half-Life/<modfolder>/liblist.gam.
    Replace gamedll "dlls/hl.dll"
       to gamedll "addons/parabot/dlls/parabot.dll". // for Windows
    Replace gamedll_linux "dlls/hl.so"
       to gamedll_linux "addons/parabot/dlls/parabot.so". // for Linux
    Replace gamedll_osx "dlls/hl.dylib"
       to gamedll_osx "addons/parabot/dlls/parabot.dylib". // for OSX


2.) ...play a game with Parabot:

    Once you installed Parabot as explained above you can start a game following
    these steps:
      1. launch Half-Life or any of the supported Mods
      2. Chose "Multiplayer", then "LAN game", after that "Create game"
      3. Pick a map and click "OK"
    Note that you *CANNOT* play in Singleplayer-mode while the bot is installed!


3.)  ...change the difficulty level:

    If you only want to change the overall difficulty level of playing against
    the bots, the easiest way is to change the "MinAimSkill" and "MaxAimSkill"
    variables in "parabot.cfg" - look in the correspondant Mod-directory (e.g
    "valve/addons/parabot/config/valve" if you want to change the settings
    for HL-deathmatch) and open the file in a texteditor (like notepad.exe).
    If the bots are too hard for you, change MinAimSkill to 1 (or comment
    the line out by writing a # at the beginning) and MaxAimSkill to,
    let's say, 5. In case the bots are getting too easy for you,
    you should comment out MaxAimSkill and setMinAimSkill to a higher value.


4.) ...change the number of bots that join the game:

    Open the correspondant "parabot.cfg" and change the "NumBots" variable
    to the value you like (details are explained in the file).
    Note that while playing you can still add more bots by typing "addbot" in
    the console.


5.) ...edit the different bot personalities (name, model, aggressivity,...):

    Open the file "characters.cfg" in the correspondant Mod-directory as 
    explained in 3.) and have a look at the contents. All possible values
    (name, model, aim-skill, perception, aggressivity and communication) are
    explained in there and you can change them as you like or add completely 
    new bot characters as well.


6.) ...change the language the bots are speaking:

    Change the "ChatFile" setting in the appropriate "parabot.cfg". 
    The languages available in this version are English, Spanish, French, 
    German, Czech, Finnish, Romanian, Swedish, Danish and Russian.


7.) ...edit the chatfiles:

    Have a look at the chatfile you want to modify and try to understand the
    structure of it (it's not too hard). You can add as many replies as you like
    but make sure they are not too long (around 80 characters should be the 
    maximum) and don't pass one line. "%s" gets replaced by the playername the
    bot is talking to (does *NOT* work in the @GOT_WEAPON-section). "%w" gets
    replaced by the weapon that has been used/picked up (only for @GOT_KILLED,
    @KILLED_PLAYER and @GOT_WEAPON).
    You can define new keywords as well (maximum length 30 characters, only
    containing letters in uppercase) and add them to the last section.
    It is important to know that keywords appearing first in the file are chosen
    first in case that a sentence contains several known keywords.


8.) ...change other options/gamemodes:

    All other options can be adjusted in the appropriate "parabot.cfg" file.
    If you are a real newbie and the bots beat you even at aim-skill 1 (or if
    you are just sick of them blasting you away with the rocketlauncher...) you
    can restrict the weapons they can use or let them run in PeaceMode where 
    they won't attack. You can switch off the chat as well if it irritates you.




********************************************************************************

  III. Commands

********************************************************************************


Since v0.9 all commands can be executed with a menu during gameplay.
For the menu to work properly, make sure you have got the following line in your
"config.cfg":

bind "6" "slot6"

To use the menu, type "botmenu" in the console or add the following line to your
"config.cfg":

bind "F1" "botmenu"

This will raise the menu every time you press the "F1" key. (You can change "F1"
to any other key you like...)


The Parabot adds the following new commands to the game. Unless otherwise 
specified they work on both normal and dedicated servers:

"addbot" 
   - adds a new bot to the game
"peacemode on" / "peacemode off" 
   - enables/disables PeaceMode
"restrictedweapons on" / "restrictedweapons off"
   - enables/disables RestrictedWeaponMode
"chatlog on" / "chatlog off"
   - enables/disables chat logging to the file parabot/chatlog.txt
"hidewelcome on" / "hidewelcome off"
   - if active the "Welcome to Parabot" message at game start is surpressed
"botcam" (no DS)
   - spectator mode for observing the bots, consecutive calls switch the camera 
     to follow different bots
"camstop" (no DS)
   - ends the spectator mode
"botstop" (no DS)
   - pauses all bots
"botgo" (no DS)
   - continues after a botstop

You can type these commands in your console or bind them to special keys by 
editing the corresponding "config.cfg" (in Half-Life\valve, \dmc, \gearbox, 
\holywars, \ag or \Hunger ). For the latter you could just copy and paste
the following lines:

bind "F4" "addbot"
bind "F5" "botcam"
bind "F6" "camstop"
bind "F7" "botstop"
bind "F8" "botgo"
bind "F9" "peacemode on"
bind "F10" "peacemode off"
bind "F11" "restrictedweapons on"
bind "F12" "restrictedweapons off"




********************************************************************************

  IV. Version history

********************************************************************************

v0.92.1 (21.05.2017)

- fixed crashes with steam hl libraries and some mods.
- fixed crashes with test android version of Xash3D FWGS.


v0.92 (21.05.2017):

- added Steam HL and Xash3D support
- added metamod support
- added Linux/OSX/*BSD/Android support
- added Adrenaline Gamer support
- added They Hunger Trilogy support
- added BubbleMOD support
- added Severians support
- added Opposing Force CTF support
- added case-insensitive UTF-8 characters comparison
- added new chatfile (Russian)
- all files moved to <modfolder>/addons/parabot
- removed debug code from release builds
- fixed crashes
- fixed gauss usage
- fixed bug with models in teamplay
- fixed speaker nickname color in chat in teamplay
- fixed bots behavior in teamplay
- fixed hangs in DMC


v0.91 (09.10.2002):

- fixed freeze bug on crowded maps
- fixed focus calculations in terrain analyzer
- aiming refined again


v0.9 (07.07.2002):

- added botmenu
- reduced CPU usage
- new aiming & lookaround code (no pendulum heads anymore)
- internet server simulation mode: bots join and leave randomly
- updated for HL & DMC 1.1.1.0
- updated for Opposing Force 1.1.1.0
- updated for HolyWars 2.0
- improved dedicated server support
- lots of bugfixes (memory leaks)


v0.8 (10.03.2002):

- bots adding waypoints without player movement
- automatic terrain analysis with visibility table 
  (thanks to William of CGF for tips and feedback on that one)
- new combat behaviour (using terrain data)
- improved hunting and fleeing
- basic support for Opposing Force (no CTF)
- fixed various bugs on dedicated servers: "addbot" command working now,
  botchat as well
- fixed bug that bots don't react when shot
- fixed several crash bugs
- added new chatfile (Danish)
- bots welcome new players on a server
- added possibility of chat logging to a file
- fixed bugs with botcam


v0.71 (28.09.2001):

- fixed crashes in latest HL version 1108
- fixed a bug in the navigation routines: bots learn correctly to use the 
  longjump module and to avoid dangerous paths again
- added two new chatfiles (Romanian and Swedish)


v0.7 (21.07.2001):

- full support for Deathmatch Classic (DMC) 
- improved pathfinding system (more doors and platforms) 
- better roaming navigation in unknown maps 
- bots hunt enemies they can track by hearing 
- fully configurable chat-and-response system 
- new playmodes: RestrictedWeapons and PeaceMode 
- more flexible installation
- lots of bug-fixes 


v0.6 (18.05.2001):

- added full support for the Mod Holy Wars
- added support for Half-Life teamplay
- added tank/turret usage
- added basic grenade usage (handgrenades, satchels and snarks)
- added bunker usage: bots will learn to run for airstrike cover in Crossfire
- added two new configuration variables (MinAimSkill and MaxAimSkill)
- improved perception-code (damage and sounds)


v0.5 (04.04.2001):

- improved overall behaviour (tweaked goalfinder settings)
- added goalSilentAttack: bots will try to get single-kill shots on surprised 
  players
- improved lift usage
- added platform/train usage
- added tripmine usage and basic evasion
- added longjump usage
- improved botcam
- several bug-fixes
- added documentation




********************************************************************************

  V. Disclaimer

********************************************************************************


You know the stuff: If your computer explodes or other minor damage occurs while
running this program I cannot be hold responsible.
You can distribute the original Parabot archive as you like as long as it is 
free of charge.




********************************************************************************

  VI. Credits

********************************************************************************

Original Author: Tobias "Killaruna" Heimann ( killaruna@nuclearbox.com )

The Parabot wouldn't have been possible without Botman releasing his 
HPB Template Code which used as a base for getting the bot into the game 
( http://www.planethalflife.com/botman ).
A big "THANK YOU" to Botman for this great work!

Special thanks go to William ( http://www.cgf-ai.com )
and Count Floyd ( http://podbot.nuclearbox.com )
for the inspiring discussions and comments!

The chat-and-response system was inspired by discussions from the Nuclearbox-
forums, thanks to Hampst0r and the others for all their suggestions (Only 
afterwards Killaruna played Q3). The original chatfile was written
in collaboration with Turms and Homer. Thanks to all the translators!

Thanks to Rhino for testing Parabot on his dedicated server at Fragville.net!

Thanks to Whistler for "Parabot Reloaded" ( http://www.bots-united.com )

And now: Stop reading and have some fragging fun!


********************************************************************************

For any questions or comments e-mail me at uselessd11@gmail.com
or visit at the github page: https://github.com/nekonomicon/Parabot
