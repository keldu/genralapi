#include "JsonRpcAdapter.h"

#include <kj/array.h>

#include "JsonRpcDefinitions.h"

#include <iostream>

namespace gen{
	static constexpr uint64_t JSON_NAME_ANNOTATION_ID = 0xfa5b1fd61c2e7c3dull;

	JsonRpcAdapter::JsonRpcAdapter(capnp::JsonCodec& cCodec, capnp::DynamicCapability::Client cInterface):
		codec(&cCodec),
		interface(cInterface),
		taskset(*this)
	{

		for(auto method : interface.getSchema().getMethods()){
			auto proto = method.getProto();
			kj::StringPtr name = proto.getName();
			for(auto annotation : proto.getAnnotations()){
				switch(annotation.getId()){
					case JSON_NAME_ANNOTATION_ID:
						name = annotation.getValue().getText();
						break;
					default:
						break;
				}
			}
			method_map.insert(name, method);
		}
	}

	void JsonRpcAdapter::taskFailed(kj::Exception &&exception){
		std::cout<<"Error: "<<exception.getDescription().cStr()<<std::endl;
	}
	
	kj::Promise<kj::String> JsonRpcAdapter::interpret(kj::String request)
	{
		capnp::MallocMessageBuilder request_builder;
		kj::Own<capnp::MallocMessageBuilder> response_builder = kj::heap<capnp::MallocMessageBuilder>();

		auto rpc_array_builder = request_builder.getRoot<gen::json::RpcArray>();
		KJ_IF_MAYBE(exception, kj::runCatchingExceptions([&](){
			codec->decode(request, rpc_array_builder);
		})){
			return writeSingleJsonResponse(*response_builder, nullptr, static_cast<int>(JsonErrorCode::PARSE_ERROR), kj::str("Parse Error: ",exception->getDescription()));
		}

		auto rpc_array_reader = rpc_array_builder.asReader();

		switch(rpc_array_reader.which()){
			case gen::json::RpcArray::SINGLE:{
				return handleSingle(rpc_array_reader.getSingle(), *response_builder).attach(kj::mv(response_builder));
			}
			break;
			case gen::json::RpcArray::ARRAY:{
				return handleArray(rpc_array_reader.getArray(), *response_builder).attach(kj::mv(response_builder));
			}
			break;
		}

		return writeSingleJsonResponse(*response_builder, nullptr, static_cast<int>(JsonErrorCode::SERVER_UNIMPLEMENTED),
			kj::str("Unhandled case. This shouldn't happen. Means the library doesn't handle all cases. Contact the Maintainer"));
	}
	
	kj::Promise<kj::String> JsonRpcAdapter::handleSingle(gen::json::RpcMessage::Reader request_reader, capnp::MallocMessageBuilder& response_builder)
	{
	
		auto response = respond(request_reader,response_builder);
		KJ_IF_MAYBE(r, response){
			return r->then([this,&response_builder](capnp::Orphan<gen::json::RpcMessage> orphan){
				auto response_array = response_builder.getRoot<gen::json::RpcArray>();
				response_array.adoptSingle(kj::mv(orphan));
				
				return codec->encode(response_array);
			});
		}else{
			return kj::str("");
		}
	}
	
