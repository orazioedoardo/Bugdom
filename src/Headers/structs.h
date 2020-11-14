//
// structs.h
//

#ifndef STRUCTS_H
#define STRUCTS_H


#include "globals.h"

#define	MAX_ANIMS			25
#define	MAX_KEYFRAMES		15
#define	MAX_JOINTS			20
#define	MAX_CHILDREN		(MAX_JOINTS-1)
#define	MAX_LIMBS	MAX_JOINTS

#define MAX_FLAGS_IN_OBJNODE			4		// # flags in ObjNode


#define	MAX_DECOMPOSED_POINTS	1000
#define	MAX_DECOMPOSED_NORMALS	800
#define	MAX_POINTS_ON_BONE		100		// DONT CHANGE!! MUST MATCH VALUE IN BIO-OREO PRO SINCE IS WIRED INTO ARRAY IN SKELETON FILE!!
#define	MAX_POINT_REFS			10		// max times a point can be re-used in multiple places
#define	MAX_DECOMPOSED_TRIMESHES 10

#define	MAX_MORPH_TRIMESHES		10
#define	MAX_MORPH_POINTS		1000



			/*********************/
			/* SPLINE STRUCTURES */
			/*********************/

typedef	struct
{
	float	x,z;
}SplinePointType;

typedef struct
{
	float			placement;			// where on spline to start item (0=front, 1.0 = end)
	u_short			type;
	Byte			parm[4];
	u_short			flags;
}SplineItemType;


typedef struct
{
	short			numNubs;			// # nubs in spline
	SplinePointType	**nubList;			// handle to nub list
	long			numPoints;			// # points in spline
	SplinePointType	**pointList;		// handle to calculated spline points
	short			numItems;			// # items on the spline
	SplineItemType	**itemList;			// handle to spline items
	
	Rect			bBox;				// bounding box of spline area
}SplineDefType;




		/* COLLISION BOX */
		
typedef struct
{
	long	left,right,front,back,top,bottom;
	long	oldLeft,oldRight,oldFront,oldBack,oldTop,oldBottom;
}CollisionBoxType;


		/****************************/
		/* POLYGONAL COLLISION INFO */
		/****************************/
		
typedef struct
{
	TQ3Point3D			verts[3];			// coords of each vertex
	TQ3PlaneEquation	planeEQ;			// plane equation of triangle
}CollisionTriangleType;

typedef struct
{
	TQ3BoundingBox			bBox;				// bounding box of these triangles
	short					numTriangles;		// # triangles in list
	CollisionTriangleType	*triangles;			// ptr to list of collision triangles
}TriangleCollisionList;


//*************************** SKELETON *****************************************/

		/* BONE SPECIFICATIONS */
		//
		//
		// NOTE: Similar to joint definition but lacks animation, rot/scale info.
		//
		
typedef struct
{
	long 				parentBone;			 			// index to previous bone
	TQ3GroupObject		ignored1;			
	TQ3Matrix4x4		ignored2;	
	TQ3TransformObject	ignored3;
	unsigned char		ignored4[32];		
	TQ3Point3D			coord;							// absolute coord (not relative to parent!) 
	u_short				numPointsAttachedToBone;		// # vertices/points that this bone has
	u_short				*pointList;						// indecies into gDecomposedPointList
	u_short				numNormalsAttachedToBone;		// # vertex normals this bone has
	u_short				*normalList;					// indecies into gDecomposedNormalsList
}BoneDefinitionType;


			/* DECOMPOSED POINT INFO */
			
typedef struct
{
	TQ3Point3D	realPoint;							// point coords as imported in 3DMF model
	TQ3Point3D	boneRelPoint;						// point relative to bone coords (offset from bone)
	
	Byte		numRefs;							// # of places this point is used in the geometry data
	Byte		whichTriMesh[MAX_POINT_REFS];		// index to trimeshes
	short		whichPoint[MAX_POINT_REFS];			// index into pointlist of triMesh above
	short		whichNormal[MAX_POINT_REFS];		// index into gDecomposedNormalsList
}DecomposedPointType;



		/* CURRENT JOINT STATE */
		
typedef struct
{
	long		tick;					// time at which this state exists
	long		accelerationMode;		// mode of in/out acceleration
	TQ3Point3D	coord;					// current 3D coords of joint (relative to link)
	TQ3Vector3D	rotation;				// current rotation values of joint (relative to link)
	TQ3Vector3D	scale;					// current scale values of joint mesh
}JointKeyframeType;


		/* JOINT DEFINITIONS */
		
