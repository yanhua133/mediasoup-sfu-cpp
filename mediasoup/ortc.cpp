#define MSC_CLASS "ortc"

#include "ortc.hpp"
#include "LoggerC.hpp"
#include "MediaSoupClientErrors.hpp"
#include "sdp/h264_profile_level_id.h"//<media/base/h264_profile_level_id.h>
#include <algorithm> // std::find_if
#include <regex>
#include <stdexcept>
#include <string>
#include "RtpParameters.hpp"
#include "SupportedRtpCapabilities.hpp"
#include "scalabilityModes.hpp"
#include "UtilsC.hpp"
#include <iostream>
using json = nlohmann::json;
using namespace mediasoupclient;
using namespace mediasoup;
static constexpr uint32_t ProbatorSsrc{ 1234u };
static const std::string ProbatorMid("probator");

// Static functions declaration.
static bool isRtxCodec(const json& codec);
static bool matchCodecs(json& aCodec, const json& bCodec, bool strict = false, bool modify = false);
static bool matchHeaderExtensions(const json& aExt, const json& bExt);
static json reduceRtcpFeedback(const json& codecA, const json& codecB);
static uint8_t getH264PacketizationMode(const json& codec);
static uint8_t getH264LevelAssimetryAllowed(const json& codec);
static std::string getH264ProfileLevelId(const json& codec);
static std::string getVP9ProfileId(const json& codec);


// Private helpers used in this file.
static bool is_element_in_vector(std::vector<std::vector<int>> vec, std::vector<int> element) {
    std::vector<std::vector<int>>::iterator it;
    it = find(vec.begin(), vec.end(), element);
    if (it != vec.end()) {
        return true;
    }
    else {
        return false;
    }
}
static bool isRtxCodec(const json& codec)
{
    MSC_TRACE();

    static const std::regex RtxMimeTypeRegex(
      "^(audio|video)/rtx$", std::regex_constants::ECMAScript | std::regex_constants::icase);

    std::smatch match;
    auto mimeType = codec["mimeType"].get<std::string>();

    return std::regex_match(mimeType, match, RtxMimeTypeRegex);
}

static bool matchCodecs(json& aCodec, const json& bCodec, bool strict, bool modify)
{
    MSC_TRACE();

    auto aMimeTypeIt = aCodec.find("mimeType");
    auto bMimeTypeIt = bCodec.find("mimeType");
    auto aMimeType   = aMimeTypeIt->get<std::string>();
    auto bMimeType   = bMimeTypeIt->get<std::string>();

    std::transform(aMimeType.begin(), aMimeType.end(), aMimeType.begin(), ::tolower);
    std::transform(bMimeType.begin(), bMimeType.end(), bMimeType.begin(), ::tolower);

    if (aMimeType != bMimeType)
        return false;

    if (aCodec["clockRate"] != bCodec["clockRate"])
        return false;

    if (aMimeType == "audio/opus") {
        if (aCodec.contains("channels") != bCodec.contains("channels"))
            return false;
        if (aCodec.contains("channels") && aCodec["channels"] != bCodec["channels"])
            return false;
    }
   

   

    // Match H264 parameters.
    if (aMimeType == "video/h264")
    {
        auto aPacketizationMode = getH264PacketizationMode(aCodec);
        auto bPacketizationMode = getH264PacketizationMode(bCodec);

        if (aPacketizationMode != bPacketizationMode)
            return false;

        // If strict matching check profile-level-id.
        if (strict)
        {
            webrtc::H264::CodecParameterMap aParameters;
            webrtc::H264::CodecParameterMap bParameters;

            aParameters["level-asymmetry-allowed"] = std::to_string(getH264LevelAssimetryAllowed(aCodec));
            aParameters["packetization-mode"]      = std::to_string(aPacketizationMode);
            aParameters["profile-level-id"]        = getH264ProfileLevelId(aCodec);
            bParameters["level-asymmetry-allowed"] = std::to_string(getH264LevelAssimetryAllowed(bCodec));
            bParameters["packetization-mode"]      = std::to_string(bPacketizationMode);
            bParameters["profile-level-id"]        = getH264ProfileLevelId(bCodec);

            if (!webrtc::H264::IsSameH264Profile(aParameters, bParameters))
                return false;

            webrtc::H264::CodecParameterMap newParameters;

            try
            {
                webrtc::H264::GenerateProfileLevelIdForAnswer(aParameters, bParameters, &newParameters);
            }
            catch (std::runtime_error)
            {
                return false;
            }

            if (modify)
            {
                auto profileLevelIdIt = newParameters.find("profile-level-id");

                if (profileLevelIdIt != newParameters.end())
                    aCodec["parameters"]["profile-level-id"] = profileLevelIdIt->second;
                else
                    aCodec["parameters"].erase("profile-level-id");
            }
        }
    }
    // Match VP9 parameters.
    else if (aMimeType == "video/vp9")
    {
        // If strict matching check profile-id.
        if (strict)
        {
            auto aProfileId = getVP9ProfileId(aCodec);
            auto bProfileId = getVP9ProfileId(bCodec);

            if (aProfileId != bProfileId)
                return false;
        }
    }

    return true;
}

static bool matchHeaderExtensions(const json& aExt, const json& bExt)
{
    MSC_TRACE();

    if (aExt["kind"] != bExt["kind"])
        return false;

    return aExt["uri"] == bExt["uri"];
}

static json reduceRtcpFeedback(const json& codecA, const json& codecB)
{
    MSC_TRACE();

    auto reducedRtcpFeedback = json::array();
    auto rtcpFeedbackAIt     = codecA.find("rtcpFeedback");
    auto rtcpFeedbackBIt     = codecB.find("rtcpFeedback");

    for (const auto& aFb : *rtcpFeedbackAIt)
    {
        auto rtcpFeedbackIt =
          std::find_if(rtcpFeedbackBIt->begin(), rtcpFeedbackBIt->end(), [&aFb](const json& bFb) {
              return (aFb["type"] == bFb["type"] && aFb["parameter"] == bFb["parameter"]);
          });

        if (rtcpFeedbackIt != rtcpFeedbackBIt->end())
            reducedRtcpFeedback.push_back(*rtcpFeedbackIt);
    }

    return reducedRtcpFeedback;
}

static uint8_t getH264PacketizationMode(const json& codec)
{
    MSC_TRACE();

    auto& parameters         = codec["parameters"];
    auto packetizationModeIt = parameters.find("packetization-mode");

    // clang-format off
    if (
        packetizationModeIt == parameters.end() ||
        !packetizationModeIt->is_number_integer()
    )
    // clang-format on
    {
        return 0;
    }

    return packetizationModeIt->get<uint8_t>();
}

static uint8_t getH264LevelAssimetryAllowed(const json& codec)
{
    MSC_TRACE();

    const auto& parameters       = codec["parameters"];
    auto levelAssimetryAllowedIt = parameters.find("level-asymmetry-allowed");

    // clang-format off
    if (
        levelAssimetryAllowedIt == parameters.end() ||
        !levelAssimetryAllowedIt->is_number_integer()
    )
    // clang-format on
    {
        return 0;
    }

    return levelAssimetryAllowedIt->get<uint8_t>();
}

static std::string getH264ProfileLevelId(const json& codec)
{
    MSC_TRACE();

    const auto& parameters = codec["parameters"];
    auto profileLevelIdIt  = parameters.find("profile-level-id");

    if (profileLevelIdIt == parameters.end())
        return "";
    else if (profileLevelIdIt->is_number())
        return std::to_string(profileLevelIdIt->get<int32_t>());
    else
        return profileLevelIdIt->get<std::string>();
}

static std::string getVP9ProfileId(const json& codec)
{
    MSC_TRACE();

    const auto& parameters = codec["parameters"];
    auto profileIdIt       = parameters.find("profile-id");

    if (profileIdIt == parameters.end())
        return "0";

    if (profileIdIt->is_number())
        return std::to_string(profileIdIt->get<int32_t>());
    else
        return profileIdIt->get<std::string>();
}

namespace mediasoupclient
{

std::vector<int> DynamicPayloadTypes =
{
  100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110,
  111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121,
  122, 123, 124, 125, 126, 127, 96, 97, 98, 99
};
void from_json(const json& j, RtpMappingCodec& st) {
    if(j.contains("payloadType"))
      j.at("payloadType").get_to(st.payloadType);
    if(j.contains("mappedPayloadType"))
      j.at("mappedPayloadType").get_to(st.mappedPayloadType);
 
}
void to_json(json& j, RtpMappingCodec& st) {
      j = json{
        {"payloadType", st.payloadType},
        {"mappedPayloadType", st.mappedPayloadType}
    };
}

void from_json(const json& j, RtpMappingEncoding& st) {
    if(j.contains("ssrc"))
      j.at("ssrc").get_to(st.ssrc);
    if(j.contains("rid"))
   j.at("rid").get_to(st.rid);
    if(j.contains("scalabilityMode"))
    j.at("scalabilityMode").get_to(st.scalabilityMode);
    if(j.contains("mappedSsrc"))
     j.at("mappedSsrc").get_to(st.mappedSsrc);
}
void to_json(json& j, RtpMappingEncoding& st) {
      j = json{
        {"ssrc", st.ssrc},
        {"rid", st.rid},
          {"scalabilityMode", st.scalabilityMode},
          {"mappedSsrc", st.mappedSsrc}
        
        
    };
}

void from_json(const json& j, RtpMapping& st) {
    if(j.contains("codecs"))
        j.at("codecs").get_to(st.codecs);
    if(j.contains("encodings"))
        j.at("encodings").get_to(st.encodings);
}
void to_json(json& j, RtpMapping& st) {
    j = json{
        {"codecs", st.codecs},
        {"encodings", st.encodings}
   };
}


	namespace ortc
	{

		/**
		 * Validates RtpCapabilities. It may modify given data by adding missing
		 * fields with default values.
		 * It throws if invalid.
		 */
		void validateRtpCapabilities(json& caps)
		{
			MSC_TRACE();

			if (!caps.is_object())
				MSC_THROW_TYPE_ERROR("caps is not an object");

			auto codecsIt           = caps.find("codecs");
			auto headerExtensionsIt = caps.find("headerExtensions");

			// codecs is optional. If unset, fill with an empty array.
			if (codecsIt != caps.end() && !codecsIt->is_array())
			{
				MSC_THROW_TYPE_ERROR("caps.codecs is not an array");
			}
			else if (codecsIt == caps.end())
			{
				caps["codecs"] = json::array();
				codecsIt       = caps.find("codecs");
			}

			for (auto& codec : *codecsIt)
			{
				validateRtpCodecCapability(codec);
			}

			// headerExtensions is optional. If unset, fill with an empty array.
			if (headerExtensionsIt != caps.end() && !headerExtensionsIt->is_array())
			{
				MSC_THROW_TYPE_ERROR("caps.headerExtensions is not an array");
			}
			else if (headerExtensionsIt == caps.end())
			{
				caps["headerExtensions"] = json::array();
				headerExtensionsIt       = caps.find("headerExtensions");
			}

			for (auto& ext : *headerExtensionsIt)
			{
				validateRtpHeaderExtension(ext);
			}
		}

