#include "Core.h"

#include "UnCore.h"
#include "UnObject.h"
#include "UnPackage.h"

// #define PROFILE_PACKAGE_TABLES	1

/*-----------------------------------------------------------------------------
	Unreal package structures
-----------------------------------------------------------------------------*/

bool FPackageFileSummary::Serialize(FArchive &Ar)
{
	guard(FPackageFileSummary <<);
	assert(Ar.IsLoading); // saving is not supported

	// read package tag
	Ar << Tag;

	// some games has special tag constants
#if SPECIAL_TAGS
	if (Tag == 0x9E2A83C2)
		goto tag_ok; // Killing Floor
	if (Tag == 0x7E4A8BCA)
		goto tag_ok; // iStorm
#endif				 // SPECIAL_TAGS
#if NURIEN
	if (Tag == 0xA94E6C81)
		goto tag_ok; // Nurien
#endif
#if BATTLE_TERR
	if (Tag == 0xA1B2C93F)
	{
		Ar.Game = GAME_BattleTerr;
		goto tag_ok; // Battle Territory Online
	}
#endif // BATTLE_TERR
#if LOCO
	if (Tag == 0xD58C3147)
	{
		Ar.Game = GAME_Loco;
		goto tag_ok; // Land of Chaos Online
	}
#endif // LOCO
#if BERKANIX
	if (Tag == 0xF2BAC156)
	{
		Ar.Game = GAME_Berkanix;
		goto tag_ok;
	}
#endif // BERKANIX
#if HAWKEN
	if (Tag == 0xEA31928C)
	{
		Ar.Game = GAME_Hawken;
		goto tag_ok;
	}
#endif // HAWKEN
#if TAO_YUAN
	if (Tag == 0x12345678)
	{
		int tmp; // some additional version?
		Ar << tmp;
		Ar.Game = GAME_TaoYuan;
		if (!GForceGame)
			GForceGame = GAME_TaoYuan;
		goto tag_ok;
	}
#endif // TAO_YUAN
#if STORMWAR
	if (Tag == 0xEC201133)
	{
		byte Count;
		Ar << Count;
		Ar.Seek(Ar.Tell() + Count);
		goto tag_ok;
	}
#endif // STORMWAR
#if GUNLEGEND
	if (Tag == 0x879A4B41)
	{
		Ar.Game = GAME_GunLegend;
		if (!GForceGame)
			GForceGame = GAME_GunLegend;
		goto tag_ok;
	}
#endif // GUNLEGEND
#if MMH7
	if (Tag == 0x4D4D4837)
	{
		Ar.Game = GAME_UE3; // version conflict with Guilty Gear Xrd
		goto tag_ok;		// Might & Magic Heroes 7
	}
#endif // MMH7
#if DEVILS_THIRD
	if (Tag == 0x7BC342F0)
	{
		Ar.Game = GAME_DevilsThird;
		if (!GForceGame)
			GForceGame = GAME_DevilsThird;
		goto tag_ok; // Devil's Third
	}
#endif // DEVILS_THIRD

	// support reverse byte order
	if (Tag != PACKAGE_FILE_TAG)
	{
		if (Tag != PACKAGE_FILE_TAG_REV)
		{
			UnPackage *file = Ar.CastTo<UnPackage>();
			appNotify("Wrong package tag (%08X) in file %s. Probably the file is encrypted.", Tag, file ? *file->GetFilename() : "(unknown)");
			return false;
		}
		Ar.ReverseBytes = true;
		Tag = PACKAGE_FILE_TAG;
	}

	// read version
tag_ok:
	unsigned int Version;
	Ar << Version;

#if UNREAL4
	// UE4 has negative version value, growing from -1 towards negative direction. This value is followed
	// by "UE3 Version", "UE4 Version" and "Licensee Version" (parsed in SerializePackageFileSummary4).
	// The value is used as some version for package header, and it's not changed frequently. We can't
	// expect these values to have large values in the future. The code below checks this value for
	// being less than zero, but allows UE1-UE3 LicenseeVersion up to 32767.
	if ((Version & 0xFFFFF000) == 0xFFFFF000)
	{
		LegacyVersion = Version;
		Ar.Game = GAME_UE4_BASE;
		Serialize4(Ar);
		//!! note: UE4 requires different DetectGame way, perhaps it's not possible at all
		//!! (but can use PAK file names for game detection)
		return true;
	}
#endif // UNREAL4

#if UNREAL3
	if (Version == PACKAGE_FILE_TAG || Version == 0x20000)
		appError("Fully compressed package header?");
#endif // UNREAL3

	FileVersion = Version & 0xFFFF;
	LicenseeVersion = Version >> 16;
	// store file version to archive (required for some structures, for UNREAL3 path)
	Ar.ArVer = FileVersion;
	Ar.ArLicenseeVer = LicenseeVersion;
	// read other fields

#if UNREAL3
	if (Ar.Game >= GAME_UE3)
		Serialize3(Ar);
	else
#endif
		Serialize2(Ar);

#if DEBUG_PACKAGE
	appPrintf("EngVer:%d CookVer:%d CompF:%d CompCh:%d\n", EngineVersion, CookerVersion, CompressionFlags, CompressedChunks.Num());
	appPrintf("Names:%X[%d] Exports:%X[%d] Imports:%X[%d]\n", NameOffset, NameCount, ExportOffset, ExportCount, ImportOffset, ImportCount);
	appPrintf("HeadersSize:%X Group:%s DependsOffset:%X U60:%X\n", HeadersSize, *PackageGroup, DependsOffset, U3unk60);
#endif

	return true;

	unguardf("Ver=%d/%d", FileVersion, LicenseeVersion);
}

