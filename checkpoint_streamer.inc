//Checkpoint Streamer
//mattcomeback

#if defined _pawncpstream_included
	#endinput
#endif
#define _pawncpstream_included

#define INVALID_CHECKPOINT_ID 65535

#if !defined MAX_CHECKPOINTS
	#define MAX_CHECKPOINTS 250
#endif

#if !defined CHECKPOINT_STREAM_UPDATE_TIME
	#define CHECKPOINT_STREAM_UPDATE_TIME 2500
#endif

#if !defined CHECKPOINT_SIZE
	#define CHECKPOINT_SIZE 3.0
#endif

new g_CheckpointTimer = -1;



/*
--> Definitions
#define INVALID_CHECKPOINT_ID 65535 //Invalid checkpoint ID.
#define MAX_CHECKPOINTS //Maximum amount of created checkpoints at one time (keep atleast at the max number of checkpoints you have).
#define CHECKPOINT_STREAM_UPDATE_TIME //The amount of time for checkpoint streaming to update - Default: 2500ms
#define CHECKPOINT_SIZE //The radius of the checkpoint.

--> Creation and Deletion
native CreateCheckpoint(cptype, Float:x, Float:y, Float:z, Float:streamdistance, interior = -1, virtualworld = -1); //Returns the id of the checkpoint created or INVALID_CHECKPOINT_ID if no empty slots exist.
native DestroyCheckpoint(cpid); //Returns INVALID_CHECKPOINT_ID.

--> Data Retrieving
native GetCheckpointOwner(cpid); //Returns the owner of the checkpoint or INVALID_CHECKPOINT_ID if it doesn't exist.
native GetCheckpointType(cpid); //Returns the type of the checkpoint or INVALID_CHECKPOINT_ID if it doesn't exist.
native GetCheckpointPos(cpid, &Float:x, &Float:y, &Float:z); //Returns the checkpoint id.  Position is passed by reference.
native GetCheckpointStreamDistance(cpid, &Float:streamdistance); //Returns the checkpoint id.  Stream Distance is passed by reference.
native GetCheckpointInterior(cpid); //Returns the interior of the checkpoint or INVALID_CHECKPOINT_ID if it doesn't exist.
native GetCheckpointVirtualWorld(cpid); //Returns the virtual world of the checkpoint or INVALID_CHECKPOINT_ID if it doesn't exist.

--> Data Changing
native SetCheckpointOwner(cpid, playerid); //Returns the ID of the checkpoint or INVALID_CHECKPOINT_ID if it doesn't exist.
native SetCheckpointType(cpid, newcptype); //Returns the ID of the checkpoint or INVALID_CHECKPOINT_ID if it doesn't exist.
native SetCheckpointPos(cpid, Float:x, Float:y, Float:z); //Returns the ID of the checkpoint or INVALID_CHECKPOINT_ID if it doesn't exist.
native SetCheckpointStreamDistance(cpid, streamdistance); //Returns the ID of the checkpoint or INVALID_CHECKPOINT_ID if it doesn't exist.
native SetCheckpointInterior(cpid, interior); //Returns the ID of the checkpoint or INVALID_CHECKPOINT_ID if it doesn't exist.
native SetCheckpointVirtualWorld(cpid, vworld); //Returns the ID of the checkpoint or INVALID_CHECKPOINT_ID if it doesn't exist.

--> External Callbacks - Called inside of the gamemode.
forward OnPlayerEnterStreamCheckpoint(playerid, cpid, cptype); //Called when a player enters a streamed checkpoint.
forward OnPlayerLeaveStreamCheckpoint(playerid, cpid, cptype); //Called when a player leaves a streamed checkpoint.

--> Internal Callbacks
forward StreamServerCheckpoints(); //Timer callback that executes UpdatePlayerCheckpoint in a player loop.

--> Hooked Callbacks
forward OnGameModeInit(); //Hooked to initiate the timer for checkpoint streaming.
forward OnGameModeExit(); //Hooked to kill the timer for checkpoint streaming.
forward OnPlayerEnterCheckpoint(playerid); //Hooked to call OnPlayerEnterStreamCheckpoint when a player enters a checkpoint.
forward OnPlayerLeaveCheckpoint(playerid); //Hooked to call OnPlayerLeaveStreamCheckpoint when a player leaves a checkpoint.

--> Internal Functions
static GetFreeCheckpointID(); //Returns a free checkpoint ID for creation.
static UpdatePlayerCheckpoint(playerid); //Runs an update on current player position for closest eligible checkpoint.
	--> Note: Designed this way for possible future expansion.
*/









