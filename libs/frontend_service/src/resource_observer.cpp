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
    collectResources();
}

ResourceObserver::~ResourceObserver()
{
    close(fd);
}

std::unordered_set<std::filesystem::path> ResourceObserver::getTrackedDirs() const
{
    std::unordered_set<std::filesystem::path> result;
    for (const auto& node : dirs_wd) {
        result.insert(node.node_path);
    }
    return result;
}

std::filesystem::path ResourceObserver::getResourcePath(const std::string_view resource) const
{
    size_t begin = resource.find_last_of('/');
    auto corrected_target = resource.substr(begin == std::string_view::npos ? 0 : begin + 1);
    auto it = resource_index.find(corrected_target);
    return (it != resource_index.end()) ? it->second : "";
}

int ResourceObserver::trackChangesInFile(const std::filesystem::path& file, tracking_handler_t handler)
{
    int wd = inotify_add_watch(fd, file.c_str(), IN_DELETE_SELF | IN_MODIFY);
    external_tracking.insert({wd, std::move(handler)});
    return wd;
}

void ResourceObserver::untrackFile(int id)
{
    external_tracking.erase(id);
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
    if (auto iter = index.find(wd); iter != index.end()) {
        for (const auto& [_, child_wd] : iter->childs) {
            deleteSubdirWatchers(child_wd);
        }
        inotify_rm_watch(fd, wd);
        index.erase(iter);
    }
}

void ResourceObserver::handleIn()
{
    unsigned int avail;
    int ri = ioctl(fd, FIONREAD, &avail);
    if (ri != 0) {
        throw std::runtime_error(std::string("ioctl with inotify fd error:") + std::strerror(errno));
    }
    if (avail == 0) {
        return;
    }
    std::vector<char> buffer(avail, 0);
    ssize_t read_count = read(fd, buffer.data(), buffer.size());
    if (read_count == -1) {
        if (errno != EAGAIN) {
            throw std::runtime_error(std::string("read() error: ") + std::strerror(errno));
        }
        return;
    }
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

// Ugly! Rework soon
void ResourceObserver::handleEvent(const inotify_event& event)
{
    if (event.mask & (IN_CREATE | IN_MOVED_TO)) {
        if (event.mask & IN_ISDIR) {
            logger.log(std::string("Handle event. Add subdir: ") + event.name + " in " +
                       getNode(event.wd).node_path.string());
            dirAppears(event);
        }  // else file created
        collectResources();
    }
    else if (event.mask & (IN_DELETE | IN_MOVED_FROM)) {
        if (event.mask & IN_ISDIR) {
            logger.log(std::string("Handle event. Remove subdir: ") + event.name + " from " +
                       getNode(event.wd).node_path.string());
            dirDisappears(event);
        }  // else file deletion
        collectResources();
    }
    else if (auto iter = external_tracking.find(event.wd); iter != external_tracking.end()) {
        if (event.mask & IN_DELETE_SELF) {
            logger.log("Delete tracking file: ", iter->first);
            external_tracking.erase(iter);
        }
        else {
            logger.log("Update tracking file: ", iter->first);
            iter->second();
        }
    }
}

void ResourceObserver::dirAppears(const inotify_event& event)
{
    const Node& parent = getNode(event.wd);
    std::filesystem::path path = parent.node_path / event.name;
    int wd = createDirWatcher(path.c_str());
    parent.childs.emplace_back(event.name, wd);
    addSubdirWatchers(path, wd);
}

void ResourceObserver::dirDisappears(const inotify_event& event)
{
    const Node& parent = getNode(event.wd);
    std::filesystem::path path = parent.node_path / event.name;
    const Node& deleted = getNode(path);
    deleteSubdirWatchers(deleted.watcher);
    auto pos = std::find_if(
        parent.childs.begin(), parent.childs.end(),
        [&event](const std::pair<std::string, int>& child) { return child.first == event.name; });
    parent.childs.erase(pos);
}

void ResourceObserver::collectResources()
{
    resource_index.clear();
    namespace fs = std::filesystem;
    for (const fs::directory_entry& dir_entry : fs::recursive_directory_iterator(resource_dir)) {
        if (dir_entry.is_regular_file()) {
            auto path = dir_entry.path();
            resource_index.emplace(path.filename(), path);
        }
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