FArchive &operator<<(FArchive &Ar, FObjectExport &E)
{
#if UNREAL4
	if (Ar.Game >= GAME_UE4_BASE)
	{
		E.Serialize4(Ar);
		return Ar;
	}
#endif
#if UNREAL3
	if (Ar.Game >= GAME_UE3)
	{
		E.Serialize3(Ar);
		return Ar;
	}
#endif
#if UC2
	if (Ar.Engine() == GAME_UE2X)
	{
		E.Serialize2X(Ar);
		return Ar;
	}
#endif
	E.Serialize2(Ar);
	return Ar;
}

void FObjectImport::Serialize(FArchive &Ar)
{
	guard(FObjectImport::Serialize);

#if USE_COMPACT_PACKAGE_STRUCTS
	FName ClassPackage;
#endif

#if UC2
	if (Ar.Engine() == GAME_UE2X && Ar.ArVer >= 150)
	{
		int16 idx = PackageIndex;
		Ar << ClassPackage << ClassName << idx << ObjectName;
		PackageIndex = idx;
		return;
	}
#endif // UC2
#if PARIAH
	if (Ar.Game == GAME_Pariah)
		return Ar << I.PackageIndex << I.ObjectName << I.ClassPackage << I.ClassName;
#endif
#if AA2
	if (Ar.Game == GAME_AA2)
	{
		byte unk; // serialized length of ClassName string?
		Ar << ClassPackage << ClassName << unk << ObjectName << PackageIndex;
		return;
	}
#endif

	// this code is the same for all engine versions
	Ar << ClassPackage << ClassName << PackageIndex << ObjectName;

#if UNREAL4
	if (Ar.Game >= GAME_UE4_BASE && Ar.ArVer >= VER_UE4_NON_OUTER_PACKAGE_IMPORT && Ar.ContainsEditorData())
	{
		FName PackageName;
		Ar << PackageName;
	}
#endif // UNREAL4

#if MKVSDC
	if (Ar.Game == GAME_MK && Ar.ArVer >= 677)
	{
		// MK X
		FGuid unk;
		Ar << unk;
	}
#endif // MKVSDC

	unguard;
}

/*-----------------------------------------------------------------------------
	Package loading (creation) / unloading
-----------------------------------------------------------------------------*/

/*static*/ UnPackage *UnPackage::LoadPackageFromMemory(const char *Name, const std::vector<unsigned char> &Data, int Game)
{
	guard(UnPackage::LoadPackageFromMemory);

	UnPackage *package = new UnPackage();

	package->IsLoading = true;
	package->FileInfo = NULL;
	package->FilenameNoInfo = Name;

	package->Loader = new FMemReader(Data.data(), Data.size());
	package->SetupFrom(*package->Loader);

	if (Game != GAME_UNKNOWN)
	{
		package->Game = Game;
	}
	else
	{
		delete package;
		return NULL;
	}

	if (!package->Summary.Serialize(*package))
	{
		delete package;
		return NULL;
	}

	package->Loader->SetupFrom(*package);

	package->LoadNameTable();
	package->LoadImportTable();
	package->LoadExportTable();

	package->CloseReader();
	return package;

	unguardf("%s", Name);
}

