# GenralApi  

This project is a small extension of the capn'proto library.
Since I didn't like the jsonrpc 2.0 incompleteness from capn'proto, I took it up on myself to extend the library in a way that achieves a closer jsonrpc2.0 standard compliancy.  

The idea is to merge this into the capn proto master. I'm just unsure how I should restructure this. The whole IRequestAdapter thing is quite neat since it is easily extendable. JsonRpcAdapter should be designed with a pimpl pattern. But the existing jsonrpc interface structure in capnproto is quite different to my implementation.  

The jsonrpc implementation in capn proto is also quite strange and is way closer to the 1.0 standard than the 2.0 standard, so I don't even know why it's claimed that it is the 2.0 standard. In my opinion it's not enough to be transport agnostic. The inherent server-client structure is important as well as the array calls.  

I don't know why capn'proto allows the peer-peer structure in their jsonrpc 2.0 implementation which was part of the jsonrpc1.0 standard.  
