#include "cheats.hpp"
#include "Helpers/PlayerClass.hpp"
#include "TextFileParser.hpp"
#include "Helpers/Player.hpp"
#include "Helpers/IDList.hpp"
#include "Helpers/Wrapper.hpp"
#include "Helpers/Game.hpp"
#include "Helpers/Address.hpp"
#include "Helpers/GameStructs.hpp"
#include "Helpers/Converters.hpp"
#include "Color.h"
#include "Files.h"

namespace CTRPluginFramework {
//Name Changer | Player specific save code
	void NameChanger(MenuEntry* entry) {
		if(Player::GetSaveOffset(4) == 0) {
			Sleep(Milliseconds(100));
			MessageBox(Language->Get("SAVE_PLAYER_NO")).SetClear(ClearScreen::Top)();
			return;
		}
		
		Keyboard keyboard(Language->Get("NAME_CHANGER_ENTER_NAME"));
		std::string input = "";
		keyboard.SetMaxLength(8);

		Sleep(Milliseconds(100));
		if(keyboard.Open(input) < 0) 
			return;

		PlayerName::Set(input);
	}

//Player Appearance Changer	
	void playermod(MenuEntry *entry) {
		if(Player::GetSaveOffset(4) == 0) {
			Sleep(Milliseconds(100));
			MessageBox(Language->Get("SAVE_PLAYER_NO")).SetClear(ClearScreen::Both)();
			return;
		}	

		static const u8 ValidID[5][2] = {
			{ 0x00, 0x21 }, { 0x00, 0x0F }, 
			{ 0x00, 0x0B }, { 0x00, 0x05 }
		};

		static const u16 ValidID2[6][2] = {
			{ 0x280B, 0x28F3 },
			{ 0x28F5, 0x295B },
			{ 0x2493, 0x26F5 },
			{ 0x26F8, 0x2776 },
			{ 0x2777, 0x279E },
			{ 0x279F, 0x27E5 }
		};
		
		static const std::vector<std::string> playeropt = {
			Language->Get("VECTOR_PLAYER_MOD_HAIR_STYLE"),
			Language->Get("VECTOR_PLAYER_MOD_HAIR_COLOR"),
			Language->Get("VECTOR_PLAYER_MOD_EYE_STYLE"),
			Language->Get("VECTOR_PLAYER_MOD_EYE_COLOR"),
			
			Language->Get("VECTOR_PLAYER_MOD_GENDER"),
			Language->Get("VECTOR_PLAYER_MOD_TAN"),
			Language->Get("VECTOR_PLAYER_MOD_OUTFIT")
		};
		
		static const std::vector<std::string> genderopt = {
			Language->Get("VECTOR_PLAYER_MOD_GENDER_MALE"), 
			Language->Get("VECTOR_PLAYER_MOD_GENDER_FEMALE"),
		};
		
		static const std::vector<std::string> tanopt = {
			Language->Get("VECTOR_PLAYER_MOD_TAN_DARK"),
			Language->Get("VECTOR_PLAYER_MOD_TAN_TAN"),
			Language->Get("VECTOR_PLAYER_MOD_TAN_FAIR"),
			Language->Get("VECTOR_PLAYER_MOD_TAN_CUSTOM"),
		};
		
		static const std::vector<std::string> outfitplayeropt = {
			Language->Get("VECTOR_OUTFIT_HEADGEAR"), 
			Language->Get("VECTOR_OUTFIT_GLASSES"), 
			Language->Get("VECTOR_OUTFIT_SHIRT"), 
			Language->Get("VECTOR_OUTFIT_PANTS"), 
			Language->Get("VECTOR_OUTFIT_SOCKS"), 
			Language->Get("VECTOR_OUTFIT_SHOES")
		};

		ACNL_Player *player = Player::GetData();

		u8 ID = 0;
		u16 item = 0;

		Keyboard optKb(Language->Get("KEY_CHOOSE_OPTION"), playeropt);
		
		Sleep(Milliseconds(100));
		s8 choice = optKb.Open();
		if(choice < 0)
			return;
			
	//Standard Face Appearance Change
		if(choice < 4) {
			KeyRange::Set({ ValidID[choice][0], ValidID[choice][1] });
			if(Wrap::KB<u8>(Language->Get("ENTER_ID") << Utils::Format("%02X -> %02X", ValidID[choice][0], ValidID[choice][1]), true, 2, ID, ID, ValidKeyboardCheck)) {
				switch(choice) {
					case 0: player->HairStyle = ID; goto update;
					case 1: player->HairColor = ID; goto update;
					case 2: player->Face = ID; goto update;
					case 3: player->EyeColor = ID; goto update;
				}
			}
		}
	//Gender Change
		if(choice == 4) {
			optKb.Populate(genderopt);

			Sleep(Milliseconds(100));
			s8 res = optKb.Open();
			if(res < 0)
				return;

			PlayerName::UpdateReference(4, "", res);
		}
				
		if(choice == 5) {
			optKb.Populate(tanopt);

			Sleep(Milliseconds(100));
			switch(optKb.Open()) {
				default: break;	
				case 0: player->Tan = 0xF; goto tanupdate;
				case 1: player->Tan = 0xA; goto tanupdate;
				case 2: player->Tan = 0; goto tanupdate;
				case 3: {
					u8 val = 0;
					if(Wrap::KB<u8>(Language->Get("PLAYER_APPEARANCE_TAN_LEVEL") << "0x00 -> 0x0F", false, 2, val, 0)) 
						player->Tan = val;
				} goto tanupdate;
			}
		}
			
		if(choice == 6) {
			optKb.Populate(outfitplayeropt);

			Sleep(Milliseconds(100));
			s8 res = optKb.Open();
			if(res < 0)
				return;

			KeyRange::Set({ ValidID2[res][0], ValidID2[res][1] });
			if(Wrap::KB<u16>(Language->Get("ENTER_ID") << Utils::Format("%04X -> %04X", ValidID2[res][0], ValidID2[res][1]), true, 4, item, item, ValidKeyboardCheck)) {
				switch(res) {
					case 0: player->Hat.ID = item; break;
					case 1: player->Accessory.ID = item; break;
					case 2: player->TopWear.ID = item; break;
					case 3: player->BottomWear.ID = item; break;
					case 4: player->Socks.ID = item; break;
					case 5: player->Shoes.ID = item; break;
				}

				Player::WriteOutfit(GameHelper::GetOnlinePlayerIndex(), player->Hat, player->Accessory, player->TopWear, player->BottomWear, player->Socks, player->Shoes);
			}
		}

		update:
			Player::UpdateStyle();
		return;
		
		tanupdate:
			Player::UpdateTan();
	}

//Random Outfit
	void randomoutfit(MenuEntry *entry) {
		ACNL_Player *player = Player::GetData();

		if(!player) {
			Sleep(Milliseconds(100));
			MessageBox(Language->Get("SAVE_PLAYER_NO")).SetClear(ClearScreen::Both)();
			return;
		}
	
		static const std::vector<std::string> randomopt = {
			Language->Get("VECTOR_RANDOM_OUTFIT"),
			Language->Get("VECTOR_RANDOM_PLAYER")
		};	

		Keyboard randkb(Language->Get("KEY_RANDOMIZE_PLAYER"), randomopt);
		Sleep(Milliseconds(100));
		switch(randkb.Open()) {
			default: break;			
			case 0: 
				Player::WriteOutfit(GameHelper::GetOnlinePlayerIndex(), U32_TO_ITEM(Utils::Random(0x280B, 0x28F3)), 
																		U32_TO_ITEM(Utils::Random(0x28F5, 0x295B)), 
																		U32_TO_ITEM(Utils::Random(0x2493, 0x26F5)), 
																		U32_TO_ITEM(Utils::Random(0x26F8, 0x2776)), 
																		U32_TO_ITEM(Utils::Random(0x2777, 0x279E)), 
																		U32_TO_ITEM(Utils::Random(0x279F, 0x27E5)));
			break;
			case 1: {
				player->HairStyle = Utils::Random(0, 0x21);
				player->HairColor = Utils::Random(0, 0xF);
				player->Face = Utils::Random(0, 4);
				player->EyeColor = Utils::Random(0, 4);
				player->Tan = Utils::Random(0, 0xF);
				
				player->Hat.ID = Utils::Random(0x280B, 0x28F3);
				player->Accessory.ID = Utils::Random(0x28F5, 0x295B);
				player->TopWear.ID = Utils::Random(0x2493, 0x26F5);
				player->BottomWear.ID = Utils::Random(0x26F8, 0x2776);
				player->Socks.ID = Utils::Random(0x2777, 0x279E);
				player->Shoes.ID = Utils::Random(0x279F, 0x27E5);

			//Reloads player style
				Player::UpdateStyle();
			} break;	
		}
	}

//Player Backup/Restore
	void playerbackup(MenuEntry *entry) {
		if(Player::GetSaveOffset(4) == 0) {
			Sleep(Milliseconds(100));
			MessageBox(Language->Get("SAVE_PLAYER_NO")).SetClear(ClearScreen::Both)();
			return;
		}
		
		static const std::vector<std::string> backopt = {
			Language->Get("VECTOR_RANDOM_BACKUP"),
			Language->Get("VECTOR_RANDOM_RESTORE"),
			Language->Get("FILE_DELETE"),  
		};

		Keyboard backkb(Language->Get("KEY_CHOOSE_OPTION"), backopt);
		Sleep(Milliseconds(100));
		switch(backkb.Open()) {
			default: break;		
			case 0: {
				std::string filename = "";
				Keyboard KB(Language->Get("RANDOM_PLAYER_DUMP"));

				Sleep(Milliseconds(100));
				if(KB.Open(filename) == -1)
					return;

				Wrap::Dump(Utils::Format(PATH_PLAYER, regionName.c_str()), filename, ".player", WrapLoc{ Player::GetSaveOffset(4), 0xA480 }, WrapLoc{ (u32)-1, (u32)-1 });
			} break;
			case 1: {
				Wrap::Restore(Utils::Format(PATH_PLAYER, regionName.c_str()), ".player", Language->Get("RANDOM_PLAYER_RESTORE"), nullptr, true, WrapLoc{ Player::GetSaveOffset(4), 0xA480 }, WrapLoc{ (u32)-1, (u32)-1 }); 
				Player::UpdateTan();
				Player::UpdateStyle();
			} break;	
			case 2: 
				Wrap::Delete(Utils::Format(PATH_PLAYER, regionName.c_str()), ".player");
			break;
		}
	}

//TPC Message Changer | Player specific save code
	void tpcmessage(MenuEntry* entry) {
		ACNL_Player *player = Player::GetData();
		if(!player) {
			Sleep(Milliseconds(100));
			MessageBox(Language->Get("SAVE_PLAYER_NO")).SetClear(ClearScreen::Top)();
			return;
		}
		
		std::string input = "";

		Keyboard KB(Language->Get("TPC_MESSAGE_ENTER_NAME"));
		KB.SetMaxLength(26);

		Sleep(Milliseconds(100));
		if(KB.Open(input) >= 0) {
			Convert::STR_TO_U16(input, player->TPCText);
		}
	}

//TPC Image Dumper | non player specific save code
	void tpc(MenuEntry *entry) {
		ACNL_Player *player = Player::GetData();
		if(!player) {
			Sleep(Milliseconds(100));
			MessageBox(Language->Get("SAVE_PLAYER_NO")).SetClear(ClearScreen::Top)();
			return;
		}

		static const std::vector<std::string> g_player = {
			Language->Get("VECTOR_PLAYER_1"), 
			Language->Get("VECTOR_PLAYER_2"), 
			Language->Get("VECTOR_PLAYER_3"), 
			Language->Get("VECTOR_PLAYER_4"), 
		};
		
		static const std::vector<std::string> tpcselectopt = {
			Language->Get("VECTOR_TPCDUMP_DUMP"),
			Language->Get("VECTOR_TPCDUMP_RESTORE"),
			Language->Get("FILE_DELETE"),  
		};
			
		Keyboard KB(Language->Get("KEY_CHOOSE_OPTION"), tpcselectopt);

		Sleep(Milliseconds(100));
		switch(KB.Open()) {
			default: break;
			
			case 0: {
				Keyboard PKB(Language->Get("KEY_SELECT_PLAYER"), g_player);

				Sleep(Milliseconds(100));
				s8 index = PKB.Open();
				if(index < 0)
					return;
					
				if(Player::GetSaveOffset(index) != 0) {
					std::string filename = "";
					Keyboard KB(Language->Get("TPC_DUMPER_NAME"));

					Sleep(Milliseconds(100));
					if(KB.Open(filename) < 0)
						return;

					Wrap::Dump(Utils::Format(PATH_TPC, regionName.c_str()), filename, ".jpg", WrapLoc{ Player::GetSaveOffset(index) + 0x5738, 0x1400 }, WrapLoc{ (u32)-1, (u32)-1 });
				}
			} break;
			
			case 1: 
				Wrap::Restore(Utils::Format(PATH_TPC, regionName.c_str()), ".jpg", Language->Get("TPC_DUMPER_RESTORE"), nullptr, true, WrapLoc{ Player::GetSaveOffset(4) + 0x5738, 0x1400 }, WrapLoc{ (u32)-1, (u32)-1 });
			break;
			
			case 2: 
				Wrap::Delete(Utils::Format(PATH_TPC, regionName.c_str()), ".jpg");
			break;
		}
	}

