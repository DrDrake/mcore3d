#ifndef __MCD_CORE_SYSTEM_DBGINT__
#define __MCD_CORE_SYSTEM_DBGINT__

// A clone Microsoft Visual Studio internal header, providing rich
// heap memory block information.
// An online version is here:
// http://nate.deepcreek.org.au/svn/DigitalWatch/trunk/src/dbgint.h

#define nNoMansLandSize 4

typedef struct _CrtMemBlockHeader
{
        struct _CrtMemBlockHeader * pBlockHeaderNext;
        struct _CrtMemBlockHeader * pBlockHeaderPrev;
        char *                      szFileName;
        int                         nLine;
#ifdef _WIN64
        /* These items are reversed on Win64 to eliminate gaps in the struct
         * and ensure that sizeof(struct)%16 == 0, so 16-byte alignment is
         * maintained in the debug heap.
         */
        int                         nBlockUse;
        size_t                      nDataSize;
#else  /* _WIN64 */
        size_t                      nDataSize;
        int                         nBlockUse;
#endif  /* _WIN64 */
        long                        lRequest;
        unsigned char               gap[nNoMansLandSize];
        /* followed by:
         *  unsigned char           data[nDataSize];
         *  unsigned char           anotherGap[nNoMansLandSize];
         */
} _CrtMemBlockHeader;

#define pHdr(pbData) (((_CrtMemBlockHeader *)pbData)-1)

#endif	// __MCD_CORE_SYSTEM_DBGINT__
