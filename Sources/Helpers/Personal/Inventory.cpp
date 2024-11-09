#include "Helpers/Inventory.hpp"
#include "Helpers/Game.hpp"
#include "Helpers/Player.hpp"
#include "Helpers/IDList.hpp"
#include "RegionCodes.hpp"
#include "Files.h"

namespace CTRPluginFramework {
	ItemVec* ItemList = new ItemVec();
	int ItemFileLength = 0;
	bool ItemFileExists = true;

	CustomItemVec* CustomItemList = new CustomItemVec();
	int CustomItemFileLength = 0;
	bool CustomItemFileExists = true;

//reserver data into pointer so search doesnt take so long
	void ReserveItemData(ItemVec* out) {
		if(out == nullptr) 
			return;

		File file(ITEMLIST, File::READ);
		if(!file.IsOpen()) {
			ItemFileExists = false;
			return;
		}

		std::string line;
		LineReader reader(file);

		ItemFileLength = 0; //reset file length if called again
		u32 lineNumber = 0;
		int count = 0;

	//Read all lines in file
		for(; reader(line); lineNumber++) {
		//If line is empty, skip it
			if(line.empty())
				continue;

			std::string lowcaseInput(line);
			for(char& c : lowcaseInput)
				c = std::tolower(c);

			std::string Name = lowcaseInput.substr(5, 30); //lets make max 30 for now
			std::string SID = lowcaseInput.substr(0, 4); 
			Item ID = (Item)StringToHex<u16>(SID, 0xFFFF);
			out->Name.push_back(Name);
			out->ID.push_back(ID);
			ItemFileLength++; //adds to file length to know how many items are in it
		}
	}

	void ReserveCustomItemData(CustomItemVec* out) {
		if(out == nullptr) 
			return;

		File file(CUSTOMITEMLIST, File::READ);
		if(!file.IsOpen()) {
			CustomItemFileExists = false;
			return;
		}

		std::string line;
		LineReader reader(file);

		CustomItemFileLength = 0; //reset file length if called again
		u32 lineNumber = 1;
		int count = 0;

		Item currItem;
		CustomItemPartVec currParts;
		std::string currPartName[2] = {"", ""};
		CustomItemOptionVec currPartOptions[2];
		int currPartIndex = 0;
		std::vector<std::string> currLineVec;

		//Read our file until the last line
		for(; reader(line); lineNumber++) {
			currLineVec.clear();
			//If line is empty, skip it
			if(line.empty())
				continue;
			
			std::string part;
			int i = 0;

			for(;;i++) {
				// Search our delimiter
				auto pos = line.find(",");

				// If we couldn't find the delimiter, use the rest of the string
				if(pos == std::string::npos) {
					part = line;
				}
				else if (i == 0 && pos >= 4) {
					part = line.substr(0, 4);
				}
				else {
					part = line.substr(0, pos);
				}

				g_Trim(part);
				currLineVec.push_back(part);

				if(pos == std::string::npos) {
					break;
				}
				else {
					line = line.substr(pos + 1);
				}
			}

			if(currLineVec.empty()) {
				continue;
			}
			// Parse the line into custom item data
			else {
				if (!currLineVec[0].empty()) {
					// First push the previous item if we have one
					if(currItem.ID != 0) {
						for (int i=0; i<2; i++) {
							std::sort(currPartOptions[i].begin(), currPartOptions[i].end());
							currParts.Name.push_back(currPartName[i]);
							currParts.Options.push_back(currPartOptions[i]);
						}

						out->ID.push_back(currItem);
						out->CustomParts.push_back(currParts);
						CustomItemFileLength++; //adds to file length to know how many items are in it
					}
					currItem = (Item)StringToHex<u16>(currLineVec[0], 0xFFFF);
					currParts.Name.clear();
					currParts.Options.clear();
					for (int i=0; i<2; i++) {
						currPartName[i] = {""};
						currPartOptions[i].clear();
					}
				}
				if (!currLineVec[1].empty()) {
					currPartIndex = StringToHex<u16>(currLineVec[3], 0xFFFF) > 0 ? 0 : 1;
					currPartName[currPartIndex] = currLineVec[1];
				}
				if (!currLineVec[2].empty()) {
					u16 flag = 0;
					if(currPartIndex == 0)
						flag = StringToHex<u16>(currLineVec[3], 0xFFFF) << 8;
					else
						flag = StringToHex<u16>(currLineVec[4], 0xFFFF);

					currPartOptions[currPartIndex].push_back(std::make_pair(flag, currLineVec[2]));
				}
			}
		}
	}

