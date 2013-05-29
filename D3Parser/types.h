//
//  types.h
//  D3Parser
//
//  Created by Artem Shimanski on 20.09.12.
//  Copyright (c) 2012 Artem Shimanski. All rights reserved.
//

#ifndef D3Parser_types_h
#define D3Parser_types_h

//#include <stdint.h>
typedef int int32_t;
typedef long long int64_t;
typedef int32_t Hash;
typedef unsigned char Byte;

const int DataArrayEntrySize = 25;

typedef struct {
	int32_t mpqMagicNumber;
	int32_t fileTypeId;
	int32_t unused[2];
} MPQHeader;

typedef struct 	// sizeof 0x10
{
	int32_t dataOffset;
	int32_t dataNumBytes;
	int32_t unused[2];
} MPQGamDataArrayEntry;


typedef struct
{
	int32_t fileId;
	int32_t unused[2];
	int32_t recordIndex;
	char fixedString[256];
	char resourceName[256];
	int32_t unknown;
	int32_t unused2;
	MPQGamDataArrayEntry dataArrayEntry[DataArrayEntrySize];	// array of DataArrayEntry (see below)
} MPQGamHeader;

typedef struct  	// sizeof 0x140
{
	int32_t fileID;	// always 0x00000000 (probably the runtime vtbl_ptr)
	char itemTypeName[256];	// non-NLS key name of the item type
	Hash parentHash;	// hash of the parent item type
	int32_t unused2;	// always 0x00000000
	int32_t flags;	// class mask
	int32_t slot[4];	// item slot
	Hash inherentAffixHashes[3];	// always 0x00000000
	Hash inherentAffixGroupHash;	// affix group hash
	int32_t bitMask[4];	// bit mask
} MPQItemType;

typedef struct	// sizeof 0x18
{
	int32_t modCode;	// modCode (id)
	int32_t modParam1;	// param used for elemental dmg and resists
	int32_t modParam2;	// param
	int32_t modParam3;	// param
	int32_t varDataOffset;	// variable data offset (from the start of the data section)
	int32_t varDataLength;	// variable data length
} MPQModCode;

typedef struct	// sizeof 0x5F0
{
	int32_t fileID;	// always 0x00000000
	char itemName[256];	// non-NLS key name of the item
	int32_t actorId;	// Actor MPQ Id
	Hash itemTypeHash;	// hash of the ItemType
	int32_t flags;	// possibly related to Act level
	int32_t unknown3;	// ???
	int32_t iLevel;	// item level
	int32_t act;	// ???
	int32_t unknown5;	// ???
	int32_t numRandomAffixes;	// number of random affixes
	int32_t numSockets;	// number of sockets the item can have
	int32_t stackSize;	// stack size
	int32_t goldPrice;	// gold buy price (sell price is 15% of buy)
	int32_t unknown6[2];	// related to level?
	int32_t cLevel;	// character level required to use the item
	int32_t unknown15;	// related to level?
	int32_t durabilityMin;	// min durability
	int32_t durabilityMax;	// max durability (offset from min)
	int32_t baseItemHash;	// hash of the base item
	int32_t setItemBonuses;	// set item bonuses
	int32_t salvageCommon;	// salvage common TreasureClass
	int32_t salvageMagic;	// salvage magic TreasureClass
	int32_t salvageRare;	// salvage rare TreasureClass
	int32_t rareGroupPrefixId;	// MPQ Id of RareNameStrings_Prefix_*.stl file
	int32_t rareGroupSuffixId;	// MPQ Id of RareNameStrings_Suffix_*.stl file
	int32_t unknown7[26];	// ???
	float minDamage;	// weapon min damage
	float maxDamage;	// weapon max damage (offset from min)
	int32_t unknown8[21];	// ???
	float minArmor;	// min armor
	float maxArmor;	// max armor (offset from min)
	int32_t unknown9[42];	// ???
	float weaponSpeed;	// weapon speed
	int32_t unknown10[21];	// ???
	float minDamageMod;	// weapon min damage modifier
	float maxDamageMod;	// weapon max damage modifier (offset from min)
	int32_t unknown10A[26];	// ???
	int32_t unknown11;	// ???
	int32_t unknown12[19];	// ???
	MPQModCode mods[16];	// see ModCode structure below
	int32_t itemQuality;	// -1: invalid, 3: magic, 6: rare, 9: lgd.
	int32_t teaches1;	// recipe hash (in Recipes*.gam)
	int32_t teaches2;	// recipe hash (in Recipes*.gam)
	int32_t teaches3;	// recipe hash (in Recipes*.gam)
	int32_t teaches4;	// recipe hash (in Recipes*.gam)
	int32_t teaches5;	// recipe hash (in Recipes*.gam)
	int32_t teaches6;	// recipe hash (in Recipes*.gam)
	int32_t teaches7;	// recipe hash (in Recipes*.gam)
	int32_t unknown13[3];	// 0x00000000 (added in Beta 7728)
	int32_t enchants;	// recipe hash (in ItemEnhancements.gam)
	int32_t affixGroupHash[6];	// hash of the Affix Group
	int32_t affixLevel[6];	// level of the Affix
	int32_t gemType;	// gem type: 1=Amethyst, 2=Emerald, 3=Ruby, 4=Topaz
	int32_t craftingMatTier;	// crafting material tier
	int32_t craftingMatRarity;	// crafting material rarity
	int32_t unknown14[7];	// always 0x00000000
} MPQItem;

