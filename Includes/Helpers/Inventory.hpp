#ifndef INVENTORY_HPP
#define INVENTORY_HPP

#include <CTRPluginFramework.hpp>
#include "Helpers/GameStructs.hpp"

#define IsItemSlot(X)	(X >= 0x00 && X <= 0x0F)
#define IsMailSlot(X)	(X >= 0x14 && X <= 0x1D)

namespace CTRPluginFramework {
	struct ItemVec {
		std::vector<std::string> Name;
		std::vector<Item> ID;
	};

	typedef std::vector<std::pair<u16, std::string>> CustomItemOptionVec;

	struct CustomItemPartVec {
		std::vector<std::string> Name;
		std::vector<CustomItemOptionVec> Options;
	};

	struct CustomItemVec {
		std::vector<Item> ID;
		std::vector<CustomItemPartVec> CustomParts;
	};
	
	extern ItemVec* ItemList;
	extern int ItemFileLength;
	extern bool ItemFileExists;

	extern CustomItemVec* CustomItemList;
	extern int CustomItemFileLength;
	extern bool CustomItemFileExists;

	using slotReader = void(*)(u32, u8);

	namespace Inventory {
		u32 		GetCurrentItemData(int i = 0);
		bool 		GetNextItem(Item itemID, u8 &slot, bool checkFlags = true);
		bool 		GetNextClosetItem(Item itemID, u8 &slot);
		bool 		GetSelectedSlot(u8& slot);
		bool 		GetHoveredSlot(u8& slot);
		u16			GetAddData();
		bool		Opened();
		u8			GetCurrent();

		bool 		WriteSlot(int slot, Item item, u8 lock = 0);
		bool 		ReadSlot(int slot, Item& item);
		void		ReloadIcons(void);
	}

	void ReserveItemData(ItemVec* out);
	void ReserveCustomItemData(CustomItemVec* out);
	int ItemSearch(const std::string& match, ItemVec& out);
	std::string ItemIDSearch(Item ItemID);
	int GetCustomItemIndex(Item ItemID);
	bool GetCustomOption(int customItemIndex, bool isPart1, u16& optionIndex, int increment, std::pair<u16, std::string>& option);
}
#endif
