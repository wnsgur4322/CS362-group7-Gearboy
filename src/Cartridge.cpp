#include <string>
#include <algorithm>
#include <ctype.h>
#include "Cartridge.h"
#include "miniz/miniz.c"

Cartridge::Cartridge()
{
    // Compare with InitiPointer() function, SafeDeleteArray 
    // and InitPointer are almost same like below:
    // #define SafeDeleteArray(pointer) if(pointer != NULL) {delete [] pointer; pointer = NULL;}
    // #define InitPointer(pointer) ((pointer) = NULL)
    // Thus, it doesn't need both function to minimize the code.
    // Suggestion: Delete InitPointer function, and use only SafeDeleteArray.
    InitPointer(m_pTheROM);
    m_iTotalSize = 0;
    m_szName[0] = 0;
    m_iROMSize = 0;
    m_iRAMSize = 0;
    m_Type = CartridgeNotSupported;
    m_bValidROM = false;
    m_bCGB = false;
    m_bSGB = false;
    m_iVersion = 0;
    m_bLoaded = false;
    m_RTCCurrentTime = 0;
    m_bBattery = false;
    m_szFilePath[0] = 0;
    m_szFileName[0] = 0;
    m_bRTCPresent = false;
    m_bRumblePresent = false;
    m_iRAMBankCount = 0;
    m_iROMBankCount = 0;
}

Cartridge::~Cartridge()
{
    SafeDeleteArray(m_pTheROM);
}

void Cartridge::Init()
{
    Reset();
}

