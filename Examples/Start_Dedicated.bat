start PVZBattleforNeighborville.exe ^
-server ^
::-Server.ServerPassword password123^
::-usePlaylist ^
::-playlistFilename playlist_mixed.json ^
-allUnlocksUnlocked ^
-allowMultipleInstances ^
-serverInstancePath "./Instance/" ^
-enableServerLog ^
-dsub DSub_SocialSpace ^
-inclusion GameMode=Mode_SocialSpace ^
-startpoint StartPoint_SocialSpace ^
-listen 192.168.1.70:25200 ^
-Online.ClientIsPresenceEnabled 0 ^
-Online.ServerIsPresenceEnabled 0 ^
-GameMode.ForceHUBSeason 4 ^
-name "BFN Server" ^
-platform Win32 ^
-Network.ServerPort 25200 ^
-Network.ClientPort 25100 ^
-Network.ServerAddress 192.168.1.70 ^
-Online.Backend Backend_Local ^
-Online.PeerBackend Backend_Local ^
-PVZServer.MapSequencerEnabled false ^
-Online.DirtySockMaxConnectionCount 48 ^
-Network.MaxClientCount 48 ^
-NetObjectSystem.MaxServerConnectionCount 48 ^
-GameMode.OverrideRoundStartPlayerCount 1 ^
::-dataPath ModData/Default ^