//
//  main.cpp
//  D3Parser
//
//  Created by Artem Shimanski on 19.09.12.
//  Copyright (c) 2012 Artem Shimanski. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <stdio.h>

#include "types.h"
#include <map>
#include <string>
#include <vector>
#include <list>

#ifdef WIN32
#include <io.h>
#else
#include <dirent.h>
#endif

#include <unordered_set>
#include "tinyxml.h"

class Range {
public:
	Range operator+(Range& other) {
		Range result = *this;
		result.min += other.min;
		result.max += other.max;
		return result;
	}
	
	Range operator-(Range& other) {
		Range result = *this;
		result.min -= other.min;
		result.max -= other.max;
		return result;
	}
	
	Range operator*(Range& other) {
		Range result = *this;
		result.min *= other.min;
		result.max *= other.max;
		return result;
	}
	
	Range operator/(Range& other) {
		Range result = *this;
		result.min /= other.min;
		result.max /= other.max;
		return result;
	}
	
	float min;
	float max;
	
};

typedef int FileID;
std::map<FileID, std::string> filesMap;

std::vector<MPQItemType> itemTypes;
std::vector<MPQItem> items;
std::vector<MPQAffix> affixes;
std::vector<MPQSetItemBonus> setItemBonuses;
std::vector<MPQSocketedEffect> socketedEffects;

typedef int64_t ModDataKey;
std::map<ModDataKey, Range> modData;

std::map<Hash, std::string> affixGroups;

typedef std::map<Hash, std::vector<std::string> > StringsHashMap;
//std::map<Hash, std::vector<std::string> > stringsHashMap;
std::map<int, StringsHashMap > stringsHashMap;

std::map<int, RawAttribute> attributessMap;
//std::unordered_set<int> usedModCodesMap;

std::map<int, Attribute> attributesMap;

std::map<Hash, std::vector<int> > itemAttributes;
std::map<Hash, std::vector<int> > affixAttributes;
std::map<Hash, std::vector<int> > setAttributes;
std::map<Hash, std::vector<int> > socketAttributes;

Hash hash(const char* string) {
	Hash hash = 0;
	size_t n = strlen(string);
	for (int i = 0; i < n; i++)
		hash = (hash * 0x21) + tolower(string[i]);
	return hash;
}

bool isValidHash(Hash h) {
	return h != 0 && h != -1;
}

Range modDataResult(void* modData, int size) {
	Byte* p = static_cast<Byte*>(modData);
	Byte* end = p + size;
	std::vector<Range> stack;
	std::string outputString;
	while (p < end) {
		switch (*p) {
			case 0x00:
				p = end;
				break;
			case 0x01:
				p += 4;
				switch (*p) {
					case 0x03:
					case 0x04: {
						assert(stack.size() > 1);
						Range b = stack.back();
						stack.pop_back();
						Range a = stack.back();
						stack.pop_back();
						Range c;
						c.min = a.min;
						c.max = (*p) == 0x03 ? a.min + b.min : b.min;
						stack.push_back(c);
						break;
					}
				}
				break;
			case 0x06:
				p += 4;
				Range range;
				range.min = *reinterpret_cast<float*>(p);
				range.max = range.min;
				stack.push_back(range);
				break;
			case 0x0b:
			case 0x0c:
			case 0x0d:
			case 0x0e:
			{
				assert(stack.size() > 1);
				Range b = stack.back();
				stack.pop_back();
				Range a = stack.back();
				stack.pop_back();
				Range c;
				if (*p == 0x0b)
					c = a + b;
				else if (*p == 0x0c)
					c = a - b;
				else if (*p == 0x0d)
					c = a * b;
				else if (*p == 0x0e)
					c = a / b;
				stack.push_back(c);
				break;
			}
		}
		p += 4;
	}
	assert(stack.size() == 1);
	Range range = stack.back();
	return range;
}

Byte* fileContents(const char* fileName, size_t* fileSize) {
	FILE* f = fopen(fileName, "rb");
	if (!f)
		return NULL;
	
	fseek(f, 0, SEEK_END);
	size_t size = ftell(f);
	fseek(f, 0, SEEK_SET);
	if (size == 0)
		return NULL;
	
	Byte* memory = new Byte[size];
	fread(memory, size, 1, f);
	fclose(f);
	
	if (fileSize)
		*fileSize = size;
	return memory;
}

void processItemTypes(MPQGamHeader* gamHeader) {
	for (int i = 0; i < DataArrayEntrySize; i++) {
		if (gamHeader->dataArrayEntry[i].dataOffset > 0) {
			MPQGamDataArrayEntry& entry = gamHeader->dataArrayEntry[i];
			int count = entry.dataNumBytes / sizeof(MPQItemType);
			MPQItemType* pItemType = reinterpret_cast<MPQItemType*>(reinterpret_cast<Byte*>(gamHeader) + entry.dataOffset);
			for (int j = 0; j < count; j++) {
				itemTypes.push_back(*pItemType);
				pItemType += 1;
			}
		}
	}
}