bool UnPackage::VerifyName(FString &nameStr, int nameIndex)
{
	// Verify name, some Korean games (B&S) has garbage in FName (unicode?)
	bool goodName = true;
	int numBadChars = 0;
	for (char c : nameStr.GetDataArray())
	{
		if (c < ' ' || c > 0x7F)
		{
			if (c == 0)
				break; // end of line is included into FString
			// unreadable character
			goodName = false;
			break;
		}
		if (c == '$')
			numBadChars++; // unicode characters replaced with '$' in FString serializer
	}
	if (goodName && numBadChars)
	{
		int nameLen = nameStr.Len();
		if (nameLen >= 64)
			goodName = false;
		if (numBadChars >= nameLen / 2 && nameLen > 16)
			goodName = false;
	}
	if (!goodName)
	{
		// replace name
		appPrintf("WARNING: %s: fixing name %d (%s)\n", *GetFilename(), nameIndex, *nameStr);
		char buf[64];
		appSprintf(ARRAY_ARG(buf), "__name_%d__", nameIndex);
		nameStr = buf;
	}
	return goodName;
}

void UnPackage::LoadNameTable()
{
	if (Summary.NameCount == 0)
		return;

	Seek(Summary.NameOffset);
	NameTable = new const char *[Summary.NameCount];

#if UNREAL4
	if (Game >= GAME_UE4_BASE)
	{
		LoadNameTable4();
		return;
	}
#endif
#if UNREAL3
	if (Game >= GAME_UE3)
	{
		LoadNameTable3();
		return;
	}
#endif
	LoadNameTable2();
}

void UnPackage::LoadImportTable()
{
	guard(UnPackage::LoadImportTable);

	if (Summary.ImportCount == 0)
		return;

	Seek(Summary.ImportOffset);
	FObjectImport *Imp = ImportTable = new FObjectImport[Summary.ImportCount];
	for (int i = 0; i < Summary.ImportCount; i++, Imp++)
	{
		Imp->Serialize(*this);
#if DEBUG_PACKAGE
		PKG_LOG("Import[%d]: %s'%s'\n", i, *Imp->ClassName, *Imp->ObjectName);
#endif
	}

	unguard;
}

// Game-specific de-obfuscation of export tables
void PatchBnSExports(FObjectExport *Exp, const FPackageFileSummary &Summary);
void PatchDunDefExports(FObjectExport *Exp, const FPackageFileSummary &Summary);

void UnPackage::LoadExportTable()
{
	// load exports table
	guard(UnPackage::LoadExportTable);

	if (Summary.ExportCount == 0)
		return;

	Seek(Summary.ExportOffset);
	FObjectExport *Exp = ExportTable = new FObjectExport[Summary.ExportCount];
	memset(ExportTable, 0, sizeof(FObjectExport) * Summary.ExportCount);
	for (int i = 0; i < Summary.ExportCount; i++, Exp++)
	{
		*this << *Exp;
	}

#if BLADENSOUL
	if (Game == GAME_BladeNSoul && (Summary.PackageFlags & 0x08000000))
		PatchBnSExports(ExportTable, Summary);
#endif
#if DUNDEF
	if (Game == GAME_DunDef)
		PatchDunDefExports(ExportTable, Summary);
#endif

#if DEBUG_PACKAGE
	Exp = ExportTable;
	for (int i = 0; i < Summary.ExportCount; i++, Exp++)
	{
		//		USE_COMPACT_PACKAGE_STRUCTS - makes impossible to dump full information
		//		Perhaps add full support to extract.exe?
		//		PKG_LOG("Export[%d]: %s'%s' offs=%08X size=%08X parent=%d flags=%08X:%08X, exp_f=%08X arch=%d\n", i, GetClassNameFor(*Exp),
		//			*Exp->ObjectName, Exp->SerialOffset, Exp->SerialSize, Exp->PackageIndex, Exp->ObjectFlags2, Exp->ObjectFlags, Exp->ExportFlags, Exp->Archetype);
		PKG_LOG("Export[%d]: %s'%s' offs=%08X size=%08X parent=%d  exp_f=%08X\n", i, GetClassNameFor(*Exp),
				*Exp->ObjectName, Exp->SerialOffset, Exp->SerialSize, Exp->PackageIndex, Exp->ExportFlags);
	}
#endif // DEBUG_PACKAGE

	unguard;
}

