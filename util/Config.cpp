#include "Config.h"

#include <thirdparty/cpptoml/include/cpptoml.h>

namespace gen{
	std::unique_ptr<Config> Config::parse(const std::string& path){
		auto config_file = cpptoml::parse_file(path);
		std::unique_ptr<Config> u_config = std::make_unique<Config>();
		Config& config = *u_config;

		{
			auto table = config_file->get_table("Network");
			config.network.ipv4 = table->get_as<std::string>("IPv4").value_or("127.0.0.1");
			config.network.ipv6 = table->get_as<std::string>("IPv6").value_or("::1");
			config.network.port = table->get_as<uint32_t>("Port").value_or(9001);
		}
		{
			auto table = config_file->get_table("Secret");
			config.secret.shared = table->get_as<std::string>("Shared").value_or("");
		}

		return u_config;
	}
	std::unique_ptr<Config> Config::parse(){
		return parse("genral.toml");
	}
}
