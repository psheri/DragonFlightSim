

#pragma once
#include <fstream>

#include "CoreMinimal.h"

#define LogMain LogHelper()
#define ENABLE_DEBUG_TO_FILE 0 //change to 1/0 to enable or disable

/**
 * Simplifies logging to console/screen.
 *
 * Usage:
 * LogMain << "hello " << 9999 << " world"; */
class DRAGONFLIGHTSIM_API LogHelper
{
private:
    std::stringstream MessageStream;

public:
    LogHelper() {}

    template <typename T>
    LogHelper& operator<<(const T& Value)
    {
        MessageStream << Value;
        return *this;
    }
    //handle FString
    template <>
    LogHelper& operator<<(const FString& Value)
    {
        MessageStream << TCHAR_TO_UTF8(*Value);
        return *this;
    }

    //handle FVector
    template <>
    LogHelper& operator<<(const FVector& Value)
    {
        MessageStream << "{ " << Value.X << ", " << Value.Y << ", " << Value.Z << " }";
        return *this;
    }

    ~LogHelper()
    {
        #if ENABLE_DEBUG_TO_FILE
        FDateTime DateNow = FDateTime::Now();
        FTimespan Time = DateNow.GetTimeOfDay();
        FString HH = FString::Printf(TEXT("%02d"), Time.GetHours());
        FString MM = FString::Printf(TEXT("%02d"), Time.GetMinutes());
        FString SS = FString::Printf(TEXT("%02d"), Time.GetSeconds());
        FString TimeString = "[" + HH + TEXT(":") + MM + TEXT(":") + SS + "] ";

        FString FilePath = FPaths::Combine(FPaths::ProjectDir(), TEXT("debugOut.txt"));
        FString Text = TimeString + FString(MessageStream.str().c_str()) + LINE_TERMINATOR;
        FFileHelper::SaveStringToFile(Text, *FilePath, FFileHelper::EEncodingOptions::AutoDetect, &IFileManager::Get(), FILEWRITE_Append);
        #endif

        FString Message = FString(MessageStream.str().c_str());
        UE_LOG(LogTemp, Log, TEXT("%s"), *Message);
        GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Cyan, Message, true, { 1.5, 1.5 });
    }
};

