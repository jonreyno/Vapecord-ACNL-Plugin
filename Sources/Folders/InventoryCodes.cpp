#include "cheats.hpp"
#include "Helpers/Game.hpp"
#include "Helpers/IDList.hpp"
#include "Helpers/Inventory.hpp"
#include "Helpers/PlayerClass.hpp"
#include "Helpers/Wrapper.hpp"
#include "Helpers/Player.hpp"
#include "Helpers/GameKeyboard.hpp"
#include "Helpers/Animation.hpp"
#include "RegionCodes.hpp"
#include "Color.h"
#include "Files.h"

#define MAXCOUNT 25

namespace CTRPluginFramework {
	void ItemListCallBack(Keyboard& keyboard, KeyboardEvent& event) {
		std::string& input = keyboard.GetInput();

		std::string lowcaseInput(input);
		for(char& c : lowcaseInput)
			c = std::tolower(c);

		if(event.type == KeyboardEvent::CharacterRemoved) {
			keyboard.SetError(Language->Get("TEXT_2_ITEM_SEARCH_ERR1"));
			return;
		}

		if(lowcaseInput.size() < 3) {
			keyboard.SetError(Language->Get("TEXT_2_ITEM_SEARCH_ERR2"));
			return;
		}

		ItemVec match;
		int res = ItemSearch(lowcaseInput, match);

		if(res == 0) {
			keyboard.SetError(Language->Get("TEXT_2_ITEM_SEARCH_ERR3"));
			return;
		}

		if(res > MAXCOUNT) {
			keyboard.SetError(Utils::Format(Language->Get("TEXT_2_ITEM_SEARCH_ERR4").c_str(), res));
			return;
		}

		Keyboard KB(Language->Get("TEXT_2_ITEM_SEARCH_KB"), match.Name);
		int kres = KB.Open();
		if(kres < 0) {
			input.clear();
			return;
		}
		
		input.clear();
		u8 slot = 0;
		if(!Inventory::GetNextItem({0x7FFE, 0}, slot)) {
			keyboard.Close();
			OSD::Notify("Inventory Full!",  Color::Red);
			return;
		}
			
		if(!IDList::ItemValid(Item(match.ID[kres]), false)) {
			keyboard.Close();
			OSD::Notify("Invalid Item!",  Color::Red);
			return;
		}	

		Inventory::WriteSlot(slot, match.ID[kres]);
		OSD::Notify(Utils::Format("Set Item %04X to slot %d", match.ID[kres], slot));
	}

//Text to Item
	void t2i(MenuEntry *entry) {
		Item val;
		ACNL_Player *player = Player::GetSaveData();

		if(entry->Hotkeys[0].IsPressed()) {
			if(!player) {
				OSD::Notify("Error: Player needs to be loaded!", Color::Red);
				return;
			}

			Inventory::ReadSlot(0, val);
			if(Wrap::KB<u32>(Language->Get("ENTER_ID"), true, 8, *(u32 *)&val, *(u32 *)&val, TextItemChange)) 		
				Inventory::WriteSlot(0, val);
		}
		
		else if(entry->Hotkeys[1].IsPressed()) {
			if(!player) {
				OSD::Notify("Error: Player needs to be loaded!", Color::Red);
				return;
			}

			if(Wrap::KB<u32>(Language->Get("TEXT_2_ITEM_SET"), true, 8, *(u32 *)&val, 0x7FFE, TextItemChange)) {
				for(u16 i = 0; i < 0x10; ++i) {
					Item item = {(u16)(val.ID + i), 0};
					Inventory::WriteSlot(i, item);
				}
			}
		} 
		
		else if(entry->Hotkeys[2].IsPressed()) {
			if(!player) {
				OSD::Notify("Error: Player needs to be loaded!", Color::Red);
				return;
			}

			u32 x, y;
			if(PlayerClass::GetInstance()->GetWorldCoords(&x, &y)) {
				Item *item = GameHelper::GetItemAtWorldCoords(x, y);
				if(item) {
					Inventory::WriteSlot(0, *item);
					OSD::Notify(Utils::Format("Item ID: %08X", *(u32 *)item));
				}
			}
		}

		else if(entry->Hotkeys[3].IsPressed()) {
			if(!player) {
				OSD::Notify("Error: Player needs to be loaded!", Color::Red);
				return;
			}

			if(!ItemFileExists) {
				OSD::Notify("Error: item.txt missing!", Color::Red);
				return;
			}
			std::string input = "";
			Keyboard KB(Language->Get("TEXT_2_ITEM_SEARCH_KB2"));
			KB.OnKeyboardEvent(ItemListCallBack);
			KB.Open(input);
		}
	}
//Duplicate Items
	void duplication(MenuEntry *entry) {
		ACNL_Player *player = Player::GetSaveData();
		if(!player) 
			return;

		if(entry->Hotkeys[0].IsPressed()) {
			Inventory::WriteSlot(1, player->Inventory[0], player->InventoryItemLocks[0]);
		}
		else if(entry->Hotkeys[1].IsPressed()) {
			for(int i = 0; i <= 0xF; ++i) 
				Inventory::WriteSlot(i, player->Inventory[0], player->InventoryItemLocks[0]);
		}
	}

