/****************************/
/*      FILE ROUTINES       */
/* (c)1997-99 Pangea Software  */
/* By Brian Greenstone      */
/****************************/


/***************/
/* EXTERNALS   */
/***************/


extern	short			gMainAppRezFile,gTextureRezfile;
extern  TQ3Object		gObjectGroupList[MAX_3DMF_GROUPS][MAX_OBJECTS_IN_GROUP];
extern  short			gNumObjectsInGroupList[MAX_3DMF_GROUPS];
extern	short			gNumTerrainItems;
extern	short			gPrefsFolderVRefNum;
extern	long			gPrefsFolderDirID,gNumTerrainTextureTiles;
extern	long			gTerrainTileWidth,gTerrainTileDepth,gTerrainUnitWidth,gTerrainUnitDepth;		
extern	long			gNumSuperTilesDeep,gNumSuperTilesWide;
extern	u_short			**gFloorMap,**gCeilingMap,**gTileDataHandle;
extern	u_char			**gPathMap;
extern	long			gCurrentSuperTileRow,gCurrentSuperTileCol;
extern	long			gMyStartX,gMyStartZ,gNumSplines;
extern	FSSpec			gDataSpec;
extern	TerrainInfoMatrixType	**gMapInfoMatrix;
extern	TerrainYCoordType		**gMapYCoords;
extern	TerrainItemEntryType 	**gMasterItemList;
extern	u_short			**gVertexColors[2],gLevelType,gAreaNum,gRealLevel;
extern	Boolean			gDoCeiling,gRestoringSavedGame,gMuteMusicFlag;
extern	SplineDefType	**gSplineList;
extern	FenceDefType	*gFenceList;
extern	long			gNumFences;
extern	u_long			gScore;
extern	float			gMyHealth,gBallTimer,gDemoVersionTimer;
extern	short			gNumLives;
extern	short			gNumGoldClovers;

/****************************/
/*    PROTOTYPES            */
/****************************/

static void ReadDataFromSkeletonFile(SkeletonDefType *skeleton, FSSpec *fsSpec);
static void ReadDataFromPlayfieldFile(FSSpec *specPtr);
pascal void myEventProc(NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms,
						NavCallBackUserData callBackUD);
pascal Boolean myFilterProc(AEDesc*theItem,void*info, NavCallBackUserData callBackUD, NavFilterModes filterMode);


/****************************/
/*    CONSTANTS             */
/****************************/

#define	BASE_PATH_TILE		900					// tile # of 1st path tile

#define	PICT_HEADER_SIZE	512

#define	SKELETON_FILE_VERS_NUM	0x0110			// v1.1

#define	SAVE_GAME_VERSION	0x0100		// 1.0


		/* SAVE GAME */
		
typedef struct
{
	u_long		version;
	u_long		score;
	short		realLevel;
	short		numLives;
	float		health;
	float		ballTimer;
	Byte		numGoldClovers;
}SaveGameType;



		/* PLAYFIELD HEADER */
		
typedef struct
{
	NumVersion	version;							// version of file
	long		numItems;							// # items in map
	long		mapWidth;							// width of map
	long		mapHeight;							// height of map	
	long		numTilePages;						// # tile pages
	long		numTilesInList;						// # extracted tiles in list
	float		tileSize;							// 3D unit size of a tile
	float		minY,maxY;							// min/max height values
	long		numSplines;							// # splines
	long		numFences;							// # fences
}PlayfieldHeaderType;

		/* FENCE STRUCTURE IN FILE */
		//
		// note: we copy this data into our own fence list
		//		since the game uses a slightly different
		//		data structure.
		//
		
typedef struct
{
	u_short			type;				// type of fence
	short			numNubs;			// # nubs in fence
	FencePointType	**nubList;			// handle to nub list	
	Rect			bBox;				// bounding box of fence area	
}FileFenceDefType;


/**********************/
/*     VARIABLES      */
/**********************/

float	g3DTileSize, g3DMinY, g3DMaxY;

char		gDataFolderPath[1500];

//static	FSSpec	gApplicationFSSpec;								// spec of this application

/******************* LOAD SKELETON *******************/
//
// Loads a skeleton file & creates storage for it.
// 
// NOTE: Skeleton types 0..NUM_CHARACTERS-1 are reserved for player character skeletons.
//		Skeleton types NUM_CHARACTERS and over are for other skeleton entities.
//
// OUTPUT:	Ptr to skeleton data
//

SkeletonDefType *LoadSkeletonFile(short skeletonType)
{
QDErr		iErr;
short		fRefNum;
FSSpec		fsSpec;
SkeletonDefType	*skeleton;					

				/* SET CORRECT FILENAME */
					
	switch(skeletonType)
	{
		case	SKELETON_TYPE_BOXERFLY:
				FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:Skeletons:BoxerFly.skeleton", &fsSpec);
				break;
				
		case	SKELETON_TYPE_ME:
				FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:Skeletons:DoodleBug.skeleton", &fsSpec);
				break;

		case	SKELETON_TYPE_SLUG:
				FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:Skeletons:Slug.skeleton", &fsSpec);
				break;

		case	SKELETON_TYPE_ANT:
				FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:Skeletons:Ant.skeleton", &fsSpec);
				break;

		case	SKELETON_TYPE_FIREANT:
				FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:Skeletons:FireAnt.skeleton", &fsSpec);
				break;
		
		case	SKELETON_TYPE_WATERBUG:
				FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:Skeletons:WaterBug.skeleton", &fsSpec);
				break;

		case	SKELETON_TYPE_DRAGONFLY:
				FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:Skeletons:DragonFly.skeleton", &fsSpec);
				break;

		case	SKELETON_TYPE_PONDFISH:
				FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:Skeletons:PondFish.skeleton", &fsSpec);
				break;
		
		case	SKELETON_TYPE_MOSQUITO:
				FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:Skeletons:Mosquito.skeleton", &fsSpec);
				break;

		case	SKELETON_TYPE_FOOT:
				FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:Skeletons:Foot.skeleton", &fsSpec);
				break;
				
		case	SKELETON_TYPE_SPIDER:
				FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:Skeletons:Spider.skeleton", &fsSpec);
				break;

		case	SKELETON_TYPE_CATERPILLER:
				FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:Skeletons:Caterpiller.skeleton", &fsSpec);
				break;
				
		case	SKELETON_TYPE_FIREFLY:
				FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:Skeletons:FireFly.skeleton", &fsSpec);
				break;

		case	SKELETON_TYPE_BAT:
				FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:Skeletons:Bat.skeleton", &fsSpec);
				break;

		case	SKELETON_TYPE_LADYBUG:
				FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:Skeletons:LadyBug.skeleton", &fsSpec);
				break;
		
		case	SKELETON_TYPE_ROOTSWING:
				FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:Skeletons:RootSwing.skeleton", &fsSpec);
				break;
				
		case	SKELETON_TYPE_LARVA:
				FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:Skeletons:Larva.skeleton", &fsSpec);
				break;
		
		case	SKELETON_TYPE_FLYINGBEE:
				FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:Skeletons:FlyingBee.skeleton", &fsSpec);
				break;
				
		case	SKELETON_TYPE_WORKERBEE:
				FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:Skeletons:WorkerBee.skeleton", &fsSpec);
				break;
		
		case	SKELETON_TYPE_QUEENBEE:
				FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:Skeletons:QueenBee.skeleton", &fsSpec);
				break;

		case	SKELETON_TYPE_ROACH:
				FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:Skeletons:Roach.skeleton", &fsSpec);
				break;
		
		case	SKELETON_TYPE_BUDDY:
				FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:Skeletons:Buddy.skeleton", &fsSpec);
				break;

		case	SKELETON_TYPE_SKIPPY:
				FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:Skeletons:Skippy.skeleton", &fsSpec);
				break;

		case	SKELETON_TYPE_KINGANT:
				FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:Skeletons:AntKing.skeleton", &fsSpec);
				break;
		
		default:
				DoFatalAlert("\pLoadSkeleton: Unknown skeletonType!");
	}
	
	
			/* OPEN THE FILE'S REZ FORK */

	fRefNum = FSpOpenResFile(&fsSpec,fsRdPerm);
	if (fRefNum == -1)
	{
		iErr = ResError();
		DoAlert("\pError opening Skel Rez file");
		ShowSystemErr(iErr);
	}
	
	UseResFile(fRefNum);
	if (ResError())
		DoFatalAlert("\pError using Rez file!");

			
			/* ALLOC MEMORY FOR SKELETON INFO STRUCTURE */
			
	skeleton = (SkeletonDefType *)AllocPtr(sizeof(SkeletonDefType));
	if (skeleton == nil)
		DoFatalAlert("\pCannot alloc SkeletonInfoType");


			/* READ SKELETON RESOURCES */
			
	ReadDataFromSkeletonFile(skeleton,&fsSpec);
	PrimeBoneData(skeleton);
	
			/* CLOSE REZ FILE */
			
	CloseResFile(fRefNum);
	UseResFile(gMainAppRezFile);
	UseResFile(gTextureRezfile);
		
		
	return(skeleton);
}


