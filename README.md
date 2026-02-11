# godot-kcp
KCP bindings for godot engine

## Build

1. Install SCons if needed: `pip install --user scons`
2. Build the extension library (debug template example):

```bash
scons platform=linux target=template_debug
```

The built binary is placed under `bin/` with the platform/target/arch suffix used in `godot-kcp.gdextension`.

## Usage

1. Copy `godot-kcp.gdextension` and the compiled library in `bin/` into your Godot project.
2. Add `godot-kcp.gdextension` to your project (Project Settings → Plugins or drag into the FileSystem).
3. Basic script example:

```gdscript
var kcp := KcpPeer.new()
kcp.initialize(1234) # conv id shared by peers
kcp.set_output(func(payload: PackedByteArray):
    udp_socket.put_packet(payload) # send via UDP
)

func _process(_delta):
    # feed incoming UDP packets
    while udp_socket.get_available_packet_count() > 0:
        kcp.input(udp_socket.get_packet())
    kcp.update()
    var msg := kcp.recv()
    if msg.size() > 0:
        print("Received: ", msg)
```
