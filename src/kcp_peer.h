#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/callable.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>

extern "C" {
#include "kcp/ikcp.h"
}

namespace godot {

class KcpPeer : public RefCounted {
    GDCLASS(KcpPeer, RefCounted);

private:
    ikcpcb *kcp = nullptr;
    Callable output_callable;

    static int kcp_output(const char *buf, int len, ikcpcb *kcp, void *user);
    void reset();

protected:
    static void _bind_methods();

public:
    KcpPeer();
    ~KcpPeer();

    bool initialize(int64_t conv);
    void set_output(const Callable &p_output);
    Callable get_output() const;

    bool is_valid() const;
    int32_t get_conv() const;

    int input(const PackedByteArray &data);
    PackedByteArray recv(int64_t max_len = -1);
    int peek_size() const;
    int send(const PackedByteArray &data);
    void update(int64_t now_msec = -1);
    int check(int64_t now_msec = -1) const;
    int set_mtu(int mtu);
    int set_wndsize(int sndwnd, int rcvwnd);
    int set_nodelay(int nodelay, int interval, int resend, int nc);
    int get_waitsnd() const;
};

} // namespace godot