void processItems(MPQGamHeader* gamHeader) {
	for (int i = 0; i < DataArrayEntrySize; i++) {
		if (gamHeader->dataArrayEntry[i].dataOffset > 0) {
			MPQGamDataArrayEntry& entry = gamHeader->dataArrayEntry[i];
			int count = entry.dataNumBytes / sizeof(MPQItem);
			MPQItem* pItem = reinterpret_cast<MPQItem*>(reinterpret_cast<Byte*>(gamHeader) + entry.dataOffset);
			for (int j = 0; j < count; j++) {
				pItem->fileID = gamHeader->fileId;
				items.push_back(*pItem);
				for (int k = 0; k < 16; k++) {
					MPQModCode& modCode = pItem->mods[k];
					if (modCode.modCode > 0 && modCode.varDataOffset > 0 && modCode.varDataLength > 0) {
						Byte* pModCodeData = reinterpret_cast<Byte*>(reinterpret_cast<char*>(gamHeader) + modCode.varDataOffset);
						Range r = modDataResult(pModCodeData, modCode.varDataLength);
						
						ModDataKey key = ((ModDataKey)gamHeader->fileId) << 32 | modCode.varDataOffset;
						modData[key] = r;
						//usedModCodesMap.insert(modCode.modCode);
						
						int id = modCode.modCode;
						std::map<int, Attribute>::iterator i, j = attributesMap.end();
						i = attributesMap.find(id);
						if (modCode.modParam1 != -1) {
							id <<= 22;
							id |= modCode.modParam1;
							j = attributesMap.find(id);
						}
						if (j != attributesMap.end())
							i = j;
						if (i != attributesMap.end()) {
							itemAttributes[hash(pItem->itemName)].push_back(i->first);
						}
						else {
							//assert(0);
							printf("%d, %d\t", modCode.modCode, modCode.modParam1);
						}
					}
				}
				//pItem = (MPQItem*) ((char*) pItem + 1488);
				pItem++;
			}
		}
	}
}

void processAffixes(MPQGamHeader* gamHeader) {
	for (int i = 0; i < DataArrayEntrySize; i++) {
		if (gamHeader->dataArrayEntry[i].dataOffset > 0) {
			MPQGamDataArrayEntry& entry = gamHeader->dataArrayEntry[i];
			int count = entry.dataNumBytes / sizeof(MPQAffix);
			MPQAffix* pAffix = reinterpret_cast<MPQAffix*>(reinterpret_cast<Byte*>(gamHeader) + entry.dataOffset);
			for (int j = 0; j < count; j++) {
				pAffix->fileID = gamHeader->fileId;
				affixes.push_back(*pAffix);
				for (int k = 0; k < 4; k++) {
					MPQModCode& modCode = pAffix->mods[k];
					if (modCode.modCode > 0 && modCode.varDataOffset > 0 && modCode.varDataLength > 0) {
						Byte* pModCodeData = reinterpret_cast<Byte*>(reinterpret_cast<char*>(gamHeader) + modCode.varDataOffset);
						Range r = modDataResult(pModCodeData, modCode.varDataLength);
						
						ModDataKey key = ((ModDataKey)gamHeader->fileId) << 32 | modCode.varDataOffset;
						modData[key] = r;
						//usedModCodesMap.insert(modCode.modCode);

						int id = modCode.modCode;
						std::map<int, Attribute>::iterator i, j = attributesMap.end();
						i = attributesMap.find(id);
						if (modCode.modParam1 != -1) {
							id <<= 22;
							id |= modCode.modParam1;
							j = attributesMap.find(id);
						}
						if (j != attributesMap.end())
							i = j;
						if (i != attributesMap.end()) {
							affixAttributes[hash(pAffix->affixName)].push_back(i->first);
						}
						else {
//							assert(0);
							printf("%d\t", modCode.modCode);
						}
					}
				}
				
				for (int k = 0; k < 2; k++) {
					if (isValidHash(pAffix->affixGroups[k])) {
						if (affixGroups.find(pAffix->affixGroups[k]) == affixGroups.end())
							affixGroups[pAffix->affixGroups[k]] = pAffix->affixName;
						else {
							std::string s = affixGroups[pAffix->affixGroups[k]];
							int i = 0;
							for (; i < 256;i++) {
								if (s[i] == '0' || pAffix->affixName[i] == 0 || s[i] != pAffix->affixName[i])
									break;
							}
							i--;
							while (i > 0) {
								if (pAffix->affixName[i] == ' ' || pAffix->affixName[i] == '_')
									i--;
								else
									break;
							}
							affixGroups[pAffix->affixGroups[k]] = s.substr(0, i + 1);
						}
					}
				}
				
				pAffix += 1;
				//pAffix = (MPQAffix*) (((char*) pAffix) + 544);
			}
		}
	}
}