forward StreamServerCheckpoints();
forward OnPlayerEnterStreamCheckpoint(playerid, cpid, cptype);
forward OnPlayerLeaveStreamCheckpoint(playerid, cpid, cptype);


enum e_checkpoint_struct
{
	bool:CheckpointActive,
	CheckpointType,
	CheckpointOwner,
	Float:CheckpointPosX,
	Float:CheckpointPosY,
	Float:CheckpointPosZ,
	Float:CheckpointStreamDistance,
	CheckpointInterior,
	CheckpointVirtualWorld
}
static CheckpointData[MAX_CHECKPOINTS][e_checkpoint_struct];

enum e_player_checkpoint_struct
{
	PlayerCurrentCheckpoint,
	PlayerCheckpointType
}
static PlayerCheckpointData[MAX_PLAYERS][e_player_checkpoint_struct];

public OnGameModeInit()
{
	g_CheckpointTimer = SetTimer("StreamServerCheckpoints", CHECKPOINT_STREAM_UPDATE_TIME, true);
	
	#if defined cp_OnGameModeInit
		return cp_OnGameModeInit();
	#else
		return 1;
	#endif
}
#if defined _ALS_OnGameModeInit
	#undef OnGameModeInit
#else
	#define _ALS_OnGameModeInit
#endif

#define OnGameModeInit cp_OnGameModeInit
#if defined cp_OnGameModeInit
	forward cp_OnGameModeInit();
#endif

public OnGameModeExit()
{
	KillTimer(g_CheckpointTimer);
	
	#if defined cp_OnGameModeExit
		return cp_OnGameModeExit();
	#else
		return 1;
	#endif
}
#if defined _ALS_OnGameModeExit
	#undef OnGameModeExit
#else
	#define _ALS_OnGameModeExit
#endif

#define OnGameModeExit cp_OnGameModeExit
#if defined cp_OnGameModeExit
	forward cp_OnGameModeExit();
#endif

public OnPlayerEnterCheckpoint(playerid)
{
    CallLocalFunction("OnPlayerEnterStreamCheckpoint", "ddd", playerid, PlayerCheckpointData[playerid][PlayerCurrentCheckpoint], PlayerCheckpointData[playerid][PlayerCheckpointType]);
    
    #if defined cp_OnPlayerEnterCheckpoint
		return cp_OnPlayerEnterCheckpoint(playerid);
	#else
		return 1;
	#endif
}
#if defined _ALS_OnPlayerEnterCheckpoint
	#undef OnPlayerEnterCheckpoint
#else
	#define _ALS_OnPlayerEnterCheckpoint
#endif

#define OnPlayerEnterCheckpoint cp_OnPlayerEnterCheckpoint
#if defined cp_OnPlayerEnterCheckpoint
	forward cp_OnPlayerEnterCheckpoint(playerid);
#endif

public OnPlayerLeaveCheckpoint(playerid)
{
    CallLocalFunction("OnPlayerLeaveStreamCheckpoint", "ddd", playerid, PlayerCheckpointData[playerid][PlayerCurrentCheckpoint], PlayerCheckpointData[playerid][PlayerCheckpointType]);
	
    #if defined cp_OnPlayerLeaveCheckpoint
		return cp_OnPlayerLeaveCheckpoint(playerid);
	#else
		return 1;
	#endif
}
#if defined _ALS_OnPlayerLeaveCheckpoint
	#undef OnPlayerLeaveCheckpoint
#else
	#define _ALS_OnPlayerLeaveCheckpoint
#endif

#define OnPlayerLeaveCheckpoint cp_OnPlayerLeaveCheckpoint
#if defined cp_OnPlayerLeaveCheckpoint
	forward cp_OnPlayerLeaveCheckpoint(playerid);
#endif

public StreamServerCheckpoints()
{
	for(new i = 0; i < MAX_PLAYERS; i++)
	{
		if(!IsPlayerConnected(i)) continue;
		
		UpdatePlayerCheckpoint(i);		
	}
	return 1;
}