/************* READ DATA FROM SKELETON FILE *******************/
//
// Current rez file is set to the file. 
//

static void ReadDataFromSkeletonFile(SkeletonDefType *skeleton, FSSpec *fsSpec)
{
Handle				hand;
short				i,k,j;
long				numJoints,numAnims,numKeyframes;
AnimEventType		*animEventPtr;
JointKeyframeType	*keyFramePtr;
SkeletonFile_Header_Type	*headerPtr;
short				version;
AliasHandle				alias;
OSErr					iErr;
FSSpec					target;
Boolean					wasChanged;
TQ3Point3D				*pointPtr;
SkeletonFile_AnimHeader_Type	*animHeaderPtr;


			/************************/
			/* READ HEADER RESOURCE */
			/************************/

	hand = GetResource('Hedr',1000);
	if (hand == nil)
		DoFatalAlert("\pReadDataFromSkeletonFile: Error reading header resource!");
	headerPtr = (SkeletonFile_Header_Type *)*hand;
	version = headerPtr->version;
	if (version != SKELETON_FILE_VERS_NUM)
		DoFatalAlert("\pSkeleton file has wrong version #");
	
	numAnims = skeleton->NumAnims = headerPtr->numAnims;			// get # anims in skeleton
	numJoints = skeleton->NumBones = headerPtr->numJoints;			// get # joints in skeleton
	ReleaseResource(hand);

	if (numJoints > MAX_JOINTS)										// check for overload
		DoFatalAlert("\pReadDataFromSkeletonFile: numJoints > MAX_JOINTS");


				/*************************************/
				/* ALLOCATE MEMORY FOR SKELETON DATA */
				/*************************************/

	AllocSkeletonDefinitionMemory(skeleton);



		/********************************/
		/* 	LOAD THE REFERENCE GEOMETRY */
		/********************************/
		
	alias = (AliasHandle)GetResource(rAliasType,1000);				// alias to geometry 3DMF file
	if (alias != nil)
	{
		iErr = ResolveAlias(fsSpec, alias, &target, &wasChanged);	// try to resolve alias
		if (!iErr)
			LoadBonesReferenceModel(&target,skeleton);
		else
			DoFatalAlert("\pReadDataFromSkeletonFile: Cannot find Skeleton's 3DMF file!");
		ReleaseResource((Handle)alias);
	}


		/***********************************/
		/*  READ BONE DEFINITION RESOURCES */
		/***********************************/

	for (i=0; i < numJoints; i++)
	{
		File_BoneDefinitionType	*bonePtr;
		u_short					*indexPtr;

			/* READ BONE DATA */
			
		hand = GetResource('Bone',1000+i);
		if (hand == nil)
			DoFatalAlert("\pError reading Bone resource!");
		HLock(hand);
		bonePtr = (File_BoneDefinitionType *)*hand;

			/* COPY BONE DATA INTO ARRAY */
		
		skeleton->Bones[i].parentBone = bonePtr->parentBone;								// index to previous bone
		skeleton->Bones[i].coord = bonePtr->coord;											// absolute coord (not relative to parent!)
		skeleton->Bones[i].numPointsAttachedToBone = bonePtr->numPointsAttachedToBone;		// # vertices/points that this bone has
		skeleton->Bones[i].numNormalsAttachedToBone = bonePtr->numNormalsAttachedToBone;	// # vertex normals this bone has		
		ReleaseResource(hand);

			/* ALLOC THE POINT & NORMALS SUB-ARRAYS */
				
		skeleton->Bones[i].pointList = (u_short *)AllocPtr(sizeof(u_short) * (int)skeleton->Bones[i].numPointsAttachedToBone);
		if (skeleton->Bones[i].pointList == nil)
			DoFatalAlert("\pReadDataFromSkeletonFile: AllocPtr/pointList failed!");

		skeleton->Bones[i].normalList = (u_short *)AllocPtr(sizeof(u_short) * (int)skeleton->Bones[i].numNormalsAttachedToBone);
		if (skeleton->Bones[i].normalList == nil)
			DoFatalAlert("\pReadDataFromSkeletonFile: AllocPtr/normalList failed!");

			/* READ POINT INDEX ARRAY */
			
		hand = GetResource('BonP',1000+i);
		if (hand == nil)
			DoFatalAlert("\pError reading BonP resource!");
		HLock(hand);
		indexPtr = (u_short *)(*hand);
			
			/* COPY POINT INDEX ARRAY INTO BONE STRUCT */

		for (j=0; j < skeleton->Bones[i].numPointsAttachedToBone; j++)
			skeleton->Bones[i].pointList[j] = indexPtr[j];
		ReleaseResource(hand);


			/* READ NORMAL INDEX ARRAY */
			
		hand = GetResource('BonN',1000+i);
		if (hand == nil)
			DoFatalAlert("\pError reading BonN resource!");
		HLock(hand);
		indexPtr = (u_short *)(*hand);
			
			/* COPY NORMAL INDEX ARRAY INTO BONE STRUCT */

		for (j=0; j < skeleton->Bones[i].numNormalsAttachedToBone; j++)
			skeleton->Bones[i].normalList[j] = indexPtr[j];
		ReleaseResource(hand);
						
	}
	
	
		/*******************************/
		/* READ POINT RELATIVE OFFSETS */
		/*******************************/
		//
		// The "relative point offsets" are the only things
		// which do not get rebuilt in the ModelDecompose function.
		// We need to restore these manually.
	
	hand = GetResource('RelP', 1000);
	if (hand == nil)
		DoFatalAlert("\pError reading RelP resource!");
	HLock(hand);
	pointPtr = (TQ3Point3D *)*hand;
	
	if ((long)(GetHandleSize(hand) / sizeof(TQ3Point3D)) != skeleton->numDecomposedPoints)
		DoFatalAlert("\p# of points in Reference Model has changed!");
	else
		for (i = 0; i < skeleton->numDecomposedPoints; i++)
			skeleton->decomposedPointList[i].boneRelPoint = pointPtr[i];

	ReleaseResource(hand);
	
	
			/*********************/
			/* READ ANIM INFO   */
			/*********************/
			
	for (i=0; i < numAnims; i++)
	{
				/* READ ANIM HEADER */

		hand = GetResource('AnHd',1000+i);
		if (hand == nil)
			DoFatalAlert("\pError getting anim header resource");
		HLock(hand);
		animHeaderPtr = (SkeletonFile_AnimHeader_Type *)*hand;

		skeleton->NumAnimEvents[i] = animHeaderPtr->numAnimEvents;			// copy # anim events in anim	
		ReleaseResource(hand);

			/* READ ANIM-EVENT DATA */
			
		hand = GetResource('Evnt',1000+i);
		if (hand == nil)
			DoFatalAlert("\pError reading anim-event data resource!");
		animEventPtr = (AnimEventType *)*hand;
		for (j=0;  j < skeleton->NumAnimEvents[i]; j++)
			skeleton->AnimEventsList[i][j] = *animEventPtr++;
		ReleaseResource(hand);		


			/* READ # KEYFRAMES PER JOINT IN EACH ANIM */
					
		hand = GetResource('NumK',1000+i);									// read array of #'s for this anim
		if (hand == nil)
			DoFatalAlert("\pError reading # keyframes/joint resource!");
		for (j=0; j < numJoints; j++)
			skeleton->JointKeyframes[j].numKeyFrames[i] = (*hand)[j];
		ReleaseResource(hand);
	}


	for (j=0; j < numJoints; j++)
	{
				/* ALLOC 2D ARRAY FOR KEYFRAMES */
				
		Alloc_2d_array(JointKeyframeType,skeleton->JointKeyframes[j].keyFrames,	numAnims,MAX_KEYFRAMES);
		
		if ((skeleton->JointKeyframes[j].keyFrames == nil) || (skeleton->JointKeyframes[j].keyFrames[0] == nil))
			DoFatalAlert("\pReadDataFromSkeletonFile: Error allocating Keyframe Array.");

					/* READ THIS JOINT'S KF'S FOR EACH ANIM */
					
		for (i=0; i < numAnims; i++)								
		{
			numKeyframes = skeleton->JointKeyframes[j].numKeyFrames[i];					// get actual # of keyframes for this joint
			if (numKeyframes > MAX_KEYFRAMES)
				DoFatalAlert("\pError: numKeyframes > MAX_KEYFRAMES");
		
					/* READ A JOINT KEYFRAME */
					
			hand = GetResource('KeyF',1000+(i*100)+j);
			if (hand == nil)
				DoFatalAlert("\pError reading joint keyframes resource!");
			keyFramePtr = (JointKeyframeType *)*hand;
			for (k = 0; k < numKeyframes; k++)												// copy this joint's keyframes for this anim
				skeleton->JointKeyframes[j].keyFrames[i][k] = *keyFramePtr++;
			ReleaseResource(hand);		
		}
	}
	
}

