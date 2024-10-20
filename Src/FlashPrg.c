/*********************************************************************
*            (c) 1995 - 2018 SEGGER Microcontroller GmbH             *
*                        The Embedded Experts                        *
*                           www.segger.com                           *
**********************************************************************
----------------------------------------------------------------------
File    : FlashPrg.c
Purpose : Implementation of RAMCode template
--------  END-OF-HEADER  ---------------------------------------------
*/
#include "FlashOS.h"
#include <board.h>
#include <stddef.h>
#include <stdint.h>

#define READ_BUF_SIZE 512

/*********************************************************************
 *
 *       Types
 *
 **********************************************************************
 */

/*********************************************************************
 *
 *       Static data
 *
 **********************************************************************
 */
//
// We use this dummy variable to make sure that the PrgData
// section is present in the output elf-file as this section
// is mandatory in current versions of the J-Link DLL
//
static volatile int _Dummy;

#if (SUPPORT_ERASE_CHIP == 0)
#define EraseChip NULL
#endif
#if (SUPPORT_NATIVE_VERIFY == 0)
#define Verify NULL
#endif
#if (SUPPORT_NATIVE_READ_FUNCTION == 0)
#define SEGGER_OPEN_Read NULL
#define EnableMMAP(en) borad_exFlash_EnableMMAP(en);
#else
#define EnableMMAP(en)
#endif

#if (SUPPORT_BLANK_CHECK == 0)
#define BlankCheck NULL
#endif
#if (SUPPORT_SEGGER_OPEN_ERASE == 0)
#define SEGGER_OPEN_Erase NULL
#endif
#if (SUPPORT_TURBO_MODE == 0)
#define SEGGER_OPEN_Start NULL
#endif

// Mark start of <PrgData> segment. Non-static to make sure linker can keep this symbol. Dummy needed to make sure that
// <PrgData> section in resulting ELF file is present. Needed by open flash loader logic on PC side
volatile int PRGDATA_StartMarker __attribute__((section("PrgData")));

// Mark start of <PrgCode> segment. Non-static to make sure linker can keep this symbol.
const SEGGER_OFL_API SEGGER_OFL_Api __attribute__((section("PrgCode"))) = {
    NULL,
    Init,
    UnInit,
    EraseSector,
    ProgramPage,
    BlankCheck,
    EraseChip,
    Verify,
    SEGGER_OPEN_CalcCRC,
    SEGGER_OPEN_Read,
    SEGGER_OPEN_Program,
    SEGGER_OPEN_Erase,
    SEGGER_OPEN_Start,
};

/*********************************************************************
 *
 *       Init
 *
 *  Function description
 *    Handles the initialization of the flash module.
 *
 *  Parameters
 *    Addr: Flash base address
 *    Freq: Clock frequency in Hz
 *    Func: Specifies the action followed by Init() (e.g.: 1 - Erase, 2 - Program, 3 - Verify / Read)
 *
 *  Return value
 *    0 O.K.
 *    1 Error
 */
int Init(U32 Addr, U32 Freq, U32 Func)
{
    (void)PRGDATA_StartMarker;
    int ret = 0;

    if ((ret = board_init(Addr, Freq, Func)) != 0)
    {
        return 1;
    }
    EnableMMAP(1);
    return 0;
}

/*********************************************************************
 *
 *       UnInit
 *
 *  Function description
 *    Handles the de-initialization of the flash module.
 *
 *  Parameters
 *    Func: Caller type (e.g.: 1 - Erase, 2 - Program, 3 - Verify)
 *
 *  Return value
 *    0 O.K.
 *    1 Error
 */
int UnInit(U32 Func)
{
    EnableMMAP(0);
    int ret = board_deinit(Func);
    return ret == 0 ? 0 : 1;
}

/*********************************************************************
 *
 *       EraseSector
 *
 *  Function description
 *    Erases one flash sector.
 *
 *  Parameters
 *    SectorAddr: Absolute address of the sector to be erased
 *
 *  Return value
 *    0 O.K.
 *    1 Error
 */
int EraseSector(U32 SectorAddr)
{
    EnableMMAP(0);
    int ret = board_exFlash_EraseSector(SectorAddr - EXFLASH_BASE_ADDR);
    EnableMMAP(1);
    return ret;
}

/*********************************************************************
 *
 *       ProgramPage
 *
 *  Function description
 *    Programs one flash page.
 *
 *  Parameters
 *    DestAddr: Destination address
 *    NumBytes: Number of bytes to be programmed (always a multiple of program page size, defined in FlashDev.c)
 *    pSrcBuff: Point to the source buffer
 *
 *  Return value
 *    0 O.K.
 *    1 Error
 */
int ProgramPage(U32 DestAddr, U32 NumBytes, U8 *pSrcBuff)
{
    EnableMMAP(0);
    int ret = board_exFlash_WritePage(DestAddr - EXFLASH_BASE_ADDR, pSrcBuff, NumBytes) < 0 ? 1 : 0;
    EnableMMAP(1);
    return ret;
}