static UpdatePlayerCheckpoint(playerid)
{
	new Float:CurrentDistance;
	new Float:startdis = 6000.0, newcp, Float:newcpdistance;
	new interior = GetPlayerInterior(playerid);
	new vworld = GetPlayerVirtualWorld(playerid);
	
	for(new x = 0; x < MAX_CHECKPOINTS; x++)
	{
		if((CheckpointData[x][CheckpointOwner] != INVALID_PLAYER_ID) && (CheckpointData[x][CheckpointOwner] != playerid)) continue;
		
		if(CheckpointData[x][CheckpointActive])
		{
			if((CheckpointData[x][CheckpointInterior] != -1) && (CheckpointData[x][CheckpointInterior] != interior)) continue;
			if((CheckpointData[x][CheckpointVirtualWorld] != -1) && (CheckpointData[x][CheckpointVirtualWorld] != vworld)) continue;
		
			CurrentDistance = GetPlayerDistanceFromPoint(playerid, CheckpointData[x][CheckpointPosX], CheckpointData[x][CheckpointPosY], CheckpointData[x][CheckpointPosZ]); 
			if(startdis > CurrentDistance)
			{
				startdis = CurrentDistance;
				newcpdistance = CurrentDistance;
				newcp = x;
			}
		}
	}
	if(CheckpointData[newcp][CheckpointStreamDistance] > newcpdistance)
	{
		if(newcp != PlayerCheckpointData[playerid][PlayerCurrentCheckpoint])
		{
			PlayerCheckpointData[playerid][PlayerCurrentCheckpoint] = newcp;
			PlayerCheckpointData[playerid][PlayerCheckpointType] = CheckpointData[newcp][CheckpointType];
			SetPlayerCheckpoint(playerid, CheckpointData[newcp][CheckpointPosX], CheckpointData[newcp][CheckpointPosY], CheckpointData[newcp][CheckpointPosZ], CHECKPOINT_SIZE);
		}
	}
	else
	{
		PlayerCheckpointData[playerid][PlayerCurrentCheckpoint] = INVALID_CHECKPOINT_ID;
		DisablePlayerCheckpoint(playerid);
	}
	return 1;
}




stock CreateCheckpoint(cptype, Float:x, Float:y, Float:z, Float:streamdistance, interior = -1, virtualworld = -1)
{
	new cpid = GetFreeCheckpointID();
	if(cpid == INVALID_CHECKPOINT_ID)
	{
		print("ERROR: Cannot create checkpoint - MAX_CHECKPOINTS reached.");
		return INVALID_CHECKPOINT_ID;
	}
	
	//add checkpoints to enum to be streamed
	CheckpointData[cpid][CheckpointActive] = true;
	CheckpointData[cpid][CheckpointType] = cptype;
	CheckpointData[cpid][CheckpointPosX] = x;
	CheckpointData[cpid][CheckpointPosY] = y;
	CheckpointData[cpid][CheckpointPosZ] = z;
	CheckpointData[cpid][CheckpointStreamDistance] = streamdistance;
	CheckpointData[cpid][CheckpointInterior] = interior;
	CheckpointData[cpid][CheckpointVirtualWorld] = virtualworld;
	return cpid;
}

stock SetCheckpointType(cpid, newcptype)
{
	if(cpid >= MAX_CHECKPOINTS || cpid < 0)
	{
		return INVALID_CHECKPOINT_ID;
	}
	if(CheckpointData[cpid][CheckpointActive] == false)
	{
		return INVALID_CHECKPOINT_ID;
	}
	
	CheckpointData[cpid][CheckpointType] = newcptype;
	
	return cpid;
}

stock SetCheckpointPos(cpid, Float:x, Float:y, Float:z)
{
	if(cpid >= MAX_CHECKPOINTS || cpid < 0)
	{
		return INVALID_CHECKPOINT_ID;
	}
	if(CheckpointData[cpid][CheckpointActive] == false)
	{
		return INVALID_CHECKPOINT_ID;
	}
	
	CheckpointData[cpid][CheckpointPosX] = x;
	CheckpointData[cpid][CheckpointPosY] = y;
	CheckpointData[cpid][CheckpointPosZ] = z;
	
	return cpid;
}

stock SetCheckpointStreamDistance(cpid, Float:streamdistance)
{
	if(cpid >= MAX_CHECKPOINTS || cpid < 0)
	{
		return INVALID_CHECKPOINT_ID;
	}
	if(CheckpointData[cpid][CheckpointActive] == false)
	{
		return INVALID_CHECKPOINT_ID;
	}
	
	CheckpointData[cpid][CheckpointStreamDistance] = streamdistance;
	
	return cpid;
}



stock SetCheckpointInterior(cpid, interior)
{
	if(cpid >= MAX_CHECKPOINTS || cpid < 0)
	{
		return INVALID_CHECKPOINT_ID;
	}
	if(CheckpointData[cpid][CheckpointActive] == false)
	{
		return INVALID_CHECKPOINT_ID;
	}
	
	CheckpointData[cpid][CheckpointInterior] = interior;
	
	return cpid;
}