#pragma mark -

/**************** OPEN GAME FILE **********************/

void	OpenGameFile(Str255 filename,short *fRefNumPtr, Str255 errString)
{
OSErr		iErr;
FSSpec		spec;
Str255		s;

				/* FIRST SEE IF WE CAN GET IT OFF OF DEFAULT VOLUME */

	iErr = FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, filename, &spec);
	if (iErr == noErr)
	{
		iErr = FSpOpenDF(&spec, fsRdPerm, fRefNumPtr);
		if (iErr == noErr)
			return;
	}

	if (iErr == fnfErr)
		DoFatalAlert2(errString,"\pFILE NOT FOUND.");
	else
	{
		NumToString(iErr,s);
		DoFatalAlert2(errString,s);
	}
}




/******************** LOAD PREFS **********************/
//
// Load in standard preferences
//

OSErr LoadPrefs(PrefsType *prefBlock)
{
OSErr		iErr;
short		refNum;
FSSpec		file;
long		count;
				
				/*************/
				/* READ FILE */
				/*************/
					
	FSMakeFSSpec(gPrefsFolderVRefNum, gPrefsFolderDirID, "\p:Bugdom:Prefs2", &file);
	iErr = FSpOpenDF(&file, fsRdPerm, &refNum);	
	if (iErr)
		return(iErr);

	count = sizeof(PrefsType);
	iErr = FSRead(refNum, &count,  (Ptr)prefBlock);		// read data from file
	if (iErr)
	{
		FSClose(refNum);			
		return(iErr);
	}
	
	FSClose(refNum);			
	
	return(noErr);
}


/******************** SAVE PREFS **********************/

void SavePrefs(PrefsType *prefs)
{
FSSpec				file;
OSErr				iErr;
short				refNum;
long				count;
						
				/* CREATE BLANK FILE */
				
	FSMakeFSSpec(gPrefsFolderVRefNum, gPrefsFolderDirID, "\p:Bugdom:Prefs2", &file);
	FSpDelete(&file);															// delete any existing file
	iErr = FSpCreate(&file, 'BalZ', 'Pref', smSystemScript);					// create blank file
	if (iErr)
		return;

				/* OPEN FILE */
					
	iErr = FSpOpenDF(&file, fsRdWrPerm, &refNum);
	if (iErr)
	{
		FSpDelete(&file);
		return;
	}
		
				/* WRITE DATA */

	count = sizeof(PrefsType);
	FSWrite(refNum, &count, prefs);	
	FSClose(refNum);
}

#pragma mark -


/***************************** SAVE GAME ********************************/