typedef struct
{
	signed char			numKeyFrames[MAX_ANIMS];				// # keyframes
	JointKeyframeType 	**keyFrames;							// 2D array of keyframe data keyFrames[anim#][keyframe#]
}JointKeyFrameHeader;

			/* ANIM EVENT TYPE */
			
typedef struct
{
	short	time;
	Byte	type;
	Byte	value;
}AnimEventType;


			/* SKELETON INFO */
		
typedef struct
{
	Byte				NumBones;						// # joints in this skeleton object
	JointKeyFrameHeader	JointKeyframes[MAX_JOINTS];		// array of joint definitions

	Byte				numChildren[MAX_JOINTS];		// # children each joint has
	Byte				childIndecies[MAX_JOINTS][MAX_CHILDREN];	// index to each child
	
	Byte				NumAnims;						// # animations in this skeleton object
	Byte				*NumAnimEvents;					// ptr to array containing the # of animevents for each anim
	AnimEventType		**AnimEventsList;				// 2 dimensional array which holds a anim event list for each anim AnimEventsList[anim#][event#]

	BoneDefinitionType	*Bones;							// data which describes bone heirarachy
	
	long				numDecomposedTriMeshes;			// # trimeshes in skeleton
	TQ3TriMeshData		*decomposedTriMeshes;			// array of triMeshData

	long				numDecomposedPoints;			// # shared points in skeleton
	DecomposedPointType	*decomposedPointList;			// array of shared points

	short				numDecomposedNormals ;			// # shared normal vectors
	TQ3Vector3D			*decomposedNormalsList;			// array of shared normals


}SkeletonDefType;


		/* THE STRUCTURE ATTACHED TO AN OBJNODE */
		//
		// This contains all of the local skeleton data for a particular ObjNode
		//

typedef struct
{
	Boolean			JointsAreGlobal;				// true when joints are already in world-space coords
	Byte			AnimNum;						// animation #

	Boolean			IsMorphing;						// flag set when morphing from an anim to another
	float			MorphSpeed;						// speed of morphing (1.0 = normal)
	float			MorphPercent;					// percentage of morph from kf1 to kf2 (0.0 - 1.0)

	JointKeyframeType	JointCurrentPosition[MAX_JOINTS];	// for each joint, holds current interpolated keyframe values
	JointKeyframeType	MorphStart[MAX_JOINTS];		// morph start & end keyframes for each joint
	JointKeyframeType	MorphEnd[MAX_JOINTS];

	float			CurrentAnimTime;				// current time index for animation	
	float			LoopBackTime;					// time to loop or zigzag back to (default = 0 unless set by a setmarker)
	float			MaxAnimTime;					// duration of current anim
	float			AnimSpeed;						// time factor for speed of executing current anim (1.0 = normal time)
	Byte			AnimEventIndex;					// current index into anim event list
	Byte			AnimDirection;					// if going forward in timeline or backward			
	Byte			EndMode;						// what to do when reach end of animation
	Boolean			AnimHasStopped;					// flag gets set when anim has reached end of sequence (looping anims don't set this!)

	TQ3Matrix4x4	jointTransformMatrix[MAX_JOINTS];	// holds matrix xform for each joint

	SkeletonDefType	*skeletonDefinition;						// point to skeleton's common/shared data	
//	TQ3TriMeshData	localTriMeshes[MAX_DECOMPOSED_TRIMESHES];	// the triMeshes to submit for this ObjNode
}SkeletonObjDataType;


			/* MORPH DATA STRUCTURE */
			
typedef struct
{	
	float			percent;							// % of current morph from A to B (0 = start, 1 = end)
	
	long			numTriMeshesA;						// # trimeshes in A
	long			numTriMeshesB;						//  # trimeshes in B
	
	TQ3TriMeshData	startTriMeshA[MAX_MORPH_TRIMESHES];					// A's starting mesh
	TQ3Point3D		endCoordA[MAX_MORPH_TRIMESHES][MAX_MORPH_POINTS];	// A's ending points

	TQ3TriMeshData	startTriMeshB[MAX_MORPH_TRIMESHES];					// B's starting mesh
	TQ3Point3D		endCoordB[MAX_MORPH_TRIMESHES][MAX_MORPH_POINTS];	// B's ending points
}MorphObjDataType;


			/* TERRAIN ITEM ENTRY TYPE */