stock SetCheckpointVirtualWorld(cpid, vworld)
{
	if(cpid >= MAX_CHECKPOINTS || cpid < 0)
	{
		return INVALID_CHECKPOINT_ID;
	}
	if(CheckpointData[cpid][CheckpointActive] == false)
	{
		return INVALID_CHECKPOINT_ID;
	}
	
	CheckpointData[cpid][CheckpointVirtualWorld] = vworld;
	
	return cpid;
}

stock SetCheckpointOwner(cpid, playerid)
{
	if(cpid >= MAX_CHECKPOINTS || cpid < 0)
	{
		return INVALID_CHECKPOINT_ID;
	}
	if(CheckpointData[cpid][CheckpointActive] == false)
	{
		return INVALID_CHECKPOINT_ID;
	}
	
	CheckpointData[cpid][CheckpointOwner] = playerid;
	
	return cpid;
}


stock DestroyCheckpoint(cpid)
{
	if(cpid >= MAX_CHECKPOINTS || cpid < 0)
	{
		print("ERROR: Invalid Checkpoint ID passed to DestroyCheckpoint.");
	}
	else
	{
		CheckpointData[cpid][CheckpointActive] = false;
		CheckpointData[cpid][CheckpointType] = 0;
		CheckpointData[cpid][CheckpointOwner] = INVALID_PLAYER_ID;
		CheckpointData[cpid][CheckpointPosX] = 0.0;
		CheckpointData[cpid][CheckpointPosY] = 0.0;
		CheckpointData[cpid][CheckpointPosZ] = 0.0;
		CheckpointData[cpid][CheckpointStreamDistance] = 0.0;
		CheckpointData[cpid][CheckpointInterior] = -1;
		CheckpointData[cpid][CheckpointVirtualWorld] = -1;
	}
	return INVALID_CHECKPOINT_ID;
}

stock GetCheckpointOwner(cpid)
{
	if(cpid >= MAX_CHECKPOINTS || cpid < 0)
	{
		return INVALID_CHECKPOINT_ID;
	}
	if(CheckpointData[cpid][CheckpointActive] == false)
	{
		return INVALID_CHECKPOINT_ID;
	}
	return CheckpointData[cpid][CheckpointOwner];
}

stock GetCheckpointType(cpid)
{
	if(cpid >= MAX_CHECKPOINTS || cpid < 0)
	{
		return INVALID_CHECKPOINT_ID;
	}
	if(CheckpointData[cpid][CheckpointActive] == false)
	{
		return INVALID_CHECKPOINT_ID;
	}
	return CheckpointData[cpid][CheckpointType];
}

stock GetCheckpointPos(cpid, &Float:x, &Float:y, &Float:z)
{
	if(cpid >= MAX_CHECKPOINTS || cpid < 0)
	{
		return INVALID_CHECKPOINT_ID;
	}
	if(CheckpointData[cpid][CheckpointActive] == false)
	{
		return INVALID_CHECKPOINT_ID;
	}
	x = CheckpointData[cpid][CheckpointPosX];
	y = CheckpointData[cpid][CheckpointPosY];
	z = CheckpointData[cpid][CheckpointPosZ];
	
	return cpid;
}

stock GetCheckpointStreamDistance(cpid, &Float:streamdistance)
{
	if(cpid >= MAX_CHECKPOINTS || cpid < 0)
	{
		return INVALID_CHECKPOINT_ID;
	}
	if(CheckpointData[cpid][CheckpointActive] == false)
	{
		return INVALID_CHECKPOINT_ID;
	}
	streamdistance = CheckpointData[cpid][CheckpointStreamDistance];
	
	return cpid;
}

stock GetCheckpointInterior(cpid)
{
	if(cpid >= MAX_CHECKPOINTS || cpid < 0)
	{
		return INVALID_CHECKPOINT_ID;
	}
	if(CheckpointData[cpid][CheckpointActive] == false)
	{
		return INVALID_CHECKPOINT_ID;
	}
	return CheckpointData[cpid][CheckpointInterior];
}

stock GetCheckpointVirtualWorld(cpid)
{
	if(cpid >= MAX_CHECKPOINTS || cpid < 0)
	{
		return INVALID_CHECKPOINT_ID;
	}
	if(CheckpointData[cpid][CheckpointActive] == false)
	{
		return INVALID_CHECKPOINT_ID;
	}
	return CheckpointData[cpid][CheckpointVirtualWorld];
}


static GetFreeCheckpointID()
{
	for(new i = 0; i < MAX_CHECKPOINTS; i++)
	{
		if(CheckpointData[i][CheckpointActive] == false)
		{
			return i;
		}
	}
	return INVALID_CHECKPOINT_ID;
}