void SaveGame(void)
{
#if 0
SaveGameType	**dataHandle,*saveData;
Str255			name = "\pBugdom Saved Game";
short			fRefNum;
FSSpec			*specPtr;
NavReplyRecord	navReply;
FSSpec			spec;

			
	InitCursor();		
	FlushEvents (everyEvent, REMOVE_ALL_EVENTS);	
	
	
	if (!NavServicesAvailable())
	{	
		StandardFileReply	reply;
		
		/* DO STANDARD FILE DIALOG */
do_std:	
		StandardPutFile("\pSave Game As...",name,&reply);
		HideCursor();
		CleanScreenBorder();		
		if (!reply.sfGood)											// see if cancelled
			return;

		specPtr = &reply.sfFile;

		if (reply.sfReplacing)										// delete existing file
			FSpDelete(specPtr);
	}
	else
	{		
			/* DO NAV SERVICES */

		if (PutFileWithNavServices(&navReply, &spec))
			goto do_std;	
		specPtr = &spec;	
		if (navReply.replacing)										// see if delete
			FSpDelete(specPtr);
	}

			
			/*************************/	
			/* CREATE SAVE GAME DATA */
			/*************************/	
					
	dataHandle = (SaveGameType **)AllocHandle(sizeof(SaveGameType));
	if (dataHandle == nil)
		DoFatalAlert("\pSaveGame: AllocHandle failed!");
	HLock((Handle)dataHandle);
	saveData = *dataHandle;

	saveData->version		= SAVE_GAME_VERSION;				// save file version #
	saveData->score 		= gScore;
	saveData->realLevel		= gRealLevel+1;						// save @ beginning of next level
	saveData->numLives 		= gNumLives;
	saveData->health		= gMyHealth;
	saveData->ballTimer		= gBallTimer;
	saveData->numGoldClovers= gNumGoldClovers;

				/* CREATE & OPEN THE REZ-FORK */
			
	FSpCreateResFile(specPtr,'BalZ','BSav',nil);
	if (ResError())
	{
		DoAlert("\pError creating Save file");
		DisposeHandle((Handle)dataHandle);
		return;
	}
	fRefNum = FSpOpenResFile(specPtr,fsRdWrPerm);
	if (fRefNum == -1)
	{
		DoAlert("\pError opening Save Rez file");
		DisposeHandle((Handle)dataHandle);
		return;
	}
				
	UseResFile(fRefNum);
	
				/* WRITE TO FILE */

	AddResource((Handle)dataHandle, 'Save', 1000, "\pSave Data");
	WriteResource((Handle)dataHandle);									// force the update
	ReleaseResource((Handle)dataHandle);								// free the data
	
			/* CLOSE REZ FILE */
			
	CloseResFile(fRefNum);
	
	
	if (NavServicesAvailable())											// do nav svcs cleanup
	{
		NavCompleteSave(&navReply, kNavTranslateInPlace);
		NavDisposeReply(&navReply);
	}
#endif	
	HideCursor();
}


/***************************** LOAD SAVED GAME ********************************/

OSErr LoadSavedGame(void)
{
#if 0
SaveGameType	**dataHandle,*saveData;
short			fRefNum;
FSSpec			spec, *specPtr;
StandardFileReply	reply;
SFTypeList		typeList;
Boolean			hasNav = false;
Boolean			restartMusic = false;

	if (!gMuteMusicFlag)
	{
		ToggleMusic();
		restartMusic = true;
	}
	

	InitCursor();
	FlushEvents (everyEvent, REMOVE_ALL_EVENTS);	

	{EventRecord theEvent;
	WaitNextEvent(everyEvent,&theEvent, 0, 0);}
	

			/* SEE IF WE HAVE NAV SERVICES */
		
	hasNav = NavServicesAvailable();

	if (!hasNav)
	{
						/* DO STANDARD FILE DIALOG */

		typeList[0] = 'BSav';
		StandardGetFile(nil,1,typeList,&reply);
		HideCursor();
		CleanScreenBorder();		
		
		if (!reply.sfGood)													// see if cancelled
			goto err_exit;
		specPtr = &reply.sfFile;											// get ptr to FSSPEC
	}	
	else
	{
				/* GET FILE WITH NAVIGATION SERVICES */
			
		if (GetFileWithNavServices(nil, &spec) != noErr)
			goto err_exit;

		specPtr = &spec;
	}
	
			
				/* OPEN THE REZ-FORK */
			
	fRefNum = FSpOpenResFile(specPtr,fsRdPerm);
	if (fRefNum == -1)
	{
err_exit:	
		if (restartMusic)
			ToggleMusic();
	
		return(1);
	}
	UseResFile(fRefNum);

				/* READ FROM FILE */

	dataHandle = (SaveGameType **)GetResource('Save',1000);
	if (dataHandle == nil)
	{
		DoAlert("\pError reading save game resource!");
		return(!noErr);
	}
	HLock((Handle)dataHandle);
	saveData = *dataHandle;
	
			/**********************/	
			/* USE SAVE GAME DATA */
			/**********************/	
					
	gScore 			= saveData->score;
	gRealLevel 		= saveData->realLevel;
	gMyHealth		= saveData->health;
	gNumLives		= saveData->numLives;
	gBallTimer		= saveData->ballTimer;
	gNumGoldClovers = saveData->numGoldClovers;
		
	ReleaseResource((Handle)dataHandle);
	
	
			/* CLOSE REZ FILE */
			
	CloseResFile(fRefNum);	
	
	if (restartMusic)
		ToggleMusic();
		
	gRestoringSavedGame = true;
#endif	
	return(noErr);
}



#pragma mark -

/******************* LOAD PLAYFIELD *******************/

void LoadPlayfield(FSSpec *specPtr)
{
short	fRefNum;

			
				/* OPEN THE REZ-FORK */
			
	fRefNum = FSpOpenResFile(specPtr,fsCurPerm);
	if (fRefNum == -1)
		DoFatalAlert("\pLoadPlayfield: FSpOpenResFile failed");				
	UseResFile(fRefNum);
	
	
			/* READ PLAYFIELD RESOURCES */
						
	ReadDataFromPlayfieldFile(specPtr);
	
	
			/* CLOSE REZ FILE */
			
	CloseResFile(fRefNum);
	UseResFile(gMainAppRezFile);
	UseResFile(gTextureRezfile);
	
		
	
				/***********************/
				/* DO ADDITIONAL SETUP */
				/***********************/
	
	gTerrainTileWidth = (gTerrainTileWidth/SUPERTILE_SIZE)*SUPERTILE_SIZE;		// round size down to nearest supertile multiple
	gTerrainTileDepth = (gTerrainTileDepth/SUPERTILE_SIZE)*SUPERTILE_SIZE;	
	
	gTerrainUnitWidth = gTerrainTileWidth*TERRAIN_POLYGON_SIZE;					// calc world unit dimensions of terrain
	gTerrainUnitDepth = gTerrainTileDepth*TERRAIN_POLYGON_SIZE;
	gNumSuperTilesDeep = gTerrainTileDepth/SUPERTILE_SIZE;						// calc size in supertiles
	gNumSuperTilesWide = gTerrainTileWidth/SUPERTILE_SIZE;	
	
			/* PRECALC THE TILE SPLIT MODE MATRIX */
			
	CalculateSplitModeMatrix();

		
	BuildTerrainItemList();	

	
				/* INITIALIZE CURRENT SCROLL SETTINGS */

	InitCurrentScrollSettings();
}


/********************** READ DATA FROM PLAYFIELD FILE ************************/