typedef struct 	// sizeof 0x290
{
	int32_t fileID;	// always 0x00000000 (probably the runtime vtbl_ptr)
	char affixName[256];	// non-NLS key name of the affix
	int32_t unknown0;	// ??? (new in Beta 7728)
	int32_t unknown1;	// ? (not unique so it's not an id)
	int32_t aLvl;	// affix level?
	int32_t supMask;	// mask for superior or socket affixes?
	int32_t unknown2;	// ? (increasing values)
	int32_t unknown3;	// ? (values range from 0 to 3F)
	int32_t requiredLevel;	// ? (cLvl of the affix?)
	int32_t unknown6;	// ? (level-related?)
	int32_t type;	// used for some affixes such as elemental dmg and class skills
	int32_t effectTier;	// used for some affixes such as MF and elemental dmg
	int32_t convertsTo;	// used for some affixes such as MF and elemental dmg
	int32_t rareNamePrefixId;	// file Id of the RareNameStrings_Prefix_*.stl file
	int32_t rareNameSuffixId;	// file Id of the RareNameStrings_Suffix_*.stl file
	Hash affixGroups[2];	// affix group hash
	int32_t resourceOrClass;	// resource/class
	Hash resourceTypeHash;	// resource type hash
	int32_t unknown5;	// hash? (only populated for Legendary Mana items)
	int32_t unused2[5];	// always 0xFFFFFFFF
	int32_t itemTypes[16];	// item group hashes which can have this affix (up to32)
	int32_t legendaryItemTypes[16];	// item group hashes which can have this affix (up to32)
	int32_t qualityMask;	// item quality mask?
	int32_t propertyType;
	Hash primaryGroupHash;	// primary group hash for secondary affixes
	int32_t socketMask;	// socket mask
	MPQModCode mods[4];	// see ModCode structure below
	int32_t unused4[18];	// always 0x00000000
} MPQAffix;

typedef struct 	// sizeof 0x28
{
	int32_t stlFileId;	// Stl file Id
	int32_t unknown1[5];	// always 0x00000000
	int32_t headerSize;	// size (in bytes) of the StlHeader? (always 0x00000028)
	int32_t entriesSize;	// size (in bytes) of the StlEntries
	int32_t unknown2[2];	// always 0x00000000
} MPQStlHeader;

typedef struct	// sizeof 0x50
{
	int32_t unknown1[2];	// always 0x00000000
	int32_t string1offset;	// file offset for string1 (non-NLS key)
	int32_t string1size;	// size of string1
	int32_t unknown2[2];	// always 0x00000000
	int32_t string2offset;	// file offset for string2
	int32_t string2size;	// size of string2
	int32_t unknown3[2];	// always 0x00000000
	int32_t string3offset;	// file offset for string3
	int32_t string3size;	// size of string3
	int32_t unknown4[2];	// always 0x00000000
	int32_t string4offset;	// file offset for string4
	int32_t string4size;	// size of string4
	int32_t unknown5;	// always 0xFFFFFFFF
	int32_t unknown6[3];	// always 0x00000000
} MPQStlEntry;

typedef struct
{
	int32_t fileID;	// always 0x00000000
	char itemSetName[256];	// non-NLS key name of the Item Set
	Hash parentHash;	// hash of the parent Item Set
	int32_t numOfSet;	// number in set to activate bonus
	int32_t unknown2;	// always 0x00000000
	MPQModCode mods[8];	// ModCodes (Attributes)
} MPQSetItemBonus;

typedef struct
{
	int32_t fileID;	// always 0x00000000
	char socketedEffectName[256];	// non-NLS key name of the Item Set
	Hash itemHash;	// hash of the parent Item Set
	Hash itemTypeHash;	// hash of the parent Item Set
	int32_t unknown;
	MPQModCode mods[5];	// ModCodes (Attributes)
	char attributeName1[256];	// non-NLS key name of the Item Set
	char attributeName2[256];	// non-NLS key name of the Item Set
	char attributeName3[256];	// non-NLS key name of the Item Set
	char attributeName4[256];	// non-NLS key name of the Item Set
//	int32_t reqAttrib[2];
//	Hash attributeID;
} MPQSocketedEffect;

typedef struct {
	int attributeID;
	int modCode;
	int param;
	std::string nonNlsKey;
	std::string name;
	std::string value1;
	std::string value2;
	std::string value3;
	std::string format;
} Attribute;

typedef struct {
	int rawAttributeID;
	std::string nonNlsKey;
	std::string scriptA;
	std::string scriptB;
	std::string encodingType;
} RawAttribute;

#endif