	void CatalogGetItem(u32 invData) {
		Item CurrentItem = *(Item *)(invData + 0x3B9C - 0x28);

		if(GameHelper::SetItem(&CurrentItem)) {
			std::string itemName = "";
			if(IDList::GetSeedName(CurrentItem, itemName))
				OSD::Notify(Utils::Format("Spawned Item: %s (%04X)", itemName.c_str(), CurrentItem.ID));
			else
				OSD::Notify(Utils::Format("Spawned Item: %04X", CurrentItem.ID));
		}
		else
			OSD::Notify("Inventory Full!");

		static Address argData(0x8499E4, 0x8489DC, 0x848870, 0x848870, 0x84886C, 0x84786C, 0x84786C, 0x84786C);

		static Address restoreButton(0x81825C, 0x81715C, 0x817264, 0x81723C, 0x816A04, 0x8169DC, 0x8165A4, 0x81657C);
		restoreButton.Call<void>(invData, *(u32 *)argData.addr, *(u32 *)(argData.addr + 4));
	}

	static bool allItemsBuyable = false;

	void SetAllItemsBuyable(bool buyable)
	{
		static Address AllItemsBuyable(0x70E494, 0x70D944, 0x70D4B4, 0x70D48C, 0x70CC60, 0x70CC38, 0x70C808, 0x70C7E0);
		
		if (buyable) {
			Process::Patch(AllItemsBuyable.addr, 0xE3A00000);
			Process::Patch(AllItemsBuyable.addr + 4, 0xEA00000B);
			allItemsBuyable = true;
		} else {
			Process::Patch(AllItemsBuyable.addr, 0x03A00001);
			Process::Patch(AllItemsBuyable.addr + 4, 0x0A00000B);
			allItemsBuyable = false;
		}
	}

	static Address currentItemAdr(0x32D97680);
	static Address itemSelectedAdr(0x0094F32C);
	static Item currentCatalogItem;
	static int ciIndex = -1;
	static u16 optionPartIndex[2] = {0, 0};
	static const std::pair<u16, std::string> originalPart = std::make_pair(0, "Original");
	static std::pair<u16, std::string> optionPart[2] = {originalPart, originalPart};

	void UpdateCustomFlags() {
		u16 flags = optionPart[0].first + optionPart[1].first;
		//OSD::Notify(Utils::Format("Setting Flags: %04X", flags), Color::Green);
		Process::Write16(currentItemAdr.addr + 2, flags);
	}

	void CycleCatalogFurniturePart(int increment, int partIndex)
	{
		if (GetCustomOption(ciIndex, partIndex == 0, optionPartIndex[partIndex], increment, optionPart[partIndex])) {
			//OSD::Notify(Utils::Format("%s: %s (%04X) Index: %d", CustomItemList->CustomParts[ciIndex].Name[partIndex].c_str(), optionPart[partIndex].second.c_str(), optionPart[partIndex].first, optionPartIndex[partIndex]), Color::Green);
			UpdateCustomFlags();
		}
	}

