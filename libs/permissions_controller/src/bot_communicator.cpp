#include "bot_communicator.hpp"
#include "permissions_controller.hpp"
#include <algorithm>
#include <cassert>
#include <fcntl.h>
#include <iterator>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

BotCommunicator::BotCommunicator(PermissionsController& controller)
: permissions_controller{controller}
{}

void BotCommunicator::askRequest(const BotRequest& request)
{
    auto serialized = serializeRequest(request);
    std::ranges::copy(serialized, std::back_inserter(write_buffer));
    updateSerializedMode();
}

void BotCommunicator::setSocket(int fd)
{
    assert(lcon.closed());
    lcon.setSocket(fd);
}

void BotCommunicator::connectBot()
{
    setSocket(openSocket());
}

int BotCommunicator::openSocket() const
{
    sockaddr_un serv_addr{};
    memset(reinterpret_cast<uint8_t *>(&serv_addr), 0, sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, socket_path);
    size_t servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);

    int sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sockfd < 0) {
        throw std::runtime_error{"Can't create unix socket for tg bot"};
    }
    int conn_res = connect(sockfd, reinterpret_cast<struct sockaddr *>(&serv_addr), servlen);
    if (conn_res < 0) {
        throw std::runtime_error{"Can't connect unix socket for tg bot"};
    }
    int flags = fcntl(sockfd, F_GETFL);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
    return sockfd;
}

void BotCommunicator::handleIn()
{
    try {
        std::vector<uint8_t> readed = lcon.readBuffer();
        std::ranges::copy(readed, std::back_inserter(read_buffer));
        while (read_buffer.size() >= sizeof(BotResponse)) {
            auto response = parseResponse(read_buffer);
            permissions_controller.handleResponse(response);
            read_buffer.erase(read_buffer.begin(), read_buffer.begin() + sizeof(BotResponse));
        }
    }
    catch (const std::exception& exception) {
        logger.log("handleIn error: ", exception.what());
        lcon.closeConnection();
    }
}

void BotCommunicator::handleOut()
{
    try {
        if (!lcon.closed()) {
            int writted = lcon.writeBuffer(write_buffer);
            write_buffer.erase(write_buffer.begin(), write_buffer.begin() + writted);
        }
    }
    catch (const std::exception& exception) {
        logger.log("handleOut error: ", exception.what());
        lcon.closeConnection();
    }
}

[[nodiscard]] int BotCommunicator::getFd() const
{
    return lcon.getSocket();
}
[[nodiscard]] bool BotCommunicator::wantIn() const
{
    return !lcon.closed();
}
[[nodiscard]] bool BotCommunicator::wantOut() const
{
    return !lcon.closed() && !write_buffer.empty();
}

[[nodiscard]] std::vector<uint8_t> BotCommunicator::serializeRequest(const BotRequest& request) const
{
    constexpr size_t id_size = sizeof(uuids::uuid);
    uint32_t size = sizeof(uint32_t) + id_size + sizeof(RequestOperation) + request.name.size();
    std::vector<uint8_t> buffer(size, 0);
    std::memcpy(buffer.data(), &size, sizeof(uint32_t));
    std::memcpy(buffer.data() + sizeof(uint32_t), request.id.as_bytes().data(), id_size);
    std::memcpy(buffer.data() + sizeof(uint32_t) + id_size, &request.op, sizeof(RequestOperation));
    std::memcpy(buffer.data() + sizeof(uint32_t) + id_size + sizeof(RequestOperation),  //
                request.name.data(), request.name.size());
    return buffer;
}

[[nodiscard]] BotResponse BotCommunicator::parseResponse(const std::span<uint8_t> buffer) const
{
    constexpr size_t id_size = sizeof(uuids::uuid);
    assert(buffer.size() >= sizeof(BotResponse));
    BotResponse response;
    std::array<uuids::uuid::value_type, id_size> readed_id{};
    std::memcpy(readed_id.data(), buffer.data(), readed_id.size());
    response.id = uuids::uuid{readed_id};
    response.allowed = (*reinterpret_cast<uint8_t *>((buffer.data() + id_size)) != 0U);
    return response;
}
bool BotCommunicator::connected() const noexcept
{
    return !lcon.closed();
}