		/**
		 * Validates RtpCodecCapability. It may modify given data by adding missing
		 * fields with default values.
		 * It throws if invalid.
		 */
		void validateRtpCodecCapability(json& codec)
		{
			MSC_TRACE();

			static const std::regex MimeTypeRegex(
			  "^(audio|video)/(.+)", std::regex_constants::ECMAScript | std::regex_constants::icase);

			if (!codec.is_object())
				MSC_THROW_TYPE_ERROR("codec is not an object");

			auto mimeTypeIt             = codec.find("mimeType");
			auto preferredPayloadTypeIt = codec.find("preferredPayloadType");
			auto clockRateIt            = codec.find("clockRate");
			auto channelsIt             = codec.find("channels");
			auto parametersIt           = codec.find("parameters");
			auto rtcpFeedbackIt         = codec.find("rtcpFeedback");

			// mimeType is mandatory.
			if (mimeTypeIt == codec.end() || !mimeTypeIt->is_string())
				MSC_THROW_TYPE_ERROR("missing codec.mimeType");

			std::smatch mimeTypeMatch;
			std::string regexTarget = mimeTypeIt->get<std::string>();
			std::regex_match(regexTarget, mimeTypeMatch, MimeTypeRegex);

			if (mimeTypeMatch.empty())
				MSC_THROW_TYPE_ERROR("invalid codec.mimeType");

			// Just override kind with media component : mimeType.
			codec["kind"] = mimeTypeMatch[1].str();

			// preferredPayloadType is optional.
			if (preferredPayloadTypeIt != codec.end() && !preferredPayloadTypeIt->is_number_integer())
				MSC_THROW_TYPE_ERROR("invalid codec.preferredPayloadType");

			// clockRate is mandatory.
			if (clockRateIt == codec.end() || !clockRateIt->is_number_integer())
				MSC_THROW_TYPE_ERROR("missing codec.clockRate");

			// channels is optional. If unset, set it to 1 (just if audio).
			if (codec["kind"] == "audio")
			{
				if (channelsIt == codec.end() || !channelsIt->is_number_integer())
					codec["channels"] = 1;
			}
			else
			{
				if (channelsIt != codec.end())
					codec.erase("channels");
			}

			// parameters is optional. If unset, set it to an empty object.
			if (parametersIt == codec.end() || !parametersIt->is_object())
			{
				codec["parameters"] = json::object();
				parametersIt        = codec.find("parameters");
			}

			for (auto it = parametersIt->begin(); it != parametersIt->end(); ++it)
			{
				const auto& key = it.key();
				auto& value     = it.value();

				if (!value.is_string() && !value.is_number() && value != nullptr)
					MSC_THROW_TYPE_ERROR("invalid codec parameter");

				// Specific parameters validation.
				if (key == "apt")
				{
					if (!value.is_number_integer())
						MSC_THROW_TYPE_ERROR("invalid codec apt parameter");
				}
			}

			// rtcpFeedback is optional. If unset, set it to an empty array.
			if (rtcpFeedbackIt == codec.end() || !rtcpFeedbackIt->is_array())
			{
				codec["rtcpFeedback"] = json::array();
				rtcpFeedbackIt        = codec.find("rtcpFeedback");
			}

			for (auto& fb : *rtcpFeedbackIt)
			{
				validateRtcpFeedback(fb);
			}
		}

		/**
		 * Validates RtcpFeedback. It may modify given data by adding missing
		 * fields with default values.
		 * It throws if invalid.
		 */
		void validateRtcpFeedback(json& fb)
		{
			MSC_TRACE();

			if (!fb.is_object())
				MSC_THROW_TYPE_ERROR("fb is not an object");

			auto typeIt      = fb.find("type");
			auto parameterIt = fb.find("parameter");

			// type is mandatory.
			if (typeIt == fb.end() || !typeIt->is_string())
				MSC_THROW_TYPE_ERROR("missing fb.type");

			// parameter is optional. If unset set it to an empty string.
			if (parameterIt == fb.end() || !parameterIt->is_string())
				fb["parameter"] = "";
		}

		/**
		 * Validates RtpHeaderExtension. It may modify given data by adding missing
		 * fields with default values.
		 * It throws if invalid.
		 */
		void validateRtpHeaderExtension(json& ext)
		{
			MSC_TRACE();

			if (!ext.is_object())
				MSC_THROW_TYPE_ERROR("ext is not an object");

			auto kindIt             = ext.find("kind");
			auto uriIt              = ext.find("uri");
			auto preferredIdIt      = ext.find("preferredId");
			auto preferredEncryptIt = ext.find("preferredEncrypt");
			auto directionIt        = ext.find("direction");

			// kind is optional. If unset set it to an empty string.
			if (kindIt == ext.end() || !kindIt->is_string())
				ext["kind"] = "";

			kindIt           = ext.find("kind");
			std::string kind = kindIt->get<std::string>();

			if (!kind.empty() && kind != "audio" && kind != "video")
				MSC_THROW_TYPE_ERROR("invalid ext.kind");

			// uri is mandatory.
			if (uriIt == ext.end() || !uriIt->is_string() || uriIt->get<std::string>().empty())
				MSC_THROW_TYPE_ERROR("missing ext.uri");

			// preferredId is mandatory.
			if (preferredIdIt == ext.end() || !preferredIdIt->is_number_integer())
				MSC_THROW_TYPE_ERROR("missing ext.preferredId");

			// preferredEncrypt is optional. If unset set it to false.
			if (preferredEncryptIt != ext.end() && !preferredEncryptIt->is_boolean())
				MSC_THROW_TYPE_ERROR("invalid ext.preferredEncrypt");
			else if (preferredEncryptIt == ext.end())
				ext["preferredEncrypt"] = false;

			// direction is optional. If unset set it to sendrecv.
			if (directionIt != ext.end() && !directionIt->is_string())
				MSC_THROW_TYPE_ERROR("invalid ext.direction");
			else if (directionIt == ext.end())
				ext["direction"] = "sendrecv";
		}

		/**
		 * Validates RtpParameters. It may modify given data by adding missing
		 * fields with default values.
		 * It throws if invalid.
		 */
		void validateRtpParameters(json& params)
		{
			MSC_TRACE();

			if (!params.is_object())
				MSC_THROW_TYPE_ERROR("params is not an object");

			auto midIt              = params.find("mid");
			auto codecsIt           = params.find("codecs");
			auto headerExtensionsIt = params.find("headerExtensions");
			auto encodingsIt        = params.find("encodings");
			auto rtcpIt             = params.find("rtcp");

			// mid is optional.
			if (midIt != params.end() && (!midIt->is_string() || midIt->get<std::string>().empty()))
			{
				MSC_THROW_TYPE_ERROR("params.mid is not a string");
			}

			// codecs is mandatory.
			if (codecsIt == params.end() || !codecsIt->is_array())
				MSC_THROW_TYPE_ERROR("missing params.codecs");

			for (auto& codec : *codecsIt)
			{
				validateRtpCodecParameters(codec);
			}

			// headerExtensions is optional. If unset, fill with an empty array.
			if (headerExtensionsIt != params.end() && !headerExtensionsIt->is_array())
			{
				MSC_THROW_TYPE_ERROR("params.headerExtensions is not an array");
			}
			else if (headerExtensionsIt == params.end())
			{
				params["headerExtensions"] = json::array();
				headerExtensionsIt         = params.find("headerExtensions");
			}

			for (auto& ext : *headerExtensionsIt)
			{
				validateRtpHeaderExtensionParameters(ext);
			}

			// encodings is optional. If unset, fill with an empty array.
			if (encodingsIt != params.end() && !encodingsIt->is_array())
			{
				MSC_THROW_TYPE_ERROR("params.encodings is not an array");
			}
			else if (encodingsIt == params.end())
			{
				params["encodings"] = json::array();
				encodingsIt         = params.find("encodings");
			}

			for (auto& encoding : *encodingsIt)
			{
				validateRtpEncodingParameters(encoding);
			}

			// rtcp is optional. If unset, fill with an empty object.
			if (rtcpIt != params.end() && !rtcpIt->is_object())
			{
				MSC_THROW_TYPE_ERROR("params.rtcp is not an object");
			}
			else if (rtcpIt == params.end())
			{
				params["rtcp"] = json::object();
				rtcpIt         = params.find("rtcp");
			}

			validateRtcpParameters(*rtcpIt);
		}

		/**
		 * Validates RtpCodecParameters. It may modify given data by adding missing
		 * fields with default values.
		 * It throws if invalid.
		 */
		void validateRtpCodecParameters(json& codec)
		{
			MSC_TRACE();

			static const std::regex MimeTypeRegex(
			  "^(audio|video)/(.+)", std::regex_constants::ECMAScript | std::regex_constants::icase);

			if (!codec.is_object())
				MSC_THROW_TYPE_ERROR("codec is not an object");

			auto mimeTypeIt     = codec.find("mimeType");
			auto payloadTypeIt  = codec.find("payloadType");
			auto clockRateIt    = codec.find("clockRate");
			auto channelsIt     = codec.find("channels");
			auto parametersIt   = codec.find("parameters");
			auto rtcpFeedbackIt = codec.find("rtcpFeedback");

			// mimeType is mandatory.
			if (mimeTypeIt == codec.end() || !mimeTypeIt->is_string())
				MSC_THROW_TYPE_ERROR("missing codec.mimeType");

			std::smatch mimeTypeMatch;
			std::string regexTarget = mimeTypeIt->get<std::string>();
			std::regex_match(regexTarget, mimeTypeMatch, MimeTypeRegex);

			if (mimeTypeMatch.empty())
				MSC_THROW_TYPE_ERROR("invalid codec.mimeType");

			// payloadType is mandatory.
			if (payloadTypeIt == codec.end() || !payloadTypeIt->is_number_integer())
				MSC_THROW_TYPE_ERROR("missing codec.payloadType");

			// clockRate is mandatory.
			if (clockRateIt == codec.end() || !clockRateIt->is_number_integer())
				MSC_THROW_TYPE_ERROR("missing codec.clockRate");

			// Retrieve media kind from mimeType.
			auto kind = mimeTypeMatch[1].str();

			// channels is optional. If unset, set it to 1 (just for audio).
			if (kind == "audio")
			{
				if (channelsIt == codec.end() || !channelsIt->is_number_integer())
					codec["channels"] = 1;
			}
			else
			{
				if (channelsIt != codec.end())
					codec.erase("channels");
			}

			// parameters is optional. If unset, set it to an empty object.
			if (parametersIt == codec.end() || !parametersIt->is_object())
			{
				codec["parameters"] = json::object();
				parametersIt        = codec.find("parameters");
			}

			for (auto it = parametersIt->begin(); it != parametersIt->end(); ++it)
			{
				const auto& key = it.key();
				auto& value     = it.value();

				if (!value.is_string() && !value.is_number() && value != nullptr)
					MSC_THROW_TYPE_ERROR("invalid codec parameter");

				// Specific parameters validation.
				if (key == "apt")
				{
					if (!value.is_number_integer())
						MSC_THROW_TYPE_ERROR("invalid codec apt parameter");
				}
			}

			// rtcpFeedback is optional. If unset, set it to an empty array.
			if (rtcpFeedbackIt == codec.end() || !rtcpFeedbackIt->is_array())
			{
				codec["rtcpFeedback"] = json::array();
				rtcpFeedbackIt        = codec.find("rtcpFeedback");
			}

			for (auto& fb : *rtcpFeedbackIt)
			{
				validateRtcpFeedback(fb);
			}
		}

