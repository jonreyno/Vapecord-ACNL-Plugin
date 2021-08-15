#include <CTRPluginFramework.hpp>
#include "cheats.hpp"
#include "RegionCodes.hpp"
#include "TextFileParser.hpp"

#define RANGE(X, START, END)	(X >= START && X <= END)

namespace CTRPluginFramework {
	bool GameKeyboard::Write(const std::string& str) {
		if(!GameKeyboard::IsOpen())
			return false;

		if(*(u32 *)Code::ChatPoint == 0)
			return false;

		u16 buffer[str.size() + 1] = { 0 };
		utf8_to_utf16(buffer, reinterpret_cast<const u8*>(str.data()), str.size());

		static const u32 WriteFunc = Region::AutoRegion(0x5231D0, 0x522B24, 0x522218, 0x522218, 0x521B28, 0x521B28, 0x521514, 0x521514);	

		const u16* hex = (const u16 *)buffer;
		u8 i = *(u8 *)(*(u32 *)(Code::ChatPoint) + 0x14);
		static FUNCT func(WriteFunc);

		while(*hex) {
			u16 character = (u16)*hex++;
			func.Call<void>(*(u32 *)Code::ChatPoint, character, i, 0, 0);
			i++;
		}

		return true;
	}

	bool GameKeyboard::CopySelected(std::string& res) {
		if(!GameKeyboard::IsOpen())
			return false;

		bool IsSelected = *(bool *)(*(u32 *)(Code::ChatPoint) + 0x20); //95F11C
		if(!IsSelected) 
			return false;

		u32	ChatText = *(u32 *)(*(u32 *)(Code::ChatPoint) + 0x10);
		u8	CurrentPos = *(u8 *)(*(u32 *)(Code::ChatPoint) + 0x14);
		u8	SelectStart = *(u8 *)(*(u32 *)(Code::ChatPoint) + 0x1C);

		if(CurrentPos < SelectStart) 
			Process::ReadString(ChatText + (CurrentPos * 2), res, (SelectStart * 2) - (CurrentPos * 2), StringFormat::Utf16);

		if(CurrentPos > SelectStart) 
			Process::ReadString(ChatText + (SelectStart * 2), res, (CurrentPos * 2) - (SelectStart * 2), StringFormat::Utf16);

		return true;
	}

	bool GameKeyboard::DeleteSelected(void) {
		if(!GameKeyboard::IsOpen())
			return false;

		static const u32 DeleteFunc = Region::AutoRegion(0x523780, 0x5230D4, 0x5227C8, 0x5227C8, 0x5220D4, 0x5220D4, 0x521DCC, 0x521DCC);

		bool IsSelected = *(bool *)(*(u32 *)(Code::ChatPoint) + 0x20);
		if(!IsSelected)
			return false;

		u8	CurrentPos = *(u8 *)(*(u32 *)(Code::ChatPoint) + 0x14);
		u8	SelectStart = *(u8 *)(*(u32 *)(Code::ChatPoint) + 0x1C);

		static FUNCT func(DeleteFunc);

		if(CurrentPos < SelectStart) {
			func.Call<void>(*(u32 *)Code::ChatPoint, CurrentPos, SelectStart - CurrentPos);
		}
		if(CurrentPos > SelectStart) {
			func.Call<void>(*(u32 *)Code::ChatPoint, SelectStart, CurrentPos - SelectStart);
		}

		*(bool *)(*(u32 *)(Code::ChatPoint) + 0x20) = 0; //unselects
		return true;
	}

//If keyboard is opened 32DE75BC
	bool GameKeyboard::IsOpen() {
		static const u32 KeyBool = Region::AutoRegion(0x523F48, 0x52389C, 0x522F90, 0x522F90, 0x52287C, 0x52287C, 0x52256C, 0x52256C); 
		static FUNCT func(KeyBool);	
		bool res = func.Call<bool>();
		if(res) {
			if(*(u32 *)(*(u32 *)(*(u32 *)(Code::ChatPoint) + 4) + 0x50) == 0)
				return false;
		}
		return res;
	}

	bool GameKeyboard::IsEmpty() {
		if(!GameKeyboard::IsOpen())
			return true;

		return *(bool *)(*(u32 *)(*(u32 *)(Code::ChatPoint) + 0x10) + 0x98 + 0x11B1) != true;
	}