void processSetBonuses(MPQGamHeader* gamHeader) {
	for (int i = 0; i < DataArrayEntrySize; i++) {
		if (gamHeader->dataArrayEntry[i].dataOffset > 0) {
			MPQGamDataArrayEntry& entry = gamHeader->dataArrayEntry[i];
			int count = entry.dataNumBytes / sizeof(MPQSetItemBonus);
			MPQSetItemBonus* pBonus = reinterpret_cast<MPQSetItemBonus*>(reinterpret_cast<Byte*>(gamHeader) + entry.dataOffset);
			for (int j = 0; j < count; j++) {
				pBonus->fileID = gamHeader->fileId;
				setItemBonuses.push_back(*pBonus);
				for (int k = 0; k < 8; k++) {
					MPQModCode& modCode = pBonus->mods[k];
					if (modCode.modCode > 0 && modCode.varDataOffset > 0 && modCode.varDataLength > 0) {
						Byte* pModCodeData = reinterpret_cast<Byte*>(reinterpret_cast<char*>(gamHeader) + modCode.varDataOffset);
						Range r = modDataResult(pModCodeData, modCode.varDataLength);
						
						ModDataKey key = ((ModDataKey)gamHeader->fileId) << 32 | modCode.varDataOffset;
						modData[key] = r;
						//usedModCodesMap.insert(modCode.modCode);
						
						int id = modCode.modCode;
						std::map<int, Attribute>::iterator i, j = attributesMap.end();
						i = attributesMap.find(id);
						if (modCode.modParam1 != -1) {
							id <<= 22;
							id |= modCode.modParam1;
							j = attributesMap.find(id);
						}
						if (j != attributesMap.end())
							i = j;
						if (i != attributesMap.end()) {
							setAttributes[hash(pBonus->itemSetName)].push_back(i->first);
						}
						else {
							//assert(0);
							printf("%d\t", modCode.modCode);
						}
					}
				}
				pBonus += 1;
			}
		}
	}
}

void processSocketedEffects(MPQGamHeader* gamHeader) {
	for (int i = 0; i < DataArrayEntrySize; i++) {
		if (gamHeader->dataArrayEntry[i].dataOffset > 0) {
			MPQGamDataArrayEntry& entry = gamHeader->dataArrayEntry[i];
			int count = entry.dataNumBytes / sizeof(MPQSocketedEffect);
			MPQSocketedEffect* pEffect = reinterpret_cast<MPQSocketedEffect*>(reinterpret_cast<Byte*>(gamHeader) + entry.dataOffset);
			for (int j = 0; j < count; j++) {
				pEffect->fileID = gamHeader->fileId;
				socketedEffects.push_back(*pEffect);
				for (int k = 0; k < 5; k++) {
					MPQModCode& modCode = pEffect->mods[k];
					if (modCode.modCode > 0 && modCode.varDataOffset > 0 && modCode.varDataLength > 0) {
						Byte* pModCodeData = reinterpret_cast<Byte*>(reinterpret_cast<char*>(gamHeader) + modCode.varDataOffset);
						Range r = modDataResult(pModCodeData, modCode.varDataLength);
						
						ModDataKey key = ((ModDataKey)gamHeader->fileId) << 32 | modCode.varDataOffset;
						modData[key] = r;
						//usedModCodesMap.insert(modCode.modCode);
						
						int id = modCode.modCode;
						std::map<int, Attribute>::iterator i, j = attributesMap.end();
						i = attributesMap.find(id);
						if (modCode.modParam1 != -1) {
							id <<= 22;
							id |= modCode.modParam1;
							j = attributesMap.find(id);
						}
						if (j != attributesMap.end())
							i = j;
						if (i != attributesMap.end()) {
							socketAttributes[hash(pEffect->socketedEffectName)].push_back(i->first);
						}
						else {
							//assert(0);
							printf("%d\t", modCode.modCode);
						}
					}
				}
				pEffect += 1;
			}
		}
	}
}