typedef struct
{
	u_short							x,y;
	u_short							type;
	Byte							parm[4];
	u_short							flags;									
}TerrainItemEntryType;




			/****************************/
			/*  OBJECT RECORD STRUCTURE */
			/****************************/

struct ObjNode
{
	short			NodeNum;
	short			GroupNum;
	
	struct ObjNode	*PrevNode;			// address of previous node in linked list
	struct ObjNode	*NextNode;			// address of next node in linked list
	struct ObjNode	*ChainNode;
	struct ObjNode	*ChainHead;			// a chain's head (link back to 1st obj in chain)

	struct	ObjNode	*ShadowNode;		// ptr to node's shadow (if any)
	

	u_short			Slot;				// sort value
	Byte			Genre;				// obj genre
	Byte			Type;				// obj type
	Byte			Group;				// obj group
	void			(*MoveCall)(struct ObjNode *);			// pointer to object's move routine
	void			(*SplineMoveCall)(struct ObjNode *);	// pointer to object's spline move routine
	void			(*CustomDrawFunction)(struct ObjNode *, TQ3ViewObject view);// pointer to object's custom draw function
	u_long			StatusBits;			// various status bits
	
	TQ3Point3D		Coord;				// coord of object
	TQ3Point3D		OldCoord;			// coord @ previous frame
	TQ3Point3D		InitCoord;			// coord where was created
	TQ3Vector3D		Delta;				// delta velocity of object
	TQ3Vector3D		Rot;				// rotation of object
	TQ3Vector2D		AccelVector;		// current acceleration vector
	float			Speed;				// length of Delta vector (not scaled to fps)
	
	TQ3Vector3D		Scale;				// scale of object
	TQ3Point2D		TargetOff;			// target offsets

	u_long			CType;				// collision type bits
	u_long			CBits;				// collision attribute bits
	Byte			NumCollisionBoxes;
	CollisionBoxType	*CollisionBoxes;// Ptr to array of collision rectangles
	short			LeftOff,RightOff,FrontOff,BackOff,TopOff,BottomOff;		// box offsets (only used by simple objects with 1 collision box)
	
	struct ObjNode	*MPlatform;			// current moving platform
		
	Byte			Kind;				// kind
	signed char		Mode;				// mode
	
	signed char		Flag[6];
	long			Special[6];
	float			SpecialF[6];
	
	float			Health;				// health 0..1
	float			Damage;				// damage
	
		
	TQ3Matrix4x4		BaseTransformMatrix;	// matrix which contains all of the transforms for the object as a whole
	TQ3TransformObject	BaseTransformObject;	// extra LEGAL object ref to BaseTransformMatrix (other legal ref is kept in BaseGroup)
	TQ3Object			BaseGroup;				// group containing all geometry,etc. for this object (for drawing)
	TQ3BoundingSphere	BoundingSphere;			// radius use for object culling calculation


	SkeletonObjDataType	*Skeleton;				// pointer to skeleton record data	
	MorphObjDataType	*Morph;					// pointer to morph record data
	
	TerrainItemEntryType *TerrainItemPtr;		// if item was from terrain, then this pts to entry in array
	SplineItemType 		*SplineItemPtr;			// if item was from spline, then this pts to entry in array
	u_char				SplineNum;				// which spline this spline item is on
	float				SplinePlacement;		// 0.0->.9999 for placement on spline
	short				SplineObjectIndex;		// index into gSplineObjectList of this ObjNode

	short				EffectChannel;			// effect sound channel index (-1 = none)
	short				ParticleGroup;
};
typedef struct ObjNode ObjNode;


		/* NEW OBJECT DEFINITION TYPE */
		
typedef struct
{
	Byte		genre,group,type,animNum;
	TQ3Point3D	coord;
	unsigned long	flags;
	short		slot;
	void		(*moveCall)(ObjNode *);
	float		rot,scale;
}NewObjectDefinitionType;


		/* PREFERENCES */
		
#define	MAX_HTTP_NOTES	1000
		
typedef struct
{
	Boolean	easyMode;
	Boolean	playerRelativeKeys;
	DateTimeRec	lastVersCheckDate;
	Byte	didThisNote[MAX_HTTP_NOTES];
	Byte	reserved[8];
}PrefsType;


#endif