UnPackage::~UnPackage()
{
	guard(UnPackage::~UnPackage);

	if (Loader)
		delete Loader;

	if (!IsValid())
	{
		// The package wasn't loaded, nothing to release in destructor. Also it is possible that
		// it has zero names/imports/exports (happens with some UE3 games).
		return;
	}

	// free tables
	delete[] NameTable;
	delete[] ImportTable;
	delete[] ExportTable;
#if UNREAL4
	delete[] ExportIndices_IOS;
#endif

	unguard;
}

#if 0
// Commented, not used
// Find file archive inside a package loader
static FFileArchive* FindFileArchive(FArchive* Ar)
{
#if UNREAL3
	FUE3ArchiveReader* ArUE3 = Ar->CastTo<FUE3ArchiveReader>();
	if (ArUE3) Ar = ArUE3->Reader;
#endif

	FReaderWrapper* ArWrap = Ar->CastTo<FReaderWrapper>();
	if (ArWrap) Ar = ArWrap->Reader;

	return Ar->CastTo<FFileArchive>();
}
#endif

static TArray<UnPackage *> OpenReaders;

void UnPackage::SetupReader(int ExportIndex)
{
	guard(UnPackage::SetupReader);
	// open loader if it is closed
	if (!IsOpen())
	{
		Open();
		if (OpenReaders.Num() == 0)
		{
			OpenReaders.Empty(64);
		}
		OpenReaders.AddUnique(this);
	}
	// setup for object
	const FObjectExport &Exp = GetExport(ExportIndex);

#if UNREAL4
	if (Exp.RealSerialOffset)
	{
		FReaderWrapper *Wrapper = Loader->CastTo<FReaderWrapper>();
		Wrapper->ArPosOffset = Exp.RealSerialOffset - Exp.SerialOffset;
	}
#endif // UNREAL4

	SetStopper(Exp.SerialOffset + Exp.SerialSize);
	//	appPrintf("Setup for %s: %d + %d -> %d\n", *Exp.ObjectName, Exp.SerialOffset, Exp.SerialSize, Exp.SerialOffset + Exp.SerialSize);
	Seek(Exp.SerialOffset);
	unguard;
}

void UnPackage::CloseReader()
{
	guard(UnPackage::CloseReader);
#if 0
	FFileArchive* File = FindFileArchive(Loader);
	assert(File);
	if (File->IsOpen()) File->Close();
#else
	Loader->Close();
#endif
	unguardf("pkg=%s", *GetFilename());
}

void UnPackage::CloseAllReaders()
{
	guard(UnPackage::CloseAllReaders);

	for (UnPackage *p : OpenReaders)
	{
		p->CloseReader();
	}
	OpenReaders.Empty();

	unguard;
}

/*-----------------------------------------------------------------------------
	UObject* and FName serializers
-----------------------------------------------------------------------------*/

