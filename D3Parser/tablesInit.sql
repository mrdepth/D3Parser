DROP TABLE IF EXISTS itemType;
CREATE TABLE itemType (
itemTypeHash INTEGER NOT NULL,
fileID INTEGER NOT NULL,
nonNlsKey  TEXT(255),
parentItemTypeHash INTEGER,
flags  INTEGER,
slot1  INTEGER,
slot2  INTEGER,
slot3  INTEGER,
slot4  INTEGER,
bitMask1 INTEGER,
bitMask2 INTEGER,
bitMask3 INTEGER,
bitMask4 INTEGER,
inherentAffixGroupHash INTEGER,
PRIMARY KEY (itemTypeHash)
);

DROP TABLE IF EXISTS item;
CREATE TABLE item (
itemHash INTEGER NOT NULL,
fileID INTEGER NOT NULL,
nonNlsKey  TEXT(255),
actorID INTEGER,
itemTypeHash INTEGER NOT NULL,
flags INTEGER NOT NULL,
itemLevel  INTEGER,
numRandomAffixes  INTEGER,
numSockets  INTEGER,
goldPrice  INTEGER,
requiredLevel  INTEGER,
durabilityMin INTEGER,
durabilityDelta INTEGER,
baseItemHash INTEGER,
itemSetBonusHash INTEGER,
salvageCommon INTEGER,
salvageMagic INTEGER,
salvageRare INTEGER,
rareGroupPrefixID INTEGER,
rareGroupSuffixID INTEGER,
damageWeaponMinMin FLOAT,
damageWeaponDeltaMin FLOAT,
armorMin FLOAT,
armorDelta FLOAT,
weaponSpeed FLOAT,
damageWeaponMinDelta FLOAT,
damageWeaponDeltaDelta FLOAT,
itemQuality INTEGER,
enchantsID INTEGER,
gemType INTEGER,
craftingMatTier INTEGER,
craftingMatRarity INTEGER,
PRIMARY KEY (itemHash)
);

DROP TABLE IF EXISTS affix;
CREATE TABLE affix (
affixHash INTEGER NOT NULL,
fileID INTEGER NOT NULL,
nonNlsKey  TEXT(255),
affixLevel  INTEGER,
superiorMask  INTEGER,
requiredLevel INTEGER,
type INTEGER,
effectTier INTEGER,
convertsTo INTEGER,
rareNamePrefixID INTEGER,
rareNameSuffixID  INTEGER,
resourceOrClass  INTEGER,
resourceTypeHash  INTEGER,
qualityMask INTEGER,
propertyType INTEGER,
primaryGroupHash INTEGER,
PRIMARY KEY (affixHash)
);

DROP TABLE IF EXISTS itemSetBonus;
CREATE TABLE itemSetBonus (
itemSetBonusHash INTEGER NOT NULL,
fileID INTEGER NOT NULL,
nonNlsKey  TEXT(255),
parentHash  INTEGER,
numOfSet  INTEGER,
PRIMARY KEY (itemSetBonusHash)
);

DROP TABLE IF EXISTS socketedEffect;
CREATE TABLE socketedEffect (
socketedEffectHash INTEGER NOT NULL,
fileID INTEGER NOT NULL,
nonNlsKey  TEXT(255),
itemHash  INTEGER,
itemTypeHash  INTEGER,
PRIMARY KEY (socketedEffectHash)
);

DROP TABLE IF EXISTS affixGroup;
CREATE TABLE affixGroup (
affixGroupHash INTEGER NOT NULL,
name  TEXT(255),
PRIMARY KEY (affixGroupHash)
);

DROP TABLE IF EXISTS affixGroupAssociation;
CREATE TABLE affixGroupAssociation (
affixHash INTEGER NOT NULL,
affixGroupHash INTEGER NOT NULL,
PRIMARY KEY (affixHash, affixGroupHash)
);

DROP TABLE IF EXISTS itemTypeAffix;
CREATE TABLE itemTypeAffix (
itemTypeHash INTEGER NOT NULL,
affixHash INTEGER NOT NULL,
PRIMARY KEY (itemTypeHash, affixHash)
);

DROP TABLE IF EXISTS legendaryItemTypeAffix;
CREATE TABLE legendaryItemTypeAffix (
itemTypeHash INTEGER NOT NULL,
affixHash INTEGER NOT NULL,
PRIMARY KEY (itemTypeHash, affixHash)
);

DROP TABLE IF EXISTS itemAffix;
CREATE TABLE itemAffix (
itemHash INTEGER NOT NULL,
affixGroupHash INTEGER NOT NULL,
affixLevel INTEGER NOT NULL,
PRIMARY KEY (itemHash, affixGroupHash)
);