	kj::Promise<kj::String> JsonRpcAdapter::handleArray(capnp::List< ::gen::json::RpcMessage,  
		::capnp::Kind::STRUCT>::Reader request_reader, capnp::MallocMessageBuilder& response_builder)
	{
		if(request_reader.size() == 0){
			return writeSingleJsonResponse(response_builder, nullptr, static_cast<int>(JsonErrorCode::INVALID_REQUEST), kj::str("Invalid Request"));
		}
		//auto response_array = response_builder.getRoot<gen::json::RpcArray>();

		kj::ArrayBuilder<kj::Promise<capnp::Orphan<gen::json::RpcMessage>>> array_builder = kj::heapArrayBuilder<kj::Promise<capnp::Orphan<gen::json::RpcMessage>>>(request_reader.size());

		size_t j = 0;
		for(size_t i = 0; i < request_reader.size(); ++i){
			auto response = respond(request_reader[i],response_builder);
			KJ_IF_MAYBE(r, response){
				array_builder.add(kj::mv(*r));
				++j;
			}else{

			}
		}
		if(j > 0){
			kj::Array<kj::Promise<capnp::Orphan<gen::json::RpcMessage>>> array = array_builder.finish();
			auto promise_array = kj::joinPromises<capnp::Orphan<gen::json::RpcMessage>>(kj::mv(array));

			return promise_array.then([this,&response_builder](kj::Array<capnp::Orphan<gen::json::RpcMessage>> array_orphan){
				auto result = response_builder.getOrphanage().newOrphan<capnp::List<gen::json::RpcMessage>>(array_orphan.size());
				auto builder = result.get();
				for(size_t i = 0; i < array_orphan.size(); ++i){
					builder.adoptWithCaveats(i, kj::mv(array_orphan[i]));
				}
				auto response_array = response_builder.getRoot<gen::json::RpcArray>();
				response_array.adoptArray(kj::mv(result));
				return codec->encode(response_array);
			});
		}else{
			return kj::Promise<kj::String>{kj::str("")};
		}
	}
	
	kj::Maybe<kj::Promise<capnp::Orphan<gen::json::RpcMessage>>> JsonRpcAdapter::respond(gen::json::RpcMessage::Reader request_reader,
		capnp::MallocMessageBuilder& response_builder)
	{
		if(!request_reader.hasJsonrpc())
		{
			return kj::Promise<capnp::Orphan<gen::json::RpcMessage>>{writeError(response_builder, nullptr, static_cast<int>(JsonErrorCode::PARSE_ERROR),
				kj::str("JsonRpc Field missing."))};
		}
		
		if(request_reader.getJsonrpc() != "2.0")
		{
			return kj::Promise<capnp::Orphan<gen::json::RpcMessage>>{writeError(response_builder, nullptr, static_cast<int>(JsonErrorCode::PARSE_ERROR),
				kj::str("Unknown json-rpc version. This server implements '2.0'."))};
		}
		
		if(!request_reader.hasMethod())
		{
			return kj::Promise<capnp::Orphan<gen::json::RpcMessage>>{writeError(response_builder, nullptr, static_cast<int>(JsonErrorCode::INVALID_REQUEST), 
				kj::str("Invalid request object."))};
		}

		{
			switch(request_reader.which())
			{
			  	default:
				case gen::json::RpcMessage::NONE:
				case gen::json::RpcMessage::ERROR:
				case gen::json::RpcMessage::RESULT:
				{
					return kj::Promise<capnp::Orphan<gen::json::RpcMessage>>{writeError(response_builder, nullptr, static_cast<int>(JsonErrorCode::INVALID_PARAMS), 
						kj::str("Message has no 'params' field."))};
				}
				break;
				case gen::json::RpcMessage::PARAMS:
				{
					return process(request_reader, response_builder);
				}
				break;
			}
		}
	}

	kj::String JsonRpcAdapter::writeSingleJsonResponse(capnp::MallocMessageBuilder& response_builder, 
		kj::Maybe<capnp::json::Value::Reader> id, int code, kj::String message)
	{
		return createMessageFrom(response_builder, writeError(response_builder, id, code, kj::mv(message)));
	}

	capnp::Orphan<gen::json::RpcMessage> JsonRpcAdapter::writeError(capnp::MallocMessageBuilder& response_builder, kj::Maybe<capnp::json::Value::Reader> id, 
		int code, kj::String message)
	{
		auto orphan_rpc_message_builder = response_builder.getOrphanage().newOrphan<gen::json::RpcMessage>();
		auto rpc_message_builder = orphan_rpc_message_builder.get();
		
		rpc_message_builder.setJsonrpc("2.0");
		KJ_IF_MAYBE(i, id){
			rpc_message_builder.setId(*i);
		}else{
			rpc_message_builder.initId().setNull();
		}

		auto error = rpc_message_builder.initError();
		error.setCode(code);
		error.setMessage(message);
		
		return kj::mv(orphan_rpc_message_builder);
	}

