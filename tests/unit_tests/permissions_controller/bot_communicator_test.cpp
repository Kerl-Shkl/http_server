#include "bot_communicator.hpp"
#include "permissions_controller.hpp"
#include <gtest/gtest.h>
#include <iterator>
#include <sys/socket.h>

class PermissionsControllerFixture : public testing::Test
{
protected:
    void SetUp() override
    {
        int fds[2];
        int res = socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, fds);
        ASSERT_GE(res, 0);
        bot_communicator = &controller.getCommunicator();
        bot_communicator->setSocket(fds[0]);
        peer_socket = fds[1];
        poller.addSerialized(bot_communicator);
    }

    BotRequest readRequest(int fd)
    {
        uint32_t readed_size = 0;
        size_t readed = read(fd, &readed_size, 4);
        EXPECT_EQ(readed, 4);
        EXPECT_GE(readed_size, 21);
        uint32_t req_size = readed_size - 4;
        std::vector<uint8_t> read_buffer(1024, 0);
        readed = read(fd, read_buffer.data(), req_size);
        EXPECT_EQ(readed, req_size);
        auto readed_id = uuids::uuid{read_buffer.begin(), read_buffer.begin() + 16};
        RequestOperation op = static_cast<RequestOperation>(read_buffer[16]);
        std::string readed_name;
        std::copy(read_buffer.begin() + 17, read_buffer.begin() + req_size, std::back_inserter(readed_name));
        return BotRequest{.id = readed_id, .op = op, .name = readed_name};
    }

    int peer_socket{};
    Poller poller;
    PermissionsController controller;
    BotCommunicator *bot_communicator{nullptr};
};

TEST_F(PermissionsControllerFixture, askRequest)
{
    std::array<uuids::uuid::value_type, 16> id_buff{
        {0x47, 0x18, 0x38, 0x23, 0x25, 0x74, 0x4b, 0xfd, 0xb4, 0x11, 0x99, 0xed, 0x17, 0x7d, 0x3e, 0x43}
    };
    BotRequest req{.id = uuids::uuid{id_buff}, .op = RequestOperation::add, .name = "test name"};
    std::uint64_t request_size = 4 + 16 + 1 + req.name.size();
    ASSERT_EQ(poller.check(), nullptr);
    bot_communicator->askRequest(req);
    ASSERT_EQ(bot_communicator, poller.wait(-1));
    std::vector<uint8_t> read_buffer(1024, 0);
    size_t readed = read(peer_socket, read_buffer.data(), read_buffer.size());
    ASSERT_EQ(readed, request_size);
    uint32_t readed_size = *reinterpret_cast<uint32_t *>(read_buffer.data());
    ASSERT_EQ(request_size, readed_size);
    auto readed_id = uuids::uuid{read_buffer.begin() + 4, read_buffer.begin() + 4 + 16};
    ASSERT_EQ(readed_id, req.id);
    ASSERT_EQ(read_buffer[20], static_cast<uint8_t>(req.op));
    std::string readed_name;
    std::copy(read_buffer.begin() + 21, read_buffer.begin() + readed_size, std::back_inserter(readed_name));
    ASSERT_EQ(readed_name, req.name);
}

TEST_F(PermissionsControllerFixture, fullLoop)
{
    bool success = false;
    controller.askPermission("test name", RequestOperation::add, [&success](bool allowed) {
        success = allowed;
        ASSERT_TRUE(allowed);
    });
    ASSERT_EQ(poller.wait(-1), bot_communicator);
    BotRequest req = readRequest(peer_socket);
    ASSERT_EQ(req.name, "test name");
    size_t writed = write(peer_socket, req.id.as_bytes().data(), 16);
    ASSERT_EQ(writed, 16);
    ASSERT_EQ(poller.wait(-1), bot_communicator);
    ASSERT_FALSE(success);
    bool answer = true;
    writed = write(peer_socket, &answer, 1);
    ASSERT_EQ(writed, 1);
    ASSERT_EQ(poller.wait(-1), bot_communicator);
    ASSERT_TRUE(success);
}
