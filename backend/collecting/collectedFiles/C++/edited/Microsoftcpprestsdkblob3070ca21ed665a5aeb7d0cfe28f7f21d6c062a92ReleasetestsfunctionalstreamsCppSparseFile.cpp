

#pragma region Includes
#include "stdafx.h"
#include "CppSparseFile.h"
#pragma endregion



BOOL VolumeSupportsSparseFiles(LPCTSTR lpRootPathName)
{
    DWORD dwVolFlags;
    GetVolumeInformation(lpRootPathName, NULL, MAX_PATH, NULL, NULL,
        &dwVolFlags, NULL, MAX_PATH);

    return (dwVolFlags & FILE_SUPPORTS_SPARSE_FILES) ? TRUE : FALSE;
}



BOOL IsSparseFile(LPCTSTR lpFileName)
{
        HANDLE hFile = CreateFile(lpFileName, GENERIC_READ, 0, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return FALSE;

        BY_HANDLE_FILE_INFORMATION bhfi;
    GetFileInformationByHandle(hFile, &bhfi);
    CloseHandle(hFile);

    return (bhfi.dwFileAttributes & FILE_ATTRIBUTE_SPARSE_FILE) ? TRUE : FALSE;
}



BOOL GetSparseFileSize(LPCTSTR lpFileName)
{
            HANDLE hFile = CreateFile(lpFileName, GENERIC_READ, 0, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return FALSE;
    LARGE_INTEGER liSparseFileSize;
    GetFileSizeEx(hFile, &liSparseFileSize);

            LARGE_INTEGER liSparseFileCompressedSize;
    liSparseFileCompressedSize.LowPart = GetCompressedFileSize(lpFileName,
        (LPDWORD)&liSparseFileCompressedSize.HighPart);

        wprintf(L"\nFile total size: %I64uKB\nActual size on disk: %I64uKB\n",
        liSparseFileSize.QuadPart / 1024,
        liSparseFileCompressedSize.QuadPart / 1024);

    CloseHandle(hFile);
    return TRUE;
}



HANDLE CreateSparseFile(LPCTSTR lpFileName)
{
        HANDLE hSparseFile = CreateFile(lpFileName, GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hSparseFile == INVALID_HANDLE_VALUE)
        return hSparseFile;

                    DWORD dwTemp;
    DeviceIoControl(hSparseFile, FSCTL_SET_SPARSE, NULL, 0, NULL, 0, &dwTemp,
        NULL);

    return hSparseFile;
}



void SetSparseRange(HANDLE hSparseFile, LONGLONG start, LONGLONG size)
{
            FILE_ZERO_DATA_INFORMATION fzdi;
    fzdi.FileOffset.QuadPart = start;
    fzdi.BeyondFinalZero.QuadPart = start + size;
 
        DWORD dwTemp;
    DeviceIoControl(hSparseFile, FSCTL_SET_ZERO_DATA, &fzdi, sizeof(fzdi),
        NULL, 0, &dwTemp, NULL);
}



BOOL GetSparseRanges(LPCTSTR lpFileName)
{
        HANDLE hFile = CreateFile(lpFileName, GENERIC_READ, 0, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return FALSE;

    LARGE_INTEGER liFileSize;
    GetFileSizeEx(hFile, &liFileSize);

        FILE_ALLOCATED_RANGE_BUFFER queryRange;
    queryRange.FileOffset.QuadPart = 0;
    queryRange.Length = liFileSize;

        FILE_ALLOCATED_RANGE_BUFFER allocRanges[1024];

    DWORD nbytes;
    BOOL fFinished;
    _putws(L"\nAllocated ranges in the file:");
    do
    {
        fFinished = DeviceIoControl(hFile, FSCTL_QUERY_ALLOCATED_RANGES,
            &queryRange, sizeof(queryRange), allocRanges,
            sizeof(allocRanges), &nbytes, NULL);

        if (!fFinished)
        {
            DWORD dwError = GetLastError();

                        if (dwError != ERROR_MORE_DATA)
            {
                wprintf(L"DeviceIoControl failed w/err 0x%08lx\n", dwError);
                CloseHandle(hFile);
                return FALSE;
            }
        }

                DWORD dwAllocRangeCount = nbytes /
            sizeof(FILE_ALLOCATED_RANGE_BUFFER);

                for (DWORD i = 0; i < dwAllocRangeCount; i++)
        {
            wprintf(L"allocated range: [%I64u] [%I64u]\n",
                allocRanges[i].FileOffset.QuadPart,
                allocRanges[i].Length.QuadPart);
        }

                if (!fFinished && dwAllocRangeCount > 0)
        {
            queryRange.FileOffset.QuadPart =
                allocRanges[dwAllocRangeCount - 1].FileOffset.QuadPart +
                allocRanges[dwAllocRangeCount - 1].Length.QuadPart;
 
            queryRange.Length.QuadPart = liFileSize.QuadPart -
                queryRange.FileOffset.QuadPart;
        }

    } while (!fFinished);

    CloseHandle(hFile);
    return TRUE;
}