FArchive &UnPackage::operator<<(FName &N)
{
	guard(UnPackage::SerializeFName);

	assert(IsLoading);

	// Declare aliases for FName.Index and ExtraIndex to allow USE_COMPACT_PACKAGE_STRUCTS to work
#if !USE_COMPACT_PACKAGE_STRUCTS
	int32 &N_Index = N.Index;
#if UNREAL3 || UNREAL4
	int32 &N_ExtraIndex = N.ExtraIndex;
#endif
#else
	int32 N_Index = 0, N_ExtraIndex = 0;
#endif // USE_COMPACT_PACKAGE_STRUCTS

#if BIOSHOCK
	if (Game == GAME_Bioshock)
	{
		*this << AR_INDEX(N_Index) << N_ExtraIndex;
		if (N_ExtraIndex == 0)
		{
			N.Str = GetName(N_Index);
		}
		else
		{
			N.Str = appStrdupPool(va("%s%d", GetName(N_Index), N_ExtraIndex - 1)); // without "_" char
		}
		return *this;
	}
#endif // BIOSHOCK

#if UC2
	if (Engine() == GAME_UE2X && ArVer >= 145)
	{
		*this << N_Index;
	}
	else
#endif // UC2
#if LEAD
		if (Game == GAME_SplinterCellConv && ArVer >= 64)
	{
		*this << N_Index;
	}
	else
#endif // LEAD
#if UNREAL3 || UNREAL4
		if (Engine() >= GAME_UE3)
	{
		*this << N_Index;
		if (Game >= GAME_UE4_BASE)
			goto extra_index;
#if R6VEGAS
		if (Game == GAME_R6Vegas2)
		{
			N_ExtraIndex = N_Index >> 19;
			N_Index &= 0x7FFFF;
		}
#endif // R6VEGAS
		if (ArVer >= 343)
		{
		extra_index:
			*this << N_ExtraIndex;
		}
	}
	else
#endif // UNREAL3 || UNREAL4
	{
		// UE1 and UE2
		*this << AR_INDEX(N_Index);
	}

	// Convert name index to string
#if UNREAL3 || UNREAL4
	if (N_ExtraIndex == 0)
	{
		N.Str = GetName(N_Index);
	}
	else
	{
		N.Str = appStrdupPool(va("%s_%d", GetName(N_Index), N_ExtraIndex - 1));
	}
#else
	// no modern engines compiled
	N.Str = GetName(N_Index);
#endif // UNREAL3 || UNREAL4

	return *this;

	unguardf("pos=%08X", Tell());
}

FArchive &UnPackage::operator<<(UObject *&Obj)
{
	guard(UnPackage::SerializeUObject);

	assert(IsLoading);
	int index;
#if UC2
	if (Engine() == GAME_UE2X && ArVer >= 145)
		*this << index;
	else
#endif
#if UNREAL3 || UNREAL4
		if (Engine() >= GAME_UE3)
		*this << index;
	else
#endif // UNREAL3 || UNREAL4
		*this << AR_INDEX(index);

	if (index < 0)
	{
		//		const FObjectImport &Imp = GetImport(-index-1);
		//		appPrintf("PKG: Import[%s,%d] OBJ=%s CLS=%s\n", GetObjectName(Imp.PackageIndex), index, *Imp.ObjectName, *Imp.ClassName);
		Obj = CreateImport(-index - 1);
	}
	else if (index > 0)
	{
		//		const FObjectExport &Exp = GetExport(index-1);
		//		appPrintf("PKG: Export[%d] OBJ=%s CLS=%s\n", index, *Exp.ObjectName, GetClassNameFor(Exp));
		Obj = CreateExport(index - 1);
	}
	else // index == 0
	{
		Obj = NULL;
	}
	return *this;

	unguard;
}

/*-----------------------------------------------------------------------------
	Loading particular import or export package entry
-----------------------------------------------------------------------------*/

int UnPackage::FindExport(const char *name, const char *className, int firstIndex) const
{
	guard(UnPackage::FindExport);

	FastNameComparer cmp(name);
	for (int i = firstIndex; i < Summary.ExportCount; i++)
	{
		const FObjectExport &Exp = ExportTable[i];
		// compare object name
		if (cmp(Exp.ObjectName))
		{
			// if class name specified - compare it too
			const char *foundClassName = GetClassNameFor(Exp);
			if (className && stricmp(foundClassName, className) != 0)
				continue;
			return i;
		}
	}
	return INDEX_NONE;

	unguard;
}

bool UnPackage::CompareObjectPaths(int PackageIndex, UnPackage *RefPackage, int RefPackageIndex) const
{
	guard(UnPackage::CompareObjectPaths);

	/*	appPrintf("Compare %s.%s [%d] with %s.%s [%d]\n",
			Name, GetObjectName(PackageIndex), PackageIndex,
			RefPackage->Name, RefPackage->GetObjectName(RefPackageIndex), RefPackageIndex
		); */

	while (PackageIndex || RefPackageIndex)
	{
		const char *PackageName, *RefPackageName;

		if (PackageIndex < 0)
		{
			const FObjectImport &Rec = GetImport(-PackageIndex - 1);
			PackageIndex = Rec.PackageIndex;
			PackageName = Rec.ObjectName;
		}
		else if (PackageIndex > 0)
		{
			// possible for UE3 forced exports
			const FObjectExport &Rec = GetExport(PackageIndex - 1);
			PackageIndex = Rec.PackageIndex;
			PackageName = Rec.ObjectName;
		}
		else
			PackageName = Name;

		if (RefPackageIndex < 0)
		{
			const FObjectImport &Rec = RefPackage->GetImport(-RefPackageIndex - 1);
			RefPackageIndex = Rec.PackageIndex;
			RefPackageName = Rec.ObjectName;
		}
		else if (RefPackageIndex > 0)
		{
			// possible for UE3 forced exports
			const FObjectExport &Rec = RefPackage->GetExport(RefPackageIndex - 1);
			RefPackageIndex = Rec.PackageIndex;
			RefPackageName = Rec.ObjectName;
		}
		else
			RefPackageName = RefPackage->Name;
		//		appPrintf("%20s -- %20s\n", PackageName, RefPackageName);
		if (stricmp(RefPackageName, PackageName) != 0)
			return false;
	}

	return true;

	unguard;
}

