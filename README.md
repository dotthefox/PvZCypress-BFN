# Cypress - BFN
Cypress - BFN is a modified version of [Cypress,](https://github.com/BreakfastBrainz2/PvZCypress/) an Open-Source reimplementation of Dedicated Servers for PvZ: Garden Warfare 1 and 2, based off of [KYBER V1 (a Private Server project for Star Wars: Battlefront 2).](https://github.com/ArmchairDevelopers/Kyber) to add support for Battle for Neighborville. Made by your's truly, Ghup!

# Credits
* [KYBER](https://github.com/ArmchairDevelopers/Kyber) - Without their reimplementation of Frostbite's Socket Manager, this project would likely not exist.
* [Andersson799](https://github.com/Andersson799) - He figured out how to reimplement Dedicated Server functionality in Frostbite games, and without his amazing work, this project would not be able to host dedicated servers.
* Ghup - Implementing Cypress to Battle For Neighborville (thanks buddy 🥹)

# License
See [LICENSE.txt](LICENSE.txt)

# Building
Cypress - BFN should build out out of the box as long as you're using Visual Studio 2026.

# Usage
TODO - more in depth instructions here

Cypress - BFN is only compatible with the v1.0.55.50002 (last pre-EA Anticheat version) of Battle for Neighborville
After compiling the project, place the compiled dll in the game's directory and rename it to dinput8.dll

An example batch file that launches a dedicated server can be found at Examples/Start_Dedicated.bat
When run from the game directory, this will launch a Giddy Park server, with a max player count of 48.

For joining, see Examples/Start_Join.bat