	bool ResetPartIndex(u8 partIndex)
	{
		if(optionPartIndex[partIndex] > 0) {
			optionPartIndex[partIndex] = 0;
			optionPart[partIndex] = originalPart;
			return true;
		}

		return false;
	}

	void ResetCustomFlags(int partIndex = -1)
	{
		bool updated = false;

		if(partIndex >= 0) {
			updated = ResetPartIndex(partIndex);
		}
		else
		{
			for(int i=0; i<2; i++) {
				updated = ResetPartIndex(i) || updated;
			}
		}

		if (updated)
			UpdateCustomFlags();
	}

	//catalog OSD
	bool catalogOSD(const Screen &screen) {
		if(!screen.IsTop || ciIndex < 0)
			return 0;

		u16 itemSelected;
		Process::Read16(itemSelectedAdr.addr, itemSelected);

		if(itemSelected == 0x7ffe)
			return 0;

		CustomItemOptionVec options1 = CustomItemList->CustomParts[ciIndex].Options[0];
		CustomItemOptionVec options2 = CustomItemList->CustomParts[ciIndex].Options[1];

		if(options1.size() > 0) {
			screen.Draw(Utils::Format("%s: %s", CustomItemList->CustomParts[ciIndex].Name[0].c_str(), optionPart[0].second.c_str()), 10, 10);
		}

		if(options2.size() > 0) {
			screen.Draw(Utils::Format("%s: %s", CustomItemList->CustomParts[ciIndex].Name[1].c_str(), optionPart[1].second.c_str()), 10, 20);
		}
		return 1;
	}

