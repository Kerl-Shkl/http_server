#include "resource_observer.hpp"
#include <cassert>
#include <cstring>
#include <fcntl.h>
#include <sys/inotify.h>
#include <sys/ioctl.h>
#include <unistd.h>

ResourceObserver::ResourceObserver(std::string the_resource_dir)
: resource_dir(std::move(the_resource_dir))
{
    initInotify();
    addRootWatcher();
}

std::unordered_set<std::filesystem::path> ResourceObserver::getTrackedDirs() const
{
    std::unordered_set<std::filesystem::path> result;
    for (const auto& node : dirs_wd) {
        result.insert(node.node_path);
    }
    return result;
}

ResourceObserver::~ResourceObserver()
{
    if (fd != -1) {
        for (const auto& node : dirs_wd) {
            deleteDirWatcher(node.watcher);
        }
        close(fd);
    }
}

void ResourceObserver::initInotify()
{
    fd = inotify_init1(0);
    if (fd == -1) {
        throw std::runtime_error(std::string("inotify_init1(0) error: ") + std::strerror(errno));
    }
    int flags = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void ResourceObserver::addRootWatcher()
{
    int root_wd = createDirWatcher(resource_dir.c_str());
    addSubdirWatchers(resource_dir, root_wd);
}

int ResourceObserver::createDirWatcher(const char *dir_name)
{
    int wd = inotify_add_watch(fd, dir_name, IN_MOVED_FROM | IN_MOVED_TO | IN_CREATE | IN_DELETE);
    dirs_wd.insert(Node{.watcher = wd, .node_path = dir_name});
    return wd;
}

void ResourceObserver::deleteDirWatcher(int wd)
{
    inotify_rm_watch(fd, wd);
    auto& index = dirs_wd.get<0>();
    auto iter = index.find(wd);
    assert(iter != index.end());
    index.erase(iter);
}

void ResourceObserver::addSubdirWatchers(const std::filesystem::path& dir, int parent_wd)
{
    namespace fs = std::filesystem;
    const Node& parent_node = getNode(parent_wd);
    for (const auto& dir_entry : fs::directory_iterator(dir)) {
        if (dir_entry.is_directory()) {
            logger.log("Add subdirectory: ", dir_entry.path().string());
            int wd = createDirWatcher(dir_entry.path().c_str());
            std::string dirname = dir_entry.path().filename().string();
            parent_node.childs.emplace_back(std::move(dirname), wd);
            addSubdirWatchers(dir_entry.path(), wd);
        }
    }
}

void ResourceObserver::deleteSubdirWatchers(int wd)
{
    auto& index = dirs_wd.get<0>();
    auto iter = index.find(wd);
    if (iter == index.end()) {
        return;
    }
    for (const auto& [_, child_wd] : iter->childs) {
        deleteSubdirWatchers(child_wd);
    }
    inotify_rm_watch(fd, wd);
    index.erase(iter);
}

void ResourceObserver::handleIn()
{
    unsigned int avail;
    int ri = ioctl(fd, FIONREAD, &avail);
    if (ri != 0) {
        throw std::runtime_error(std::string("ioctl with inotify fd error:") + std::strerror(errno));
    }
    std::vector<char> buffer(avail, 0);
    ssize_t read_count = read(fd, buffer.data(), buffer.size());
    assert(read_count == avail);

    for (size_t i = 0; i < buffer.size();) {
        auto *event = reinterpret_cast<inotify_event *>(buffer.data() + i);
        handleEvent(*event);
        i += sizeof(inotify_event) + event->len;
    }
}

void ResourceObserver::handleOut()
{
    logger.log("Something went wrong. ResourceObserver out??");
    assert(false);
}

void ResourceObserver::handleEvent(const inotify_event& event)
{
    if ((event.mask & IN_ISDIR) && (event.mask & (IN_CREATE | IN_MOVED_TO))) {
        logger.log(std::string("Handle event. Add subdir: ") + event.name +
                   " wd: " + std::to_string(event.wd));
        const Node& parent = getNode(event.wd);
        std::filesystem::path path = parent.node_path / event.name;
        int wd = createDirWatcher(path.c_str());
        parent.childs.emplace_back(event.name, wd);
        addSubdirWatchers(path, wd);
    }
    else if ((event.mask & IN_ISDIR) && (event.mask & (IN_DELETE | IN_MOVED_FROM))) {
        logger.log(std::string("Handle event. Remove subdir: ") + event.name +
                   " wd: " + std::to_string(event.wd));
        const Node& parent = getNode(event.wd);
        std::filesystem::path path = parent.node_path / event.name;
        const Node& deleted = getNode(path);
        deleteSubdirWatchers(deleted.watcher);
        auto pos = std::find_if(
            parent.childs.begin(), parent.childs.end(),
            [&event](const std::pair<std::string, int>& child) { return child.first == event.name; });
        parent.childs.erase(pos);
    }
}

[[nodiscard]] int ResourceObserver::getFd() const
{
    return fd;
}

[[nodiscard]] bool ResourceObserver::wantIn() const
{
    return true;
}

[[nodiscard]] bool ResourceObserver::wantOut() const
{
    return false;
}

auto ResourceObserver::getNode(int wd) -> const Node&
{
    auto& index = dirs_wd.get<0>();
    auto iter = index.find(wd);
    assert(iter != index.end());
    return *iter;
}

auto ResourceObserver::getNode(const std::filesystem::path& path) -> const Node&
{
    auto& index = dirs_wd.get<1>();
    auto iter = index.find(path);
    assert(iter != index.end());
    return *iter;
}