void processGamFile(MPQGamHeader* gamHeader) {
	static const int32_t ItemTypes = hash(".\\Resources\\Excel\\ItemTypes.xls");
	static const int32_t ItemsArmor = hash(".\\Resources\\Excel\\Items_Armor.xls");
	static const int32_t ItemsLegendaryOther = hash(".\\Resources\\Excel\\Items_Legendary_Other.xls");
	static const int32_t ItemsLegendaryWeapon = hash(".\\Resources\\Excel\\Items_Legendary_Weapons.xls");
	static const int32_t ItemsLegendary = hash(".\\Resources\\Excel\\Items_Legendary.xlsx");
	static const int32_t ItemsOther = hash(".\\Resources\\Excel\\Items_Other.xls");
	static const int32_t ItemsQuestsBeta = hash(".\\Resources\\Excel\\Items_Quests_Beta.xls");
	static const int32_t ItemsQuests = hash(".\\Resources\\Excel\\Items_Quests.xls");
	static const int32_t ItemsWeapons = hash(".\\Resources\\Excel\\Items_Weapons.xls");
	static const int32_t AffixList = hash(".\\Resources\\Excel\\AffixList.xls");
	static const int32_t SetItemBonuses = hash(".\\Resources\\Excel\\SetItemBonuses.xls");
	static const int32_t SocketedEffects = hash(".\\Resources\\Excel\\SocketedEffects.xls");
	
	int32_t resourceType = hash(gamHeader->resourceName);
	
	if (resourceType == ItemTypes)
		processItemTypes(gamHeader);
	else if (resourceType == ItemsArmor ||
			 resourceType == ItemsLegendaryOther ||
			 resourceType == ItemsLegendaryWeapon ||
			 resourceType == ItemsLegendary ||
			 resourceType == ItemsOther ||
			 resourceType == ItemsQuestsBeta ||
			 resourceType == ItemsQuests ||
			 resourceType == ItemsWeapons)
		processItems(gamHeader);
	else if (resourceType == AffixList)
		processAffixes(gamHeader);
	else if (resourceType == SetItemBonuses)
		processSetBonuses(gamHeader);
	else if (resourceType == SocketedEffects)
		processSocketedEffects(gamHeader);
	else
		std::cout << "Unknown resource " << gamHeader->resourceName << std::endl;
}

void processStlFile(MPQStlHeader* stlHeader) {
	int count = stlHeader->entriesSize / sizeof(MPQStlEntry);
	MPQStlEntry* pStlEntry = reinterpret_cast<MPQStlEntry*>(stlHeader + 1);
	for (int i = 0; i < count; i++) {
		if (i == 48) {
			i = i;
		}
		std::vector<std::string> strings;
		strings.reserve(4);
		int32_t offsets[] = {pStlEntry->string1offset, pStlEntry->string2offset, pStlEntry->string3offset, pStlEntry->string4offset};
		int32_t sizes[] = {pStlEntry->string1size, pStlEntry->string2size, pStlEntry->string3size, pStlEntry->string4size};
		std::string key;
		for (int i = 0; i < 4; i++) {
			if (offsets[i] > 0 && sizes[i] > 1) {
				const char* s = reinterpret_cast<char*>(stlHeader) + offsets[i];
				strings.push_back(s);
				if (key.length() == 0)
					key = s;
			}
			else
				strings.push_back("");
		}
		Hash h = hash(key.c_str());
		stringsHashMap[stlHeader->stlFileId][h] = strings;
		pStlEntry += 1;
	}
}

void processFile(const std::string& fileName) {
	size_t size = 0;
	Byte* contents = fileContents(fileName.c_str(), &size);
	if (!contents)
		return;
	
	MPQHeader* mpqHeader = (MPQHeader*) contents;
	std::string extension = fileName.substr(fileName.find_last_of(".") + 1);
	if (extension == "gam") {
		MPQGamHeader* gamHeader = reinterpret_cast<MPQGamHeader*>(mpqHeader + 1);
		processGamFile(gamHeader);
		filesMap[gamHeader->fileId] = fileName;
	}
	else if (extension == "stl") {
		MPQStlHeader* stlHeader = reinterpret_cast<MPQStlHeader*>(mpqHeader + 1);
		processStlFile(stlHeader);
		filesMap[stlHeader->stlFileId] = fileName;
	}
	delete[] contents;
}

void loadModCodesMap(const char* filePath) {
	TiXmlDocument doc(filePath);
	if (doc.LoadFile()) {
		TiXmlNode* attributes = doc.FirstChild("Attributes");
		TiXmlNode* entry = attributes->FirstChild("Entry");
		while (entry) {
			TiXmlElement* element = entry->ToElement();
			RawAttribute attribute;
			int id = atoi(element->Attribute("Id"));
			const char* name = element->Attribute("Name");

			attribute.rawAttributeID = id;
			attribute.nonNlsKey = name;
			attribute.scriptA = element->Attribute("ScriptA");
			attribute.scriptB = element->Attribute("ScriptB");
			attribute.encodingType = element->Attribute("EncodingType");
			attributessMap[id] = attribute;
			entry = entry->NextSibling();
		}
	}
}