	static bool isCatalogOpen = false;

//Catalog To Pockets
	void catalog(MenuEntry *entry) {
		static Hook catalogHook;
		static Address cHook(0x21B4B0, 0x21AEF4, 0x21B4D0, 0x21B4D0, 0x21B3F0, 0x21B3F0, 0x21B3BC, 0x21B3BC);

		if(entry->WasJustActivated()) {
			catalogHook.Initialize(cHook.addr, (u32)CatalogGetItem);
			catalogHook.SetFlags(USE_LR_TO_RETURN);
			catalogHook.Enable();

			SetAllItemsBuyable(false);
		}

		if(entry->Hotkeys[0].IsPressed()) {
			if(!PlayerClass::GetInstance()->IsLoaded()) {
				OSD::Notify("Player needs to be loaded!");
				return;
			}
			
			//if no menu is opened 
			if(GameHelper::BaseInvPointer() == 0) {	
				GameHelper::Catalog();
				return;
			}	
		}
		
		if(Inventory::GetCurrent() == 0x7C && !isCatalogOpen) {
			isCatalogOpen = true;
		}
		
		if(isCatalogOpen)
		{
			u16 currentID;
			Process::Read16(currentItemAdr.addr, currentID);

			if(GameHelper::GetItemCategory(Item{currentID, 0}) == Item_Category::Furniture && currentCatalogItem.ID != currentID){
				currentCatalogItem = Item(currentID, 0);
				ciIndex = GetCustomItemIndex(currentCatalogItem);

				if(ciIndex >= 0) {
					for(int i = 0; i < 2; i++) {
						if(!CustomItemList->CustomParts[ciIndex].Options[i].empty()) {
							if(optionPartIndex[i] <= CustomItemList->CustomParts[ciIndex].Options[i].size()
							|| CustomItemList->CustomParts[ciIndex].Options[i].back().first == 0x8) {
								CycleCatalogFurniturePart(0, i);
								continue;
							}
						}
						ResetCustomFlags(i);
					}
					
					OSD::Run(catalogOSD);
				}
				else {
					OSD::Stop(catalogOSD);
					ResetCustomFlags();
				}
			}

			// Toggle all items buyable
			if(entry->Hotkeys[1].IsPressed()) {
				SetAllItemsBuyable(!allItemsBuyable);
				
				if (allItemsBuyable)
				{
					OSD::Notify("All items buyable ON!", Color::Green);
				}
				else
				{
					OSD::Notify("All items buyable OFF!", Color::Red);
				}
			}
			
			// Listen for custom cycle inputs
			if(ciIndex >= 0) {
				if(entry->Hotkeys[2].IsPressed()) {
					CycleCatalogFurniturePart(-1, 0);
				}
				
				if(entry->Hotkeys[3].IsPressed()) {
					CycleCatalogFurniturePart(1, 0);
				}
				
				if(entry->Hotkeys[4].IsPressed()) {
					CycleCatalogFurniturePart(-1, 1);
				}
				
				if(entry->Hotkeys[5].IsPressed()) {
					CycleCatalogFurniturePart(1, 1);
				}
			}
		}
		else {
			OSD::Stop(catalogOSD);
		}
		
		if(Inventory::GetCurrent() != 0x7C && isCatalogOpen) {
			Animation::Idle();
			isCatalogOpen = false;
		}
		
		if(!entry->IsActivated()) {
			catalogHook.Disable();
			SetAllItemsBuyable(false);
			isCatalogOpen = false;
			OSD::Stop(catalogOSD);
		}
	}
//Chat Text2Item
	void chatt2i(MenuEntry *entry) {
		if(!entry->Hotkeys[0].IsPressed())
			return;
		
		if(!PlayerClass::GetInstance()->IsLoaded()) {
			OSD::Notify("Player needs to be loaded!", Color::Red);
			return;
		}
		
		if(!GameKeyboard::IsOpen()) {
			OSD::Notify("Open your Keyboard!", Color::Red);
			return;
		}

		if(GameKeyboard::IsEmpty()) {
			OSD::Notify("Keyboard is empty!", Color::Red);
			return;
		}
		
		std::string chatStr = "";
		Item itemID;

		if(!GameKeyboard::Copy(chatStr, 0, 0x16)) {
			OSD::Notify("Somehow couldn't copy!", Color::Red);
			return;
		}

		if(!GameKeyboard::ConvertToItemID(chatStr, itemID)) {
			OSD::Notify("Invalid Character!", Color::Red);
			return;
		}
		
		u8 slot = 0;
		if(!Inventory::GetNextItem({0x7FFE, 0}, slot)) {
			OSD::Notify("Inventory Full!", Color::Red);
			return;
		}
			
		if(!IDList::ItemValid(itemID, false)) {
			OSD::Notify("Invalid Item!", Color::Red);
			return;
		}
		
		Inventory::WriteSlot(slot, itemID);

		std::string itemName = "";
		if(IDList::GetSeedName(itemID, itemName))
			OSD::Notify(Utils::Format("Spawned Item: %s (%08X)", itemName.c_str(), itemID));
		else
			OSD::Notify(Utils::Format("Spawned Item: %08X", itemID));
	}
//Clear Inventory
	void ClearInventory(MenuEntry *entry) {
		if(!Player::GetSaveData()) {
			MessageBox(Language->Get("SAVE_PLAYER_NO")).SetClear(ClearScreen::Top)();
			return;
		}

		if((MessageBox(Language->Get("REMOVE_INV_WARNING"), DialogType::DialogYesNo)).SetClear(ClearScreen::Top)()) {
			for(int i = 0; i <= 0xF; ++i)
				Inventory::WriteSlot(i, Item{ 0x7FFE, 0 });
		}
	}

//Item Settings	
	void itemsettings(MenuEntry *entry) {
		static const Address showoff(0x19BA78, 0x19B4C0, 0x19BA98, 0x19BA98, 0x19B9D8, 0x19B9D8, 0x19B9D8, 0x19B9D8);
		static const Address infinite1(0x19C574, 0x19BFBC, 0x19C594, 0x19C594, 0x19C4D4, 0x19C4D4, 0x19C4D4, 0x19C4D4);
		static const Address infinite2(0x19C4D0, 0x19BF18, 0x19C4F0, 0x19C4F0, 0x19C430, 0x19C430, 0x19C430, 0x19C430);
		static const Address eat(0x19C1F0, 0x19BC38, 0x19C210, 0x19C210, 0x19C150, 0x19C150, 0x19C150, 0x19C150);
		
		std::vector<std::string> itemsettopt = {
			Language->Get("VECTOR_ITEMSETTINGS_SHOWOFF"),
			Language->Get("VECTOR_ITEMSETTINGS_INFINITE"),
			Language->Get("VECTOR_ITEMSETTINGS_EAT"),
		};
		
		static const u32 settings[3] = {
			showoff.addr, infinite1.addr, eat.addr
		};
		
		static constexpr u32 settingsvalue[2][3] = {
			{ 0xE1A00000, 0xE2805A00, 0xE1A00000 },
			{ 0x1A000012, 0xE2805A06, 0x0A000009 },
		};

		bool IsON;
		
		for(int i = 0; i < 3; ++i) { 
			IsON = *(u32 *)settings[i] == settingsvalue[0][i];
			itemsettopt[i] = IsON ? (Color(pGreen) << itemsettopt[i]) : (Color(pRed) << itemsettopt[i]);
		}
		
		Keyboard optKb(Language->Get("KEY_CHOOSE_OPTION"), itemsettopt);

		int op = optKb.Open();
		if(op < 0)
			return;
			
		if(op == 1) {
			Process::Patch(infinite1.addr, *(u32 *)infinite1.addr == 0xE2805A06 ? 0xE2805A00 : 0xE2805A06);
			Process::Patch(infinite2.addr, *(u32 *)infinite2.addr == 0xE2805A06 ? 0xE2805A00 : 0xE2805A06);
			itemsettings(entry);
			return;
		}
			
		Process::Patch(settings[op], *(u32 *)settings[op] == settingsvalue[0][op] ? settingsvalue[1][op] : settingsvalue[0][op]);
		itemsettings(entry);
	}