		/**
		 * Validates RtpHeaderExtensionParameters. It may modify given data by adding missing
		 * fields with default values.
		 * It throws if invalid.
		 */
		void validateRtpHeaderExtensionParameters(json& ext)
		{
			MSC_TRACE();

			if (!ext.is_object())
				MSC_THROW_TYPE_ERROR("ext is not an object");

			auto uriIt        = ext.find("uri");
			auto idIt         = ext.find("id");
			auto encryptIt    = ext.find("encrypt");
			auto parametersIt = ext.find("parameters");

			// uri is mandatory.
			if (uriIt == ext.end() || !uriIt->is_string() || uriIt->get<std::string>().empty())
			{
				MSC_THROW_TYPE_ERROR("missing ext.uri");
			}

			// id is mandatory.
			if (idIt == ext.end() || !idIt->is_number_integer())
				MSC_THROW_TYPE_ERROR("missing ext.id");

			// encrypt is optional. If unset set it to false.
			if (encryptIt != ext.end() && !encryptIt->is_boolean())
				MSC_THROW_TYPE_ERROR("invalid ext.encrypt");
			else if (encryptIt == ext.end())
				ext["encrypt"] = false;

			// parameters is optional. If unset, set it to an empty object.
			if (parametersIt == ext.end() || !parametersIt->is_object())
			{
				ext["parameters"] = json::object();
				parametersIt      = ext.find("parameters");
			}

			for (auto it = parametersIt->begin(); it != parametersIt->end(); ++it)
			{
				auto& value = it.value();

				if (!value.is_string() && !value.is_number())
					MSC_THROW_TYPE_ERROR("invalid header extension parameter");
			}
		}

		/**
		 * Validates RtpEncodingParameters. It may modify given data by adding missing
		 * fields with default values.
		 * It throws if invalid.
		 */
		void validateRtpEncodingParameters(json& encoding)
		{
			MSC_TRACE();

			if (!encoding.is_object())
				MSC_THROW_TYPE_ERROR("encoding is not an object");
            std::cout << "[worker] validateRtpEncodingParameters encoding="<<encoding.dump(4);
			auto ssrcIt            = encoding.find("ssrc");
			auto ridIt             = encoding.find("rid");
			auto rtxIt             = encoding.find("rtx");
			auto dtxIt             = encoding.find("dtx");
			auto scalabilityModeIt = encoding.find("scalabilityMode");

			// ssrc is optional.
			if (ssrcIt != encoding.end() && !ssrcIt->is_number_integer())
				MSC_THROW_TYPE_ERROR("invalid encoding.ssrc");

			// rid is optional.
			if (ridIt != encoding.end() && (!ridIt->is_string()))
			{
				MSC_THROW_TYPE_ERROR("invalid encoding.rid");
			}

			// rtx is optional.
			if (rtxIt != encoding.end() && !rtxIt->is_object())
			{
				MSC_THROW_TYPE_ERROR("invalid encoding.rtx");
			}
			else if (rtxIt != encoding.end())
			{
				auto rtxSsrcIt = rtxIt->find("ssrc");

				// RTX ssrc is mandatory if rtx is present.
				if (rtxSsrcIt == rtxIt->end() || !rtxSsrcIt->is_number_integer())
					MSC_THROW_TYPE_ERROR("missing encoding.rtx.ssrc");
			}

			// dtx is optional. If unset set it to false.
			if (dtxIt == encoding.end() || !dtxIt->is_boolean())
				encoding["dtx"] = false;

			// scalabilityMode is optional.
			// clang-format off
			if (
				scalabilityModeIt != encoding.end() &&
				(!scalabilityModeIt->is_string())
			)
			// clang-format on
			{
				MSC_THROW_TYPE_ERROR("invalid encoding.scalabilityMode");
			}
		}

		/**
		 * Validates RtcpParameters. It may modify given data by adding missing
		 * fields with default values.
		 * It throws if invalid.
		 */
		void validateRtcpParameters(json& rtcp)
		{
			MSC_TRACE();

			if (!rtcp.is_object())
				MSC_THROW_TYPE_ERROR("rtcp is not an object");

			auto cnameIt       = rtcp.find("cname");
			auto reducedSizeIt = rtcp.find("reducedSize");

			// cname is optional.
			if (cnameIt != rtcp.end() && !cnameIt->is_string())
				MSC_THROW_TYPE_ERROR("invalid rtcp.cname");

			// reducedSize is optional. If unset set it to true.
			if (reducedSizeIt == rtcp.end() || !reducedSizeIt->is_boolean())
				rtcp["reducedSize"] = true;
		}

		/**
		 * Validates SctpCapabilities. It may modify given data by adding missing
		 * fields with default values.
		 * It throws if invalid.
		 */
		void validateSctpCapabilities(json& caps)
		{
			MSC_TRACE();

			if (!caps.is_object())
				MSC_THROW_TYPE_ERROR("caps is not an object");

			auto numStreamsIt = caps.find("numStreams");

			// numStreams is mandatory.
			if (numStreamsIt == caps.end() || !numStreamsIt->is_object())
				MSC_THROW_TYPE_ERROR("missing caps.numStreams");

			ortc::validateNumSctpStreams(*numStreamsIt);
		}

		/**
		 * Validates NumSctpStreams. It may modify given data by adding missing
		 * fields with default values.
		 * It throws if invalid.
		 */
		void validateNumSctpStreams(json& numStreams)
		{
			MSC_TRACE();

			if (!numStreams.is_object())
				MSC_THROW_TYPE_ERROR("numStreams is not an object");

			auto osIt  = numStreams.find("OS");
			auto misIt = numStreams.find("MIS");

			// OS is mandatory.
			if (osIt == numStreams.end() || !osIt->is_number_integer())
				MSC_THROW_TYPE_ERROR("missing numStreams.OS");

			// MIS is mandatory.
			if (misIt == numStreams.end() || !misIt->is_number_integer())
				MSC_THROW_TYPE_ERROR("missing numStreams.MIS");
		}

		/**
		 * Validates SctpParameters. It may modify given data by adding missing
		 * fields with default values.
		 * It throws if invalid.
		 */
		void validateSctpParameters(json& params)
		{
			MSC_TRACE();

			if (!params.is_object())
				MSC_THROW_TYPE_ERROR("params is not an object");

			auto portIt           = params.find("port");
			auto osIt             = params.find("OS");
			auto misIt            = params.find("MIS");
			auto maxMessageSizeIt = params.find("maxMessageSize");

			// port is mandatory.
			if (portIt == params.end() || !portIt->is_number_integer())
				MSC_THROW_TYPE_ERROR("missing params.port");

			// OS is mandatory.
			if (osIt == params.end() || !osIt->is_number_integer())
				MSC_THROW_TYPE_ERROR("missing params.OS");

			// MIS is mandatory.
			if (misIt == params.end() || !misIt->is_number_integer())
				MSC_THROW_TYPE_ERROR("missing params.MIS");

			// maxMessageSize is mandatory.
			if (maxMessageSizeIt == params.end() || !maxMessageSizeIt->is_number_integer())
			{
				MSC_THROW_TYPE_ERROR("missing params.maxMessageSize");
			}
		}

		/**
		 * Validates SctpStreamParameters. It may modify given data by adding missing
		 * fields with default values.
		 * It throws if invalid.
		 */
		void validateSctpStreamParameters(json& params)
		{
			MSC_TRACE();

			if (!params.is_object())
				MSC_THROW_TYPE_ERROR("params is not an object");

			auto streamIdIt          = params.find("streamId");
			auto orderedIt           = params.find("ordered");
			auto maxPacketLifeTimeIt = params.find("maxPacketLifeTime");
			auto maxRetransmitsIt    = params.find("maxRetransmits");
			auto labelIt             = params.find("label");
			auto protocolIt          = params.find("protocol");

			// streamId is mandatory.
			if (streamIdIt == params.end() || !streamIdIt->is_number_integer())
				MSC_THROW_TYPE_ERROR("missing params.streamId");

			// ordered is optional.
			bool orderedGiven = false;

			if (orderedIt != params.end() && orderedIt->is_boolean())
				orderedGiven = true;
			else
				params["ordered"] = true;

			// maxPacketLifeTime is optional. If unset set it to 0.
			if (maxPacketLifeTimeIt == params.end() || !maxPacketLifeTimeIt->is_number_integer())
			{
				params["maxPacketLifeTime"] = 0u;
			}

			// maxRetransmits is optional. If unset set it to 0.
			if (maxRetransmitsIt == params.end() || !maxRetransmitsIt->is_number_integer())
			{
				params["maxRetransmits"] = 0u;
			}

			if (maxPacketLifeTimeIt != params.end() && maxRetransmitsIt != params.end())
			{
				MSC_THROW_TYPE_ERROR("cannot provide both maxPacketLifeTime and maxRetransmits");
			}

			// clang-format off
			if (
				orderedGiven &&
				params["ordered"] == true &&
				(maxPacketLifeTimeIt != params.end() || maxRetransmitsIt != params.end())
			)
			// clang-format on
			{
				MSC_THROW_TYPE_ERROR("cannot be ordered with maxPacketLifeTime or maxRetransmits");
			}
			// clang-format off
			else if (
				!orderedGiven &&
				(maxPacketLifeTimeIt != params.end() || maxRetransmitsIt != params.end())
			)
			// clang-format on
			{
				params["ordered"] = false;
			}

			// label is optional. If unset set it to empty string.
			if (labelIt == params.end() || !labelIt->is_string())
				params["label"] = "";

			// protocol is optional. If unset set it to empty string.
			if (protocolIt == params.end() || !protocolIt->is_string())
				params["protocol"] = "";
		}

		/**
		 * Validates IceParameters. It may modify given data by adding missing
		 * fields with default values.
		 * It throws if invalid.
		 */
		void validateIceParameters(json& params)
		{
			MSC_TRACE();

			if (!params.is_object())
				MSC_THROW_TYPE_ERROR("params is not an object");

			auto usernameFragmentIt = params.find("usernameFragment");
			auto passwordIt         = params.find("password");
			auto iceLiteIt          = params.find("iceLite");

			// usernameFragment is mandatory.
			if (
			  usernameFragmentIt == params.end() ||
			  (!usernameFragmentIt->is_string() || usernameFragmentIt->get<std::string>().empty()))
			{
				MSC_THROW_TYPE_ERROR("missing params.usernameFragment");
			}

			// userFragment is mandatory.
			if (passwordIt == params.end() || (!passwordIt->is_string() || passwordIt->get<std::string>().empty()))
			{
				MSC_THROW_TYPE_ERROR("missing params.password");
			}

			// iceLIte is optional. If unset set it to false.
			if (iceLiteIt == params.end() || !iceLiteIt->is_boolean())
				params["iceLite"] = false;
		}

