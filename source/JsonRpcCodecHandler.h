#pragma once

#include <capnp/compat/json.h>
#include "protocol/JsonRpc.capnp.h"

namespace gen{
	class RpcArrayCodecHandler : public capnp::JsonCodec::Handler<gen::json::RpcArray>{
	public:
		void encode(const capnp::JsonCodec& codec, gen::json::RpcArray::Reader input, capnp::JsonValue::Builder output) const override;
		void decode(const capnp::JsonCodec& codec, capnp::JsonValue::Reader input, gen::json::RpcArray::Builder output) const override;
	};
	/*
	class RpcMessageCodecHandler : public capnp::JsonCodec::Handler<gen::json::RpcMessage>{
	public:
		void encode(const capnp::JsonCodec& codec, gen::json::RpcMessage::Reader input, capnp::JsonValue::Builder output) const override;
		void decode(const capnp::JsonCodec& codec, capnp::JsonValue::Reader input, gen::json::RpcMessage::Builder output) const override;
	};
	*/
}