	static bool IsMenuPatchOpen = false;
	static u8 CurrentMenu = 0xFF;
	const u8 Menus[8] = { 0x2E, 0x37, 0x38, 0x3D, 0x79, 0x89, 0x00, 0xFF };

	void Callback_MenuPatch(void) {
		if(Inventory::GetCurrent() == CurrentMenu && !IsMenuPatchOpen) 
			IsMenuPatchOpen = true;
		
		if(Inventory::GetCurrent() != CurrentMenu && IsMenuPatchOpen) {
			Animation::Idle();
			IsMenuPatchOpen = false;
			PluginMenu *menu = PluginMenu::GetRunningInstance();
			*menu -= Callback_MenuPatch; //delete itself
		}
	}

	void Hook_MenuPatch(void) {
		GameHelper::OpenMenu(CurrentMenu);
		PluginMenu *menu = PluginMenu::GetRunningInstance();
		*menu += Callback_MenuPatch;
	}
	
//Menu Changer
	void MenuChanger(MenuEntry *entry) {
		static Hook hook;
		
		std::vector<std::string> menuopt = {
			Language->Get("VECTOR_SAVEMENU_DATETIME"),
			Language->Get("VECTOR_SAVEMENU_BELLPOINT_DEPO"),
			Language->Get("VECTOR_SAVEMENU_BELLPOINT_WITHDRAW"),
			Language->Get("VECTOR_SAVEMENU_LOCKER"),
			Language->Get("VECTOR_SAVEMENU_TOWNTUNE"),
			Language->Get("VECTOR_SAVEMENU_HOUSESTORAGE"),
			Language->Get("VECTOR_SAVEMENU_CUSTOM"),
			Language->Get("VECTOR_DISABLE")
		};

		bool IsON;
		
		for(int i = 0; i < 6; ++i) { 
			IsON = CurrentMenu == Menus[i];
			menuopt[i] = (IsON ? Color(pGreen) :  Color(pRed)) << menuopt[i];
		}
		
		Keyboard optKb(Language->Get("KEY_CHOOSE_OPTION"), menuopt);

		int dChoice = optKb.Open();
		if(dChoice < 0)
			return;

		hook.Initialize(Code::nosave.addr + 8, (u32)Hook_MenuPatch);
		hook.SetFlags(USE_LR_TO_RETURN);

	//If Custom Menu is chosen
		if(dChoice == 6) {
			if(Wrap::KB<u8>(Language->Get("SAVE_MENU_CHANGER_ENTER_ID"), true, 2, CurrentMenu, 0)) {
				if(IDList::MenuValid(CurrentMenu))
					hook.Enable();
				else {
					hook.Disable();
					MessageBox(Language->Get("INVALID_ID")).SetClear(ClearScreen::Top)();
				}
			}
			return;
		}
		
		CurrentMenu = Menus[dChoice];
		if(CurrentMenu == 0xFF) {
			hook.Disable();
			return;
		}
		hook.Enable();
		MenuChanger(entry);
	}