		/**
		 * Validates IceCandidate. It may modify given data by adding missing
		 * fields with default values.
		 * It throws if invalid.
		 */
		void validateIceCandidate(json& params)
		{
			MSC_TRACE();

			static const std::regex ProtocolRegex(
			  "(udp|tcp)", std::regex_constants::ECMAScript | std::regex_constants::icase);

			static const std::regex TypeRegex(
			  "(host|srflx|prflx|relay)", std::regex_constants::ECMAScript | std::regex_constants::icase);

			if (!params.is_object())
				MSC_THROW_TYPE_ERROR("params is not an object");

			auto foundationIt = params.find("foundation");
			auto priorityIt   = params.find("priority");
			auto ipIt         = params.find("ip");
			auto protocolIt   = params.find("protocol");
			auto portIt       = params.find("port");
			auto typeIt       = params.find("type");

			// foundation is mandatory.
			if (
			  foundationIt == params.end() ||
			  (!foundationIt->is_string() || foundationIt->get<std::string>().empty()))
			{
				MSC_THROW_TYPE_ERROR("missing params.foundation");
			}

			// priority is mandatory.
			if (priorityIt == params.end() || !priorityIt->is_number_unsigned())
				MSC_THROW_TYPE_ERROR("missing params.priority");

			// ip is mandatory.
			if (ipIt == params.end() || (!ipIt->is_string() || ipIt->get<std::string>().empty()))
			{
				MSC_THROW_TYPE_ERROR("missing params.ip");
			}

			// protocol is mandatory.
			if (protocolIt == params.end() || (!protocolIt->is_string() || protocolIt->get<std::string>().empty()))
			{
				MSC_THROW_TYPE_ERROR("missing params.protocol");
			}

			std::smatch protocolMatch;
			std::string regexTarget = protocolIt->get<std::string>();
			std::regex_match(regexTarget, protocolMatch, ProtocolRegex);

			if (protocolMatch.empty())
				MSC_THROW_TYPE_ERROR("invalid params.protocol");

			// port is mandatory.
			if (portIt == params.end() || !portIt->is_number_unsigned())
				MSC_THROW_TYPE_ERROR("missing params.port");

			// type is mandatory.
			if (typeIt == params.end() || (!typeIt->is_string() || typeIt->get<std::string>().empty()))
			{
				MSC_THROW_TYPE_ERROR("missing params.type");
			}

			std::smatch typeMatch;
			regexTarget = typeIt->get<std::string>();
			std::regex_match(regexTarget, typeMatch, TypeRegex);

			if (typeMatch.empty())
				MSC_THROW_TYPE_ERROR("invalid params.type");
		}

		/**
		 * Validates IceCandidates. It may modify given data by adding missing
		 * fields with default values.
		 * It throws if invalid.
		 */
		void validateIceCandidates(json& params)
		{
			MSC_TRACE();

			if (!params.is_array())
				MSC_THROW_TYPE_ERROR("params is not an array");

			for (auto& iceCandidate : params)
			{
				validateIceCandidate(iceCandidate);
			}
		}

		/**
		 * Validates DtlsFingerprint. It may modify given data by adding missing
		 * fields with default values.
		 * It throws if invalid.
		 */
		void validateDtlsFingerprint(json& params)
		{
			MSC_TRACE();

			if (!params.is_object())
				MSC_THROW_TYPE_ERROR("params is not an object");

			auto algorithmIt = params.find("algorithm");
			auto valueIt     = params.find("value");

			// foundation is mandatory.
			if (
			  algorithmIt == params.end() ||
			  (!algorithmIt->is_string() || algorithmIt->get<std::string>().empty()))
			{
				MSC_THROW_TYPE_ERROR("missing params.algorithm");
			}

			// foundation is mandatory.
			if (valueIt == params.end() || (!valueIt->is_string() || valueIt->get<std::string>().empty()))
			{
				MSC_THROW_TYPE_ERROR("missing params.value");
			}
		}

		/**
		 * Validates DtlsParameters. It may modify given data by adding missing
		 * fields with default values.
		 * It throws if invalid.
		 */
		void validateDtlsParameters(json& params)
		{
			MSC_TRACE();

			static const std::regex RoleRegex(
			  "(auto|client|server)", std::regex_constants::ECMAScript | std::regex_constants::icase);

			if (!params.is_object())
				MSC_THROW_TYPE_ERROR("params is not an object");

			auto roleIt         = params.find("role");
			auto fingerprintsIt = params.find("fingerprints");

			// role is mandatory.
			if (roleIt == params.end() || (!roleIt->is_string() || roleIt->get<std::string>().empty()))
			{
				MSC_THROW_TYPE_ERROR("missing params.role");
			}

			std::smatch roleMatch;
			std::string regexTarget = roleIt->get<std::string>();
			std::regex_match(regexTarget, roleMatch, RoleRegex);

			if (roleMatch.empty())
				MSC_THROW_TYPE_ERROR("invalid params.role");

			// fingerprints is mandatory.
			if (fingerprintsIt == params.end() || (!fingerprintsIt->is_array() || fingerprintsIt->empty()))
			{
				MSC_THROW_TYPE_ERROR("missing params.fingerprints");
			}

			for (auto& fingerprint : *fingerprintsIt)
				validateDtlsFingerprint(fingerprint);
		}

		/**
		 * Validates Producer codec options. It may modify given data by adding missing
		 * fields with default values.
		 * It throws if invalid.
		 */
		void validateProducerCodecOptions(json& params)
		{
			MSC_TRACE();

			if (!params.is_object())
				MSC_THROW_TYPE_ERROR("params is not an object");

			auto opusStereoIt              = params.find("opusStereo");
			auto opusFecIt                 = params.find("opusFec");
			auto opusDtxIt                 = params.find("opusDtx");
			auto opusMaxPlaybackRateIt     = params.find("opusMaxPlaybackRate");
			auto opusPtimeIt               = params.find("opusPtime");
			auto videoGoogleStartBitrateIt = params.find("videoGoogleStartBitrate");
			auto videoGoogleMaxBitrateIt   = params.find("videoGoogleMaxBitrate");
			auto videoGoogleMinBitrateIt   = params.find("videoGoogleMinBitrate");

			if (opusStereoIt != params.end() && !opusStereoIt->is_boolean())
			{
				MSC_THROW_TYPE_ERROR("invalid params.opusStereo");
			}

			if (opusFecIt != params.end() && !opusFecIt->is_boolean())
			{
				MSC_THROW_TYPE_ERROR("invalid params.opusFec");
			}

			if (opusDtxIt != params.end() && !opusDtxIt->is_boolean())
			{
				MSC_THROW_TYPE_ERROR("invalid params.opusDtx");
			}

			if (opusMaxPlaybackRateIt != params.end() && !opusMaxPlaybackRateIt->is_number_unsigned())
			{
				MSC_THROW_TYPE_ERROR("invalid params.opusMaxPlaybackRate");
			}

			if (opusPtimeIt != params.end() && !opusPtimeIt->is_number_integer())
			{
				MSC_THROW_TYPE_ERROR("invalid params.opusPtime");
			}

			if (videoGoogleStartBitrateIt != params.end() && !videoGoogleStartBitrateIt->is_number_integer())
			{
				MSC_THROW_TYPE_ERROR("invalid params.videoGoogleStartBitrate");
			}

			if (videoGoogleMaxBitrateIt != params.end() && !videoGoogleMaxBitrateIt->is_number_integer())
			{
				MSC_THROW_TYPE_ERROR("invalid params.videoGoogleMaxBitrate");
			}

			if (videoGoogleMinBitrateIt != params.end() && !videoGoogleMinBitrateIt->is_number_integer())
			{
				MSC_THROW_TYPE_ERROR("invalid params.videoGoogleMinBitrate");
			}
		}

		/**
		 * Generate extended RTP capabilities for sending and receiving.
		 */
		json getExtendedRtpCapabilities(json& localCaps, json& remoteCaps)
		{
			MSC_TRACE();

			// This may throw.
			validateRtpCapabilities(localCaps);
			validateRtpCapabilities(remoteCaps);

			static const std::regex MimeTypeRegex(
			  "^(audio|video)/(.+)", std::regex_constants::ECMAScript | std::regex_constants::icase);

			// clang-format off
			json extendedRtpCapabilities =
			{
				{ "codecs",           json::array() },
				{ "headerExtensions", json::array() }
			};
			// clang-format on

			// Match media codecs and keep the order preferred by remoteCaps.
			auto remoteCapsCodecsIt = remoteCaps.find("codecs");

			for (auto& remoteCodec : *remoteCapsCodecsIt)
			{
				if (isRtxCodec(remoteCodec))
					continue;

				json& localCodecs = localCaps["codecs"];

				auto matchingLocalCodecIt =
				  std::find_if(localCodecs.begin(), localCodecs.end(), [&remoteCodec](json& localCodec) {
					  return matchCodecs(localCodec, remoteCodec, /*strict*/ true, /*modify*/ true);
				  });

				if (matchingLocalCodecIt == localCodecs.end())
					continue;

				auto& matchingLocalCodec = *matchingLocalCodecIt;

				// clang-format off
				json extendedCodec =
				{
					{ "mimeType",             matchingLocalCodec["mimeType"]                      },
					{ "kind",                 matchingLocalCodec["kind"]                          },
					{ "clockRate",            matchingLocalCodec["clockRate"]                     },
					{ "localPayloadType",     matchingLocalCodec["preferredPayloadType"]          },
					{ "localRtxPayloadType",  nullptr                                             },
					{ "remotePayloadType",    remoteCodec["preferredPayloadType"]                 },
					{ "remoteRtxPayloadType", nullptr                                             },
					{ "localParameters",      matchingLocalCodec["parameters"]                    },
					{ "remoteParameters",     remoteCodec["parameters"]                           },
					{ "rtcpFeedback",         reduceRtcpFeedback(matchingLocalCodec, remoteCodec) }
				};
				// clang-format on

				if (matchingLocalCodec.contains("channels"))
					extendedCodec["channels"] = matchingLocalCodec["channels"];

				extendedRtpCapabilities["codecs"].push_back(extendedCodec);
			}

			// Match RTX codecs.
			json& extendedCodecs = extendedRtpCapabilities["codecs"];

			for (json& extendedCodec : extendedCodecs)
			{
				auto& localCodecs = localCaps["codecs"];
				auto localCodecIt = std::find_if(
				  localCodecs.begin(), localCodecs.end(), [&extendedCodec](const json& localCodec) {
					  return isRtxCodec(localCodec) &&
					         localCodec["parameters"]["apt"] == extendedCodec["localPayloadType"];
				  });

				if (localCodecIt == localCodecs.end())
					continue;

				auto& matchingLocalRtxCodec = *localCodecIt;
				auto& remoteCodecs          = remoteCaps["codecs"];
				auto remoteCodecIt          = std::find_if(
          remoteCodecs.begin(), remoteCodecs.end(), [&extendedCodec](const json& remoteCodec) {
            return isRtxCodec(remoteCodec) &&
                   remoteCodec["parameters"]["apt"] == extendedCodec["remotePayloadType"];
          });

				if (remoteCodecIt == remoteCodecs.end())
					continue;

				auto& matchingRemoteRtxCodec = *remoteCodecIt;

				extendedCodec["localRtxPayloadType"]  = matchingLocalRtxCodec["preferredPayloadType"];
				extendedCodec["remoteRtxPayloadType"] = matchingRemoteRtxCodec["preferredPayloadType"];
			}

			// Match header extensions.
			auto& remoteExts = remoteCaps["headerExtensions"];

			for (auto& remoteExt : remoteExts)
			{
				auto& localExts = localCaps["headerExtensions"];
				auto localExtIt =
				  std::find_if(localExts.begin(), localExts.end(), [&remoteExt](const json& localExt) {
					  return matchHeaderExtensions(localExt, remoteExt);
				  });

				if (localExtIt == localExts.end())
					continue;

				auto& matchingLocalExt = *localExtIt;

				// TODO: Must do stuff for encrypted extensions.

				// clang-format off
				json extendedExt =
				{
					{ "kind",    remoteExt["kind"]                    },
					{ "uri",     remoteExt["uri"]                     },
					{ "sendId",  matchingLocalExt["preferredId"]      },
					{ "recvId",  remoteExt["preferredId"]             },
					{ "encrypt", matchingLocalExt["preferredEncrypt"] }
				};
				// clang-format on

				auto remoteExtDirection = remoteExt["direction"].get<std::string>();

				if (remoteExtDirection == "sendrecv")
					extendedExt["direction"] = "sendrecv";
				else if (remoteExtDirection == "recvonly")
					extendedExt["direction"] = "sendonly";
				else if (remoteExtDirection == "sendonly")
					extendedExt["direction"] = "recvonly";
				else if (remoteExtDirection == "inactive")
					extendedExt["direction"] = "inactive";

				extendedRtpCapabilities["headerExtensions"].push_back(extendedExt);
			}

			return extendedRtpCapabilities;
		}

