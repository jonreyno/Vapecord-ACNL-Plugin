#include "NonHacker.hpp"
#include "RegionCodes.hpp"
#include "Helpers/PlayerClass.hpp"
#include "Helpers/Animation.hpp"
#include "Helpers/Dropper.hpp"
#include "Helpers/IDList.hpp"
#include "Helpers/Game.hpp"
#include "Helpers/Inventory.hpp"
#include "TextFileParser.hpp"

namespace CTRPluginFramework {

	bool NonHacker::Accessible[5] = { true, true, true, true, true };

	NonHacker::NonHacker(u8 playerID) {
	//set player ID
		pID = playerID;
	//set player message
		Process::ReadString(GetPlayerMessageData() + 0x40C, pMessage, 0x64, StringFormat::Utf16);
	//set player name
		Process::ReadString(GetPlayerMessageData() + 0x3FA, pName, 0x16, StringFormat::Utf16);
	}

	NonHacker::~NonHacker() {
		
	}

	u32 NonHacker::GetPlayerMessageData() {
	    u32 PTR = *(u32 *)Code::chatpointer.addr; //0x94FD84
		PTR += 0x464; //33078FA0
		PTR += (0x530 * pID);
		return PTR;
	}

	std::string NonHacker::GetPlayerMessage() {
		return pMessage;
	}

	void NonHacker::ClearPlayerMessage() {
		std::memset((void *)(GetPlayerMessageData() + 0x40C), 0, 0x64);
	}

	std::string NonHacker::GetPlayerName() {
		return pName;
	}

	u8 NonHacker::GetPlayerIndex() {
		//return *(u8 *)(GetPlayerMessageData() + 0x3EC);
		return pID;
	}

	bool NonHacker::IsPlayerMessageOnScreen() {
		u8 Short;
		Process::Read8(GetPlayerMessageData() + 0x3F0, Short);
		return (Short != 0);
	}

	void ConvertToLowcase(std::string& str) {
		for(char& c : str)
			c = std::tolower(c);
	}

	void NonHacker::Animation() {
		if(!Accessible[0])
			return;

		u32 x, y;
		if(PlayerClass::GetInstance(pID)->GetWorldCoords(&x, &y)) {
			if(!IfForceAllowed && PlayerClass::GetInstance()->Offset() == PlayerClass::GetInstance(pID)->Offset())
				return;
		
			Animation::ExecuteAnimationWrapper(pID, animID, 0, 0, 0, 0, 0, x, y, true);
			Sleep(Seconds(2));
			Animation::ExecuteAnimationWrapper(pID, 6, 0, 0, 0, 0, 0, x, y, true);
			OSD::Notify("Player: " << pName);
			OSD::Notify(Utils::Format("Animation: %02X", animID)); 
		}
	}

	void NonHacker::Emotion() {
		if(!Accessible[1])
			return;

		u32 x, y;
		if(PlayerClass::GetInstance(pID)->GetWorldCoords(&x, &y)) {
			if(!IfForceAllowed && PlayerClass::GetInstance()->Offset() == PlayerClass::GetInstance(pID)->Offset())
				return;
			
			Animation::ExecuteAnimationWrapper(pID, 0xAF, 0, emotionID, 0, 0, 0, x, y, true);
			Sleep(Seconds(2));
			Animation::ExecuteAnimationWrapper(pID, 6, 0, 0, 0, 0, 0, x, y, true);
			OSD::Notify("Player: " << pName); 
			OSD::Notify(Utils::Format("Emotion: %02X", emotionID));
		}
	}

	void NonHacker::Snake() {
		if(!Accessible[2])
			return;

		u32 x, y;
		if(PlayerClass::GetInstance(pID)->GetWorldCoords(&x, &y)) {
			if(!IfForceAllowed && PlayerClass::GetInstance()->Offset() == PlayerClass::GetInstance(pID)->Offset())
				return;
			
			Animation::ExecuteAnimationWrapper(pID, 0xC5, 0, 0, snakeID, 0, 0, x, y, true);
			Sleep(Seconds(2));
			Animation::ExecuteAnimationWrapper(pID, 6, 0, 0, 0, 0, 0, x, y, true);
			OSD::Notify("Player: " << pName);
			OSD::Notify(Utils::Format("Snake: %03X", snakeID)); 
		}
	}