int UnPackage::FindExportForImport(const char *ObjectName, const char *ClassName, UnPackage *ImporterPackage, int ImporterIndex)
{
	guard(FindExportForImport);

	int ObjIndex = -1;
	while (true)
	{
		// iterate all objects with the same name and class
		ObjIndex = FindExport(ObjectName, ClassName, ObjIndex + 1);
		if (ObjIndex == INDEX_NONE)
			break; // not found
#if UNREAL4
		if (Game >= GAME_UE4_BASE)
		{
			// UE4 usually has single object in package. Plus, each object import has a parent UPackage
			// describing where to get an object, but object export has no parent UObject - so depth
			// of import path is 1 step deeper than depth of export path, and CompareObjectPaths()
			// will always fail.
			return ObjIndex;
		}
#endif // UNREAL4
	   // a few objects in package could have the same name and class but resides in different groups,
	   // so compare full object paths for sure
		if (CompareObjectPaths(ObjIndex + 1, ImporterPackage, -1 - ImporterIndex))
			return ObjIndex; // found
	}

	return INDEX_NONE; // not found

	unguard;
}

UObject *UnPackage::CreateExport(int index)
{
	guard(UnPackage::CreateExport);

	// Get previously created object if any
	FObjectExport &Exp = GetExport(index);
	if (Exp.Object)
		return Exp.Object;

	// Check if this object just contains default properties
	bool shouldSkipObject = false;

	if (!strnicmp(Exp.ObjectName, "Default__", 9))
	{
		// Default properties are not supported -- this is a clean UObject format
		shouldSkipObject = true;
	}
#if UNREAL4
	if (Game >= GAME_UE4_BASE)
	{
		// This could be a blueprint - it contains objects which are marked as 'StaticMesh' class,
		// but really containing nothing. Such objects are always contained inside some Default__... parent.
		const char *OuterName = GetObjectPackageName(Exp.PackageIndex);
		if (OuterName && !strnicmp(OuterName, "Default__", 9))
		{
			shouldSkipObject = true;
		}
	}
#endif // UNREAL4

	if (shouldSkipObject)
	{
		return NULL;
	}

	// Create empty object of desired class
	const char *ClassName = GetClassNameFor(Exp);
	UObject *Obj = Exp.Object = CreateClass(ClassName);
	if (!Obj)
	{
		if (!IsSuppressedClass(ClassName))
		{
			appPrintf("WARNING: Unknown class \"%s\" for object \"%s\"\n", ClassName, *Exp.ObjectName);
		}
#if MAX_DEBUG
		else
		{
			appPrintf("SUPPRESSED: %s\n", ClassName);
		}
#endif
		return NULL;
	}

	// Setup constant object fields
	Obj->Package = this;
	Obj->PackageIndex = index;
	Obj->Outer = NULL;
	Obj->Name = Exp.ObjectName;

	bool bLoad = true;
	if (GBeforeLoadObjectCallback)
		bLoad = GBeforeLoadObjectCallback(Obj);

	if (bLoad)
	{
		// Block UObject serialization
		UObject::BeginLoad();

		// Find and try to create outer object
		UObject *Outer = NULL;
		if (Exp.PackageIndex)
		{
			const FObjectExport &OuterExp = GetExport(Exp.PackageIndex - 1);
			Outer = OuterExp.Object;
			if (!Outer)
			{
				const char *OuterClassName = GetClassNameFor(OuterExp);
				if (IsKnownClass(OuterClassName)) // avoid error message if class name is not registered
					Outer = CreateExport(Exp.PackageIndex - 1);
			}
		}
		Obj->Outer = Outer;

		// Add object to GObjLoaded for later serialization
		UObject::GObjLoaded.Add(Obj);

		// Perform serialization
		UObject::EndLoad();
	}

	return Obj;

	unguardf("%s:%d", *GetFilename(), index);
}