		/**
		 * Generate RTP capabilities for receiving media based on the given extended
		 * RTP capabilities.
		 */
		json getRecvRtpCapabilities(const json& extendedRtpCapabilities)
		{
			MSC_TRACE();

			// clang-format off
			json rtpCapabilities =
			{
				{ "codecs",           json::array() },
				{ "headerExtensions", json::array() }
			};
			// clang-format on

			for (const auto& extendedCodec : extendedRtpCapabilities["codecs"])
			{
				// clang-format off
				json codec =
				{
					{ "mimeType",             extendedCodec["mimeType"]          },
					{ "kind",                 extendedCodec["kind"]              },
					{ "preferredPayloadType", extendedCodec["remotePayloadType"] },
					{ "clockRate",            extendedCodec["clockRate"]         },
					{ "parameters",           extendedCodec["localParameters"]   },
					{ "rtcpFeedback",         extendedCodec["rtcpFeedback"]      },
				};
				// clang-format on

				if (extendedCodec.contains("channels"))
					codec["channels"] = extendedCodec["channels"];

				rtpCapabilities["codecs"].push_back(codec);

				// Add RTX codec.
				if (extendedCodec["remoteRtxPayloadType"] == nullptr)
					continue;

				auto mimeType = extendedCodec["kind"].get<std::string>().append("/rtx");

				// clang-format off
				json rtxCodec =
				{
					{ "mimeType",             mimeType                              },
					{ "kind",                 extendedCodec["kind"]                 },
					{ "preferredPayloadType", extendedCodec["remoteRtxPayloadType"] },
					{ "clockRate",            extendedCodec["clockRate"]            },
					{
						"parameters",
						{
							{ "apt", extendedCodec["remotePayloadType"].get<uint8_t>() }
						}
					},
					{ "rtcpFeedback", json::array() }
				};
				// clang-format on

				rtpCapabilities["codecs"].push_back(rtxCodec);

				// TODO: In the future, we need to add FEC, CN, etc, codecs.
			}

			for (const auto& extendedExtension : extendedRtpCapabilities["headerExtensions"])
			{
				std::string direction = extendedExtension["direction"].get<std::string>();

				// Ignore RTP extensions not valid for receiving.
				if (direction != "sendrecv" && direction != "recvonly")
					continue;

				// clang-format off
				json ext =
				{
					{ "kind",             extendedExtension["kind"]      },
					{ "uri",              extendedExtension["uri"]       },
					{ "preferredId",      extendedExtension["recvId"]    },
					{ "preferredEncrypt", extendedExtension["encrypt"]   },
					{ "direction",        extendedExtension["direction"] }
				};
				// clang-format on

				rtpCapabilities["headerExtensions"].push_back(ext);
			}

			return rtpCapabilities;
		}

		/**
		 * Generate RTP parameters : the given kind for sending media.
		 * Just the first media codec per kind is considered.
		 * NOTE: mid, encodings and rtcp fields are left empty.
		 */
		json getSendingRtpParameters(const std::string& kind, const json& extendedRtpCapabilities)
		{
			MSC_TRACE();

			// clang-format off
			json rtpParameters =
			{
				{ "mid",              nullptr        },
				{ "codecs",           json::array()  },
				{ "headerExtensions", json::array()  },
				{ "encodings",        json::array()  },
				{ "rtcp",             json::object() }
			};
			// clang-format on

			for (const auto& extendedCodec : extendedRtpCapabilities["codecs"])
			{
				if (kind != extendedCodec["kind"].get<std::string>())
					continue;

				// clang-format off
				json codec =
				{
					{ "mimeType",     extendedCodec["mimeType"]         },
					{ "payloadType",  extendedCodec["localPayloadType"] },
					{ "clockRate",    extendedCodec["clockRate"]        },
					{ "parameters",   extendedCodec["localParameters"]  },
					{ "rtcpFeedback", extendedCodec["rtcpFeedback"]     }
				};
				// clang-format on

				if (extendedCodec.contains("channels"))
					codec["channels"] = extendedCodec["channels"];

				rtpParameters["codecs"].push_back(codec);

				// Add RTX codec.
				if (extendedCodec["localRtxPayloadType"] != nullptr)
				{
					auto mimeType = extendedCodec["kind"].get<std::string>().append("/rtx");

					// clang-format off
					json rtxCodec =
					{
						{ "mimeType",    mimeType                             },
						{ "payloadType", extendedCodec["localRtxPayloadType"] },
						{ "clockRate",   extendedCodec["clockRate"]           },
						{
							"parameters",
							{
								{ "apt", extendedCodec["localPayloadType"].get<uint8_t>() }
							}
						},
						{ "rtcpFeedback", json::array() }
					};
					// clang-format on

					rtpParameters["codecs"].push_back(rtxCodec);
				}

				// NOTE: We assume a single media codec plus an optional RTX codec.
				break;
			}

			for (const auto& extendedExtension : extendedRtpCapabilities["headerExtensions"])
			{
				if (kind != extendedExtension["kind"].get<std::string>())
					continue;

				std::string direction = extendedExtension["direction"].get<std::string>();

				// Ignore RTP extensions not valid for sending.
				if (direction != "sendrecv" && direction != "sendonly")
					continue;

				// clang-format off
				json ext =
				{
					{ "uri",        extendedExtension["uri"]     },
					{ "id",         extendedExtension["sendId"]  },
					{ "encrypt",    extendedExtension["encrypt"] },
					{ "parameters", json::object()               }
				};
				// clang-format on

				rtpParameters["headerExtensions"].push_back(ext);
			}

			return rtpParameters;
		}

		/**
		 * Generate RTP parameters : the given kind for sending media.
		 */
		json getSendingRemoteRtpParameters(const std::string& kind, const json& extendedRtpCapabilities)
		{
			MSC_TRACE();

			// clang-format off
			json rtpParameters =
			{
				{ "mid",              nullptr        },
				{ "codecs",           json::array()  },
				{ "headerExtensions", json::array()  },
				{ "encodings",        json::array()  },
				{ "rtcp",             json::object() }
			};
			// clang-format on

			for (const auto& extendedCodec : extendedRtpCapabilities["codecs"])
			{
				if (kind != extendedCodec["kind"].get<std::string>())
					continue;

				// clang-format off
				json codec =
				{
					{ "mimeType",     extendedCodec["mimeType"]         },
					{ "payloadType",  extendedCodec["localPayloadType"] },
					{ "clockRate",    extendedCodec["clockRate"]        },
					{ "parameters",   extendedCodec["remoteParameters"] },
					{ "rtcpFeedback", extendedCodec["rtcpFeedback"]     }
				};
				// clang-format on

				if (extendedCodec.contains("channels"))
					codec["channels"] = extendedCodec["channels"];

				rtpParameters["codecs"].push_back(codec);

				// Add RTX codec.
				if (extendedCodec["localRtxPayloadType"] != nullptr)
				{
					auto mimeType = extendedCodec["kind"].get<std::string>().append("/rtx");

					// clang-format off
					json rtxCodec =
					{
						{ "mimeType",    mimeType                             },
						{ "payloadType", extendedCodec["localRtxPayloadType"] },
						{ "clockRate",   extendedCodec["clockRate"]           },
						{
							"parameters",
							{
								{ "apt", extendedCodec["localPayloadType"].get<uint8_t>() }
							}
						},
						{ "rtcpFeedback", json::array() }
					};
					// clang-format on

					rtpParameters["codecs"].push_back(rtxCodec);
				}

				// NOTE: We assume a single media codec plus an optional RTX codec.
				break;
			}

			for (const auto& extendedExtension : extendedRtpCapabilities["headerExtensions"])
			{
				if (kind != extendedExtension["kind"].get<std::string>())
					continue;

				std::string direction = extendedExtension["direction"].get<std::string>();

				// Ignore RTP extensions not valid for sending.
				if (direction != "sendrecv" && direction != "sendonly")
					continue;

				// clang-format off
				json ext =
				{
					{ "uri",        extendedExtension["uri"]     },
					{ "id",         extendedExtension["sendId"]  },
					{ "encrypt",    extendedExtension["encrypt"] },
					{ "parameters", json::object()               }
				};
				// clang-format on

				rtpParameters["headerExtensions"].push_back(ext);
			}

			auto headerExtensionsIt = rtpParameters.find("headerExtensions");

			// Reduce codecs" RTCP feedback. Use Transport-CC if available, REMB otherwise.
			auto headerExtensionIt =
			  std::find_if(headerExtensionsIt->begin(), headerExtensionsIt->end(), [](json& ext) {
				  return ext["uri"].get<std::string>() ==
				         "http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01";
			  });

			if (headerExtensionIt != headerExtensionsIt->end())
			{
				for (auto& codec : rtpParameters["codecs"])
				{
					auto& rtcpFeedback = codec["rtcpFeedback"];

					for (auto it = rtcpFeedback.begin(); it != rtcpFeedback.end();)
					{
						auto& fb  = *it;
						auto type = fb["type"].get<std::string>();

						if (type == "goog-remb")
							it = rtcpFeedback.erase(it);
						else
							++it;
					}
				}

				return rtpParameters;
			}

			headerExtensionIt =
			  std::find_if(headerExtensionsIt->begin(), headerExtensionsIt->end(), [](json& ext) {
				  return ext["uri"].get<std::string>() ==
				         "http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time";
			  });

			if (headerExtensionIt != headerExtensionsIt->end())
			{
				for (auto& codec : rtpParameters["codecs"])
				{
					auto& rtcpFeedback = codec["rtcpFeedback"];

					for (auto it = rtcpFeedback.begin(); it != rtcpFeedback.end();)
					{
						auto& fb  = *it;
						auto type = fb["type"].get<std::string>();

						if (type == "transport-cc")
							it = rtcpFeedback.erase(it);
						else
							++it;
					}
				}

				return rtpParameters;
			}

			for (auto& codec : rtpParameters["codecs"])
			{
				auto& rtcpFeedback = codec["rtcpFeedback"];

				for (auto it = rtcpFeedback.begin(); it != rtcpFeedback.end();)
				{
					auto& fb  = *it;
					auto type = fb["type"].get<std::string>();

					if (type == "transport-cc" || type == "goog-remb")
						it = rtcpFeedback.erase(it);
					else
						++it;
				}
			}

			return rtpParameters;
		}

		/**
		 * Create RTP parameters for a Consumer for the RTP probator.
		 */
		const json generateProbatorRtpParameters(const json& videoRtpParameters)
		{
			MSC_TRACE();

			// This may throw.
			json validatedRtpParameters = videoRtpParameters;

			// This may throw.
			ortc::validateRtpParameters(validatedRtpParameters);

			// clang-format off
			json rtpParameters =
			{
				{ "mid",              ProbatorMid    },
				{ "codecs",           json::array()  },
				{ "headerExtensions", json::array()  },
				{ "encodings",        json::array()  },
				{
					"rtcp",
					{
						{ "cname", "probator" }
					}
				}
			};
			// clang-format on

			rtpParameters["codecs"].push_back(validatedRtpParameters["codecs"][0]);

			for (auto& ext : validatedRtpParameters["headerExtensions"])
			{
				// clang-format off
				if (
					ext["uri"] == "http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time" ||
					ext["uri"] == "http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01"
				)
				// clang-format on
				{
					rtpParameters["headerExtensions"].push_back(ext);
				}
			}

			json encoding = json::object();

			encoding["ssrc"] = ProbatorSsrc;

			rtpParameters["encodings"].push_back(encoding);

			return rtpParameters;
		}