	bool GameKeyboard::Copy(std::string& str, int pos, int lenght) {
		if(!GameKeyboard::IsOpen())
			return false;

		if(GameKeyboard::IsEmpty())
			return false;

		u32 ChatText = *(u32 *)(*(u32 *)(Code::ChatPoint) + 0x10);

		return Process::ReadString(ChatText + pos, str, lenght, StringFormat::Utf16);
	}

	bool GameKeyboard::ConvertToItemID(std::string& str, u32 &ItemID) {
		for(char& c : str)
			c = std::tolower(c);

		const u8* hex = (const u8*)str.c_str();
		while(*hex) {
			u8 byte = (u8)*hex++;

			if(RANGE(byte, '0', '9'))
				byte = byte - '0';
			else if(RANGE(byte, 'a','f'))
				byte = byte - 'a' + 10;
			else 
				return false; //Incorrect char

			ItemID = (ItemID << 4) | (byte & 0xF);
		}
		return true;
	}

	void SetCustomOnlineStack(u32 address, const std::string& str) {
	//TODO: port addresses
		static const u32 point = Region::AutoRegion(0x90B38C, 0, 0, 0, 0, 0, 0, 0);

		Process::Write32(address, address + 0x6C);
		Process::Write32(address + 0x6C, point);
		Process::Write32(address + 0x70, address + 0x84);
		Process::Write32(address + 0x74, 0x41);
		Process::Write32(address + 0x78, 4);
		Process::Write32(address + 0x7C, 0xFFFFFFFF);
		Process::Write32(address + 0x80, 0xFFFFFFFF);
		Process::WriteString(address + 0x84, str, 0x30, StringFormat::Utf16);
	}

	void GameKeyboard::SendMessage(const std::string& str) {
	//TODO: port addresses
		static const u32 func1 = Region::AutoRegion(0x81F9D0, 0, 0, 0, 0, 0, 0, 0);
		static const u32 func2 = Region::AutoRegion(0x56DE5C, 0, 0, 0, 0, 0, 0, 0);
		static const u32 func3 = Region::AutoRegion(0x5FD774, 0, 0, 0, 0, 0, 0, 0);
		static const u32 func4 = Region::AutoRegion(0x5E3920, 0, 0, 0, 0, 0, 0, 0);
		static const u32 func5 = Region::AutoRegion(0x300838, 0, 0, 0, 0, 0, 0, 0);
		static const u32 func6 = Region::AutoRegion(0x625C04, 0, 0, 0, 0, 0, 0, 0);

		u32 msgData = *(u32 *)Code::chatpointer;
		if(msgData == 0)
			return;

		u8 pIndex = GameHelper::GetOnlinePlayerIndex();

		u32 Stack = 0xA00000;
		u32 onlineStack = 0xA00030;

		SetCustomOnlineStack(onlineStack, str);

		if(*(u8 *)(msgData + 0x858) != 0) {
			*(u16 *)(msgData + 0x854) = 0;
			*(u8 *)(msgData + 0x85A) = 0;
		}

		u32 var = FUNCT(func1).Call<u32>(Stack); //Makes temporary "Stack" ready to be written to
		FUNCT(func2).Call<void>(var, pIndex); //Gets Player Name Data and writes it to temporary "Stack"

		FUNCT(func3).Call<void>((int *)(msgData + 0x47C), var); //Finished Player Name Data in Chat Box Data (?)

		Process::WriteString(msgData + 0x870, str, 0x30, StringFormat::Utf16);

		*(u8 *)(msgData + 0x84C) = 8;
		*(u8 *)(msgData + 0x850) = pIndex;

		FUNCT(func4).Call<void>(msgData + 0x6A0, msgData + 0x47C); //Actually writes Player Name Data to Chat Box Data

		*(u32 *)(msgData + 0x784) = (msgData + pIndex * 0x28 + 0x2AC);
		*(u16 *)(msgData + 0x856) = 300;
		*(u8 *)(msgData + 0x85A) = 1;
		*(u8 *)(msgData + 0x858) = 1;

		u32 val = FUNCT(func5).Call<u32>(); //Checks if playing online
		if(val != 0) 
			FUNCT(func6).Call<void>(0x8C + pIndex, onlineStack, 1); //Sends temporary Online "Stack" to others

	//Clears temporary Player Name "Stack"
		std::memset((void *)Stack, 0, 0x30);
	//Clears temporary Online "Stack"
		Process::Write32(onlineStack, 0);
		std::memset((void *)(onlineStack + 0x6C), 0, 0x30);
	}
}