	void GetCustomView(Keyboard& keyboard, KeyboardEvent& event) {
		if(event.type != KeyboardEvent::SelectionChanged)
            return;

		int index = event.selectedIndex;

		std::vector<std::string> f_file, f_Dir, f_all;
		std::vector<bool> isDir;
		File file;

		if(Wrap::restoreDIR.ListDirectories(f_Dir) == Directory::OPResult::NOT_OPEN)
			return;

		if(Wrap::restoreDIR.ListFiles(f_file, ".inv") == Directory::OPResult::NOT_OPEN) 
			return;

		if(f_Dir.empty() && f_file.empty())
			return;

		for(const std::string& str : f_Dir) {
			f_all.push_back(str);
			isDir.push_back(true);
		}

		for(const std::string& str : f_file) {
			f_all.push_back(str);
			isDir.push_back(false);
		}

		if(index == -1)
			return;

		std::string& input = keyboard.GetMessage();
		input.clear();

	//if directory return
		if(isDir[index])
			return;

		if(Wrap::restoreDIR.OpenFile(file, f_all[index], File::READ) != 0) 
			return; //error opening file

		std::string Sets[16];
		Item SetItem[16];
		std::vector<Item> OnlyItem;
		file.Read(&SetItem, sizeof(SetItem));

		for(int i = 0; i < 16; ++i) {
			if(SetItem[i].ID != 0x7FFE)
				OnlyItem.push_back(SetItem[i]);
		}

		for(int i = 0; i < 11; ++i) {
			if(i >= OnlyItem.size())
				return;

			std::string str = "";
			IDList::GetSeedName(OnlyItem[i], str);
			Sets[i] = str;

			input += Color(0x40FF40FF) << Utils::Format("%08X | ", OnlyItem[i]) << Color(0xFFFDD0FF) << Sets[i] << "\n";
		}
		input += "etc...";
		file.Flush();
		file.Close();
	}
//Get Set
	void getset(MenuEntry *entry) { 
		ACNL_Player *player = Player::GetSaveData();

		if(!player) {
			MessageBox(Language->Get("SAVE_PLAYER_NO")).SetClear(ClearScreen::Top)();
			return;
		}
	
		static const std::vector<std::string> setopt = {
			Language->Get("VECTOR_GETSET_FURN"),
			Language->Get("VECTOR_GETSET_CUSTOM"),
		};
	
		static const std::vector<std::string> custinvopt = {
			Language->Get("VECTOR_GETSET_CUSTOM_BACKUP"),
			Language->Get("VECTOR_GETSET_CUSTOM_RESTORE"),
			Language->Get("FILE_DELETE"),  
		};

		WrapLoc LocInv = { (u32 *)player->Inventory, sizeof(player->Inventory) };
		WrapLoc LocLock = { (u32 *)player->InventoryItemLocks, sizeof(player->InventoryItemLocks) };

		Keyboard optKb(Language->Get("KEY_CHOOSE_OPTION"), setopt);

		switch(optKb.Open()) {
			default: return;
			case 0: {
				Wrap::Restore(PATH_PRESET, ".inv", Language->Get("GET_SET_RESTORE"), GetCustomView, false, &LocInv, &LocLock, nullptr); 
				Inventory::ReloadIcons();
			} return;

			case 1: {
				optKb.Populate(custinvopt);

				switch(optKb.Open()) {
					default: return;
					case 0: {
						std::string filename = "";
						Keyboard KB(Language->Get("GET_SET_DUMP"));
						if(KB.Open(filename) == -1)
							return;

						Wrap::Dump(Utils::Format(PATH_ITEMSET, regionName.c_str()), filename, ".inv", &LocInv, &LocLock, nullptr);
					} return;
					
					case 1: {			
						Wrap::Restore(Utils::Format(PATH_ITEMSET, regionName.c_str()), ".inv", Language->Get("GET_SET_RESTORE"), GetCustomView, true, &LocInv, &LocLock, nullptr); 
						Inventory::ReloadIcons();
					} return;
					
					case 2: 
						Wrap::Delete(Utils::Format(PATH_ITEMSET, regionName.c_str()), ".inv");
					return;
				}
			}
		}
	}

