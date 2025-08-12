# condenstation

A PRX plugin to re-enable connecting to Steam for the PS3 for Portal 2.

This third-party project is not affiliated with nor endorsed by Valve Software.

[Downloads are available here.](https://github.com/InvoxiPlayGames/condenstation/releases)

## Info

* Only works with Portal 2 for now, and possibly forever.
    * You **must** own / have access to [Portal 2 on Steam](https://store.steampowered.com/app/620/).
* PC Portal 2 co-op partners **must** be using the "demo_viewer" beta branch of Portal 2.
* This connects you to Valve's official Steam servers.
* **I am not responsible for any damage to your Steam account, PSN account or PS3 for using this.**
    You aren't breaking any Steam rules, nor are you cheating, so you should not get (VAC) banned,
    but I am not responsible if you do somehow.
    * There is risk in connecting a PS3 with CFW/HEN to PSN. Use an alternative account if you are
    concerned.
    * Your Steam and PSN account are **not** linked, your Steam account information is stored on your PS3.
    * You may be banned from trophy websites for unlocking certain trophies, despite you being connected
    to official servers.
* If you have lots of Steam friends (100+), the game may appear frozen for several minutes when you log on.
    Please have patience - it hasn't completely frozen!
* **Team Fortress 2 (TF2) does not use Steam, and will NEVER be supported by this.**

An installation guide is found in the release download, or in [README_release.txt](https://github.com/InvoxiPlayGames/condenstation/blob/master/README_release.txt)

## TODO

These are featured I would like to support in the future, but might not get around to.

* [ ] Username/password + guard code login
    * I'm putting this off due to the complexity involved. I'm sorry.
    * [ ] RSA-4096 encryption for passwords
    * [ ] UI/UX for login flow
* [ ] Codebase cleanup
* [ ] Neat blog on what all this does and why it is how it is
* [ ] Translating our strings into other languages (Russian, Spanish, Portuguese, etc)
* [ ] Functioning on RPCS3
    * [ ] Remove hook requirement for patching CM URLs. (Can we do it with a vtable replacement?)
* [ ] Built in no-PSN patching (maybe Portal 2 only)
* [ ] Fixing or stubbing Steam Cloud
   * Anything to get rid of the endless "Syncing" on the menu.
* [ ] Upgrade netfilter to use newer authenticated encryption
   * Not *required* but would ~~avoid future deprecation.~~ be nice. It's unlikely legacy netfilter would be deprecated as they'd break any old game's dedicated server unless they asked admins to upgrade steamclient.
* [ ] CS:GO - this will be hard!
   * Uses AppID [720](https://steamdb.info/app/720/). Nobody owns this, so all calls fail by default! :(
   * Trying to use another AppID leads to "Searching" forever, even in private lobbies. Could entirely rely on [dedicated servers](https://steamdb.info/app/790/).
   * Needs a lot more research...

## License

This software is free software provided with absolutely no warranty under the
terms of the GNU General Public License version 2, or later, at your choice.
Read LICENSE.txt for more.

## Thanks/credits

* [relt from "SourceSPMP"](https://github.com/SourceSPMP/PS3Plugins) discovered Portal 2's ability to load native PRX addons without any game patches directly ([video](https://www.youtube.com/watch?v=3_jE5osEfRo)), as well as implemented the first re-implementation of crossplay (via direct IP connection, rather than Steam - meaning it will work forever!). Huge props.
* [SteamKit2](https://github.com/SteamRE/SteamKit) and SteamDB helped out a ton by having a documented history of the Steam3 communications protocol, as well as examples of how modern authentication and server communication is supposed to work. They've effectively documented almost every Steam3 protocol difference over the past 12+ years with their source code and commit history. Thanks!
* [vxzip](https://github.com/CRACKbomber/vxzip) was used to create the XZP2 files used by the console versions of the game.
* [nta](https://ntauthority.me) helped me get my bearings around Protobufs, SteamClient and the Steam API. Thanks! :3

## Third-Party Software

See "third_party_licenses" for full license text.

* [RB3Enhanced (PS3)](https://github.com/InvoxiPlayGames/RB3Enhanced) (GPLv2)
* [inih](https://github.com/benhoyt/inih) (New BSD)
* [protobuf-c](https://github.com/protobuf-c/protobuf-c) (BSD-2-Clause)
* [libqrencode](https://github.com/fukuchi/libqrencode) (LGPLv2.1)
* [tPNG](https://github.com/jcorks/tPNG) (see license)

## Note to Valve

If anyone at Valve has any concerns with this (security, backend stability, or otherwise) or has anything important they need to share, make an issue in the issue tracker if you're okay with being in public or send me an e-mail from a @valvesoftware.com address.
