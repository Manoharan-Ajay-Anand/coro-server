#include "read_awaiter.h"
#include "coros/socket.h"
#include "coros/memory/read_socket_buffer.h"

#include <algorithm>
#include <coroutine>
#include <iostream>
#include <stdexcept>

void coros::awaiter::SocketReadAwaiter::read(std::coroutine_handle<> handle) {
    try {
        while (size > 0) {
            if (buffer.remaining() > 0) {
                int size_to_read = std::min(buffer.remaining(), size);
                buffer.read(dest + offset, size_to_read);
                offset += size_to_read;
                size -= size_to_read;
                continue;
            }
            int status = buffer.recv_socket();
            if (status == SOCKET_OP_WOULD_BLOCK) {
                return socket.listen_for_read([&, handle]() {
                    read(handle);
                }, [handle]() { handle.destroy(); });
            }
        }
    } catch (std::runtime_error error) {
        this->error = error;
    }
    handle.resume();
}

bool coros::awaiter::SocketReadAwaiter::await_ready() noexcept {
    int sizeToRead = std::min(size, buffer.remaining());
    buffer.read(dest + offset, sizeToRead);
    offset += sizeToRead;
    size -= sizeToRead;
    return size == 0;
}

void coros::awaiter::SocketReadAwaiter::await_suspend(std::coroutine_handle<> handle) {
    read(handle);
}

void coros::awaiter::SocketReadAwaiter::await_resume() {
    if (size > 0) {
        throw error;
    }
}