	const bool ShowStackDebug = false;

	void StackItem(Item *item) {
		Item_Category cat = GameHelper::GetItemCategory(*item);
		u8 matchSlot = 0;
		Item match;
		Item empty = Item{0x7FFE, 0x8000};

		// Stack Fruit
		if(cat == Item_Category::Fruits || cat == Item_Category::PerfectFruits
		|| cat == Item_Category::FruitBaskets || cat == Item_Category::PerfectFruitBaskets) {
			while (matchSlot < 16) {
				Inventory::ReadSlot(matchSlot, match);

				if(match.ID == item->ID) {
					//converts single fruit to fruit basket with flag 1 (aka two fruits in basket)
					if(cat == Item_Category::Fruits || cat == Item_Category::PerfectFruits) {
						Inventory::WriteSlot(matchSlot, ((item->ID + 0x17) + (1 << 16)));
						*item = empty;
						OSD::NotifyDebug(Utils::Format("Made it into a basket in slot: %d", matchSlot), ShowStackDebug);
						return;
					}
					//combines fruit baskets
					else if(match.Flags < 8) {
						u16 flagMatch = match.Flags;
						u16 flagNew = item->Flags;

						if((flagMatch + flagNew + 1) <= 8) { //as flag "1" is 2 fruits
							OSD::NotifyDebug(Utils::Format("Slot: %2X || Flag Sum was below or 8", matchSlot), ShowStackDebug);
							Inventory::WriteSlot(matchSlot, Item(item->ID, flagMatch + flagNew + 1)); //adds new added flag to fruit basket
							*item = empty;
							return;
						}
						else {
							int diff = 8 - flagMatch;
							Inventory::WriteSlot(matchSlot, Item(item->ID, flagMatch + diff)); //adds new added flag to fruit basket
							
							flagNew -= diff;
							if(flagNew >= 1) {
								item->Flags = flagNew; //adds new added flag to fruit basket
								OSD::NotifyDebug(Utils::Format("Slot: %2X || Added %2X to the basket", matchSlot, diff), ShowStackDebug);
								continue; // Look for more baskets to combine with
							}
							else if (flagNew == 0) {
								item->ID -= 0x17; //revert to a single fruit
								item->Flags = 0;
								OSD::NotifyDebug(Utils::Format("Slot: %2X || Added %2X to the basket", matchSlot, diff), ShowStackDebug);
								return;
							}
						}
					}
				}
				//Basket Check
				else if((cat == Item_Category::Fruits || cat == Item_Category::PerfectFruits) && (match.ID == item->ID + 0x17)) {
					//If basket is not full
					if(match.Flags < 8) {
						//adds one to the flag of the fruit basket
						match.Flags += 1;
						Inventory::WriteSlot(matchSlot, match);
						*item = empty;
						OSD::NotifyDebug(Utils::Format("Added it into the basket in slot: %d", matchSlot), ShowStackDebug);
						return;
					}
				}
				matchSlot++;
			}
		}
		// Stack Bells
		else if(cat == Item_Category::Bells) {
			u32 itemValue = GameHelper::GetItemValue(*item);
			u32 newValue = itemValue;
			OSD::NotifyDebug(Utils::Format("Picked up: %d", newValue), ShowStackDebug);
			/*
			// First add as much to the wallet as possible
			ACNL_Player *player = Player::GetSaveData();
			int pocketMoney = GameHelper::DecryptValue(&player->PocketMoney);
			OSD::NotifyDebug(Utils::Format("Wallet: %d", pocketMoney), ShowStackDebug);
			
			pocketMoney += newValue;
			newValue = pocketMoney > 99999 ? itemValue < 1000 ? (pocketMoney / 100 * 100) - 99900 : (pocketMoney / 1000 * 1000) - 99000 : 0;
			pocketMoney -= newValue;
			GameHelper::EncryptValue(&player->PocketMoney, pocketMoney);

			OSD::NotifyDebug(Utils::Format("Added %d to the wallet (%d total).", itemValue - newValue, pocketMoney), ShowStackDebug);*/
			
			// Next try to stack with existing pocket money
			if(newValue > 0) {
				Item match;
				u8 matchSlot;

				while (matchSlot < 16 && newValue > 0) {
					Inventory::ReadSlot(matchSlot, match);
					if(GameHelper::GetItemCategory(match) != Item_Category::Bells){
						matchSlot++;
						continue;
					}

					u32 matchValue = GameHelper::GetItemValue(match);
					u32 newMatchValue = matchValue + newValue;

					if (newValue < 1000 && matchValue < 1000) {
						newValue = newMatchValue > 1000 ? newMatchValue - 1000 : 0;
						if(newMatchValue >= 1000){
							Inventory::WriteSlot(matchSlot, Item{0x20b5, 0});
							OSD::NotifyDebug(Utils::Format("Added %d to the bag in slot %d.", newMatchValue - matchValue, matchSlot), ShowStackDebug);
						}
						else {
							Inventory::WriteSlot(matchSlot, GameHelper::GetBellsByValue(newMatchValue));
							OSD::NotifyDebug(Utils::Format("Added %d to the bag in slot %d.", newMatchValue - matchValue, matchSlot), ShowStackDebug);
						}
					}
					else if (newValue >= 1000 && matchValue >= 1000) {
						newValue = newMatchValue > 99000 ? newMatchValue - 99000 : 0;
						if(newMatchValue - matchValue > 0) {
							Inventory::WriteSlot(matchSlot, GameHelper::GetBellsByValue(newMatchValue));
							OSD::NotifyDebug(Utils::Format("Added %d to the bag in slot %d.", newMatchValue - matchValue, matchSlot), ShowStackDebug);
						}
					}
					matchSlot++;
				}
			}
			
			// Leftover money goes to the pocket
			if(newValue > 0) {
				item->ID = GameHelper::GetBellsByValue(newValue).ID;
				OSD::NotifyDebug(Utils::Format("Added the leftover %d to the pocket.", newValue), ShowStackDebug);
			}
			else {
				*item = empty;
			}
		}
	}

