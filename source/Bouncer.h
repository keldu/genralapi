#pragma once

#include <string>

namespace gen{
/* 
	Check the http header value for a 
	Client-Session or a different one. I don't really care.
    
*/
	class IBouncer{
	public:
		virtual ~IBouncer() = default;
      
		virtual void authorize(const std::string& auth_token) = 0;
		virtual bool isAuthorized() = 0;
	};

	class IAccess{
	public:
		virtual ~IAccess() = default;

		virtual uint64_t getUid() = 0;
	};

	class SimpleSession final : public IBouncer{
	private:
		std::string token;
		bool is_auth;
	public:
		SimpleSession(const std::string& token);
      
		void authorize(const std::string& auth_token) override;
		bool isAuthorized() override;
	};

	class AllowAllSession final : public IBouncer {
	private:
		void authorize(const std::string& auth_token) override;
		bool isAuthorized() override;
	public:
		AllowAllSession() = default;
	};
}
