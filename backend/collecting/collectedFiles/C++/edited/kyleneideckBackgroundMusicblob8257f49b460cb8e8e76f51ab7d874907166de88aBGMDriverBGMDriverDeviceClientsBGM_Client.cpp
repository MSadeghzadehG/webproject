

#include "BGM_Client.h"


BGM_Client::BGM_Client(const AudioServerPlugInClientInfo* inClientInfo)
:
    mClientID(inClientInfo->mClientID),
    mProcessID(inClientInfo->mProcessID),
    mIsNativeEndian(inClientInfo->mIsNativeEndian),
    mBundleID(inClientInfo->mBundleID)
{
            if(inClientInfo->mBundleID != NULL)
    {
        CFRetain(inClientInfo->mBundleID);
    }
}

void    BGM_Client::Copy(const BGM_Client& inClient)
{
    mClientID = inClient.mClientID;
    mProcessID = inClient.mProcessID;
    mBundleID = inClient.mBundleID;
    mIsNativeEndian = inClient.mIsNativeEndian;
    mDoingIO = inClient.mDoingIO;
    mIsMusicPlayer = inClient.mIsMusicPlayer;
    mRelativeVolume = inClient.mRelativeVolume;
    mPanPosition = inClient.mPanPosition;
}

