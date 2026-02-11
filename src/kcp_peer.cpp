#include "kcp_peer.h"

#include <cstring>

#include <godot_cpp/classes/time.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/variant.hpp>

using namespace godot;

KcpPeer::KcpPeer() = default;

KcpPeer::~KcpPeer() {
    reset();
}

void KcpPeer::reset() {
    if (kcp) {
        ikcp_release(kcp);
        kcp = nullptr;
    }
}

bool KcpPeer::initialize(int64_t conv) {
    reset();
    kcp = ikcp_create(static_cast<IUINT32>(conv), this);
    if (!kcp) {
        return false;
    }
    ikcp_setoutput(kcp, &KcpPeer::kcp_output);
    // Default to a smoother, lower-latency profile.
    ikcp_nodelay(kcp, 1, 10, 2, 1);
    return true;
}

void KcpPeer::set_output(const Callable &p_output) {
    output_callable = p_output;
}

Callable KcpPeer::get_output() const {
    return output_callable;
}

bool KcpPeer::is_valid() const {
    return kcp != nullptr;
}

int32_t KcpPeer::get_conv() const {
    return kcp ? static_cast<int32_t>(kcp->conv) : -1;
}

int KcpPeer::input(const PackedByteArray &data) {
    if (!kcp) {
        return -1;
    }
    if (data.is_empty()) {
        return 0;
    }
    const int size = data.size();
    const char *ptr = reinterpret_cast<const char *>(data.ptr());
    return ikcp_input(kcp, ptr, size);
}

PackedByteArray KcpPeer::recv(int64_t max_len) {
    PackedByteArray out;
    if (!kcp) {
        return out;
    }

    const int peek = ikcp_peeksize(kcp);
    if (peek < 0) {
        return out;
    }

    if (max_len >= 0 && max_len < peek) {
        return out;
    }

    const int to_read = peek;
    out.resize(to_read);
    ikcp_recv(kcp, reinterpret_cast<char *>(out.ptrw()), to_read);
    return out;
}

int KcpPeer::peek_size() const {
    if (!kcp) {
        return -1;
    }
    return ikcp_peeksize(kcp);
}

int KcpPeer::send(const PackedByteArray &data) {
    if (!kcp) {
        return -1;
    }
    if (data.is_empty()) {
        return 0;
    }
    const int size = data.size();
    const char *ptr = reinterpret_cast<const char *>(data.ptr());
    return ikcp_send(kcp, ptr, size);
}

void KcpPeer::update(int64_t now_msec) {
    if (!kcp) {
        return;
    }
    const IUINT32 now = now_msec >= 0 ? static_cast<IUINT32>(now_msec)
                                      : static_cast<IUINT32>(Time::get_singleton()->get_ticks_msec());
    ikcp_update(kcp, now);
}

int KcpPeer::check(int64_t now_msec) const {
    if (!kcp) {
        return -1;
    }
    const IUINT32 now = now_msec >= 0 ? static_cast<IUINT32>(now_msec)
                                      : static_cast<IUINT32>(Time::get_singleton()->get_ticks_msec());
    return static_cast<int>(ikcp_check(kcp, now));
}

int KcpPeer::set_mtu(int mtu) {
    if (!kcp) {
        return -1;
    }
    return ikcp_setmtu(kcp, mtu);
}

int KcpPeer::set_wndsize(int sndwnd, int rcvwnd) {
    if (!kcp) {
        return -1;
    }
    return ikcp_wndsize(kcp, sndwnd, rcvwnd);
}

int KcpPeer::set_nodelay(int nodelay, int interval, int resend, int nc) {
    if (!kcp) {
        return -1;
    }
    return ikcp_nodelay(kcp, nodelay, interval, resend, nc);
}

int KcpPeer::get_waitsnd() const {
    if (!kcp) {
        return -1;
    }
    return ikcp_waitsnd(kcp);
}

int KcpPeer::kcp_output(const char *buf, int len, ikcpcb *, void *user) {
    if (len <= 0 || !user) {
        return 0;
    }
    KcpPeer *self = static_cast<KcpPeer *>(user);
    if (!self->output_callable.is_valid()) {
        return 0;
    }

    PackedByteArray payload;
    payload.resize(len);
    std::memcpy(payload.ptrw(), buf, len);

    Array args;
    args.append(payload);
    self->output_callable.callv(args);
    return 0;
}

void KcpPeer::_bind_methods() {
    ClassDB::bind_method(D_METHOD("initialize", "conv"), &KcpPeer::initialize);
    ClassDB::bind_method(D_METHOD("set_output", "callable"), &KcpPeer::set_output);
    ClassDB::bind_method(D_METHOD("get_output"), &KcpPeer::get_output);

    ClassDB::bind_method(D_METHOD("is_valid"), &KcpPeer::is_valid);
    ClassDB::bind_method(D_METHOD("get_conv"), &KcpPeer::get_conv);

    ClassDB::bind_method(D_METHOD("input", "packet"), &KcpPeer::input);
    ClassDB::bind_method(D_METHOD("recv", "max_len"), &KcpPeer::recv, DEFVAL(-1));
    ClassDB::bind_method(D_METHOD("peek_size"), &KcpPeer::peek_size);
    ClassDB::bind_method(D_METHOD("send", "packet"), &KcpPeer::send);
    ClassDB::bind_method(D_METHOD("update", "now_msec"), &KcpPeer::update, DEFVAL(-1));
    ClassDB::bind_method(D_METHOD("check", "now_msec"), &KcpPeer::check, DEFVAL(-1));
    ClassDB::bind_method(D_METHOD("set_mtu", "mtu"), &KcpPeer::set_mtu);
    ClassDB::bind_method(D_METHOD("set_wndsize", "sndwnd", "rcvwnd"), &KcpPeer::set_wndsize);
    ClassDB::bind_method(D_METHOD("set_nodelay", "nodelay", "interval", "resend", "nc"), &KcpPeer::set_nodelay);
    ClassDB::bind_method(D_METHOD("get_waitsnd"), &KcpPeer::get_waitsnd);

    ADD_PROPERTY(PropertyInfo(Variant::CALLABLE, "output"), "set_output", "get_output");
}