	int ItemSearch(const std::string& match, ItemVec& out) {
		int count = 0;
	//Read our file until the last line
		for(int i = 0; i < ItemFileLength; ++i) {
			auto namePos = ItemList->Name[i].find(match);
			if(namePos != std::string::npos) {
				out.Name.push_back(ItemList->Name[i]);
				out.ID.push_back(ItemList->ID[i]);
				count++;
			}
		}

		return count;
	}

	std::string ItemIDSearch(Item ItemID) {
		if(!ItemFileExists)
			return "";

	//Read our file until the last line
		for(int i = 0; i < ItemFileLength; ++i) {
			if(ItemList->ID[i] == ItemID) {
				return ItemList->Name[i];
			}
		}

		return "???";
	}

    int GetCustomItemIndex(Item ItemID)
    {
		//Read our file until the last line
		for(int i = 0; i < CustomItemFileLength; ++i) {
			if(CustomItemList->ID[i] == ItemID) {
				return i;
			}
		}

        return -1;
    }

	int GetCustomClothesIndex(int currentIndex) {
		switch(currentIndex){
			case -1: return 0x327;
			case 0x1F4: return 0x2BD;
			case 0x2BC: return 0x1F3;
			case 0x328: return 0;
		}
		return currentIndex;
	}

	std::string GetCustomClothesName(u16 clothesIndex)
	{
		std::string clothesName = "";
		const u16 clothesOffset = clothesIndex > 0x1f3 ? 0x23c5 : 0x248d;

		IDList::GetSeedName(Item{(u16)(clothesIndex + clothesOffset), 0}, clothesName);
		return Utils::Format("Clothes (%s)", clothesName.c_str());
	}

	bool GetCustomOption(int customItemIndex, bool isPart1, u16& optionIndex, int increment, std::pair<u16, std::string>& option)
	{
		CustomItemOptionVec options = CustomItemList->CustomParts[customItemIndex].Options[isPart1 ? 0 : 1];

		if(options.size() == 0)
			return false;

		bool useClothes = !isPart1 && options.back().first == 0x8;

		int newOptionIndex = optionIndex;
		increment = std::clamp(increment, -1, 1);
		newOptionIndex += increment;

		if(useClothes && options[newOptionIndex - 1].first == 0x8 && increment > 0) {
			newOptionIndex = 8;
		}	
		else if(useClothes && newOptionIndex < 8 && increment < 0) {
			newOptionIndex = std::min((int)options.size() - 1, newOptionIndex);
		}

		// Check if we can display clothing
		if(useClothes) {
			newOptionIndex = GetCustomClothesIndex(newOptionIndex);
		}
		else {
			newOptionIndex = std::clamp(newOptionIndex, 0, (int)options.size());
		}

		if (increment == 0 || newOptionIndex != optionIndex) {
			
			optionIndex = newOptionIndex;
			
			if (optionIndex > 0) {
				if (useClothes && optionIndex >= 0x8) {
					option = std::make_pair(optionIndex, GetCustomClothesName(optionIndex));
				}
				else {
					option = options[optionIndex - 1];
				}
			}
			else {
				option = std::make_pair(0x0, "Original");
			}
			
			return true;
		}

		return false;
	}