	u32 GetRealSlot(u8 slot, int pIndex) {
		ACNL_Player *player = Player::GetData(pIndex);
		return player->PatternOrder[slot];
	}
//get correct save for design
	u32 GetDesignSave(u8 slot, int pIndex) {
		return Player::GetDesign(GetRealSlot(slot, pIndex), pIndex);
	}
//dump designs | player specific save code
	void DesignDumper(MenuEntry *entry) {
		if(Player::GetSaveOffset(4) == 0) {
			Sleep(Milliseconds(100));
			MessageBox(Language->Get("SAVE_PLAYER_NO")).SetClear(ClearScreen::Top)();
			return;
		}
		
		std::vector<std::string> designslots;
		
		for(int i = 1; i <= 10; ++i)
			designslots.push_back(Utils::Format(Language->Get("VECTOR_DESIGN").c_str(), i));
		
		static const std::vector<std::string> designselect = {
			Language->Get("VECTOR_DESIGNDUMP_DUMP"), 
			Language->Get("VECTOR_DESIGNDUMP_RESTORE"), 
			Language->Get("FILE_DELETE"),  
		};
		
		Keyboard KB(Language->Get("KEY_CHOOSE_OPTION"), designselect);
		
		Sleep(Milliseconds(100));
		switch(KB.Open()) {
			default: break;
			
			case 0: { 
				Keyboard DKB(Language->Get("KEYBOARD_DESIGNDUMP"), designslots);
				
				Sleep(Milliseconds(100));
				int dSlot = DKB.Open();
				if(dSlot != -1) {
					std::string filename = "";
					Keyboard KB(Language->Get("DESIGN_DUMP_NAME"));

					Sleep(Milliseconds(100));
					if(KB.Open(filename) == -1)
						return;

					Wrap::Dump(Utils::Format(PATH_DESIGN, regionName.c_str()), filename, ".acnl", WrapLoc{ GetDesignSave(dSlot, 4), 0x26B }, WrapLoc{ (u32)-1, (u32)-1 });
				}
			} break;
			
			case 1: {
				Keyboard DKB(Language->Get("KEYBOARD_DESIGNDUMP"), designslots);
				
				Sleep(Milliseconds(100));
				int dSlot = DKB.Open();
				if(dSlot != -1) {
					Wrap::Restore(Utils::Format(PATH_DESIGN, regionName.c_str()), ".acnl", Language->Get("DESIGN_DUMP_RESTORE"), nullptr, true, WrapLoc{ GetDesignSave(dSlot, 4), 0x26B }, WrapLoc{ (u32)-1, (u32)-1 });  
					Player::ReloadDesign(GetRealSlot(dSlot, 4));
				}
			} break;
			
			case 2: 
				Wrap::Delete(Utils::Format(PATH_DESIGN, regionName.c_str()), ".acnl");
			break;
		}
	}
//Fill Emote List | player specific save code
	void emotelist(MenuEntry *entry) {
		ACNL_Player *player = Player::GetData();

		if(!player) {
			Sleep(Milliseconds(100));
			MessageBox(Language->Get("SAVE_PLAYER_NO")).SetClear(ClearScreen::Top)();
			return;
		}
		
		static const std::vector<std::string> emoteopt = {
			Language->Get("VECTOR_EMOTIONLIST_FILL_LIST"),
			Language->Get("VECTOR_EMOTIONLIST_FILL_EMOTION"),
			Language->Get("VECTOR_EMOTIONLIST_CLEAR_LIST"),
		};

		static Address emoticons(0x8902A4, 0x88F29C, 0x88F130, 0x88F130, 0x889550, 0x888550, 0x888500, 0x888500);
		Emoticons *gameEmotes = new Emoticons();
		gameEmotes = (Emoticons *)emoticons.addr;
		if(!gameEmotes)
			return;
		
		Keyboard KB(Language->Get("KEY_CHOOSE_OPTION"), emoteopt);
	
		Sleep(Milliseconds(100));
		switch(KB.Open()) {
			default: break;
			case 0: 
				player->Emotes = *gameEmotes;
			break; 
			case 1: {
				u8 emotion = 0; 
				Keyboard KB(Language->Get("EMOTION_LIST_TYPE_ID"));
				KB.IsHexadecimal(true);
				
				Sleep(Milliseconds(100));
				if(KB.Open(emotion) < 0)
					return;
				
				std::memset((void *)player->Emotes.emoticons, emotion, 0x28);
			} break;
			
			case 2: 
				std::memset((void *)player->Emotes.emoticons, 0, 0x28);
			break;
		}
	}
//Fill Enzyklopedia List | player specific save code
	void enzyklopedia(MenuEntry *entry) {
		ACNL_Player *player = Player::GetData();

		if(!player) {
			Sleep(Milliseconds(100));
			MessageBox(Language->Get("SAVE_PLAYER_NO")).SetClear(ClearScreen::Top)();
			return;
		}
		
		static const std::vector<std::string> enzyopt = {
			Language->Get("VECTOR_ENZY_FILL"),
			Language->Get("VECTOR_ENZY_CLEAR"),
		};

		static const Item_Categories EncyclopediaID[3] = { 
			Item_Categories::Bugs, Item_Categories::Fish, 
			Item_Categories::SeaCreatures
		};
		
		Keyboard KB(Language->Get("KEY_CHOOSE_OPTION"), enzyopt);
		
		Sleep(Milliseconds(100));
		switch(KB.Open()) {
			default: break;
			case 0:
				for(int i = 0; i < 3; ++i) 
					Player::SetUnlockableBitField(player, EncyclopediaID[i], true);

				for(int i = 0; i < 72; ++i) {
					player->EncyclopediaSizes.Insects[i] = Utils::Random(1, 0x3FFF);
					player->EncyclopediaSizes.Fish[i] = Utils::Random(1, 0x3FFF);

					if(i < 30)
						player->EncyclopediaSizes.SeaCreatures[i] = Utils::Random(1, 0x3FFF);
				}
			break;
			case 1: 
				for(int i = 0; i < 3; ++i) 
					Player::SetUnlockableBitField(player, EncyclopediaID[i], false);

				for(int i = 0; i < 72; ++i) {
					player->EncyclopediaSizes.Insects[i] = 0;
					player->EncyclopediaSizes.Fish[i] = 0;

					if(i < 30)
						player->EncyclopediaSizes.SeaCreatures[i] = 0;
				}
			break;
		} 
	}
//Change Dream Code | player specific save code
	void comodifier(MenuEntry *entry) {
		ACNL_Player *player = Player::GetData();

		if(!player) {
			Sleep(Milliseconds(100));
			MessageBox(Language->Get("SAVE_PLAYER_NO")).SetClear(ClearScreen::Top)();
			return;
		}

		Keyboard kb(Language->Get("DREAM_CODE_ENTER_ID"));
		kb.IsHexadecimal(true);
		kb.DisplayTopScreen = true;
		
		u16 part1, part2, part3;

		Sleep(Milliseconds(100));
		if(kb.Open(part1, 0) >= 0) {
			Sleep(Milliseconds(100));
			if(kb.Open(part2, 0) >= 0) {
				Sleep(Milliseconds(100));
				if(kb.Open(part3, 0) >= 0) {
					player->DreamCode.DCPart1 = (part2 << 16) + part3;
					player->DreamCode.DCPart2 = (part1 & 0xFF);
					player->DreamCode.DCPart3 = (part1 >> 8);

					player->DreamCode.HasDreamAddress = true;
				}
			}
		}	
	}
//Enable Unused Menu | player specific save code
	void debug1(MenuEntry *entry) {
		ACNL_Player *player = Player::GetData();

		if(!player) {
			Sleep(Milliseconds(100));
			MessageBox(Language->Get("SAVE_PLAYER_NO")).SetClear(ClearScreen::Top)();
			return;
		}
		
		std::vector<std::string> cmnOpt =  { "" };
		
		bool IsON = player->PlayerFlags.CanUseCensusMenu == 1;

		cmnOpt[0] = IsON ? (Color(pGreen) << Language->Get("VECTOR_ENABLED")) : (Color(pRed) << Language->Get("VECTOR_DISABLED"));
		
		Keyboard KB(Language->Get("KEY_CHOOSE_OPTION"), cmnOpt);

		Sleep(Milliseconds(100));
		s8 op = KB.Open();
		if(op < 0)
			return;

		player->PlayerFlags.CanUseCensusMenu = IsON ? 0 : 1;

		debug1(entry);
	}

//Fill Song List | player specific save code
	void FillSongs(MenuEntry *entry) {
		ACNL_Player *player = Player::GetData();

		if(!player) {
			Sleep(Milliseconds(100));
			MessageBox(Language->Get("SAVE_PLAYER_NO")).SetClear(ClearScreen::Top)();
			return;
		}
		
		static const std::vector<std::string> songopt = {
			Language->Get("VECTOR_ENZY_FILL"),
			Language->Get("VECTOR_ENZY_CLEAR"),
		};

		static const std::pair<u16, u16> Pairs = { 0x212B, 0x2186 };

		static Address calcBitField(0x2FF76C, 0, 0, 0, 0, 0, 0, 0);
		
		Keyboard optKb(Language->Get("KEY_CHOOSE_OPTION"), songopt);
		
		Sleep(Milliseconds(100));
		switch(optKb.Open()) {
			default: break;
			case 0: 
				for(u16 i = Pairs.first; i < Pairs.second; ++i) {
					int field = calcBitField.Call<int>(&i);
					player->AddedSongs[(field >> 5)] |= (1 << (field & 0x1F));
				}
			break;
			case 1:
				for(u16 i = Pairs.first; i < Pairs.second; ++i) {
					int field = calcBitField.Call<int>(&i);
					player->AddedSongs[(field >> 5)] &= ~(1 << (field & 0x1F));
				}
			break;
		}
	}

//Fill Catalog | player specific save code	
	void FillCatalog(MenuEntry *entry) {
		ACNL_Player *player = Player::GetData();

		if(!player) {
			Sleep(Milliseconds(100));
			MessageBox(Language->Get("SAVE_PLAYER_NO")).SetClear(ClearScreen::Top)();
			return;
		}
		
		static const std::vector<std::string> songopt = {
			Language->Get("VECTOR_ENZY_FILL"),
			Language->Get("VECTOR_ENZY_CLEAR"),
		};	

		static const Item_Categories CatalogID[15] = {
			Item_Categories::Wallpaper, Item_Categories::Carpets,
			Item_Categories::Furniture, Item_Categories::Shirts,
			Item_Categories::Dresses, Item_Categories::Trousers,
			Item_Categories::Socks, Item_Categories::Shoes,
			Item_Categories::Hats, Item_Categories::Accesories,
			Item_Categories::Umbrellas, Item_Categories::MailPapers,
			Item_Categories::Songs, Item_Categories::Gyroids,
			Item_Categories::AnalyzedFossils
		};
		
		Keyboard optKb(Language->Get("KEY_CHOOSE_OPTION"), songopt);
		
		Sleep(Milliseconds(100));
		switch(optKb.Open()) {
			default: break;
			case 0: 
				for(int i = 0; i < 15; ++i)
					Player::SetUnlockableBitField(player, CatalogID[i], true);
			break;
			case 1: 
				for(int i = 0; i < 15; ++i)
					Player::SetUnlockableBitField(player, CatalogID[i], false);
			break;
		}
    }
}