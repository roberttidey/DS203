#define OK 0 
#define SEC_ERR 1 
#define FAT_ERR 2 
#define OVER 3 
#define NEW 4 
#define EMPT 4 
#define VER_ERR 5 
#define SUM_ERR 6 
#define RD_ERR 8 
#define WR_ERR 9 
#define DISK_ERR 10 

/*static*/ ui16 BIOS::DSK::SectorSize()
{
	static ui16 pSectorSize;
   if (pSectorSize == 0 ) {
      u32 ptr;
      u8 Ver[8];
      uc8 DiskDevInfo_8M[]={"8MB Internal"};
      ptr=__Get(DFUVER, 0);
      memcpy(Ver,(u8*)ptr,5);
      ptr=(Ver[1]-'0')*100 +(Ver[3]-'0')*10 +(Ver[4]-'0'); 
      if(ptr<=311){
         pSectorSize = FLASH_2M_SECTOR_SIZE;
      } else {
         ptr=__Get(DEVICEINFO, 0);
         if(memcmp((u8*)ptr,DiskDevInfo_8M,3)==0){
            pSectorSize = FLASH_8M_SECTOR_SIZE;
         } else {
            pSectorSize = FLASH_2M_SECTOR_SIZE;
         }
      }  
   }
   return pSectorSize;
}

/*static*/ PVOID BIOS::DSK::GetSharedBuffer()
{
	static ui8 pSectorBuffer[LARGEST_SECTOR_SIZE];
	return (PVOID)pSectorBuffer;
}

/*static*/ BOOL BIOS::DSK::Open(FILEINFO* pFileInfo, const char* strName, ui8 nIoMode)
{
	pFileInfo->nMode = nIoMode;
	pFileInfo->nSectors = 0;
	if ( nIoMode == BIOS::DSK::IoRead )
	{
		if ( __OpenFileRd((ui8*)GetSharedBuffer(), (char*)strName, pFileInfo->pCluster, pFileInfo->pDirAddr) != OK )
			return FALSE;
	}
	if ( nIoMode == BIOS::DSK::IoWrite )
	{
		if ( __OpenFileWr((ui8*)GetSharedBuffer(), (char*)strName, pFileInfo->pCluster, pFileInfo->pDirAddr) != OK )
			return FALSE;
	}
	return TRUE;
}

/*static*/ BOOL BIOS::DSK::Read(FILEINFO* pFileInfo, ui8* pSectorData)
{
	if ( __ReadFileSec(pSectorData, pFileInfo->pCluster) != OK )
		return FALSE;
	return TRUE;
}

/*static*/ BOOL BIOS::DSK::Write(FILEINFO* pFileInfo, ui8* pSectorData)
{
	if ( __ProgFileSec(pSectorData, pFileInfo->pCluster) != OK )
		return FALSE;
	pFileInfo->nSectors++;
	return TRUE;
}

/*static*/ BOOL BIOS::DSK::Close(FILEINFO* pFileInfo, int nSize /*=-1*/)
{
	if ( pFileInfo->nMode == BIOS::DSK::IoWrite )
	{
		ui32 nWritten = (nSize!=-1) ? nSize : ((ui32)pFileInfo->nSectors) << 9;

		//BIOS::DBG::Print("Writing %d bytes (%d sectors)...\n", nWritten, pFileInfo->nSectors);

		if ( __CloseFile( (ui8*)GetSharedBuffer(), nWritten,
			pFileInfo->pCluster, pFileInfo->pDirAddr ) != OK )
		{
			return FALSE;
		}
		pFileInfo->nMode = BIOS::DSK::IoClosed;
	}
	return TRUE;
}