void loadAttributes(const char* filePath) {
	TiXmlDocument doc(filePath);
	if (doc.LoadFile()) {
		TiXmlNode* root = doc.FirstChild("MagicPropertyDescriptions");
		TiXmlNode* description = root->FirstChild("Description");
		while (description) {
			Attribute attr;
			attr.modCode = -1;
			attr.param = -1;
			TiXmlElement* element = description->ToElement();
			const char* name = element->Attribute("Name");
			const char* key = element->Attribute("Key");
			const char* format = element->Attribute("Format");

			if (name)
				attr.name = name;
			if (key)
				attr.nonNlsKey = key;
			if (format)
				attr.format = key;

			TiXmlNode* property = description->FirstChild("Property");
			while (property) {
				TiXmlElement* element = property->ToElement();
				const char* name = element->Attribute("Name");
				const char* code = element->Attribute("Code");
				const char* param = element->Attribute("Param");
				const char* index = element->Attribute("Index");
				const char* token = element->Attribute("Token");
				const char* text = element->Attribute("Text");

				int nValue = 0;
				if (token)
					nValue = token[strlen(token) - 1] - '0';

				if ((name && strcmp(name, "Value") == 0) || (code && !index && attr.modCode == -1)) {
					attr.modCode = atoi(code);
					if (param)
						attr.param = atoi(param);
					else
						attr.param = -1;
				}

				if (!nValue) {
					if (index)
						nValue = atoi(index);
					else
						nValue = 1;
				}

				char valueString[128];
				if (code) {
					if (param)
						sprintf(valueString, "{%s|%s}", code, param);
					else
						sprintf(valueString, "{%s}", code);
				}
				else if (text) {
					sprintf(valueString, "%s", text);
				}

				if (nValue == 1)
					attr.value1 = valueString;
				else if (nValue == 2)
					attr.value2 = valueString;
				else if (nValue == 3)
					attr.value3 = valueString;

				property = property->NextSibling();
			}
			if (key) {
				Hash h = hash(key);
				if (stringsHashMap[51480].find(h) != stringsHashMap[51480].end())
					attr.format = stringsHashMap[51480][h][1];
			}
			int id = attr.modCode;
			if (attr.param != -1) {
				id <<= 22;
				id |= attr.param;
			}
			attr.attributeID = id;
			if (attributesMap.find(id) != attributesMap.end() ) {
				printf("%d %d\n", attr.modCode, attr.param);
			}
			attributesMap[id] = attr;
			description = description->NextSibling();
		}
	}
}

std::vector<std::string> stlFiles() {
	std::string dirPath = "StringList/";
	
#ifdef WIN32
	struct _finddata_t c_file;
	intptr_t hFile;

	std::vector<std::string> files;
	if( (hFile = _findfirst( "./StringList/*.stl", &c_file )) != -1) {
		do {
			std::string fileName = c_file.name;
			files.push_back(dirPath + fileName);
		} while( _findnext( hFile, &c_file ) == 0 );
		_findclose( hFile );
	}

#else
	DIR* dir = opendir(dirPath.c_str());
	std::vector<std::string> files;
	struct dirent* ent;
	if (dir) {
		while ((ent = readdir(dir)) != NULL) {
			std::string fileName = ent->d_name;
			std::string extension = fileName.substr(fileName.find_last_of(".") + 1);
			if (extension == "stl")
				files.push_back(dirPath + fileName);
		}
	}
#endif
	return files;
}

/*std::string stringRepresentation(const ModCodeData& memoryBlock)
{
	size_t len = 3 + memoryBlock.size() * 2;
	char *buf = new char[len + 1];
	int i = 2;
	//const Byte *ptr = memoryBlock.c_str();
	//const Byte *end = ptr + memoryBlock.size();
	buf[0] = 'X';
	buf[1] = buf[len - 1] = '\'';
	buf[len] = 0;
	
	const char *map= "0123456789ABCDEF";
	
//	for (; ptr != end; ptr++)
	for (Byte byte: memoryBlock)
	{
		Byte l = byte & 0xf;
		Byte h = byte >> 4;
		buf[i++] = map[h];
		buf[i++] = map[l];
	}
	
	std::string s(buf);
	delete[] buf;
	return s;
}*/

std::string replaceQuotes(const std::string& s) {
	std::string ss;
	for (auto c: s) {
		if (c == '"')
			ss += '"';
		ss += c;
	}
	return ss;
}