void Cartridge::Reset()
{
    SafeDeleteArray(m_pTheROM);
    m_iTotalSize = 0;
    m_szName[0] = 0;
    m_iROMSize = 0;
    m_iRAMSize = 0;
    m_Type = CartridgeNotSupported;
    m_bValidROM = false;
    m_bCGB = false;
    m_bSGB = false;
    m_iVersion = 0;
    m_bLoaded = false;
    m_RTCCurrentTime = 0;
    m_bBattery = false;
    m_szFilePath[0] = 0;
    m_szFileName[0] = 0;
    m_bRTCPresent = false;
    m_bRumblePresent = false;
    m_iRAMBankCount = 0;
    m_iROMBankCount = 0;
    m_GameGenieList.clear();
}
void Cartridge::CheckCartridgeType(int type)
{
    if ((type != 0xEA) && (GetROMSize() == 0))
        type = 0;

    switch (type)
    {
        case 0x00:
            m_Type = CartridgeNoMBC; // Because of not setting type (CartridgeNoMBC), some of games like NoMapper cartridge don't work in the emulator
                                     // Set the type of Cartrdige, No mapper games which have type = 0x00 work.
            break;
            // NO MBC
        case 0x08:
            // ROM
            // SRAM
        case 0x09:
            // ROM
            // SRAM
            // BATT
            m_Type = CartridgeNoMBC;
            break;
        case 0x01:
            // MBC1
        case 0x02:
            // MBC1
            // SRAM
        case 0x03:
            // MBC1
            // SRAM
            // BATT
        case 0xEA:
            // Hack to accept 0xEA as a MBC1 (Sonic 3D Blast 5)
        case 0xFF:
            // Hack to accept HuC1 as a MBC1
            m_Type = CartridgeMBC1;
            break;
        case 0x05:
            // MBC2
            // SRAM
        case 0x06:
            // MBC2
            // SRAM
            // BATT
            m_Type = CartridgeMBC2;
            break;
        case 0x0F:
            // MBC3
            // TIMER
            // BATT
        case 0x10:
            // MBC3
            // TIMER
            // BATT
            // SRAM
        case 0x11:
            // MBC3
        case 0x12:
            // MBC3
            // SRAM
        case 0x13:
            // MBC3
            // BATT
            // SRAM
        case 0xFC:
            // Game Boy Camera
            m_Type = CartridgeMBC3;
            break;
        case 0x19:
            // MBC5
        case 0x1A:
            // MBC5
            // SRAM
        case 0x1B:
            // MBC5
            // BATT
            // SRAM
        case 0x1C:
            // RUMBLE
        case 0x1D:
            // RUMBLE
            // SRAM
        case 0x1E:
            // RUMBLE
            // BATT
            // SRAM
            m_Type = CartridgeMBC5;
            break;
        case 0x0B:
            // MMMO1
        case 0x0C:
            // MMM01
            // SRAM
        case 0x0D:
            // MMM01
            // SRAM
            // BATT
            m_Type = CartridgeMBC1Multi; // Accoridng to Memory Bank Controller from WIkipedia 
                                         //(https://gbdev.gg8.se/wiki/articles/Memory_Bank_Controllers#MBC5_.28max_8MByte_ROM_and.2For_128KByte_RAM.29)
                                         // MMM01 similar to MBC1Multicart, so I tried to set MMM01 to MBC1Multi, and I checked it works with MBC1Multi.
                                         // Therefore, the emulator supports MMM01 games now.
            break;
        case 0x15:
            // MBC4
        case 0x16:
            // MBC4
            // SRAM
        case 0x17:
            // MBC4
            // SRAM
            // BATT
        case 0x22:
            m_Type = CartridgeMBC5; // MBC7 supports almost similar memory size of MBC5(but Ram size is different (MBC5: A000~AFFF / MBC7 : A000~BFFF)
                                    // However, except RAM size, it has similar memory system, so MBC7 games works with MBC5 memory system
                                    // Therefore, set MBC5 instead, the emulator supports MBC7 games which didnt' support before.
            break;
            // MBC7
            // BATT
            // SRAM
        case 0x55:
            // GG
        case 0x56:
            // GS3
        case 0xFD:
            // TAMA 5
        case 0xFE:
            // HuC3
            m_Type = CartridgeNotSupported;
            Log("--> ** This cartridge is not supported. Type: %d", type);
            break;
        default:
            m_Type = CartridgeNotSupported;
            Log("--> ** Unknown cartridge type: %d", type);
    }

    switch (type)
    {
        case 0x03:
        case 0x06:
        case 0x09:
        case 0x0D:
        case 0x0F:
        case 0x10:
        case 0x13:
        case 0x17:
        case 0x1B:
        case 0x1E:
        case 0x22:
        case 0xFD:
        case 0xFF:
            m_bBattery = true;
            break;
        default:
            m_bBattery = false;
    }

    switch (type)
    {
        case 0x0F:
        case 0x10:
            m_bRTCPresent = true;
            break;
        default:
            m_bRTCPresent = false;
    }

    switch (type)
    {
        case 0x1C:
        case 0x1D:
        case 0x1E:
            m_bRumblePresent = true;
            break;
        default:
            m_bRumblePresent = false;
    }
}
bool Cartridge::GatherMetadata()
{
    char name[12] = {0};
    name[11] = 0;

    for (int i = 0; i < 11; i++)
    {
        name[i] = m_pTheROM[0x0134 + i];

        if (name[i] == 0)
        {
            break;
        }
    }

    strcpy(m_szName, name);

    m_bCGB = (m_pTheROM[0x143] == 0x80) || (m_pTheROM[0x143] == 0xC0);
    m_bSGB = (m_pTheROM[0x146] == 0x03);
    int type = m_pTheROM[0x147];
    m_iROMSize = m_pTheROM[0x148];
    m_iRAMSize = m_pTheROM[0x149];
    m_iVersion = m_pTheROM[0x14C];

    CheckCartridgeType(type);
    // If the catrdige type is invailed, it doesn't need to do 
    // below switches after CheckCartridgeType funtion
    // because this function changes m_Type as CartridgeNotSupported 
    // which just return false at the end of Cartridge::GatherMetadata()
    // Threfore, after CheckCatrdiegeType, if m_Type = CartridgeNotSupported, 
    // then it should return false right after the function
    // because if it's in switchs, it's possible to occur error 
    // becasue of keeping using invailed cartridge type for switches.
    // Suggestion: CheckCartridgeType(type); ====> 
    //             if (CheckCartridgeType(type) == CartridgeNotSupported) { return false; }

    switch (m_iRAMSize)
    {
        case 0x00:
            m_iRAMBankCount = (m_Type == Cartridge::CartridgeMBC2) ? 1 : 0;
            break;
        case 0x01:
        case 0x02:
            m_iRAMBankCount = 1;
            break;
        case 0x04:
            m_iRAMBankCount = 16;
            break;
        default:
            m_iRAMBankCount = 4;
            break;
    }

    m_iROMBankCount = std::max(Pow2Ceil(m_iTotalSize / 0x4000), 2u);

    bool presumeMultiMBC1 = ((type == 1) && (m_iRAMSize == 0) && (m_iROMBankCount == 64));

    if ((m_Type == Cartridge::CartridgeMBC1) && presumeMultiMBC1)
    {
        m_Type = Cartridge::CartridgeMBC1Multi;
        Log("Presumed Multi 64");
    }

    Log("Cartridge Size %d", m_iTotalSize);
    Log("ROM Name %s", m_szName);
    Log("ROM Version %d", m_iVersion);
    Log("ROM Type %X", type);
    Log("ROM Size %X", m_iROMSize);
    Log("ROM Bank Count %d", m_iROMBankCount);
    Log("RAM Size %X", m_iRAMSize);
    Log("RAM Bank Count %d", m_iRAMBankCount);

    switch (m_Type)
    {
        case Cartridge::CartridgeNoMBC:
            Log("No MBC found");
            break;
        case Cartridge::CartridgeMBC1:
            Log("MBC1 found");
            break;
        case Cartridge::CartridgeMBC1Multi:
            Log("MBC1 Multi 64 found");
            break;
        case Cartridge::CartridgeMBC2:
            Log("MBC2 found");
            break;
        case Cartridge::CartridgeMBC3:
            Log("MBC3 found");
            break;
        case Cartridge::CartridgeMBC5:
            Log("MBC5 found");
            break;
        case Cartridge::CartridgeNotSupported:
            Log("Cartridge not supported!!");
            break;
        default:
            break;
    }

    if (m_bBattery)
    {
        Log("Battery powered RAM found");
    }

    if (m_pTheROM[0x143] == 0xC0)
    {
        Log("Game Boy Color only");
    }
    else if (m_bCGB)
    {
        Log("Game Boy Color supported");
    }

    if (m_bSGB)
    {
        Log("Super Game Boy supported");
    }

    int checksum = 0;

    for (int j = 0x134; j < 0x14E; j++)
    {
        checksum += m_pTheROM[j];
    }

    m_bValidROM = ((checksum + 25) & 0xFF) == 0;

    if (m_bValidROM)
    {
        Log("Checksum OK!");
    }
    else
    {
        Log("Checksum FAILED!!!");
    }

    return (m_Type != CartridgeNotSupported);
}