#pragma once

#include <string>
#include <memory>

/*
Load Config with cpptoml
Section - Network
IPv4: IPv4 Values. IPs to bind. Default is "127.0.0.1"
IPv6: IPv6 Values. IPs to bind. Default is "::1"

Port: Port to listen for connection.

Section - Process
Threads: Number of threads to use. Default is 1

Section - Secret
Shared: Shared_secret as string. Default is empty
*/

namespace gen{
	class Config{
	public:
		static std::unique_ptr<Config> parse(const std::string& path);
		static std::unique_ptr<Config> parse();

		struct Network{
			std::string ipv4;
			std::string ipv6;

			uint32_t port;
		};
		const Network& getNetwork() const {return network;}

		struct Secret{
			std::string shared;
		};
		const Secret& getSecret() const {return secret;}
	private:
		Network network;
		Secret secret;
	};
}