/*********************************************************************
 *
 *       Verify
 *
 *  Function description
 *    Compares a specified number of bytes of a provided data
 *    buffer with the content of the device
 *
 *  Parameters
 *    Addr: Start address in memory which should be compared
 *    NumBytes: Number of bytes to be compared
 *    pBuff: Pointer to the data to be compared
 *
 *  Return value
 *    == (Addr + NumBytes): O.K.
 *    != (Addr + NumBytes): *not* O.K. (ideally the fail address is returned)
 *
 */
#if SUPPORT_NATIVE_VERIFY
U32 Verify(U32 Addr, U32 NumBytes, U8 *pBuff)
{
#if SUPPORT_NATIVE_READ_FUNCTION
    uint8_t buf[READ_BUF_SIZE];
    uint32_t FlashAddr = Addr - EXFLASH_BASE_ADDR;

    while (NumBytes)
    {
        int op_len = NumBytes > READ_BUF_SIZE ? READ_BUF_SIZE : NumBytes;
        int ret = board_exFlash_Read(FlashAddr, buf, op_len);
        if (ret != op_len)
            return FlashAddr + EXFLASH_BASE_ADDR;
        for (int i = 0; i < op_len; i++)
        {
            if (buf[i] != *pBuff++)
                return FlashAddr + EXFLASH_BASE_ADDR + i;
        }
    }
    return Addr + NumBytes;
#else
    unsigned char *pFlash;
    unsigned long r;
    pFlash = (unsigned char *)Addr;
    r = Addr + NumBytes;
    do
    {
        if (*pFlash != *pBuff)
        {
            r = (unsigned long)pFlash;
            break;
        }
        pFlash++;
        pBuff++;
    } while (--NumBytes);
    return r;
#endif
}
#endif

/*********************************************************************
 *
 *       BlankCheck
 *
 *  Function description
 *    Checks if a memory region is blank
 *
 *  Parameters
 *    Addr: Blank check start address
 *    NumBytes: Number of bytes to be checked
 *    BlankData: Pointer to the destination data
 *
 *  Return value
 *    0: O.K., blank
 *    1: O.K., *not* blank
 *    < 0: Error
 *
 */
#if SUPPORT_BLANK_CHECK
int BlankCheck(U32 Addr, U32 NumBytes, U8 BlankData)
{
#if SUPPORT_NATIVE_READ_FUNCTION
    uint8_t buf[READ_BUF_SIZE];
    uint32_t FlashAddr = Addr - EXFLASH_BASE_ADDR;

    while (NumBytes)
    {
        int op_len = NumBytes > READ_BUF_SIZE ? READ_BUF_SIZE : NumBytes;
        int ret = board_exFlash_Read(FlashAddr, buf, op_len);
        if (ret != op_len)
            return ret;
        for (int i = 0; i < op_len; i++)
        {
            if (buf[i] != BlankData)
                return 1;
        }
    }
    return 0;
#else

    U8 *pData = (U8 *)Addr;
    do
    {
        if (*pData++ != BlankData)
        {
            return 1;
        }
    } while (--NumBytes);
    return 0;
#endif
}
#endif

/*********************************************************************
 *
 *       EraseChip
 *
 *  Function description
 *    Erases the entire flash
 *
 *  Return value
 *    0: O.K.
 *    1: Error
 */
#if SUPPORT_ERASE_CHIP
int EraseChip()
{
    EnableMMAP(0);
    int ret = board_exFlash_EraseChip();
    EnableMMAP(1);
    return ret;
}
#endif

/*********************************************************************
 *
 *       SEGGER_OPEN_Read
 *
 *  Function description
 *    Reads a specified number of bytes into the provided buffer
 *
 *  Parameters
 *    Addr: Start read address
 *    NumBytes: Number of bytes to be read
 *    pBuff: Pointer to the destination data
 *
 *  Return value
 *    >= 0: O.K., NumBytes read
 *    <  0: Error
 *
 */
#if SUPPORT_NATIVE_READ_FUNCTION
int SEGGER_OPEN_Read(U32 Addr, U32 NumBytes, U8 *pDestBuff)
{
    return !!board_exFlash_Read(Addr - EXFLASH_BASE_ADDR, pDestBuff, NumBytes);
}
#endif

/*********************************************************************
 *
 *       SEGGER_OPEN_Program
 *
 *  Function description
 *    Programs a specified number of bytes into the target flash.
 *    NumBytes is either EXFLASH_PAGE_SIZE or a multiple of it.
 *
 *  Notes
 *    (1) This function can rely on that at least EXFLASH_PAGE_SIZE will be passed
 *    (2) This function must be able to handle multiple of EXFLASH_PAGE_SIZE
 *
 *  Parameters
 *    Addr: Start read address
 *    NumBytes: Number of bytes to be read
 *    pBuff: Pointer to the destination data
 *
 *  Return value
 *    0 O.K.
 *    1 Error
 *
 */
