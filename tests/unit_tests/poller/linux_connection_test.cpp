#include "linux_connection.hpp"
#include <fcntl.h>
#include <gtest/gtest.h>
#include <random>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

class ConnectionFixture : public testing::Test
{
protected:
    ~ConnectionFixture() override
    {
        if (!lcon.closed()) {
            lcon.closeConnection();
        }
        if (peer_fd != -1) {
            close(peer_fd);
            peer_fd = -1;
        }
    }

    void SetUp() override
    {
        int fds[2];
        int res = socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, fds);
        ASSERT_GE(res, 0);
        lcon.setSocket(fds[0]);
        peer_fd = fds[1];
    }

    [[nodiscard]] std::string getRandomString() const
    {
        constexpr size_t size = 4096;
        std::random_device r;
        unsigned int seed = r();
        std::cout << "String seed: " << seed << std::endl;
        std::mt19937 mt(seed);
        std::uniform_int_distribution<std::mt19937::result_type> dist(0, 'z' - 'a' - 1);
        std::string result;
        for (int i = 0; i < size; ++i) {
            result += ('a' + dist(mt));
        }
        return result;
    }

    [[nodiscard]] std::vector<uint8_t> getRandomBuffer() const
    {
        constexpr size_t size = 4096;
        std::random_device r;
        unsigned int seed = r();
        std::cout << "Buffer seed: " << seed << std::endl;
        std::mt19937 mt(seed);
        std::uniform_int_distribution<std::mt19937::result_type> dist(0, 255);
        std::vector<uint8_t> result;
        result.reserve(size);
        for (int i = 0; i < size; ++i) {
            result.push_back(dist(mt));
        }
        return result;
    }

    std::string test_str = getRandomString();
    std::vector<uint8_t> test_buffer = getRandomBuffer();
    LinuxConnection lcon;
    int peer_fd = -1;
};

TEST_F(ConnectionFixture, closeConnectionCloseSocket)
{
    lcon.closeConnection();

    ASSERT_EQ(lcon.getSocket(), -1);
    char buf{};
    ssize_t readed = read(peer_fd, &buf, 1);
    ASSERT_EQ(readed, 0);
}

TEST_F(ConnectionFixture, writeMessage)
{
    lcon.writeMessage(test_str);

    std::string getted(test_str.size(), 0);
    ssize_t readed = read(peer_fd, getted.data(), getted.size());
    ASSERT_EQ(readed, test_str.size());
    ASSERT_EQ(test_str, getted);
}

TEST_F(ConnectionFixture, readMessage)
{
    ssize_t writed = write(peer_fd, test_str.data(), test_str.size());
    ASSERT_EQ(writed, test_str.size());
    std::string getted = lcon.readMessage();

    ASSERT_EQ(test_str, getted);
}

TEST_F(ConnectionFixture, readSmallMessage)
{
    constexpr int write_size = 10;
    ssize_t writed = write(peer_fd, test_str.data(), write_size);
    ASSERT_EQ(writed, write_size);
    std::string getted = lcon.readMessage();

    ASSERT_EQ(test_str.substr(0, write_size), getted);
}

TEST_F(ConnectionFixture, writeBuffer)
{
    int writed = lcon.writeBuffer(test_buffer);
    ASSERT_EQ(writed, test_buffer.size());

    std::vector<uint8_t> getted(test_buffer.size(), 0);
    ssize_t readed = read(peer_fd, getted.data(), getted.size());
    ASSERT_EQ(readed, test_buffer.size());
    ASSERT_EQ(test_buffer, getted);
}

TEST_F(ConnectionFixture, readBuffer)
{
    ssize_t writed = write(peer_fd, test_buffer.data(), test_buffer.size());
    ASSERT_EQ(writed, test_buffer.size());
    std::vector<uint8_t> getted = lcon.readBuffer();

    ASSERT_EQ(getted, test_buffer);
    ASSERT_EQ(getted.size(), test_buffer.size());
}

TEST_F(ConnectionFixture, readSmallBuffer)
{
    constexpr int write_size = 10;
    ssize_t writed = write(peer_fd, test_buffer.data(), write_size);
    ASSERT_EQ(writed, write_size);
    auto getted = lcon.readBuffer();
    ASSERT_EQ(getted.size(), write_size);
    for (int i = 0; i < write_size; ++i) {
        ASSERT_EQ(getted[i], test_buffer[i]);
    }
}

TEST_F(ConnectionFixture, readMessageAfterClose)
{
    ssize_t writed = write(peer_fd, test_str.data(), test_str.size());
    ASSERT_EQ(writed, test_str.size());
    close(peer_fd);
    peer_fd = -1;
    std::string getted = lcon.readMessage();
    ASSERT_EQ(test_str, getted);
    ASSERT_TRUE(lcon.closed());
}

TEST_F(ConnectionFixture, readBufferAfterClose)
{
    ssize_t writed = write(peer_fd, test_buffer.data(), test_buffer.size());
    ASSERT_EQ(writed, test_buffer.size());
    close(peer_fd);
    peer_fd = -1;
    auto getted = lcon.readBuffer();
    ASSERT_EQ(test_buffer, getted);
    ASSERT_TRUE(lcon.closed());
}