void processResults() {
	std::ofstream modCodesSQL("modCodes.sql", std::ios::out | std::ios::trunc);
	std::ofstream attributesSQL("attributes.sql", std::ios::out | std::ios::trunc);
	std::ofstream affixesSQL("affixes.sql", std::ios::out | std::ios::trunc);
	std::ofstream affixGroupsSQL("affixGroups.sql", std::ios::out | std::ios::trunc);
	std::ofstream affixGroupAssociationSQL("affixGroupAssociation.sql", std::ios::out | std::ios::trunc);
	std::ofstream affixAttributesSQL("affixeAttributes.sql", std::ios::out | std::ios::trunc);
	std::ofstream itemTypesSQL("itemTypes.sql", std::ios::out | std::ios::trunc);
	std::ofstream itemTypeAffixesSQL("itemTypeAffixes.sql", std::ios::out | std::ios::trunc);
	std::ofstream legendaryItemTypeAffixesSQL("legendaryItemTypeAffixes.sql", std::ios::out | std::ios::trunc);
	std::ofstream itemsSQL("items.sql", std::ios::out | std::ios::trunc);
	std::ofstream setItemsSQL("setItems.sql", std::ios::out | std::ios::trunc);
	std::ofstream socketedEffectsSQL("socketedEffects.sql", std::ios::out | std::ios::trunc);
	std::ofstream socketedAttributesSQL("socketedAttributes.sql", std::ios::out | std::ios::trunc);
	std::ofstream itemAffixesSQL("itemAffixes.sql", std::ios::out | std::ios::trunc);
	std::ofstream itemAttributesSQL("itemAttributes.sql", std::ios::out | std::ios::trunc);
	std::ofstream setItemAttributesSQL("setItemAttributes.sql", std::ios::out | std::ios::trunc);
	std::ofstream stringsSQL("strings.sql", std::ios::out | std::ios::trunc);
	std::ofstream itemModifiersSQL("itemModifiers.sql", std::ios::out | std::ios::trunc);
	std::ofstream affixModifiersSQL("affixModifiers.sql", std::ios::out | std::ios::trunc);
	std::ofstream setItemModifiersSQL("setItemModifiers.sql", std::ios::out | std::ios::trunc);
	std::ofstream socketModifiersSQL("socketModifiers.sql", std::ios::out | std::ios::trunc);
	std::ofstream resourceFilesSQL("resourceFiles.sql", std::ios::out | std::ios::trunc);
	
	for (auto i: attributessMap) {
		modCodesSQL << "INSERT INTO rawAttribute VALUES(" << i.second.rawAttributeID <<
			", \"" << i.second.nonNlsKey <<
			"\", \"" << i.second.scriptA <<
			"\", \"" << i.second.scriptB <<
			"\", \"" << i.second.encodingType << "\");" << std::endl;
	}
	
	for (auto i: attributesMap) {
		attributesSQL << "INSERT INTO attribute VALUES(" << i.first <<
		", " << i.second.modCode << ", ";
		if (i.second.param != -1)
			attributesSQL << i.second.param << "";
		else
			attributesSQL << "NULL";
		std::string strings[] = {i.second.nonNlsKey, i.second.name, i.second.value1, i.second.value2, i.second.value3, i.second.format};
		for (auto j:  strings) {
			if (j.length() > 0)
				attributesSQL << ", \"" << j <<"\"";
			else
				attributesSQL << ", NULL";
		}
		attributesSQL << ");" << std::endl;
	}
	
	for (auto i: affixGroups) {
		affixGroupsSQL << "INSERT INTO affixGroup VALUES(" << i.first <<", \"" << i.second << "\");" << std::endl;
	}
	
	for (auto i: affixAttributes) {
		for (auto j: i.second)
			affixAttributesSQL << "INSERT INTO affixAttribute VALUES(" << i.first << ", " << j << ");" << std::endl;
	}

	for (auto i: itemAttributes) {
		for (auto j: i.second)
			itemAttributesSQL << "INSERT INTO itemAttribute VALUES(" << i.first << ", " << j << ");" << std::endl;
	}

	for (auto i: setAttributes) {
		for (auto j: i.second)
			setItemAttributesSQL << "INSERT INTO itemSetBonusAttribute VALUES(" << i.first << ", " << j << ");" << std::endl;
	}

	for (auto i: socketAttributes) {
		for (auto j: i.second)
			socketedAttributesSQL << "INSERT INTO socketedEffectAttribute VALUES(" << i.first << ", " << j << ");" << std::endl;
	}

	for (auto affix: affixes) {
		Hash affixHash = hash(affix.affixName);
		affixesSQL << "INSERT INTO affix VALUES(" << affixHash <<
			", " << affix.fileID <<
			", \"" << affix.affixName <<
			"\", " << affix.aLvl <<
			", " << affix.supMask <<
			", " << affix.requiredLevel <<
			", " << affix.type <<
//			", " << affix.effectTier <<
	//		", " << affix.convertsTo <<
			", " << affix.rareNamePrefixId <<
			", " << affix.rareNameSuffixId <<
			", " << affix.resourceOrClass <<
			", " << affix.resourceTypeHash <<
			", " << affix.qualityMask <<
			", " << affix.propertyType <<
//			", " << affix.primaryGroupHash <<
			");" << std::endl;
		
		for (int i = 0; i < 2; i++) {
			if (isValidHash(affix.affixGroups[i])) {
				affixGroupAssociationSQL << "INSERT INTO affixGroupAssociation VALUES(" << affixHash <<
				", " << affix.affixGroups[i] << ");" << std::endl;
			}
		}
		
		
		for (int i = 0; i < 4; i++) {
			MPQModCode& modCode = affix.mods[i];
			if (modCode.modCode > 0 && modCode.varDataOffset) {
				ModDataKey key = ((ModDataKey)affix.fileID) << 32 | modCode.varDataOffset;
				Range r = modData[key];
				affixModifiersSQL << "INSERT INTO affixModifier VALUES(" << affixHash <<
				", " << modCode.modCode <<
				", " << modCode.modParam1 <<
				", " << modCode.modParam2 <<
				", " << modCode.modParam3 <<
				", " << r.min <<
				", " << r.max << ");" << std::endl;
			}
		}

		std::unordered_set<Hash> itemTypes;
		std::unordered_set<Hash> legendaryItemTypes;
		for (int i = 0; i < 16; i++) {
			if (isValidHash(affix.itemTypes[i])) {
				if (itemTypes.find(affix.itemTypes[i]) == itemTypes.end()) {
					itemTypeAffixesSQL << "INSERT INTO itemTypeAffix VALUES(" << affix.itemTypes[i] <<
						"," << affixHash << ");" << std::endl;
					itemTypes.insert(affix.itemTypes[i]);
				}
			}
			if (isValidHash(affix.legendaryItemTypes[i])) {
				if (legendaryItemTypes.find(affix.legendaryItemTypes[i]) == legendaryItemTypes.end()) {
					legendaryItemTypeAffixesSQL << "INSERT INTO legendaryItemTypeAffix VALUES(" << affix.legendaryItemTypes[i] <<
					"," << affixHash << ");" << std::endl;
					legendaryItemTypes.insert(affix.legendaryItemTypes[i]);
				}
			}
		}
	}
	
	for (auto itemType: itemTypes) {
		Hash itemTypeHash = hash(itemType.itemTypeName);
		itemTypesSQL << "INSERT INTO itemType VALUES(" << itemTypeHash <<
			", " << itemType.fileID <<
			", \"" << itemType.itemTypeName <<
			"\", " << itemType.parentHash <<
			", " << itemType.flags <<
			", " << itemType.slot[0] <<
			", " << itemType.slot[1] <<
			", " << itemType.slot[2] <<
			", " << itemType.slot[3] <<
			", " << itemType.bitMask[0] <<
			", " << itemType.bitMask[1] <<
			", " << itemType.bitMask[2] <<
			", " << itemType.bitMask[3] <<
			", " << itemType.inherentAffixGroupHash << ");" << std::endl;
		
		/*for (int i = 0; i < 3; i++) {
			if (isValidHash(itemType.inherentAffixHashes[i])) {
				itemTypeAffixesSQL << "INSERT INTO itemTypeAffix VALUES(" << itemTypeHash <<
				"," << itemType.inherentAffixHashes[i] << ");" << std::endl;
			}
		}*/
	}
	
	for (auto item: items) {
		int itemHash = hash(item.itemName);
		itemsSQL << "INSERT INTO item VALUES (" << itemHash <<
			", " << item.fileID <<
			", \"" << item.itemName <<
			"\", " << item.actorId <<
			", " << item.itemTypeHash <<
			", " << item.flags <<
			", " << item.iLevel <<
			", " << item.numRandomAffixes <<
			", " << item.numSockets <<
			", " << item.goldPrice <<
			", " << item.cLevel <<
			", " << item.durabilityMin <<
			", " << item.durabilityMax <<
			", " << item.baseItemHash <<
			", " << item.setItemBonuses <<
			", " << item.salvageCommon <<
			", " << item.salvageMagic <<
			", " << item.salvageRare <<
			", " << item.rareGroupPrefixId <<
			", " << item.rareGroupSuffixId <<
			", " << item.minDamage <<
			", " << item.maxDamage <<
			", " << item.minArmor <<
			", " << item.maxArmor <<
			", " << item.weaponSpeed <<
			", " << item.minDamageMod <<
			", " << item.maxDamageMod <<
			", " << item.itemQuality <<
			", " << item.enchants <<
			", " << item.gemType <<
			", " << item.craftingMatTier <<
			", " << item.craftingMatRarity << ");" << std::endl;
		
		for (int i = 0; i < 6; i++) {
			if (isValidHash(item.affixGroupHash[i])) {
				itemAffixesSQL << "INSERT INTO itemAffix VALUES(" << itemHash <<
				", " << item.affixGroupHash[i] <<
				", " << item.affixLevel[i] << ");" << std::endl;
			}
		}

		for (int i = 0; i < 16; i++) {
			MPQModCode& modCode = item.mods[i];
			if (modCode.modCode > 0 && modCode.varDataOffset) {
				ModDataKey key = ((ModDataKey)item.fileID) << 32 | modCode.varDataOffset;
				
				Range r = modData[key];
				itemModifiersSQL << "INSERT INTO itemModifier VALUES(" << itemHash <<
				", " << modCode.modCode <<
				", " << modCode.modParam1 <<
				", " << modCode.modParam2 <<
				", " << modCode.modParam3 <<
				", " << r.min <<
				", " << r.max << ");" << std::endl;
			}
		}
	}
	
	for (auto setItem: setItemBonuses) {
		int itemSetHash = hash(setItem.itemSetName);
		setItemsSQL << "INSERT INTO itemSetBonus VALUES (" << itemSetHash <<
		", " << setItem.fileID <<
		", \"" << setItem.itemSetName <<
		"\", " << setItem.parentHash <<
		", " << setItem.numOfSet << ");" << std::endl;
		
		for (int i = 0; i < 8; i++) {
			MPQModCode& modCode = setItem.mods[i];
			if (modCode.modCode > 0 && modCode.varDataOffset) {
				ModDataKey key = ((ModDataKey)setItem.fileID) << 32 | modCode.varDataOffset;
				
				Range r = modData[key];
				setItemModifiersSQL << "INSERT INTO itemSetBonusModifier VALUES(" << itemSetHash <<
				", " << modCode.modCode <<
				", " << modCode.modParam1 <<
				", " << modCode.modParam2 <<
				", " << modCode.modParam3 <<
				", " << r.min <<
				", " << r.max << ");" << std::endl;
			}
		}
	}
	
	for (auto effect: socketedEffects) {
		int effectHash = hash(effect.socketedEffectName);
		socketedEffectsSQL << "INSERT INTO socketedEffect VALUES (" << effectHash <<
		", " << effect.fileID <<
		", \"" << effect.socketedEffectName <<
		"\", " << effect.itemHash <<
		", " << effect.itemTypeHash << ");" << std::endl;
		
		for (int i = 0; i < 5; i++) {
			MPQModCode& modCode = effect.mods[i];
			if (modCode.modCode > 0 && modCode.varDataOffset) {
				ModDataKey key = ((ModDataKey)effect.fileID) << 32 | modCode.varDataOffset;
				
				Range r = modData[key];
				socketModifiersSQL << "INSERT INTO socketedEffectModifier VALUES(" << effectHash <<
				", " << modCode.modCode <<
				", " << modCode.modParam1 <<
				", " << modCode.modParam2 <<
				", " << modCode.modParam3 <<
				", " << r.min <<
				", " << r.max << ");" << std::endl;
			}
		}
	}

	for (auto i: stringsHashMap) {
		for (auto j: i.second) {
			const std::vector<std::string>& strings = j.second;
			stringsSQL << "INSERT INTO string VALUES(" << j.first<<
				", " << i.first <<
				", \"" << replaceQuotes(strings[0]) <<
				"\", \"" << replaceQuotes(strings[1]) <<
				"\", \"" << replaceQuotes(strings[2]) <<
				"\", \"" << replaceQuotes(strings[3]) << "\");" << std::endl;
			}
	}
	
	for (auto i: filesMap) {
		resourceFilesSQL << "INSERT INTO resourceFile VALUES(" << i.first <<
			", \"" << i.second << "\");" << std::endl;
	}
}

int main(int argc, const char * argv[])
{
	int s = sizeof(MPQItem);
	std::vector<std::string> stls = stlFiles();
	std::string gams[] = {
		"GameBalance/SocketedEffects.gam",
		"GameBalance/AffixList.gam",
		"GameBalance/ItemTypes.gam",
		"GameBalance/Items_Armor.gam",
		"GameBalance/Items_Legendary_Weapons.gam",
		"GameBalance/Items_Legendary_Other.gam",
		"GameBalance/Items_Legendary.gam",
		"GameBalance/Items_Other.gam",
		"GameBalance/Items_Quests_Beta.gam",
		"GameBalance/Items_Quests.gam",
		"GameBalance/Items_Weapons.gam",
		"GameBalance/SetItemBonuses.gam"};

	loadModCodesMap("attributes.xml");
	for (auto path: stls) {
		processFile(path.c_str());
	}
	loadAttributes("MagicPropertyDescriptions.xml");

	for (auto path: gams) {
		processFile(path.c_str());
	}
	
	processResults();
}