#if SUPPORT_SEGGER_OPEN_Program
int SEGGER_OPEN_Program(U32 DestAddr, U32 NumBytes, U8 *pSrcBuff)
{
    EnableMMAP(0);
    U32 NumPages;
    int r;

    NumPages = NumBytes / EXFLASH_PAGE_SIZE;
    r = 0;
    do
    {
        r = board_exFlash_WritePage(DestAddr - EXFLASH_BASE_ADDR, pSrcBuff, EXFLASH_PAGE_SIZE);
        if (r != EXFLASH_PAGE_SIZE)
        {
            EnableMMAP(1);
            return r;
        }
        DestAddr += EXFLASH_PAGE_SIZE;
        pSrcBuff += EXFLASH_PAGE_SIZE;
    } while (--NumPages);
    EnableMMAP(1);
    return 0;
}
#endif

/*********************************************************************
 *
 *       SEGGER_OPEN_Erase
 *
 *  Function description
 *    Erases one or more flash sectors
 *
 *  Notes
 *    (1) This function can rely on that at least one sector will be passed
 *    (2) This function must be able to handle multiple sectors at once
 *    (3) This function can rely on that only multiple sectors of the same sector
 *        size will be passed. (e.g. if the device has two sectors with different
 *        sizes, the DLL will call this function two times with NumSectors = 1)
 *
 *  Parameters
 *    SectorAddr: Address of the start sector to be erased
 *    SectorIndex: Index of the start sector to be erased
 *    NumSectors: Number of sectors to be erased. At least 1 sector is passed.
 *
 *  Return value
 *    0 O.K.
 *    1 Error
 *
 */
#if SUPPORT_SEGGER_OPEN_ERASE
int SEGGER_OPEN_Erase(U32 SectorAddr, U32 SectorIndex, U32 NumSectors)
{
    EnableMMAP(0);
    int ret = 0;
    uint32_t FlashAddr = SectorAddr - EXFLASH_BASE_ADDR;
    while (NumSectors)
    {
        if (NumSectors >= 16)
        {
            ret = !!board_exFlash_EraseBlock64K(FlashAddr);
            NumSectors -= 16;
            FlashAddr += 16 * 0x1000;
        }
        else if (NumSectors >= 8)
        {
            ret = !!board_exFlash_EraseBlock32K(FlashAddr);
            NumSectors -= 8;
            FlashAddr += 8 * 0x1000;
        }
        else
        {
            ret = !!board_exFlash_EraseSector(FlashAddr);
            NumSectors -= 1;
            FlashAddr += 0x1000;
        }
        if (ret)
            break;
    }
    EnableMMAP(1);
    return ret;
}
#endif

/*********************************************************************
 *
 *       SEGGER_OPEN_CalcCRC
 *
 *  Function description
 *    Calculates the CRC over a specified number of bytes
 *    Even more optimized version of Verify() as this avoids downloading the compare data into the RAMCode for
 * comparison. Heavily reduces traffic between J-Link software and target and therefore speeds up verification process
 * significantly.
 *
 *  Parameters
 *    CRC       CRC start value
 *    Addr      Address where to start calculating CRC from
 *    NumBytes  Number of bytes to calculate CRC on
 *    Polynom   Polynom to be used for CRC calculation
 *
 *  Return value
 *    CRC
 *
 *  Notes
 *    (1) This function is optional
 *    (2) Use "noinline" attribute to make sure that function is never inlined and label not accidentally removed by
 * linker from ELF file.
 */
U32 SEGGER_OPEN_CalcCRC(U32 CRC, U32 Addr, U32 NumBytes, U32 Polynom)
{
#if SUPPORT_NATIVE_READ_FUNCTION
    uint8_t buf[READ_BUF_SIZE];
    uint32_t FlashAddr = Addr - EXFLASH_BASE_ADDR;

    while (NumBytes)
    {
        int op_len = NumBytes > READ_BUF_SIZE ? READ_BUF_SIZE : NumBytes;
        int ret = board_exFlash_Read(FlashAddr, buf, op_len);
        if (ret != op_len)
            return ret;
        CRC = SEGGER_OFL_Lib_CalcCRC(&SEGGER_OFL_Api, CRC, (uint32_t)&FlashAddr, op_len, Polynom);
    }
    return CRC;
#else
    // Use lib function from SEGGER by default. Pass API pointer to it because it
    // may need to call the read function (non-memory mapped flashes)
    CRC = SEGGER_OFL_Lib_CalcCRC(&SEGGER_OFL_Api, CRC, Addr, NumBytes, Polynom);
    return CRC;
#endif
}

/*********************************************************************
 *
 *       SEGGER_OPEN_Start
 *
 *  Function description
 *    Starts the turbo mode of flash algo.
 *    Currently only available for Cortex-M based targets.
 */
#if SUPPORT_TURBO_MODE
void SEGGER_OPEN_Start(volatile struct SEGGER_OPEN_CMD_INFO *pInfo)
{
    SEGGER_OFL_Lib_StartTurbo(&SEGGER_OFL_Api, pInfo);
}
#endif

/**************************** End of file ***************************/