static void ReadDataFromPlayfieldFile(FSSpec *specPtr)
{
Handle					hand;
PlayfieldHeaderType		**header;
long					i,row,col,numLayers;
float					yScale;
short					**xlateTableHand,*xlateTbl;

#pragma unused (specPtr)
	
	if (gDoCeiling)									// see if need to read in ceiling data
		numLayers = 2;
	else
		numLayers = 1;

			/************************/
			/* READ HEADER RESOURCE */
			/************************/

	hand = GetResource('Hedr',1000);
	if (hand == nil)
	{
		DoAlert("\pReadDataFromPlayfieldFile: Error reading header resource!");
		return;
	}
	
	header = (PlayfieldHeaderType **)hand;	
	gNumTerrainItems		= (**header).numItems;
	gTerrainTileWidth		= (**header).mapWidth;
	gTerrainTileDepth		= (**header).mapHeight;	
	gNumTerrainTextureTiles	= (**header).numTilesInList;	
	g3DTileSize				= (**header).tileSize;
	g3DMinY					= (**header).minY;
	g3DMaxY					= (**header).maxY;
	gNumSplines				= (**header).numSplines;
	gNumFences				= (**header).numFences;
	ReleaseResource(hand);

				/* CALC SOME GLOBALS HERE */
				
	if (gNumTerrainTextureTiles > MAX_TERRAIN_TILES)
		DoFatalAlert("\pReadDataFromPlayfieldFile: gNumTerrainTextureTiles > MAX_TERRAIN_TILES");				
				

			/**************************/
			/* TILE RELATED RESOURCES */
			/**************************/


			/* READ TILE IMAGE DATA */

	hand = GetResource('Timg',1000);
	if (hand == nil)
		DoFatalAlert("\pReadDataFromPlayfieldFile: Error reading tile image data resource!");
	else
	{
		DetachResource(hand);							// lets keep this data around
		gTileDataHandle = (u_short **)hand;
	}


#if 0
			/* READ TILE ATTRIBUTES */

	hand = GetResource('Atrb',1000);
	if (hand == nil)
		DoFatalAlert("\pReadDataFromPlayfieldFile: Error reading tile attrib resource!");
	else
	{
		DetachResource(hand);							// lets keep this data around
		gTileAttributes = (TileAttribType **)hand;
	}
#endif

			/* READ TILE->IMAGE XLATE TABLE */

	hand = GetResource('Xlat',1000);
	if (hand == nil)
		DoFatalAlert("\pReadDataFromPlayfieldFile: Error reading tile image xlate resource!");
	else
	{
		DetachResource(hand);
		HLockHi(hand);									// hold on to this rez until we're done reading maps below
		xlateTableHand = (short **)hand;
		xlateTbl = *xlateTableHand;
	}



			/*******************************/
			/* MAP LAYER RELATED RESOURCES */
			/*******************************/

			/* READ FLOOR MAP */
				
	Alloc_2d_array(u_short, gFloorMap, gTerrainTileDepth, gTerrainTileWidth);		// alloc 2D array for floor map
	
	
	hand = GetResource('Layr',1000);												// load map from rez
	if (hand == nil)
		DoFatalAlert("\pReadDataFromPlayfieldFile: Error reading floor resource!");
	else																			// copy rez into 2D array
	{
		u_short	*src;
		src = (u_short *)*hand;					
		for (row = 0; row < gTerrainTileDepth; row++)
			for (col = 0; col < gTerrainTileWidth; col++)
			{
				u_short	tile, imageNum;
				
				tile = *src++;														// get original tile with all bits
				imageNum = xlateTbl[tile & TILENUM_MASK];							// get image # from xlate table
				gFloorMap[row][col] = (tile&(~TILENUM_MASK)) | imageNum;			// insert image # into bitfield
			}
		ReleaseResource(hand);
	}		


	if (gDoCeiling)
	{
				/* READ CEILING MAP */
					
		Alloc_2d_array(u_short, gCeilingMap, gTerrainTileDepth, gTerrainTileWidth);		// alloc 2D array for map
		
		hand = GetResource('Layr',1001);												// load map from rez
		if (hand == nil)
			DoFatalAlert("\pReadDataFromPlayfieldFile: Error reading ceiling resource!");
		else																			// copy rez into 2D array
		{
			u_short	*src;
			src = (u_short *)*hand;					
			for (row = 0; row < gTerrainTileDepth; row++)
				for (col = 0; col < gTerrainTileWidth; col++)
				{
					u_short	tile, imageNum;
					
					tile = *src++;														// get original tile with all bits
					imageNum = xlateTbl[tile & TILENUM_MASK];							// get image # from xlate table
					gCeilingMap[row][col] = (tile&(~TILENUM_MASK)) | imageNum;			// insert image # into bitfield
				}
			ReleaseResource(hand);
		}
	}
	
	ReleaseResource((Handle)xlateTableHand);								// we dont need the xlate table anymore


#if USE_PATH_LAYER
			/* READ PATH MAP */
				
	Alloc_2d_array(u_char, gPathMap, gTerrainTileDepth, gTerrainTileWidth);			// alloc 2D array for map
	
	hand = GetResource('Layr',1002);												// load map from rez
	if (hand == nil)
		DoFatalAlert("\pReadDataFromPlayfieldFile: Error reading ceiling resource!");
	else																			// copy rez into 2D array
	{
		u_short	*src;
		src = (u_short *)*hand;					
		for (row = 0; row < gTerrainTileDepth; row++)
			for (col = 0; col < gTerrainTileWidth; col++)
			{
				int	num = *src++ - BASE_PATH_TILE;				
				if (num < 0)
					num = 0;
				gPathMap[row][col] = num;						// convert to 0 based (0 = blank, >0 = tile)
			}
				
		ReleaseResource(hand);
	}		
#endif	
	
			/* READ HEIGHT DATA MATRIX */
	
	yScale = TERRAIN_POLYGON_SIZE / g3DTileSize;						// need to scale original geometry units to game units
	
	Alloc_2d_array(TerrainYCoordType, gMapYCoords, gTerrainTileDepth+1, gTerrainTileWidth+1);	// alloc 2D array for map
	
	for (i = 0; i < numLayers; i++)
	{		
		float	*src;
	
		hand = GetResource('YCrd',1000+i);
		if (hand == nil)
			DoAlert("\pReadDataFromPlayfieldFile: Error reading height data resource!");
		else
		{
			src = (float *)*hand;					
			for (row = 0; row <= gTerrainTileDepth; row++)
				for (col = 0; col <= gTerrainTileWidth; col++)
					gMapYCoords[row][col].layerY[i] = *src++ * yScale;
			ReleaseResource(hand);
		}
	}			


			/* READ VERTEX COLOR MATRIX */
			
	
	for (i = 0; i < numLayers; i++)
	{		
		u_short	*src;

		Alloc_2d_array(u_short, gVertexColors[i], gTerrainTileDepth+1, gTerrainTileWidth+1);	// alloc 2D array for map
		if (gVertexColors[i] == nil)
			DoFatalAlert("\pReadDataFromPlayfieldFile: Alloc_2d_array failed!");
	
		hand = GetResource('Vcol',1000+i);
		if (hand == nil)
			DoFatalAlert("\pReadDataFromPlayfieldFile: Error reading color data resource!");
		else
		{
			src = (u_short *)*hand;					
			for (row = 0; row <= gTerrainTileDepth; row++)
				for (col = 0; col <= gTerrainTileWidth; col++)
					gVertexColors[i][row][col] = *src++;
			ReleaseResource(hand);
		}
	}			

			/* READ SPLIT MODE MATRIX */
	
	Alloc_2d_array(TerrainInfoMatrixType, gMapInfoMatrix, gTerrainTileDepth, gTerrainTileWidth);	// alloc 2D array for map
	
	for (i = 0; i < numLayers; i++)												// floor & ceiling
	{		
		Byte	*src;
	
		hand = GetResource('Splt',1000+i);
		if (hand == nil)
			DoFatalAlert("\pReadDataFromPlayfieldFile: Error reading splitmode data resource!");
		else
		{	
			src = (Byte *)*hand;					
			for (row = 0; row < gTerrainTileDepth; row++)						
				for (col = 0; col < gTerrainTileWidth; col++)
					gMapInfoMatrix[row][col].splitMode[i] = *src++;
			ReleaseResource(hand);
		}
	}

	
				/**************************/
				/* ITEM RELATED RESOURCES */
				/**************************/
	
				/* READ ITEM LIST */
				
	hand = GetResource('Itms',1000);
	if (hand == nil)
		DoAlert("\pReadDataFromPlayfieldFile: Error reading itemlist resource!");
	else
	{
		DetachResource(hand);							// lets keep this data around		
		HLockHi(hand);									// LOCK this one because we have the lookup table into this
		gMasterItemList = (TerrainItemEntryType **)hand;
	}
		
	
			/****************************/
			/* SPLINE RELATED RESOURCES */
			/****************************/
	
			/* READ SPLINE LIST */
			
	hand = GetResource('Spln',1000);
	if (hand)
	{	
		DetachResource(hand);
		HLockHi(hand);
		gSplineList = (SplineDefType **)hand;
	}
	else
		gNumSplines = 0;	

	
			/* READ SPLINE POINT LIST */
			
	for (i = 0; i < gNumSplines; i++)
	{
		hand = GetResource('SpPt',1000+i);
		if (hand)
		{	
			DetachResource(hand);
			HLockHi(hand);
			(*gSplineList)[i].pointList = (SplinePointType **)hand;
		}
		else
			DoFatalAlert("\pReadDataFromPlayfieldFile: cant get spline points rez");
	}	


			/* READ SPLINE ITEM LIST */
			
	for (i = 0; i < gNumSplines; i++)
	{
		hand = GetResource('SpIt',1000+i);
		if (hand)
		{	
			DetachResource(hand);
			HLockHi(hand);
			(*gSplineList)[i].itemList = (SplineItemType **)hand;
		}
		else
			DoFatalAlert("\pReadDataFromPlayfieldFile: cant get spline items rez");
	}	
	
	
			/****************************/
			/* FENCE RELATED RESOURCES */
			/****************************/
	
			/* READ FENCE LIST */
			
	hand = GetResource('Fenc',1000);
	if (hand)
	{	
		FileFenceDefType *inData;

		gFenceList = (FenceDefType *)AllocPtr(sizeof(FenceDefType) * gNumFences);	// alloc new ptr for fence data
		if (gFenceList == nil)
			DoFatalAlert("\pReadDataFromPlayfieldFile: AllocPtr failed");
			
		inData = (FileFenceDefType *)*hand;								// get ptr to input fence list
		
		for (i = 0; i < gNumFences; i++)								// copy data from rez to new list
		{
			gFenceList[i].type 		= inData[i].type;
			gFenceList[i].numNubs 	= inData[i].numNubs;
			gFenceList[i].nubList 	= nil;
			gFenceList[i].bBox.top		= inData[i].bBox.top;
			gFenceList[i].bBox.bottom	= inData[i].bBox.bottom;
			gFenceList[i].bBox.left		= inData[i].bBox.left;
			gFenceList[i].bBox.right	= inData[i].bBox.right;
		}		
		ReleaseResource(hand);
	}
	else
		gNumFences = 0;	

	
			/* READ FENCE NUB LIST */
			
	for (i = 0; i < gNumFences; i++)
	{
		hand = GetResource('FnNb',1000+i);
		if (hand)
		{	
			DetachResource(hand);
			HLockHi(hand);
			gFenceList[i].nubList = (FencePointType **)hand;
		}
		else
			DoFatalAlert("\pReadDataFromPlayfieldFile: cant get fence nub rez");
	}
}