	kj::String JsonRpcAdapter::createMessageFrom(capnp::MallocMessageBuilder& response_builder, capnp::Orphan<gen::json::RpcMessage> orphaned_message){
		auto rpc_array = response_builder.getRoot<gen::json::RpcArray>();
		rpc_array.adoptSingle(kj::mv(orphaned_message));
		return codec->encode(rpc_array);
	}


	kj::Maybe<kj::Promise<capnp::Orphan<gen::json::RpcMessage>>> JsonRpcAdapter::process(gen::json::RpcMessage::Reader request_reader, 
		capnp::MallocMessageBuilder& response_builder)
	{
		auto schema = interface.getSchema();
		KJ_IF_MAYBE(method, schema.findMethodByName(request_reader.getMethod())){
			auto request = interface.newRequest(*method);
			KJ_IF_MAYBE(exception, kj::runCatchingExceptions([&](){
				codec->decode(request_reader.getParams(), request);
			})){
				kj::Maybe<capnp::JsonValue::Reader> id;
				if (request_reader.hasId())
				{
					id = request_reader.getId();
			}
				return kj::Promise<capnp::Orphan<gen::json::RpcMessage>>{writeError(response_builder, id, static_cast<int>(JsonErrorCode::INVALID_PARAMS), 
					kj::str("Type Error in method params: ", exception->getDescription()))};
			}
			if(request_reader.hasId()){
				auto id = request_reader.getId();
				
				auto idCopy = kj::heapArray<capnp::word>(id.totalSize().wordCount + 1);
				memset(idCopy.begin(), 0, idCopy.asBytes().size());
				copyToUnchecked(id, idCopy);
				auto idPtr = capnp::readMessageUnchecked<capnp::json::Value>(idCopy.begin());
					
				auto sent_promise = request.send().then([this,idPtr,&response_builder](capnp::Response<capnp::DynamicStruct> dyn_res) mutable{
					auto orphan = response_builder.getOrphanage().newOrphan<gen::json::RpcMessage>();
					auto json_response = orphan.get();
					json_response.setJsonrpc("2.0");
					json_response.setId(idPtr);
					auto dyn_reader = capnp::DynamicStruct::Reader(dyn_res);
					codec->encode(capnp::DynamicValue::Reader(dyn_reader), dyn_reader.getSchema(), json_response.initResult());
					return kj::Promise<capnp::Orphan<gen::json::RpcMessage>>{kj::mv(orphan)};
				}, [this, idPtr, &response_builder](kj::Exception&& e){
					JsonErrorCode id_v = JsonErrorCode::SERVER_UNKNOWN_ERROR;
					switch(e.getType()){
						case kj::Exception::Type::FAILED:
							id_v = JsonErrorCode::SERVER_FAILED;
							break;
						case kj::Exception::Type::DISCONNECTED:
							id_v = JsonErrorCode::SERVER_DISCONNECTED;
							break;
						case kj::Exception::Type::OVERLOADED:
							id_v = JsonErrorCode::SERVER_OVERLOADED;
							break;
						case kj::Exception::Type::UNIMPLEMENTED:
							id_v = JsonErrorCode::SERVER_UNIMPLEMENTED_METHOD;
							break;
					}
					auto orphan = writeError(response_builder, idPtr, static_cast<int>(id_v), kj::str(e.getDescription()));
					return kj::mv(orphan);
				});
				return sent_promise.attach(kj::mv(idCopy));

			}else{
				kj::Promise<void> promise = request.send().ignoreResult().catch_([](kj::Exception&& exception){
					if(exception.getType() != kj::Exception::Type::UNIMPLEMENTED){
						KJ_LOG(ERROR, "JSON-RPC Notification in abyss", exception);
					}
				});
				taskset.add(kj::mv(promise));
				return nullptr;
			}
		}else{
			if(request_reader.hasId())
			{
				return kj::Promise<capnp::Orphan<gen::json::RpcMessage>>{writeError(response_builder, request_reader.getId(), static_cast<int>(JsonErrorCode::METHOD_NOT_FOUND), 
					kj::str("Method not found."))};
			}
			else
			{
				return nullptr;
			}
		}
		return nullptr;
	}
}
