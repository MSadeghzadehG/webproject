

#ifndef _PARSE_VERSIONS_INCLUDED_
#define _PARSE_VERSIONS_INCLUDED_

#include "../Public/ShaderLang.h"
#include "../Include/InfoSink.h"
#include "Scan.h"

#include <map>

namespace glslang {

class TParseVersions {
public:
    TParseVersions(TIntermediate& interm, int version, EProfile profile,
                   const SpvVersion& spvVersion, EShLanguage language, TInfoSink& infoSink,
                   bool forwardCompatible, EShMessages messages)
        : infoSink(infoSink), version(version), profile(profile), language(language),
          spvVersion(spvVersion), forwardCompatible(forwardCompatible),
          intermediate(interm), messages(messages), numErrors(0), currentScanner(0) { }
    virtual ~TParseVersions() { }
    virtual void initializeExtensionBehavior();
    virtual void requireProfile(const TSourceLoc&, int queryProfiles, const char* featureDesc);
    virtual void profileRequires(const TSourceLoc&, int queryProfiles, int minVersion, int numExtensions, const char* const extensions[], const char* featureDesc);
    virtual void profileRequires(const TSourceLoc&, int queryProfiles, int minVersion, const char* const extension, const char* featureDesc);
    virtual void requireStage(const TSourceLoc&, EShLanguageMask, const char* featureDesc);
    virtual void requireStage(const TSourceLoc&, EShLanguage, const char* featureDesc);
    virtual void checkDeprecated(const TSourceLoc&, int queryProfiles, int depVersion, const char* featureDesc);
    virtual void requireNotRemoved(const TSourceLoc&, int queryProfiles, int removedVersion, const char* featureDesc);
    virtual void requireExtensions(const TSourceLoc&, int numExtensions, const char* const extensions[], const char* featureDesc);
    virtual void ppRequireExtensions(const TSourceLoc&, int numExtensions, const char* const extensions[], const char* featureDesc);
    virtual TExtensionBehavior getExtensionBehavior(const char*);
    virtual bool extensionTurnedOn(const char* const extension);
    virtual bool extensionsTurnedOn(int numExtensions, const char* const extensions[]);
    virtual void updateExtensionBehavior(int line, const char* const extension, const char* behavior);
    virtual void fullIntegerCheck(const TSourceLoc&, const char* op);
    virtual void doubleCheck(const TSourceLoc&, const char* op);
    virtual void int64Check(const TSourceLoc&, const char* op, bool builtIn = false);
    virtual void spvRemoved(const TSourceLoc&, const char* op);
    virtual void vulkanRemoved(const TSourceLoc&, const char* op);
    virtual void requireVulkan(const TSourceLoc&, const char* op);
    virtual void requireSpv(const TSourceLoc&, const char* op);
    virtual bool checkExtensionsRequested(const TSourceLoc&, int numExtensions, const char* const extensions[], const char* featureDesc);
    virtual void updateExtensionBehavior(const char* const extension, TExtensionBehavior);

    virtual void C_DECL error(const TSourceLoc&, const char* szReason, const char* szToken,
        const char* szExtraInfoFormat, ...) = 0;
    virtual void C_DECL  warn(const TSourceLoc&, const char* szReason, const char* szToken,
        const char* szExtraInfoFormat, ...) = 0;
    virtual void C_DECL ppError(const TSourceLoc&, const char* szReason, const char* szToken,
        const char* szExtraInfoFormat, ...) = 0;
    virtual void C_DECL ppWarn(const TSourceLoc&, const char* szReason, const char* szToken,
        const char* szExtraInfoFormat, ...) = 0;

    void addError() { ++numErrors; }
    int getNumErrors() const { return numErrors; }

    void setScanner(TInputScanner* scanner) { currentScanner = scanner; }
    TInputScanner* getScanner() const { return currentScanner; }
    const TSourceLoc& getCurrentLoc() const { return currentScanner->getSourceLoc(); }
    void setCurrentLine(int line) { currentScanner->setLine(line); }
    void setCurrentColumn(int col) { currentScanner->setColumn(col); }
    void setCurrentSourceName(const char* name) { currentScanner->setFile(name); }
    void setCurrentString(int string) { currentScanner->setString(string); }

    void getPreamble(std::string&);
    bool relaxedErrors()    const { return (messages & EShMsgRelaxedErrors) != 0; }
    bool suppressWarnings() const { return (messages & EShMsgSuppressWarnings) != 0; }

    TInfoSink& infoSink;

        int version;                     EProfile profile;                EShLanguage language;            SpvVersion spvVersion;
    bool forwardCompatible;          TIntermediate& intermediate; 
protected:
    EShMessages messages;            int numErrors;                   TInputScanner* currentScanner;

private:
    TMap<TString, TExtensionBehavior> extensionBehavior;        explicit TParseVersions(const TParseVersions&);
    TParseVersions& operator=(const TParseVersions&);
};

} 
#endif 