/**************** DRAW PICTURE INTO GWORLD ***********************/
//
// Uses Quicktime to load any kind of picture format file and draws
// it into the GWorld
//
//
// INPUT: myFSSpec = spec of image file
//
// OUTPUT:	theGWorld = gworld contining the drawn image.
//

OSErr DrawPictureIntoGWorld(FSSpec *myFSSpec, GWorldPtr *theGWorld)
{
OSErr						iErr;
GraphicsImportComponent		gi;
Rect						r;
ComponentResult				result;


			/* PREP IMPORTER COMPONENT */
			
	result = GetGraphicsImporterForFile(myFSSpec, &gi);		// load importer for this image file
	if (result != noErr)
	{
		DoAlert("\pDrawPictureIntoGWorld: GetGraphicsImporterForFile failed!");
		return(result);
	}
	GraphicsImportGetBoundsRect(gi, &r);					// get dimensions of image


			/* UPDATE PICT GWORLD */
	
	iErr = NewGWorld(theGWorld, 16, &r, nil, nil, 0);					// try app mem
	if (iErr)
	{
		iErr = NewGWorld(theGWorld, 16, &r, nil, nil, useTempMem);		// try sys mem
		if (iErr)
		{
			DoAlert("\pDrawPictureIntoGWorld: MakeMyGWorld failed");
			return(1);
		}
	}
	


			/* DRAW INTO THE GWORLD */
	
	DoLockPixels(*theGWorld);	
	GraphicsImportSetGWorld(gi, *theGWorld, nil);				// set the gworld to draw image into
	result = GraphicsImportDraw(gi);						// draw into gworld
	CloseComponent(gi);										// cleanup
	if (result != noErr)
	{
		DoAlert("\pDrawPictureIntoGWorld: GraphicsImportDraw failed!");
		ShowSystemErr(result);
		DisposeGWorld (*theGWorld);
		return(result);
	}
	return(noErr);
}


#pragma mark -

/************************** LOAD LEVEL ART ***************************/

