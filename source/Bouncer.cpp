#include "Bouncer.h"

namespace gen{
	SimpleSession::SimpleSession(const std::string& name):
		token{name},
		is_auth{false}
	{
		
	}
	void SimpleSession::authorize(const std::string& auth_token){
		if( !is_auth && token == auth_token ){
			is_auth = true;
        }
	}
  
	bool SimpleSession::isAuthorized(){
		return is_auth;
	}

	void AllowAllSession::authorize(const std::string& auth_token){

	}

	bool AllowAllSession::isAuthorized(){
		return true;
	}
}
