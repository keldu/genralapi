#pragma once

#include <kj/compat/http.h>
#include <kj/string.h>
#include <kj/map.h>
#include <capnp/compat/json.h>
#include "JsonRpc.capnp.h"
#include <unordered_map>
#include <vector>

#include "Bouncer.h"

namespace gen{
	class IRequestAdapter;
	class JsonRpcService final : public kj::HttpService{
	public:
		JsonRpcService(kj::Own<IRequestAdapter> cAdapter, const kj::HttpHeaderTable& header_table, kj::Own<IBouncer> ses);

		kj::Promise<void> request(kj::HttpMethod method, 
			kj::StringPtr url, 
			const kj::HttpHeaders& headers, 
			kj::AsyncInputStream& request_body, 
			kj::HttpService::Response& response) override;

	private:

		kj::Own<IRequestAdapter> adapter;
		kj::Own<IBouncer> bouncer;
      
		kj::HttpHeaders response_headers;
	};
}