		/**
		 * Whether media can be sent based on the given RTP capabilities.
		 */
		bool canSend(const std::string& kind, const json& extendedRtpCapabilities)
		{
			MSC_TRACE();

			const auto& codecs = extendedRtpCapabilities["codecs"];
			auto codecIt       = std::find_if(codecs.begin(), codecs.end(), [&kind](const json& codec) {
        return kind == codec["kind"].get<std::string>();
      });

			return codecIt != codecs.end();
		}

		/**
		 * Whether the given RTP parameters can be received with the given RTP
		 * capabilities.
		 */
		bool canReceive(json& rtpParameters, const json& extendedRtpCapabilities)
		{
			MSC_TRACE();

			// This may throw.
			validateRtpParameters(rtpParameters);

			if (rtpParameters["codecs"].empty())
				return false;

			auto& firstMediaCodec = rtpParameters["codecs"][0];
			const auto& codecs    = extendedRtpCapabilities["codecs"];
			auto codecIt =
			  std::find_if(codecs.begin(), codecs.end(), [&firstMediaCodec](const json& codec) {
				  return codec["remotePayloadType"] == firstMediaCodec["payloadType"];
			  });

			return codecIt != codecs.end();
		}

   

    /**
     * Generate RTP capabilities for the Router based on the given media codecs and
     * mediasoup supported RTP capabilities.
     */
    RtpCapabilities generateRouterRtpCapabilities(
      std::vector<RtpCodecCapability> &mediaCodecs
      //mediaCodecs: RtpCodecCapability[] = []
    )//: RtpCapabilities
    {
      // Normalize supported RTP capabilities.
      json cap = supportedRtpCapabilities;
      validateRtpCapabilities(cap);

      //if (!Array.isArray(mediaCodecs))
      //  MSC_THROW_TYPE_ERROR("mediaCodecs must be an Array");

        auto clonedSupportedRtpCapabilities = supportedRtpCapabilities;
      //  utils.clone(supportedRtpCapabilities) as RtpCapabilities;
      std::vector<int> dynamicPayloadTypes;//= DynamicPayloadTypes;//utils.clone(DynamicPayloadTypes) as number[];
      dynamicPayloadTypes.insert(dynamicPayloadTypes.end(), DynamicPayloadTypes.begin(), DynamicPayloadTypes.end());
        
      RtpCapabilities caps;
      //{
          caps.headerExtensions =  clonedSupportedRtpCapabilities.headerExtensions;
      //};

      for (auto &mediaCodec : mediaCodecs)
      {
        // This may throw.
        json jmediaCodec = mediaCodec;
        validateRtpCodecCapability(jmediaCodec);

//        auto  matchedSupportedCodec = clonedSupportedRtpCapabilities
//          .codecs
//          .find((supportedCodec) => (
//            matchCodecs(mediaCodec, supportedCodec, { strict: false }))
//          );
        RtpCodecCapability matchedSupportedCodec;
        bool matched = false;
        for(auto &supportedCodec : clonedSupportedRtpCapabilities.codecs)
        {
            json jsupportedCodec = supportedCodec;
            matched = matchCodecs(jmediaCodec, jsupportedCodec,/*strict*/ false, /*modify*/ false);
            if(matched == true) {
                matchedSupportedCodec = supportedCodec;
                break;
            }
        }
          
        if (matched == false)
        {
          MSC_ERROR(
            "media codec not supported [mimeType:%s]",mediaCodec.mimeType.c_str());
            continue;
        }

        // Clone the supported codec.
        auto codec = matchedSupportedCodec;//utils.clone(matchedSupportedCodec) as RtpCodecCapability;

        // If the given media codec has preferredPayloadType, keep it.
        //if (typeof mediaCodec.preferredPayloadType == "number")
        if (mediaCodec.preferredPayloadType != 0)
        {
          codec.preferredPayloadType = mediaCodec.preferredPayloadType;

          // Also remove the pt from the list : available dynamic values.
          // const idx = dynamicPayloadTypes.indexOf(codec.preferredPayloadType);
          // if (idx > -1) {
          //     dynamicPayloadTypes.splice(idx, 1); 
          // }
          auto idx = std::find(dynamicPayloadTypes.begin(), dynamicPayloadTypes.end(), codec.preferredPayloadType);
          if (idx != dynamicPayloadTypes.end())
          {
             dynamicPayloadTypes.erase(idx);
          }

           
        }
        // Otherwise if the supported codec has preferredPayloadType, use it.
        else if (codec.preferredPayloadType != 0)
        {
          // No need to remove it from the list since it"s not a dynamic value.
        }
        // Otherwise choose a dynamic one.
        else
        {
          // Take the first available pt and remove it from the list.
          //auto pt = dynamicPayloadTypes.shift();
          auto k = dynamicPayloadTypes.begin();
          auto pt = *k;
          dynamicPayloadTypes.erase(k);
            
          if (!pt)
            MSC_THROW_TYPE_ERROR("cannot allocate more dynamic codec payload types");

          codec.preferredPayloadType = pt;
        }

        // Ensure there is not duplicated preferredPayloadType values.
        // if (caps.codecs.some((c) => c.preferredPayloadType == codec.preferredPayloadType))
        //   MSC_THROW_TYPE_ERROR("duplicated codec.preferredPayloadType");

        for (auto it = caps.codecs.begin(); it != caps.codecs.end();)
        {
            if ((*it).preferredPayloadType == codec.preferredPayloadType)
              MSC_THROW_TYPE_ERROR("duplicated codec.preferredPayloadType=%d",codec.preferredPayloadType);
            else
              ++it;
        }

        // Merge the media codec parameters.
        // codec.parameters = { ...codec.parameters, ...mediaCodec.parameters };
        for(auto &kv:mediaCodec.parameters )
        {
            codec.parameters[kv.first] = kv.second;
        }

        // Append to the codec list.
        caps.codecs.push_back(codec);

        // Add a RTX video codec if video.
        if (codec.kind == "video")
        {
          // Take the first available pt and remove it from the list.
          //auto pt = dynamicPayloadTypes.shift();
          auto k = dynamicPayloadTypes.begin();
          auto pt = *k;
          dynamicPayloadTypes.erase(k);
          if (!pt)
            MSC_THROW_TYPE_ERROR("cannot allocate more dynamic codec payload types");
            std::map<std::string, json> parameters;
            //std::map<std::string, int> param;
            parameters[std::string("apt")] = codec.preferredPayloadType;
            //parameters.push_back(param);
          RtpCodecCapability rtxCodec;//  =
          //{
            rtxCodec.kind                 = codec.kind;
            rtxCodec.mimeType             = codec.kind+"/rtx";
            rtxCodec.preferredPayloadType = pt;
            rtxCodec.clockRate            = codec.clockRate;
            rtxCodec.parameters           = parameters;
            //rtcpFeedback : []
          //};

          // Append to the codec list.
          caps.codecs.push_back(rtxCodec);
        }
      }

      return caps;
    }

    /**
     * Get a mapping : codec payloads and encodings : the given Producer RTP
     * parameters as values expected by the Router.
     *
     * It may throw if invalid or non supported RTP parameters are given.
     */
    json getProducerRtpParametersMapping(
      RtpParameters &params,//: RtpParameters,
      RtpCapabilities &caps//: RtpCapabilities
    )//: RtpMapping
    {
        RtpMapping rtpMapping;

      // Match parameters media codecs to capabilities media codecs.
      //auto codecToCapCodec: Map<RtpCodecParameters, RtpCodecCapability> = new Map();
      std::unordered_map<int,std::pair<RtpCodecParameters,RtpCodecCapability>> codecToCapCodec;
      for (auto &codec : params.codecs)
      {
        if (isRtxCodec(codec))
          continue;

        // Search for the same media codec in capabilities.
        // auto matchedCapCodec = caps.codecs!
        //   .find((capCodec) => (
        //     matchCodecs(codec, capCodec, { strict: true, modify: true }))
        //   );
        RtpCodecCapability matchedCapCodec;
        bool matched = false;
        for(auto &capCodec : caps.codecs)
        {
            json jcapCodec = capCodec;
            json jcodec = codec;
            matched = matchCodecs(jcodec, jcapCodec,/*strict*/ true, /*modify*/ true);
            if(matched == true) {
                matchedCapCodec = capCodec;
                break;
            }
        }
          
        if (matched == false)
        {
          MSC_THROW_TYPE_ERROR(
              "unsupported codec [mimeType:%s, payloadType:%d]",codec.mimeType.c_str(),codec.payloadType);
        }

        codecToCapCodec[codec.payloadType] = std::make_pair(codec,matchedCapCodec);
      }

      // Match parameters RTX codecs to capabilities RTX codecs.
      for (auto codec : params.codecs)
      {
        if (!isRtxCodec(codec))
          continue;

        // Search for the associated media codec.
        // auto associatedMediaCodec = params.codecs
        //   .find((mediaCodec) => mediaCodec.payloadType == codec.parameters.apt);

        RtpCodecParameters associatedMediaCodec;
        bool matched = false;
        int apt = 0;
        for(auto & pair : codec.parameters)
        {
            if(pair.first == "apt") {
                apt = pair.second;
            }
        }
        for(auto &mediaCodec : params.codecs)
        {
            matched = (mediaCodec.payloadType == apt);
            if(matched == true) {
                associatedMediaCodec = mediaCodec;
                break;
            }
        }
          
        if (matched == false)
        {
          MSC_THROW_TYPE_ERROR(
             "missing media codec found for RTX PT codec.payloadType=%d",codec.payloadType);
        }
        // if (!associatedMediaCodec)
        // {
        //   MSC_THROW_TYPE_ERROR(
        //     "missing media codec found for RTX PT ${codec.payloadType}");
        // }

          
        auto capMediaCodec = codecToCapCodec[associatedMediaCodec.payloadType].second;

        // Ensure that the capabilities media codec has a RTX codec.
        // auto associatedCapRtxCodec = caps.codecs
        //   .find((capCodec) => (
        //     isRtxCodec(capCodec) &&
        //     capCodec.parameters.apt == capMediaCodec.preferredPayloadType
        //   ));
        RtpCodecCapability associatedCapRtxCodec;
          bool associatedCapRtxCodecMatched = false;
        for(auto &capCodec : caps.codecs)
        {
            int capapt = 0;
            for(auto & pair : capCodec.parameters)
            {
                if(pair.first == "apt") {
                    capapt = pair.second;
                }
            }
            if(isRtxCodec(capCodec) &&
               capapt == capMediaCodec.preferredPayloadType)
            {
                associatedCapRtxCodec = capCodec;
                associatedCapRtxCodecMatched = true;
            }
        }
          
        if (associatedCapRtxCodecMatched == false)
        {
          MSC_THROW_TYPE_ERROR(
            "no RTX codec for capability codec PT ${capMediaCodec.preferredPayloadType}");
        }
        codecToCapCodec[codec.payloadType] = std::make_pair(codec,associatedCapRtxCodec);
       
      }
      // for (auto [ codec, capCodec ] : codecToCapCodec)
      // {
      // Generate codecs mapping.
      for (const auto& kp : codecToCapCodec)
      {
          auto & codec1        = kp.first;
          const auto& kv = kp.second;
          
        auto & codec        = kv.first;
        const auto& capCodec = kv.second;
        rtpMapping.codecs.push_back(
          {
            {"payloadType"       , codec.payloadType},
            {"mappedPayloadType" , capCodec.preferredPayloadType}
          });
      }

      // Generate encodings mapping.
      auto mappedSsrc = UtilsC::getRandomInteger(100000000,999999999);

      for (auto &encoding : params.encodings)
      {
        RtpMappingEncoding mappedEncoding;

        mappedEncoding.mappedSsrc = mappedSsrc++;

        if (!encoding.rid.empty())
          mappedEncoding.rid = encoding.rid;
        if (encoding.ssrc != 0)
          mappedEncoding.ssrc = encoding.ssrc;
        if (!encoding.scalabilityMode.empty())
          mappedEncoding.scalabilityMode = encoding.scalabilityMode;
       
        json jmappedEncoding = json{
            {"ssrc", mappedEncoding.ssrc},
            {"rid", mappedEncoding.rid},
            {"scalabilityMode", mappedEncoding.scalabilityMode},
            {"mappedSsrc", mappedEncoding.mappedSsrc}
            
        };
        if(mappedEncoding.ssrc == 0) {
            jmappedEncoding.erase("ssrc");
        }
        if(mappedEncoding.rid.empty()) {
            jmappedEncoding.erase("rid");
        }
        if(mappedEncoding.scalabilityMode.empty()) {
            jmappedEncoding.erase("scalabilityMode");
        }
        if(mappedEncoding.mappedSsrc == 0) {
            jmappedEncoding.erase("mappedSsrc");
        }
        rtpMapping.encodings.push_back(jmappedEncoding);
      }
      json jRtpMapping = {
          {"codecs", rtpMapping.codecs},
          {"encodings", rtpMapping.encodings}
      };
      return jRtpMapping;
    }

