#pragma once

#include "JsonRpc.capnp.h"
#include "IRequestAdapter.h"

#include <capnp/compat/json.h>
#include <kj/compat/http.h>
#include <kj/map.h>
#include <kj/async.h>

namespace gen{
	class JsonRpcAdapter final : public kj::TaskSet::ErrorHandler, public IRequestAdapter{
	public:
		JsonRpcAdapter(capnp::JsonCodec& cCodec, capnp::DynamicCapability::Client cInterface);
		~JsonRpcAdapter() noexcept {};
		
		kj::Promise<kj::String> interpret(kj::String request) override;
	private:
		capnp::JsonCodec* codec;
		capnp::DynamicCapability::Client interface;

		kj::HashMap<kj::StringPtr, capnp::InterfaceSchema::Method> method_map;

		kj::TaskSet taskset;
		void taskFailed(kj::Exception&& exception) override;

		kj::Promise<kj::String> handleSingle(gen::json::RpcMessage::Reader request, capnp::MallocMessageBuilder& response_builder);
		kj::Promise<kj::String> handleArray(capnp::List< ::gen::json::RpcMessage,  ::capnp::Kind::STRUCT>::Reader request_reader, capnp::MallocMessageBuilder& response_builder);

		kj::String writeSingleJsonResponse(capnp::MallocMessageBuilder& response_builder, kj::Maybe<capnp::json::Value::Reader> id, int code, kj::String message);
		
		capnp::Orphan<gen::json::RpcMessage> writeError(capnp::MallocMessageBuilder& response_builder, kj::Maybe<capnp::json::Value::Reader> id, int code, kj::String message);

		kj::String createMessageFrom(capnp::MallocMessageBuilder& response_builder, capnp::Orphan<gen::json::RpcMessage> orphaned_message);

		kj::Maybe<kj::Promise<capnp::Orphan<gen::json::RpcMessage>>> respond(gen::json::RpcMessage::Reader request, capnp::MallocMessageBuilder& response);
		kj::Maybe<kj::Promise<capnp::Orphan<gen::json::RpcMessage>>> process(gen::json::RpcMessage::Reader request, capnp::MallocMessageBuilder& response);
	};
}