	void NonHacker::Music() {
		if(!Accessible[3])
			return;

		u32 x, y;
		if(PlayerClass::GetInstance(pID)->GetWorldCoords(&x, &y)) {
			if(!IfForceAllowed && PlayerClass::GetInstance()->Offset() == PlayerClass::GetInstance(pID)->Offset())
				return;
			
			Animation::ExecuteAnimationWrapper(pID, 0xC4, 0, 0, 0, musicID, 0, x, y, true);
			Sleep(Milliseconds(100));
			Animation::ExecuteAnimationWrapper(pID, 6, 0, 0, 0, 0, 0, x, y, true);
			OSD::Notify("Player: " << pName); 
			OSD::Notify(Utils::Format("Music: %03X", musicID)); 
		}
	}

	u32 FuseItem(u16 FlagID, u16 ItemID) {
		return (FlagID << 16) + ItemID;
	}

	void NonHacker::Item() {
		if(!Accessible[4])
			return;

		u32 x, y;
		if(PlayerClass::GetInstance(pID)->GetWorldCoords(&x, &y)) {	
			u32 Item = FuseItem(flagID, itemID);
			Dropper::PlaceItemWrapper(0xA, 0xFFFFFFFF, &Item, &Item, x, y, 0, 0, 0, 0, 0, 0x56, 0xA5, false);
			OSD::Notify("Player: " << pName); 
			OSD::Notify(Utils::Format("Item: %08X", Item));
		}	
	}

	void NonHackerCommands(u8 pID) {
		NonHacker nHack(pID);

		if(!nHack.IsPlayerMessageOnScreen()) 
			return;

		std::string PlayerText = nHack.GetPlayerMessage();

		if(PlayerText.empty())
			return;

		PlayerText.resize(25, ' ');

	//command
		std::string Command = PlayerText.substr(0, 2);
		ConvertToLowcase(Command);
	//special case command (flag)
		std::string SPCommand = PlayerText.substr(6, 2);
		ConvertToLowcase(SPCommand);
	//ID's
		std::string ID_8Bit = PlayerText.substr(2, 2);
		std::string ID_12Bit = PlayerText.substr(2, 3);
		std::string ID_16Bit = PlayerText.substr(2, 4);

	//special case ID (flag)
		std::string SPID_16Bit = PlayerText.substr(8, 4);

	//Item Name
		std::string ItemName = PlayerText.substr(2, 23);

		if(Command == "a:") {
			nHack.animID = StringToHex<u8>(ID_8Bit, 6); //sets animation
			if(IDList::AnimationValid(nHack.animID)) {
				nHack.Animation();
			}
			else return;
		}

		else if(Command == "e:") {
			nHack.emotionID = StringToHex<u8>(ID_8Bit, 1); //sets emotion
			if(IDList::EmotionValid(nHack.emotionID)) {
				nHack.Emotion();
			}
			else return;
		}

		else if(Command == "s:") {
			nHack.snakeID = StringToHex<u16>(ID_12Bit, 1); //sets snake
			if(IDList::SnakeValid(nHack.snakeID)) {
				nHack.Snake();
			}
			else return;
		}

		else if(Command == "m:") {
			nHack.musicID = StringToHex<u16>(ID_12Bit, 0x660); //sets music
			if(IDList::MusicValid(nHack.musicID)) {
				nHack.Music();
			}
			else return;
		}

		else if(Command == "i:") {
			nHack.itemID = StringToHex<u16>(ID_16Bit, 0x2001); //sets item
			if(IDList::ItemValid(nHack.itemID)) {
				if(SPCommand == "f:") 
					nHack.flagID = StringToHex<u16>(SPID_16Bit, 0); //sets flag

				nHack.Item();
			}
			else return;
		}

		else if(Command == "n:") {
			if(!ItemFileExists) 
				return;

			ConvertToLowcase(ItemName);

			Trim(ItemName);

			Item match;
			int res = ItemSearch(ItemName, match);
			if(res == 0) //no item found
				return;

			int pos = 0;
		//if exact name was found
			for(int i = 0; i < res; ++i) {
				if(match.Name[i] == ItemName) {
					pos = i;
					break;
				}
			}

			nHack.itemID = match.ID[pos]; //sets item
			if(!IDList::ItemValid(nHack.itemID)) //should always be true if orig file is used
				return;

			nHack.Item();
		}

		else 
			return;

		nHack.ClearPlayerMessage();
	}
	
	void NonHackerCallBack(void) {
		for(int i = 0; i < 4; ++i) {
			NonHackerCommands(i);
		}
	}
}