    u32 Inventory::GetCurrentItemData(int i)
    {
        if(GameHelper::BaseInvPointer() == 0)
			return -1;
		
		if(!Opened())
			return -1;
			
		if(GetCurrent() != 0)
			return -1;
			
		u32 Items = *(u32 *)(*(u32 *)(GameHelper::BaseInvPointer() + 0xC) + 0xEC); //0x20 32DCEC10
		
		return (Items + (0xAC * i));
    }

//Get current inventory ID
	u8 Inventory::GetCurrent() {
		if(GameHelper::BaseInvPointer() == 0) 
			return 0xFF;
		
		return *(u8 *)(*(u32 *)(GameHelper::BaseInvPointer() + 0xC) + 0x24);
	}

//get correct inv addition data
	u16 Inventory::GetAddData() {
		if(GameHelper::BaseInvPointer() == 0) 
			return -1;
		
		switch(GetCurrent()) {
			//Base Inventory
			case 0: 
				return 0x1EC0;
			
			//Island Box Inventory
			case 0x3E:
			case 0x3F:
				return 0x3CF4;
			
			//Item borrow Inventory
			case 0x40:
				return 0x27D4;
			
			//Closet Inventory
			case 0x3D:
			case 0x89:
				return 0x50;
			
			//hopefully all other inventory
			default:
				return 0x60;
		}
	}
//if inv is opened
	bool Inventory::Opened() {
		if(GameHelper::BaseInvPointer() == 0) 
			return 0;
		
		return *(u8 *)(*(u32 *)(GameHelper::BaseInvPointer() + 0xC) + (0x8419 + GetAddData())) == 1;
	}
//Write Inventory Slot
	bool Inventory::WriteSlot(int slot, Item item, u8 lock) {
		ACNL_Player *player = Player::GetSaveData();
		if(!player) 
			return false;
			
		if(!IDList::ItemValid(item, false)) 
			return false;
		
	//Writes item and fixes lock if needed
		player->Inventory[slot] = item;
		player->InventoryItemLocks[slot] = lock;
		
		ReloadIcons();

		return true;
	}
//Read Inventory Slot	
	bool Inventory::ReadSlot(int slot, Item& item) {
		ACNL_Player *player = Player::GetSaveData();
		if(!player) 
			return false;

		item = player->Inventory[slot];
		return true;
	}
//Get asked item
	bool Inventory::GetNextItem(Item itemID, u8 &slot, bool checkFlags) {
		ACNL_Player *player = Player::GetSaveData();
		if(!player) 
			return false;
		
		slot = 0;
		while(true) {
			if(itemID == player->Inventory[slot] || (!checkFlags && itemID.ID == player->Inventory[slot].ID)) //If item found return  
				return true;

			slot++; //goto next slot
			
			if(15 < slot) //If item not found return
				return false;		
		}
	}
//Get asked closet item	
	bool Inventory::GetNextClosetItem(Item itemID, u8 &slot) {
		ACNL_Player *player = Player::GetSaveData();
		if(!player) 
			return false;
		
		slot = 0;
		while(true) {
			if(itemID == player->Dressers[slot]) //If item found return  
				return true;
			
			slot++; //goto next slot
			
			if(179 < slot) //If item not found return
				return -1;			
		}
	}

	void Inventory::ReloadIcons() {
	//if inv is not loaded return
		if(GameHelper::BaseInvPointer() == 0) 
			return;
		
	//If inv is not opened return
		if(!Opened())
			return;

		for(int i = 0; i < 16; ++i)
			Code::LoadIcon.Call<void>(*(u32 *)(GameHelper::BaseInvPointer() + 0xC) + GetAddData(), i);
	}
	
//get current selected inventory slot
    bool Inventory::GetSelectedSlot(u8& slot) {
		if(!Opened()) 
			return false;
		
		u32 offs = *(u32 *)(GameHelper::BaseInvPointer() + 0xC);
		offs += 0xCC;
		slot = *(u8 *)offs;

		if(slot != -1 && slot < 0x10) 
			return true;
		
		return false;
	}

//get hovered inv slot
	bool Inventory::GetHoveredSlot(u8& slot) {
		if(!Opened()) 
			return false;
		
		u32 offs = *(u32 *)(GameHelper::BaseInvPointer() + 0xC);
		offs += 0xD4;
		slot = *(u8 *)offs;

		if(slot != -1 && slot < 0x10) 
			return true;
		
		return false;
	}
}