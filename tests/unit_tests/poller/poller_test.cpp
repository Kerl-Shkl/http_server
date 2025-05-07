#include "poller.hpp"
#include "abstract_serialized.hpp"
#include <gtest/gtest.h>
#include <sys/socket.h>

struct MockSerialized : public AbstractSerialized
{
public:
    void handleIn() override
    {
        if (handle_in_func) {
            handle_in_func(fd);
        }
    }
    void handleOut() override
    {
        if (handle_out_func) {
            handle_out_func(fd);
        }
    }

    void setFd(int f)
    {
        fd = f;
    }

    [[nodiscard]] int getFd() const override
    {
        return fd;
    }

    [[nodiscard]] bool wantIn() const override
    {
        return want_in_func ? want_in_func() : false;
    }
    [[nodiscard]] bool wantOut() const override
    {
        return want_out_func ? want_out_func() : false;
    }

    std::function<void(int)> handle_in_func;
    std::function<void(int)> handle_out_func;
    std::function<bool()> want_in_func;
    std::function<bool()> want_out_func;

    int fd = -1;
};

class PollerFixture : public testing::Test
{
protected:
    ~PollerFixture() override
    {
        close(fds[0]);
        close(fds[1]);
    }

    void SetUp() override
    {
        int res = socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, fds);
        ASSERT_GE(res, 0);
    }

    void fillSndBuffer(int fd)
    {
        unsigned int send_size;
        socklen_t size_size;
        int sockopt_return = getsockopt(fd, SOL_SOCKET, SO_SNDBUF, &send_size, &size_size);
        ASSERT_NE(sockopt_return, -1);
        ASSERT_GT(send_size, 0);
        int write_res;
        do {
            std::vector<uint8_t> buffer(send_size, 0);
            write_res = write(fd, buffer.data(), buffer.size());
        } while (write_res > 0);
        ASSERT_EQ(errno, EAGAIN);
    }

    void readSocket(int fd)
    {
        char buffer[1024];
        while (read(fd, buffer, 1024) > 0) {
        }
    }

    int fds[2];
    Poller poller;
};

TEST_F(PollerFixture, waitIn)
{
    MockSerialized ser;
    ser.fd = fds[0];
    ser.want_in_func = []() { return true; };
    ser.handle_in_func = [this](int fd) { readSocket(fd); };
    poller.addSerialized(&ser);
    ASSERT_EQ(poller.check(), nullptr);
    write(fds[1], "Hello", 5);
    ASSERT_EQ(poller.wait(-1), &ser);
}

TEST_F(PollerFixture, unblockOut)
{
    MockSerialized ser;
    ser.fd = fds[0];
    ser.want_out_func = [oneshot = true]() mutable {
        bool tmp = oneshot;
        oneshot = false;
        return tmp;
    };
    ser.handle_out_func = [](int fd) {
        ssize_t writed = write(fd, "A", 1);
        ASSERT_EQ(writed, 1);
    };

    fillSndBuffer(fds[0]);
    poller.addSerialized(&ser);
    ASSERT_EQ(poller.check(), nullptr);
    readSocket(fds[1]);
    ASSERT_EQ(poller.wait(-1), &ser);
}

TEST_F(PollerFixture, updateMode)
{
    MockSerialized ser;
    ser.fd = fds[0];
    ser.want_in_func = []() { return true; };
    ser.handle_in_func = [this](int fd) { readSocket(fd); };
    ser.handle_out_func = [](int fd) {
        ssize_t writed = write(fd, "A", 1);
        ASSERT_EQ(writed, 1);
    };

    poller.addSerialized(&ser);
    ASSERT_EQ(poller.check(), nullptr);
    ser.want_out_func = [oneshot = true]() mutable {
        bool tmp = oneshot;
        oneshot = false;
        return tmp;
    };
    ASSERT_EQ(poller.check(), nullptr);
    poller.updateSerializedMode(ser);
    ASSERT_EQ(poller.wait(-1), &ser);
}
