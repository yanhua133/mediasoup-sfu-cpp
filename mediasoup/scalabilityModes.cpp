#define MSC_CLASS "scalabilityMode"

#include "scalabilityModes.hpp"
#include "Log.hpp"
#include <regex>

using json = nlohmann::json;

static const std::regex ScalabilityModeRegex(
  "^[LS]([1-9]\\d{0,1})T([1-9]\\d{0,1}).*", std::regex_constants::ECMAScript);

namespace mediasoupclient
{
	json parseScalabilityMode(const std::string& scalabilityMode)
	{
		/* clang-format off */
	json jsonScalabilityMode
	{
		{ "spatialLayers",  1 },
		{ "temporalLayers", 1 }
	};
		/* clang-format on */

		std::smatch match;

		std::regex_match(scalabilityMode, match, ScalabilityModeRegex);

		if (!match.empty())
		{
			try
			{
				jsonScalabilityMode["spatialLayers"]  = std::stoul(match[1].str());
				jsonScalabilityMode["temporalLayers"] = std::stoul(match[2].str());
			}
			catch (std::exception& e)
			{
				MS_lOGW("invalid scalabilityMode: %s", e.what());
			}
		}
		else
		{
            MS_lOGW("invalid scalabilityMode: %s", scalabilityMode.c_str());
		}

		return jsonScalabilityMode;
	}
} // namespace mediasoupclient
