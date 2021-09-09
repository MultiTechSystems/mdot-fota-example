
/* Manifest parser
 *
 * Parses a SUIT manifest
 */

#ifndef _SUIT_MANIFEST_H_
#define _SUIT_MANIFEST_H_

#include "tinycbor.h"

namespace lora {
namespace app {

const uint8_t MANIFEST_VERSION_SIZE = 4;
const size_t MANIFEST_VENDOR_ID_SIZE = 16;
const size_t MANIFEST_CLASS_ID_SIZE = 16;
const size_t MANIFEST_DIGEST_SIZE = 32;
const size_t MANIFEST_SIGNATURE_SIZE = 72;
const size_t MANIFEST_BUFFER_SIZE = 512;

class SuitManifest {
public:
    enum VersionCondition {
        VER_CMP_NONE,
        VER_CMP_GT,
        VER_CMP_GTE,
        VER_CMP_EQ,
        VER_CMP_LTE,
        VER_CMP_LT
    };

    enum VersionMatchResult {
        VER_MATCH_NO_CONDITION,
        VER_MATCH_COMPATIBLE,
        VER_MATCH_INCOMPATIBLE
    };

    enum ValidationResult {
        VLDN_NONE,
        VLDN_OK,
        VLDN_FAIL,
        VLDN_UNSIGNED,
        VLDN_UNSUPPORTED_ALGORITHM,
    };

    enum AuthenticationResult {
        AUTH_NONE,
        AUTH_OK,
        AUTH_FAIL,
        AUTH_FAIL_INVALID,
        AUTH_UNSIGNED,
        AUTH_UNSUPPORTED_ALGORITHM
    };

    class Authenticator
    {
    public:
        Authenticator() {}
        virtual ~Authenticator() {}

        virtual AuthenticationResult authenticate(SuitManifest*  manifest) const = 0;
    };
    
    class Validator
    {
    public:
        Validator() {}
        virtual ~Validator() {}

        virtual ValidationResult validate(SuitManifest* manifest) const = 0;
    };

    uint8_t buffer[MANIFEST_BUFFER_SIZE];

	size_t size;
    size_t coseSize;
    size_t manifestSize;
    VersionCondition versionCondition;
    int8_t version[MANIFEST_VERSION_SIZE];
    int8_t requiredVersion[MANIFEST_VERSION_SIZE];
    uint8_t vendorId[MANIFEST_VENDOR_ID_SIZE];
    uint8_t classId[MANIFEST_VENDOR_ID_SIZE];
    uint8_t digest[MANIFEST_DIGEST_SIZE];
    uint8_t signature[MANIFEST_SIGNATURE_SIZE];
    size_t signatureSize;
    int8_t signatureAlgorithm;
    int8_t digestAlgorithm;

    ValidationResult validationResult;
    AuthenticationResult authenticationResult;
    
    SuitManifest();

    /**
     * Parse manifest from buffer and populate fields.
     *
     * @note Fill buffer with manifest before parsing, the buffer can contain 
     *       more bytes than the actual manifest.
     *
     * @return CborNoError on success; CborErrorIllegalType or CborErrorUnexpectedEOF on failure.
     */
    int32_t parse();

    /**
     * Indicates the manifest has been signed.
     */
    bool isSigned();

    /**
     * Validate the manifest against digest in the signature.
     * 
     * @note Result is saved in validationResult.
     * @note Parse must be called before validating.
     *
     * @return Result of validation
     */
   // ValidationResult validate();

    /**
     * Authenticate the manifest against the signature using a provided public key.
     * 
     * @note Result is saved in authenticationResult.
     * @note Parse must be called before authenticating.
     *
     * @return Result of authentication
     */
    //AuthenticationResult authenticate(const uint8_t* pubKey, size_t keySize);

    /**
     * Perform version compatibility match according to manifest parameters.
     *
     * @note Parse must be called before matching.
     * 
     * @param currentVersion    The current firmware version
     * @return  VER_MATCH_NO_CONDITION if the manifest does not specify a verion comparison,
     *          VER_MATCH_COMPATIBLE if currentVersion meets the criteria in the manifest,
     *          VER_MATCH_INCOMPATIBLE if currentVersion does not meet the criteria in the manifest
     */
    VersionMatchResult versionMatch(const int8_t* currentVersion, uint8_t size);
};




class SuitManifestAuthenticatorMbedTlsEcdsa : public SuitManifest::Authenticator
{
public:
    SuitManifestAuthenticatorMbedTlsEcdsa(const uint8_t* pubKey, size_t keySize) :
        _pubKey(pubKey), _keySize(keySize)
    { }

    SuitManifest::AuthenticationResult authenticate(SuitManifest* manifest) const;
private:
    const uint8_t* _pubKey;
    const size_t _keySize;
};



class SuitManifestValidatorMbedTlsSha256 : public SuitManifest::Validator
{
public:
    SuitManifest::ValidationResult validate(SuitManifest* manifest) const;
};

} } // namspace lora::app

#endif // _SUIT_MANIFEST_H_
