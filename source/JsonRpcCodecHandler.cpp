#include "JsonRpcCodecHandler.h"

namespace gen{
	void RpcArrayCodecHandler::encode(const capnp::JsonCodec& codec, gen::json::RpcArray::Reader input, capnp::JsonValue::Builder output) const{
		switch(input.which()){
			case json::RpcArray::SINGLE:
			{
				codec.encode(input.getSingle(), output);
			}
			break;
			case json::RpcArray::ARRAY:
			{
				auto array = input.getArray();
				auto out_array = output.initArray(array.size());
				for(size_t i = 0; i < array.size(); ++i){
					auto out_element = out_array[i];
					codec.encode(array[i], out_element);
				}
			}
			break;
		}
	}

	void RpcArrayCodecHandler::decode(const capnp::JsonCodec& codec, capnp::JsonValue::Reader input, gen::json::RpcArray::Builder output) const{
		//auto orphanage = capnp::Orphanage::getForMessageContaining(output);
		KJ_REQUIRE(input.isObject() || input.isArray());
		switch(input.which()){
			case capnp::JsonValue::ARRAY:
			{
				auto input_array = input.getArray();
				auto output_array = output.initArray(input_array.size());
				for(size_t i = 0; i < input_array.size(); ++i){
					auto output_element = output_array[i];
					auto input_element = input_array[i];
					codec.decode(input_element, output_element);
				}
			}
			break;
			case capnp::JsonValue::OBJECT:
			{
				auto output_single = output.initSingle();
				codec.decode(input, output_single);
			}
			break;
			default:
			break;
		}
	}
}