DROP TABLE IF EXISTS affixModifier;
CREATE TABLE affixModifier (
affixHash INTEGER NOT NULL,
modCode INTEGER NOT NULL,
modParam1 INTEGER,
modParam2 INTEGER,
modParam3 INTEGER,
min FLOAT,
max FLOAT,
PRIMARY KEY (affixHash, modCode, modParam1)
);

DROP TABLE IF EXISTS itemModifier;
CREATE TABLE itemModifier (
itemHash INTEGER NOT NULL,
modCode INTEGER NOT NULL,
modParam1 INTEGER,
modParam2 INTEGER,
modParam3 INTEGER,
min FLOAT,
max FLOAT,
PRIMARY KEY (itemHash, modCode, modParam1)
);

DROP TABLE IF EXISTS itemSetBonusModifier;
CREATE TABLE itemSetBonusModifier (
itemSetBonusHash INTEGER NOT NULL,
modCode INTEGER NOT NULL,
modParam1 INTEGER,
modParam2 INTEGER,
modParam3 INTEGER,
min FLOAT,
max FLOAT,
PRIMARY KEY (itemSetBonusHash, modCode, modParam1)
);

DROP TABLE IF EXISTS socketedEffectModifier;
CREATE TABLE socketedEffectModifier (
socketedEffectHash INTEGER NOT NULL,
modCode INTEGER NOT NULL,
modParam1 INTEGER,
modParam2 INTEGER,
modParam3 INTEGER,
min FLOAT,
max FLOAT,
PRIMARY KEY (socketedEffectHash, modCode, modParam1)
);

DROP TABLE IF EXISTS string;
CREATE TABLE string (
stringHash INTEGER NOT NULL,
fileID INTEGER NOT NULL,
nonNlsKey  TEXT(255),
description1  TEXT(255),
description2  TEXT(255),
description3  TEXT(255),
PRIMARY KEY (stringHash, fileID)
);

DROP TABLE IF EXISTS resourceFile;
CREATE TABLE resourceFile (
fileID INTEGER NOT NULL,
resourceName  TEXT(255),
PRIMARY KEY (fileID)
);

DROP TABLE IF EXISTS attribute;
CREATE TABLE attribute (
attributeID INTEGER NOT NULL,
modCode INTEGER NOT NULL,
param INTEGER,
nonNlsKey TEXT(255) DEFAULT NULL,
name TEXT(255) DEFAULT NULL,
value1 TEXT(255) DEFAULT NULL,
value2 TEXT(255) DEFAULT NULL,
value3 TEXT(255) DEFAULT NULL,
format  TEXT(255),
PRIMARY KEY (attributeID)
);

DROP TABLE IF EXISTS rawAttribute;
CREATE TABLE rawAttribute (
rawAttributeID INTEGER NOT NULL,
nonNlsKey TEXT(255) DEFAULT NULL,
scriptA TEXT(255) DEFAULT NULL,
scriptB TEXT(255) DEFAULT NULL,
encodingType TEXT(255) DEFAULT NULL,
PRIMARY KEY (rawAttributeID)
);

--DROP TABLE IF EXISTS itemAttribute;
--CREATE TABLE itemAttribute (
--itemHash INTEGER NOT NULL,
--attributeID INTEGER NOT NULL,
--PRIMARY KEY (itemHash, attributeID)
--);

--DROP TABLE IF EXISTS affixAttribute;
--CREATE TABLE affixAttribute (
--affixHash INTEGER NOT NULL,
--attributeID INTEGER NOT NULL,
--PRIMARY KEY (affixHash, attributeID)
--);

--DROP TABLE IF EXISTS itemSetBonusAttribute;
--CREATE TABLE itemSetBonusAttribute (
--itemSetBonusHash INTEGER NOT NULL,
--attributeID INTEGER NOT NULL,
--PRIMARY KEY (itemSetBonusHash, attributeID)
--);

--DROP TABLE IF EXISTS socketedEffectAttribute;
--CREATE TABLE socketedEffectAttribute (
--socketedEffectHash INTEGER NOT NULL,
--attributeID INTEGER NOT NULL,
--PRIMARY KEY (socketedEffectHash, attributeID)
--);

.read "modCodes.sql"
.read "attributes.sql"
.read "affixes.sql"
.read "affixGroups.sql"
.read "affixGroupAssociation.sql"
--.read "affixeAttributes.sql"
.read "itemTypes.sql"
.read "itemTypeAffixes.sql"
.read "legendaryItemTypeAffixes.sql"
.read "items.sql"
.read "itemAffixes.sql"
--.read "itemAttributes.sql"
.read "strings.sql"
.read "itemModifiers.sql"
.read "affixModifiers.sql"
.read "resourceFiles.sql"
--.read "setItemAttributes.sql"
.read "setItemModifiers.sql"
.read "setItems.sql"
.read "socketedEffects.sql"
.read "socketModifiers.sql"
--.read "socketedAttributes.sql"