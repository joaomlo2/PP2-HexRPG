Linux Users
========================
NEO Scavenger was build using Flash, so some Linux users may run into trouble launching the game at first.

One of the first things to note is that NEO Scavenger needs absolute path info to load resources. If you're getting stuck at 0% on the loading screen, try double-clicking the app from the desktop instead of launching from the console. Alternately, some users were also able to get it to run by launching NEO Scavenger using an absolute path like /path/to/NEOScavenger instead of ./NEOScavenger.

If that doesn't help, it may be the result of missing libs/packages on certain distros. And in some 64-bit distros, this is due to missing 32-bit versions of libs/packages, or the result of them being in non-standard paths.

Some examples include:
ia32-libs-gtk
lib32-curl

More info can be found here:
http://bluebottlegames.com/main/node/2830

How to Mod NEO Scavenger
========================
There are two types of mods in NEO Scavenger:

1 - New content
2 - Override content

New content is anything you want to add that does not already exist in the game. This content will just get added on to the list of existing content when the game is loaded.

Override content is for changing original game data. If you want to tweak hunger penalties, dogman stats, and other existing data, this is where you'd do that.

For both types of mods, you'll need to add your mod's name and URL to the getmods.php file, like so:

nRows=2&strModName0=SampleMod&strModURL0=SampleFolder&strModName1=0&strModURL1=0

The above line will add two mods. The first, "SampleMod," can be found in the folder "SampleFolder." The second, mod "0," is an override mod. "0" (note: that's number zero, not letter O) is a special ID used by the game to denote original game data.

The above "SampleMod" and "0" override data are provided in this download as an example. Simply replace the getmods.php with the provided getmods.example.php and then reload the game to see it in action. 

SampleMod and "0" adds a new "fancy shopping bag" to the default cryo hex loot. The new bag item is in SampleMod, while the adjusted cryo hex loot values are in "0."

For more information, check out the official modding forum:
http://bluebottlegames.com/main/forum/10

There are many tips and helpful folks there!