    /**
     * Generate RTP parameters to be internally used by Consumers given the RTP
     * parameters : a Producer and the RTP capabilities : the Router.
     */
    json getConsumableRtpParameters(
      std::string &kind,
      RtpParameters &params,//: RtpParameters,
      RtpCapabilities &caps,//: RtpCapabilities,
      json &rtpMapping//: RtpMapping
    )//: RtpParameters
    {
      RtpParameters consumableParams ;
      //RtpMapping  rtpMapping = jrtpMapping;
      for (auto codec : params.codecs)
      {
        if (isRtxCodec(codec))
          continue;

        // auto consumableCodecPt = rtpMapping.codecs
        //   .find((entry) => entry.payloadType == codec.payloadType)
        //   .mappedPayloadType;
        int consumableCodecPt = 0;
        for(auto &entry : rtpMapping["codecs"])
        {
            if(entry["payloadType"].get<int>() == codec.payloadType) {
                consumableCodecPt = entry["mappedPayloadType"];
                break;
            }
        }

        // auto matchedCapCodec = caps.codecs!
        //   .find((capCodec) => capCodec.preferredPayloadType == consumableCodecPt);
        RtpCodecCapability matchedCapCodec;
        bool matched = false;
        for(auto &capCodec : caps.codecs)
        {
            matched = ( capCodec.preferredPayloadType == consumableCodecPt);
            if(matched == true) {
                matchedCapCodec = capCodec;
                break;
            }
        }
          
        RtpCodecParameters consumableCodec;// =
       // {
          consumableCodec.mimeType     = matchedCapCodec.mimeType;
          consumableCodec.payloadType  = matchedCapCodec.preferredPayloadType;
          consumableCodec.clockRate    = matchedCapCodec.clockRate;
          consumableCodec.channels     = matchedCapCodec.channels;
          consumableCodec.parameters   = codec.parameters; // Keep the Producer codec parameters.
          consumableCodec.rtcpFeedback = matchedCapCodec.rtcpFeedback;
       // };

        consumableParams.codecs.push_back(consumableCodec);

        // auto consumableCapRtxCodec = caps.codecs!
        //   .find((capRtxCodec) => (
        //     isRtxCodec(capRtxCodec) &&
        //     capRtxCodec.parameters.apt == consumableCodec.payloadType
        //   ));
        RtpCodecCapability consumableCapRtxCodec;
        bool consumableCapRtxCodecMatched = false;
        for(auto &capRtxCodec : caps.codecs)
        {
            json jcapRtxCodec = capRtxCodec;
            int capapt = 0;
            for(auto & pair : capRtxCodec.parameters)
            {
                if(pair.first == "apt") {
                    capapt = pair.second;
                }
            }
            if(isRtxCodec(jcapRtxCodec) && capapt == consumableCodec.payloadType) {
                consumableCapRtxCodecMatched = true;
            }
            
            if(consumableCapRtxCodecMatched == true) {
                consumableCapRtxCodec = capRtxCodec;
                break;
            }
        }
          
        if (consumableCapRtxCodecMatched)
        {
          RtpCodecParameters consumableRtxCodec;//  =
          //{
            consumableRtxCodec.mimeType     = consumableCapRtxCodec.mimeType;
            consumableRtxCodec.payloadType  = consumableCapRtxCodec.preferredPayloadType;
            consumableRtxCodec.clockRate    = consumableCapRtxCodec.clockRate;
            consumableRtxCodec.parameters   = consumableCapRtxCodec.parameters;
            consumableRtxCodec.rtcpFeedback = consumableCapRtxCodec.rtcpFeedback;
          //};

          consumableParams.codecs.push_back(consumableRtxCodec);
        }
      }

      for (auto capExt : caps.headerExtensions)
      {

        // Just take RTP header extension that can be used in Consumers.
        if (
          capExt.kind != kind ||
          (capExt.direction != "sendrecv" && capExt.direction != "sendonly")
        )
        {
          continue;
        }

        RtpHeaderExtensionParameters consumableExt;// =
        //{
          consumableExt.uri        = capExt.uri;
          consumableExt.id         = capExt.preferredId;
          consumableExt.encrypt    = capExt.preferredEncrypt;
          //parameters : {}
        //};

        consumableParams.headerExtensions.push_back(consumableExt);
      }

      // Clone Producer encodings since we"ll mangle them.
      //auto consumableEncodings = utils.clone(params.encodings) as RtpEncodingParameters[];
      std::vector<RtpEncodingParameters> consumableEncodings;
      consumableEncodings.insert(consumableEncodings.end(), params.encodings.begin(), params.encodings.end());


      for (int i = 0; i < consumableEncodings.size(); ++i)
      {
        auto consumableEncoding = consumableEncodings[i];
        auto  mappedSsrc  = rtpMapping["encodings"][i]["mappedSsrc"];

        // Remove useless fields.
        // delete consumableEncoding.rid;
        // delete consumableEncoding.rtx;
        // delete consumableEncoding.codecPayloadType;
        consumableEncoding.rid="";
        consumableEncoding.rtx.ssrc=0;
        consumableEncoding.codecPayloadType=0;
        // Set the mapped ssrc.
        consumableEncoding.ssrc = mappedSsrc;

        consumableParams.encodings.push_back(consumableEncoding);
      }

      //consumableParams.rtcp;// =
      //{
        consumableParams.rtcp.cname       = params.rtcp.cname;
        consumableParams.rtcp.reducedSize = true;
        consumableParams.rtcp.mux         = true;
     // };
      json jconsumableParams = consumableParams;
      return jconsumableParams;
    }

    /**
     * Check whether the given RTP capabilities can consume the given Producer.
     */
    bool canConsume(
      RtpParameters consumableParams,//: RtpParameters,
      RtpCapabilities caps//: RtpCapabilities
    )//: boolean
    {
      // This may throw.
      json jcaps = caps;
      validateRtpCapabilities(jcaps);

      // auto matchingCodecs: RtpCodecParameters[] = [];
      std::vector<RtpCodecParameters> matchingCodecs;
      for (auto codec : consumableParams.codecs)
      {
        // auto matchedCapCodec = caps.codecs!
        //   .find((capCodec) => matchCodecs(capCodec, codec, { strict: true }));
        RtpCodecCapability matchedCapCodec;
        bool matched = false;
        for(auto &capCodec : caps.codecs)
        {
            json jcapCodec = capCodec;
            json jcodec = codec;
            matched = matchCodecs(jcapCodec,jcodec,/*strict*/ true, /*modify*/ false);
            if(matched == true) {
                matchedCapCodec = capCodec;
                break;
            }
        }
          
        if (!matched)
          continue;

        matchingCodecs.push_back(codec);
      }

      // Ensure there is at least one media codec.
      if (matchingCodecs.size() == 0 || isRtxCodec(matchingCodecs[0]))
        return false;

      return true;
    }

