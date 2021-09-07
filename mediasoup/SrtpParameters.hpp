#pragma once
#include <string>
#include <vector>
#include "json.hpp"
#include "SctpParameters.hpp"
using json = nlohmann::json;
namespace mediasoup
{
  
/**
 * SRTP parameters.
 */
struct  SrtpParameters
{
	/**
	 * Encryption and authentication transforms to be used.
	 */
	std::string cryptoSuite;//SrtpCryptoSuite

	/**
	 * SRTP keying material (master key and salt) in Base64.
	 */
	std::string keyBase64;
};

/**
 * SRTP crypto suite.
 */
//export type SrtpCryptoSuite =
//	| 'AES_CM_128_HMAC_SHA1_80'
//	| 'AES_CM_128_HMAC_SHA1_32';

void to_json(json& j, const SrtpParameters& st);

void from_json(const json& j, SrtpParameters& st);
}