void LoadLevelArt(void)
{
FSSpec	spec;

			/* LOAD GLOBAL STUFF */

	FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:models:Global_Models1.3dmf", &spec);
	LoadGrouped3DMF(&spec,MODEL_GROUP_GLOBAL1);	
	FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:models:Global_Models2.3dmf", &spec);
	LoadGrouped3DMF(&spec,MODEL_GROUP_GLOBAL2);	
			
	FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:Audio:Main.sounds", &spec);
	LoadSoundBank(&spec, SOUND_BANK_DEFAULT);

	LoadASkeleton(SKELETON_TYPE_ME);			
	LoadASkeleton(SKELETON_TYPE_LADYBUG);			
	LoadASkeleton(SKELETON_TYPE_BUDDY);			
	
			/*****************************/
			/* LOAD LEVEL SPECIFIC STUFF */
			/*****************************/
			
	switch(gLevelType)
	{
				/***********************/
				/* LEVEL 1: THE GARDEN */
				/***********************/
				
		case	LEVEL_TYPE_LAWN:
				if (gAreaNum == 0)
					FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:terrain:Training.ter", &spec);
				else
					FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:terrain:Lawn.ter", &spec);
				
				LoadPlayfield(&spec);

				/* LOAD MODELS */
						
				FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:models:Lawn_Models1.3dmf", &spec);
				LoadGrouped3DMF(&spec,MODEL_GROUP_LEVELSPECIFIC);	
				FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:models:Lawn_Models2.3dmf", &spec);
				LoadGrouped3DMF(&spec,MODEL_GROUP_LEVELSPECIFIC2);	
				
				
				/* LOAD SKELETON FILES */
				
				LoadASkeleton(SKELETON_TYPE_BOXERFLY);			
				LoadASkeleton(SKELETON_TYPE_SLUG);			
				LoadASkeleton(SKELETON_TYPE_ANT);			

				/* LOAD SOUNDS */
				
				FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:Audio:Lawn.sounds", &spec);
				LoadSoundBank(&spec, SOUND_BANK_LEVELSPECIFIC);
				break;


				/*****************/
				/* LEVEL 2: POND */
				/*****************/
				
		case	LEVEL_TYPE_POND:
				FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:terrain:Pond.ter", &spec);
				LoadPlayfield(&spec);

				/* LOAD MODELS */
						
				FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:models:Pond_Models.3dmf", &spec);
				LoadGrouped3DMF(&spec,MODEL_GROUP_LEVELSPECIFIC);	
				
				
				/* LOAD SKELETON FILES */
				
				LoadASkeleton(SKELETON_TYPE_MOSQUITO);			
				LoadASkeleton(SKELETON_TYPE_WATERBUG);			
				LoadASkeleton(SKELETON_TYPE_PONDFISH);			
				LoadASkeleton(SKELETON_TYPE_SKIPPY);			
				LoadASkeleton(SKELETON_TYPE_SLUG);			


				/* LOAD SOUNDS */
				
				FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:Audio:Pond.sounds", &spec);
				LoadSoundBank(&spec, SOUND_BANK_LEVELSPECIFIC);
				break;


				/*******************/
				/* LEVEL 3: FOREST */
				/*******************/
				
		case	LEVEL_TYPE_FOREST:
				if (gAreaNum == 0)
					FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:terrain:Beach.ter", &spec);
				else
					FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:terrain:Flight.ter", &spec);
				LoadPlayfield(&spec);

				/* LOAD MODELS */
						
				FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:models:Forest_Models.3dmf", &spec);
				LoadGrouped3DMF(&spec,MODEL_GROUP_LEVELSPECIFIC);	
				
				
				/* LOAD SKELETON FILES */
				
				LoadASkeleton(SKELETON_TYPE_DRAGONFLY);			
				LoadASkeleton(SKELETON_TYPE_FOOT);	
				LoadASkeleton(SKELETON_TYPE_SPIDER);	
				LoadASkeleton(SKELETON_TYPE_CATERPILLER);	
				LoadASkeleton(SKELETON_TYPE_BAT);	
				LoadASkeleton(SKELETON_TYPE_FLYINGBEE);			
				LoadASkeleton(SKELETON_TYPE_ANT);			
				
				/* LOAD SOUNDS */
				
				FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:Audio:Forest.sounds", &spec);
				LoadSoundBank(&spec, SOUND_BANK_LEVELSPECIFIC);
						
				break;


				/*************************/
				/* LEVEL 4:  BEE HIVE    */
				/*************************/
				
		case	LEVEL_TYPE_HIVE:
			
				if (gAreaNum == 0)
					FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:terrain:BeeHive.ter", &spec);
				else
					FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:terrain:QueenBee.ter", &spec);
				LoadPlayfield(&spec);

				/* LOAD MODELS */
						
				FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:models:BeeHive_Models.3dmf", &spec);
				LoadGrouped3DMF(&spec,MODEL_GROUP_LEVELSPECIFIC);	
				
				
				/* LOAD SKELETON FILES */
				
				LoadASkeleton(SKELETON_TYPE_LARVA);			
				LoadASkeleton(SKELETON_TYPE_FLYINGBEE);			
				LoadASkeleton(SKELETON_TYPE_WORKERBEE);			
				LoadASkeleton(SKELETON_TYPE_QUEENBEE);			

				
				/* LOAD SOUNDS */
				
				FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:Audio:Hive.sounds", &spec);
				LoadSoundBank(&spec, SOUND_BANK_LEVELSPECIFIC);
				
				break;


				/*******************/
				/* LEVEL 5:  NIGHT */
				/*******************/
				
		case	LEVEL_TYPE_NIGHT:
				FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:terrain:Night.ter", &spec);
				LoadPlayfield(&spec);

				/* LOAD MODELS */
						
				FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:models:Night_Models.3dmf", &spec);
				LoadGrouped3DMF(&spec,MODEL_GROUP_LEVELSPECIFIC);	
				
				
				/* LOAD SKELETON FILES */
				
				LoadASkeleton(SKELETON_TYPE_FIREANT);			
				LoadASkeleton(SKELETON_TYPE_FIREFLY);			
				LoadASkeleton(SKELETON_TYPE_CATERPILLER);	
				LoadASkeleton(SKELETON_TYPE_SLUG);	
				LoadASkeleton(SKELETON_TYPE_ROACH);	
				LoadASkeleton(SKELETON_TYPE_ANT);	

				
				/* LOAD SOUNDS */
				
				FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:Audio:Night.sounds", &spec);
				LoadSoundBank(&spec, SOUND_BANK_LEVELSPECIFIC);
				break;

	
				/*************************/
				/* LEVEL 6:  ANT HILL    */
				/*************************/
				
		case	LEVEL_TYPE_ANTHILL:
				if (gAreaNum == 0)
					FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:terrain:AntHill.ter", &spec);
				else
					FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:terrain:AntKing.ter", &spec);
				LoadPlayfield(&spec);

				/* LOAD MODELS */
						
				FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:models:AntHill_Models.3dmf", &spec);
				LoadGrouped3DMF(&spec,MODEL_GROUP_LEVELSPECIFIC);	
				
				
				/* LOAD SKELETON FILES */
				
				if (gRealLevel == LEVEL_NUM_ANTKING)
					LoadASkeleton(SKELETON_TYPE_KINGANT);			
					
				LoadASkeleton(SKELETON_TYPE_SLUG);			
				LoadASkeleton(SKELETON_TYPE_ANT);			
				LoadASkeleton(SKELETON_TYPE_FIREANT);			
				LoadASkeleton(SKELETON_TYPE_ROOTSWING);			
				LoadASkeleton(SKELETON_TYPE_ROACH);	

				/* LOAD SOUNDS */
				
				FSMakeFSSpec(gDataSpec.vRefNum, gDataSpec.parID, "\p:Audio:AntHill.sounds", &spec);
				LoadSoundBank(&spec, SOUND_BANK_LEVELSPECIFIC);
				break;

		default:
				DoFatalAlert("\pLoadLevelArt: unsupported level #");
	}
	
	
			/* CAST SHADOWS */
			
	DoItemShadowCasting();
}


#if SHAREWARE


/******************* GET DEMO TIMER *************************/

