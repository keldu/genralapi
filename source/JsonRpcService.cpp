#include "JsonRpcService.h"
#include "JsonRpcAdapter.h"

#include <iostream>

namespace gen{
	JsonRpcService::JsonRpcService(kj::Own<IRequestAdapter> cAdapter, const kj::HttpHeaderTable& header_table, 
                                   kj::Own<IBouncer> ses):
		adapter(kj::mv(cAdapter)),
		bouncer(kj::mv(ses)),
		response_headers{header_table}
	{
				response_headers.add("Connection","keep-alive");
				response_headers.add("Content-Type","application/json");
				response_headers.add("Server","genral/0.0.1");
	}

	kj::Promise<void> JsonRpcService::request(kj::HttpMethod method, 
		kj::StringPtr url,
		const kj::HttpHeaders& headers, 
		kj::AsyncInputStream& request_body, 
		kj::HttpService::Response& response)
	{	
		headers.forEach([&](kj::StringPtr name, kj::StringPtr value){
       	  	if(name == "Authorization"){
              	bouncer->authorize(value);
          	}
        });
        if(!(bouncer->isAuthorized())){
        	auto stream = response.send(401, "Unauthorized", response_headers);
        	kj::String message = kj::str("You are not authorized!");
        	return stream->write(message.begin(),message.size()).attach(kj::mv(message));
        }
        
		return request_body.readAllText().then([&](kj::String message){
			/*auto stream = response.send(200, "OK", response_headers);
			auto test_message = kj::str("This is a test");
			return stream->write(test_message.begin(), test_message.size()).attach(kj::mv(stream)).attach(kj::mv(test_message));*/

			kj::Promise<kj::String> answer = adapter->interpret(kj::mv(message));
			return answer.then([&](kj::String response_message){

				auto stream = response.send(200,"OK", response_headers);
				auto write_promise = stream->write(response_message.begin(), response_message.size()).attach(kj::mv(stream)).attach(kj::mv(response_message));
				return write_promise;
			});
		});
	}

	JsonRpcMultiService::JsonRpcMultiService(kj::TreeMap<kj::String,JsonRpcMultiService::AdapterBouncerPair>&& map, const kj::HttpHeaderTable& header_table):
		adapters{kj::mv(map)},
		response_headers{header_table}
	{
		response_headers.add("Connection","keep-alive");
		response_headers.add("Content-Type","application/json");
		response_headers.add("Server","genral/0.0.1");
	}
	
	kj::Promise<void> JsonRpcMultiService::request(kj::HttpMethod method, 
		kj::StringPtr url,
		const kj::HttpHeaders& headers, 
		kj::AsyncInputStream& request_body, 
		kj::HttpService::Response& response)
	{	
		auto finder = adapters.find(url);
		
		AdapterBouncerPair* pair = nullptr;
		KJ_IF_MAYBE(found, finder){
			pair = found;
		}else{
        	auto stream = response.send(404, "Not found", response_headers);
        	kj::String message = kj::str("Not Found");
        	return stream->write(message.begin(),message.size()).attach(kj::mv(message));
		}

		IBouncer& bouncer = *(pair->bouncer);
		IRequestAdapter& adapter = *(pair->adapter);

		headers.forEach([&](kj::StringPtr name, kj::StringPtr value){
       	  	if(name == "Authorization"){
              	bouncer.authorize(value);
          	}
        });
        if(!(bouncer.isAuthorized())){
        	auto stream = response.send(401, "Unauthorized", response_headers);
        	kj::String message = kj::str("You are not authorized!");
        	return stream->write(message.begin(),message.size()).attach(kj::mv(message));
        }
        
      	return request_body.readAllText().then([&](kj::String message){
			/*auto stream = response.send(200, "OK", response_headers);
			auto test_message = kj::str("This is a test");
			return stream->write(test_message.begin(), test_message.size()).attach(kj::mv(stream)).attach(kj::mv(test_message));*/

			kj::Promise<kj::String> answer = adapter.interpret(kj::mv(message));
			return answer.then([&](kj::String response_message){
				auto stream = response.send(200,"OK", response_headers);

				auto write_promise = stream->write(response_message.begin(), response_message.size()).attach(kj::mv(stream)).attach(kj::mv(response_message));
				return write_promise;
			});
		});
	}
}