UObject *UnPackage::CreateImport(int index)
{
	guard(UnPackage::CreateImport);

	FObjectImport &Imp = GetImport(index);
	if (Imp.Missing)
		return NULL;

	// load package
	const char *PackageName = GetObjectPackageName(Imp.PackageIndex);
#if UNREAL4
	if (!PackageName)
	{
		// This could happen with UE4 IoStore structures, if PackageId was not found
		return NULL;
	}
#endif

	// Since we don't support loading from files anymore, imports will always be missing
	Imp.Missing = true;
	return NULL;

	unguardf("%s:%d", *GetFilename(), index);
}

// get outermost package name
//?? this function is not correct, it is used in package exporter tool only
const char *UnPackage::GetObjectPackageName(int PackageIndex) const
{
	guard(UnPackage::GetObjectPackageName);

	const char *PackageName = NULL;
	while (PackageIndex)
	{
		if (PackageIndex < 0)
		{
			const FObjectImport &Rec = GetImport(-PackageIndex - 1);
			PackageIndex = Rec.PackageIndex;
			PackageName = Rec.ObjectName;
		}
		else
		{
			// possible for UE3 forced exports
			const FObjectExport &Rec = GetExport(PackageIndex - 1);
			PackageIndex = Rec.PackageIndex;
			PackageName = Rec.ObjectName;
		}
	}
	return PackageName;

	unguard;
}

void UnPackage::GetFullExportName(const FObjectExport &Exp, char *buf, int bufSize, bool IncludeObjectName, bool IncludeCookedPackageName) const
{
	guard(UnPackage::GetFullExportNameBase);

	const char *PackageNames[256];
	const char *PackageName;
	int NestLevel = 0;

	// get object name
	if (IncludeObjectName)
	{
		PackageName = Exp.ObjectName;
		PackageNames[NestLevel++] = PackageName;
	}

	// gather nested package names (object parents)
	int PackageIndex = Exp.PackageIndex;
	while (PackageIndex)
	{
		assert(NestLevel < ARRAY_COUNT(PackageNames));
		if (PackageIndex < 0)
		{
			const FObjectImport &Rec = GetImport(-PackageIndex - 1);
			PackageIndex = Rec.PackageIndex;
			PackageName = Rec.ObjectName;
		}
		else
		{
			const FObjectExport &Rec = GetExport(PackageIndex - 1);
			PackageIndex = Rec.PackageIndex;
			PackageName = Rec.ObjectName;
#if UNREAL3
			if (PackageIndex == 0 && (Rec.ExportFlags && EF_ForcedExport) && !IncludeCookedPackageName)
				break; // do not add cooked package name
#endif
		}
		PackageNames[NestLevel++] = PackageName;
	}
	// concatenate package names in reverse order (from root to object)
	*buf = 0;
	for (int i = NestLevel - 1; i >= 0; i--)
	{
		const char *PackageName = PackageNames[i];
		char *dst = strchr(buf, 0);
		appSprintf(dst, bufSize - (dst - buf), "%s%s", PackageName, i > 0 ? "." : "");
	}

	unguard;
}

const char *UnPackage::GetUncookedPackageName(int PackageIndex) const
{
	guard(UnPackage::GetUncookedPackageName);

#if UNREAL3
	if (PackageIndex != INDEX_NONE)
	{
		const FObjectExport &Exp = GetExport(PackageIndex);
		if (Game >= GAME_UE3 && (Exp.ExportFlags & EF_ForcedExport))
		{
			// find outermost package
			while (true)
			{
				const FObjectExport &Exp2 = GetExport(PackageIndex);
				if (!Exp2.PackageIndex)
					break;							  // get parent (UPackage)
				PackageIndex = Exp2.PackageIndex - 1; // subtract 1 from package index
			}
			const FObjectExport &Exp2 = GetExport(PackageIndex);
			return *Exp2.ObjectName;
		}
	}
#endif // UNREAL3
	return Name;

	unguard;
}