    /**
     * Generate RTP parameters for a specific Consumer.
     *
     * It reduces encodings to just one and takes into account given RTP capabilities
     * to reduce codecs, codecs" RTCP feedback and header extensions, and also enables
     * or disabled RTX.
     */
    RtpParameters getConsumerRtpParameters(
      RtpParameters consumableParams,//: RtpParameters,
      RtpCapabilities &caps,//: RtpCapabilities,
      bool pipe//: boolean
    )//: RtpParameters
    {
      RtpParameters consumerParams ;// =
     // {
        //codecs           : [],
        //headerExtensions : [],
        //encodings        : [],
        consumerParams.rtcp             = consumableParams.rtcp;
      //};

      for (auto capCodec : caps.codecs)
      {
        json jcapCodec = capCodec;
        validateRtpCodecCapability(jcapCodec);
      }

      // auto consumableCodecs =
      //   utils.clone(consumableParams.codecs) as RtpCodecParameters[];
      std::vector<RtpCodecParameters> consumableCodecs;
      consumableCodecs.insert(consumableCodecs.end(), consumableParams.codecs.begin(), consumableParams.codecs.end());
      auto rtxSupported = false;

      for (auto &codec : consumableCodecs)
      {
        // auto matchedCapCodec = caps.codecs!
        //   .find((capCodec) => matchCodecs(capCodec, codec, { strict: true }));
        RtpCodecCapability matchedCapCodec;
        bool matched = false;
        for(auto &capCodec : caps.codecs)
        {
            json jcapCodec = capCodec;
            json jcodec = codec;
            matched = matchCodecs(jcapCodec,jcodec,/*strict*/ true, /*modify*/ false);
            if(matched == true) {
                matchedCapCodec = capCodec;
                break;
            }
        }
          
        if (!matched)
          continue;

        codec.rtcpFeedback = matchedCapCodec.rtcpFeedback;

        consumerParams.codecs.push_back(codec);
      }

      // Must sanitize the list : matched codecs by removing useless RTX codecs.
      for (int idx = (consumerParams.codecs.size() -1); idx >= 0; --idx)
      {
        auto codec = consumerParams.codecs[idx];

        if (isRtxCodec(codec))
        {
          // Search for the associated media codec.
          // auto associatedMediaCodec = consumerParams.codecs
          //   .find((mediaCodec) => mediaCodec.payloadType == codec.parameters.apt);
          RtpCodecParameters associatedMediaCodec;
          bool associatedMediaCodecMatched = false;
          for(auto &mediaCodec : consumerParams.codecs)
          {
              json jmediaCodec = mediaCodec;
              int capapt = 0;
              for(auto & pair : codec.parameters)
              {
                  if(pair.first == "apt") {
                      capapt = pair.second;
                  }
              }
              associatedMediaCodecMatched = ( mediaCodec.payloadType == capapt);
              if(associatedMediaCodecMatched == true) {
                  associatedMediaCodec = mediaCodec;
                  break;
              }
          }
            
          if (associatedMediaCodecMatched)
            rtxSupported = true;
          else
          {
              //consumerParams.codecs.splice(idx, 1);
              int index=0;
              for (auto it = consumerParams.codecs.begin(); it != consumerParams.codecs.end();)
              {
                  if (index == idx)
                      consumerParams.codecs.erase(it);
                  else
                    ++it;
                  index++;
              }

            
          }  
        }
      }

      // Ensure there is at least one media codec.
      if (consumerParams.codecs.size() == 0 || isRtxCodec(consumerParams.codecs[0]))
      {
        MSC_THROW_TYPE_ERROR("no compatible media codecs");
      }

      // consumerParams.headerExtensions = consumableParams.headerExtensions
      //   .filter((ext) => (
      //     caps.headerExtensions!
      //       .some((capExt) => (
      //         capExt.preferredId == ext.id &&
      //         capExt.uri == ext.uri
      //       ))
      //   ));
        std::vector<RtpHeaderExtensionParameters> headerExtensions;
        bool matched = false;
        bool have01 = false;
        bool have02 = false;
        for(auto &ext : consumableParams.headerExtensions)
        {
            for(auto &capExt : caps.headerExtensions)
            {
                matched = ( (capExt.preferredId == ext.id) && (capExt.uri == ext.uri) );
                if(matched == true) {
                  headerExtensions.push_back(ext);
                }
            }
        }
        consumableParams.headerExtensions.clear();
        consumableParams.headerExtensions.insert(consumableParams.headerExtensions.end(), headerExtensions.begin(), headerExtensions.end());

        for(auto &ext : consumableParams.headerExtensions)
        {
            if(ext.uri == "http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01") {
                have01 =true;
            }else if(ext.uri == "http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time") {
                have02 =true;
            }

        }
      // Reduce codecs" RTCP feedback. Use Transport-CC if available, REMB otherwise.
      if (have01
        // consumerParams.headerExtensions.some((ext) => (
        //   ext.uri == "http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01"
        //))
      )
      {
        for (auto &codec : consumerParams.codecs)
        {
           //codec.rtcpFeedback = codec.rtcpFeedback;
          //   .filter((fb) => fb.type != "goog-remb");
          std::vector<RtcpFeedback> rtcpFeedback;
          for(auto &fb : codec.rtcpFeedback)
          {
              if(fb.type != "goog-remb") {
                 rtcpFeedback.push_back(fb);
              }
          }
          codec.rtcpFeedback.clear();
          codec.rtcpFeedback.insert(codec.rtcpFeedback.end(), rtcpFeedback.begin(), rtcpFeedback.end());

        }
      }
      else if (have02
        // consumerParams.headerExtensions.some((ext) => (
        //   ext.uri == "http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time"
        //))
      )
      {
        for (auto &codec : consumerParams.codecs)
        {
          // codec.rtcpFeedback = codec.rtcpFeedback!
          //   .filter((fb) => fb.type != "transport-cc");
           std::vector<RtcpFeedback> rtcpFeedback;
          for(auto &fb : codec.rtcpFeedback)
          {
              if(fb.type != "transport-cc") {
                 rtcpFeedback.push_back(fb);
              }
          }
          codec.rtcpFeedback.clear();
          codec.rtcpFeedback.insert(codec.rtcpFeedback.end(), rtcpFeedback.begin(), rtcpFeedback.end());   
        }
      }
      else
      {
        for (auto &codec : consumerParams.codecs)
        {
          // codec.rtcpFeedback = codec.rtcpFeedback!
          //   .filter((fb) => (
          //     fb.type != "transport-cc" &&
          //     fb.type != "goog-remb"
          //   ));
          std::vector<RtcpFeedback> rtcpFeedback;
          for(auto &fb : codec.rtcpFeedback)
          {
              if(fb.type != "transport-cc" && fb.type != "goog-remb") {
                 rtcpFeedback.push_back(fb);
              }
          }
          codec.rtcpFeedback.clear();
          codec.rtcpFeedback.insert(codec.rtcpFeedback.end(), rtcpFeedback.begin(), rtcpFeedback.end()); 
        }
      }

      if (!pipe)
      {
        RtpEncodingParameters consumerEncoding;
        consumerEncoding.ssrc = UtilsC::getRandomInteger((uint32_t)100000000,(uint32_t)999999999);

        if (rtxSupported)
        {
          consumerEncoding.rtx.ssrc =  (consumerEncoding.ssrc + 1 );
        }  

        // If any : the consumableParams.encodings has scalabilityMode, process it
        // (assume all encodings have the same value).
        // auto encodingWithScalabilityMode =
        //   consumableParams.encodings.find((encoding) => encoding.scalabilityMode);
        RtpEncodingParameters encodingWithScalabilityMode;
        for(auto &encoding : consumableParams.encodings)
        {
           if(!encoding.scalabilityMode.empty()) {
             encodingWithScalabilityMode = encoding;
             break;
           }
        }
          
        // auto scalabilityMode = encodingWithScalabilityMode
        //   ? encodingWithScalabilityMode.scalabilityMode
        //   : undefined;
        auto scalabilityMode =  encodingWithScalabilityMode.scalabilityMode;

        // If there is simulast, mangle spatial layers in scalabilityMode.
        if (consumableParams.encodings.size() > 1)
        {
          auto temporalLayers  = parseScalabilityMode(scalabilityMode);
          //"S${consumableParams.encodings.size()}T${temporalLayers}";
          scalabilityMode = "S"+std::to_string(consumableParams.encodings.size())+"T"+std::to_string(temporalLayers.get<int>());
        }

        if (!scalabilityMode.empty())
          consumerEncoding.scalabilityMode = scalabilityMode;

        // Use the maximum maxBitrate in any encoding and honor it in the Consumer"s
        // encoding.
        // auto maxEncodingMaxBitrate =
        //   consumableParams.encodings.reduce((maxBitrate, encoding) => (
        //     encoding.maxBitrate && encoding.maxBitrate > maxBitrate
        //       ? encoding.maxBitrate
        //       : maxBitrate
        //   ), 0);
        int maxEncodingMaxBitrate = 0;
        int maxBitrate=0;
        for(auto &encoding : consumableParams.encodings)
        {
          maxBitrate = encoding.maxBitrate > maxBitrate ? encoding.maxBitrate : maxBitrate;     
        }
          
        if (maxEncodingMaxBitrate != 0)
        {
          consumerEncoding.maxBitrate = maxEncodingMaxBitrate;
        }

        // Set a single encoding for the Consumer.
        consumerParams.encodings.push_back(consumerEncoding);
      }
      else
      {
        // auto consumableEncodings =
        //   utils.clone(consumableParams.encodings) as RtpEncodingParameters[];
        std::vector<RtpEncodingParameters> consumableEncodings;
        consumableEncodings.insert(consumableEncodings.end(), consumableParams.encodings.begin(), consumableParams.encodings.end());
        auto baseSsrc = UtilsC::getRandomInteger((uint32_t)100000000,(uint32_t)999999999);
        auto baseRtxSsrc = UtilsC::getRandomInteger((uint32_t)100000000,(uint32_t)999999999);

        for (auto i = 0; i < consumableEncodings.size(); ++i)
        {
          auto encoding = consumableEncodings[i];

          encoding.ssrc = baseSsrc + i;

          if (rtxSupported)
            encoding.rtx.ssrc = baseRtxSsrc + i;
          else
          {
            //delete encoding.rtx;
            encoding.rtx.ssrc = 0;
          }  

          consumerParams.encodings.push_back(encoding);
        }
      }

      return consumerParams;
    }

    /**
     * Generate RTP parameters for a pipe Consumer.
     *
     * It keeps all original consumable encodings and removes support for BWE. If
     * enableRtx is false, it also removes RTX and NACK support.
     */
    RtpParameters getPipeConsumerRtpParameters(
      RtpParameters &consumableParams,//: RtpParameters,
      bool enableRtx// = false
    )//: RtpParameters
    {
      RtpParameters consumerParams;// =
      //{
        //codecs           : [],
        //headerExtensions : [],
        //encodings        : [],
        consumerParams.rtcp             = consumableParams.rtcp;
     // };

      // auto consumableCodecs =
      //   utils.clone(consumableParams.codecs) as RtpCodecParameters[];
      std::vector<RtpCodecParameters> consumableCodecs;
      consumableCodecs.insert(consumableCodecs.end(), consumableParams.codecs.begin(), consumableParams.codecs.end());
      for (auto &codec : consumableCodecs)
      {
        if (!enableRtx && isRtxCodec(codec))
          continue;

        // codec.rtcpFeedback = codec.rtcpFeedback!
        //   .filter((fb) => (
        //     (fb.type == "nack" && fb.parameter == "pli") ||
        //     (fb.type == "ccm" && fb.parameter == "fir") ||
        //     (enableRtx && fb.type == "nack" && !fb.parameter)
        //   ));
        std::vector<RtcpFeedback> rtcpFeedback;
        for(auto &fb : codec.rtcpFeedback)
        {
            if(
              (fb.type == "nack" && fb.type == "pli") ||
               (fb.type == "ccm" && fb.type == "fir") ||
               (enableRtx && fb.type == "nack" && !fb.parameter.empty())
            ){
              rtcpFeedback.push_back(fb);
            }
        }
        codec.rtcpFeedback.clear();
        codec.rtcpFeedback.insert(codec.rtcpFeedback.end(), rtcpFeedback.begin(), rtcpFeedback.end()); 

        consumerParams.codecs.push_back(codec);
      }

      // Reduce RTP extensions by disabling transport MID and BWE related ones.
      // consumerParams.headerExtensions = consumableParams.headerExtensions!
      //   .filter((ext) => (
      //     ext.uri != "urn:ietf:params:rtp-hdrext:sdes:mid" &&
      //     ext.uri != "http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time" &&
      //     ext.uri != "http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01"
      //   ));
        std::vector<RtpHeaderExtensionParameters> headerExtensions;;
        for(auto &ext : consumableParams.headerExtensions)
        {
            if(
              ext.uri != "urn:ietf:params:rtp-hdrext:sdes:mid" &&
              ext.uri != "http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time" &&
              ext.uri != "http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01"
            ){
              headerExtensions.push_back(ext);
            }
        }
        consumableParams.headerExtensions.clear();
        consumableParams.headerExtensions.insert(consumableParams.headerExtensions.end(), headerExtensions.begin(), headerExtensions.end()); 

      // auto consumableEncodings =
      //   utils.clone(consumableParams.encodings) as RtpEncodingParameters[];
      std::vector<RtpEncodingParameters> consumableEncodings;
      consumableEncodings.insert(consumableEncodings.end(), consumableParams.encodings.begin(), consumableParams.encodings.end());
      auto baseSsrc = UtilsC::getRandomInteger((uint32_t)100000000,(uint32_t)999999999);
      auto baseRtxSsrc = UtilsC::getRandomInteger((uint32_t)100000000,(uint32_t)999999999);

      for (auto i = 0; i < consumableEncodings.size(); ++i)
      {
        auto encoding = consumableEncodings[i];

        encoding.ssrc = baseSsrc + i;

        if (enableRtx)
          encoding.rtx.ssrc =  (baseRtxSsrc + i );
        else
        {
          //delete encoding.rtx;
          encoding.rtx.ssrc = 0;
        }  

        consumerParams.encodings.push_back(encoding);
      }

      return consumerParams;
    }

	} // namespace ortc
} // namespace mediasoupclient