	//Hook stack on pickup
	u32 StackOnPickup(u8 ID, Item *ItemToReplace, Item *ItemToPlace, Item *ItemToShow, u8 worldx, u8 worldy) {	
		if(IDList::ItemValid(*ItemToReplace, true)) {
			std::string itemName = "";

			if(IDList::GetSeedName(*ItemToReplace, itemName)) {
				OSD::NotifyDebug(Utils::Format("Pick up %d: %s (%04X)", ID, itemName.c_str(), ItemToReplace->ID), ShowStackDebug);
			}
			else {
				OSD::NotifyDebug(Utils::Format("Pick up %d: %08X", ID, ItemToReplace->ID), ShowStackDebug);
			}

			StackItem(ItemToReplace);

			// If our inventory is full, but we stacked the item, don't say it's full
			if(ID == 2 && ItemToReplace->ID == 0x7FFE) {
				ID = 1;
			}

			const HookContext &curr = HookContext::GetCurrent();
			static Address func(decodeARMBranch(curr.targetAddress, curr.overwrittenInstr));
			return func.Call<u32>(ID, ItemToReplace, ItemToPlace, ItemToShow, worldx, worldy, 0, 0, 0, 0, 0);
		}
		return 0xFFFFFFFF;
	}

	void autostack(MenuEntry *entry) {
		static Hook ASHook;	
		static const Address ASOffset(0x59A258, 0x599770, 0x5992A0, 0x5992A0, 0x598B90, 0x598B90, 0x598864, 0x598864);

		if(entry->WasJustActivated()) {
			ASHook.Initialize(ASOffset.addr, (u32)StackOnPickup);
			ASHook.SetFlags(USE_LR_TO_RETURN);
			ASHook.Enable();
		}
		
		if(!entry->IsActivated()) {
			ASHook.Disable();
		}
	}
}