void GetDemoTimer(void)
{
OSErr				iErr;
short				refNum;
FSSpec				file;
long				count;

				/* READ TIMER FROM FILE */
					
	FSMakeFSSpec(gPrefsFolderVRefNum, gPrefsFolderDirID, "\pJavaY", &file);
	iErr = FSpOpenDF(&file, fsRdPerm, &refNum);	
	if (iErr)
		gDemoVersionTimer = 0;
	else
	{
		count = sizeof(gDemoVersionTimer);
		iErr = FSRead(refNum, &count,  &gDemoVersionTimer);			// read data from file
		if (iErr)
		{
			FSClose(refNum);			
			FSpDelete(&file);										// file is corrupt, so delete
			gDemoVersionTimer = 0;
			return;
		}
		FSClose(refNum);			
	}	
	
}


/************************ SAVE DEMO TIMER ******************************/

void SaveDemoTimer(void)
{
FSSpec				file;
OSErr				iErr;
short				refNum;
long				count;

				/* CREATE BLANK FILE */
				
	FSMakeFSSpec(gPrefsFolderVRefNum, gPrefsFolderDirID, "\pJavaY", &file);
	FSpDelete(&file);															// delete any existing file
	iErr = FSpCreate(&file, '????', 'xxxx', smSystemScript);					// create blank file
	if (iErr)
		return;


				/* OPEN FILE */
					
	FSMakeFSSpec(gPrefsFolderVRefNum, gPrefsFolderDirID, "\pJavaY", &file);
	iErr = FSpOpenDF(&file, fsRdWrPerm, &refNum);
	if (iErr)
		return;

				/* WRITE DATA */
				
	count = sizeof(gDemoVersionTimer);
	FSWrite(refNum, &count, &gDemoVersionTimer);	
	FSClose(refNum);			
}

#else

/******************* GET DEMO TIMER *************************/

void GetDemoTimer(void)
{
OSErr				iErr;
short				refNum;
FSSpec				file;
long				count;
	
				/* READ TIMER FROM FILE */
					
	FSMakeFSSpec(gPrefsFolderVRefNum, gPrefsFolderDirID, "\pHeeHaw", &file);
	iErr = FSpOpenDF(&file, fsRdPerm, &refNum);	
	if (iErr)
		gDemoVersionTimer = 0;
	else
	{
		count = sizeof(float);
		iErr = FSRead(refNum, &count,  &gDemoVersionTimer);			// read data from file
		if (iErr)
		{
			FSClose(refNum);			
			FSpDelete(&file);										// file is corrupt, so delete
			gDemoVersionTimer = 0;
			return;
		}
		FSClose(refNum);			
	}	
	
		/* SEE IF TIMER HAS EXPIRED */

#if EXPIREDEMO	
	if (gDemoVersionTimer > (60*30))		// let play for n minutes
	{
		DoDemoExpiredScreen();	
	}
#endif	
}


/************************ SAVE DEMO TIMER ******************************/

void SaveDemoTimer(void)
{
FSSpec				file;
OSErr				iErr;
short				refNum;
long				count;

				/* CREATE BLANK FILE */
				
	FSMakeFSSpec(gPrefsFolderVRefNum, gPrefsFolderDirID, "\pHeeHaw", &file);
	FSpDelete(&file);															// delete any existing file
	iErr = FSpCreate(&file, '????', 'xxxx', smSystemScript);					// create blank file
	if (iErr)
		return;


				/* OPEN FILE */
					
	FSMakeFSSpec(gPrefsFolderVRefNum, gPrefsFolderDirID, "\pHeeHaw", &file);
	iErr = FSpOpenDF(&file, fsRdWrPerm, &refNum);
	if (iErr)
		return;

				/* WRITE DATA */
				
	count = sizeof(float);
	FSWrite(refNum, &count, &gDemoVersionTimer);	
	FSClose(refNum);			
}

#endif

#pragma mark -


/************************ NAV SERVICES:  EVENT PROC *****************************/

pascal void myEventProc(NavEventCallbackMessage callBackSelector, NavCBRecPtr callBackParms,
						NavCallBackUserData callBackUD)
{
//	WindowPtr window = (WindowPtr)(callBackParms->eventData.eventDataParms.event->message);

#pragma unused (callBackUD)



	switch (callBackSelector)
	{
		case	kNavCBEvent:
				switch (((callBackParms->eventData).eventDataParms).event->what)
				{
					case 	updateEvt:
//							MyHandleUpdateEvent(window, (EventRecord*)callBackParms->eventData.event);
							break;
				}
				break;
	}
}


/************************ NAV SERCIES: MY FILTER PROC **************************/

pascal Boolean myFilterProc(AEDesc*theItem,void*info, NavCallBackUserData callBackUD, NavFilterModes filterMode)
{
Boolean display 			= true;
NavFileOrFolderInfo	*theInfo = (NavFileOrFolderInfo*)info;

#pragma unused (callBackUD,  filterMode)

	if (theItem->descriptorType == typeFSS)
		if (!theInfo->isFolder)
			if (theInfo->fileAndFolder.fileInfo.finderInfo.fdType != 'BSav')			// only show us the saved games
				display = false;
				
	return display;
}



/****************** SET DEFAULT DIRECTORY ********************/
//
// This function needs to be called for OS X because OS X doesnt automatically
// set the default directory to the application directory.
//

void SetDefaultDirectory(void)
{
ProcessSerialNumber serial;
ProcessInfoRec info;
FSSpec	app_spec;
WDPBRec wpb;
OSErr	iErr;		
		
	serial.highLongOfPSN = 0;
	serial.lowLongOfPSN = kCurrentProcess;
	
	
	info.processInfoLength = sizeof(ProcessInfoRec);
	info.processName = NULL;
	info.processAppSpec = &app_spec;

	iErr = GetProcessInformation(&serial, &info);

	wpb.ioVRefNum = app_spec.vRefNum;
	wpb.ioWDDirID = app_spec.parID;
	wpb.ioNamePtr = NULL;
	
	iErr = PBHSetVolSync(&wpb);
}			



/********************** CREATE PATH TO DATA FOLDER *********************/
//
// This function gets the POSIX path to the application, and then we append the 
// additional path information to get to the Data folder.
//


void CreatePathToDataFolder(void)
{
ProcessSerialNumber PSN = {0, kCurrentProcess};
FSRef				pathRef;
FSRef				fileRef;
OSStatus			err;
const char			*dataSubPath = "/Contents/Resources/Data/Images";

			/* GET FSREF OF OUR APP */
			
	err = GetProcessBundleLocation(&PSN, &pathRef);
	if (err)
		DoFatalAlert("\pGetProcessBundleLocation() failed!");


			/* CONVERT TO POSIX PATH */
			
	err = FSRefMakePath (&pathRef, (u_char *)gDataFolderPath, sizeof(gDataFolderPath));
	if (err != noErr)
		DoFatalAlert("\pCreatePathToDataFolder: FSRefMakePath() failed!");



			/* APPEND THE PATH OF OUR DATA FOLDER */
			
	strncat(gDataFolderPath, dataSubPath, strlen(dataSubPath));


	FSPathMakeRef((u_char *)gDataFolderPath, &fileRef, nil);									// convert pathname to FSRef

	err = FSGetCatalogInfo (&fileRef, kFSCatInfoNone, NULL, NULL, &gDataSpec, NULL);	// get fsspec from fsref

}


