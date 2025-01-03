#pragma once

#include "abstract_serialized.hpp"
#include "functor_utils.hpp"
#include "logger.hpp"
#include <filesystem>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <boost/multi_index/hashed_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index_container.hpp>

struct inotify_event;

class ResourceObserver : public AbstractSerialized
{
public:
    ResourceObserver(std::string resource_dir);
    ~ResourceObserver() override;

    std::filesystem::path getResourcePath(const std::string_view resource) const;
    std::unordered_set<std::filesystem::path> getTrackedDirs() const;  // for tests
    using tracking_handler_t = std::function<void()>;
    int trackChangesInFile(const std::filesystem::path& file, tracking_handler_t handler);
    void untrackFile(int id);

    void handleIn() override;
    void handleOut() override;

    [[nodiscard]] int getFd() const override;
    [[nodiscard]] bool wantIn() const override;
    [[nodiscard]] bool wantOut() const override;

private:
    void initInotify();
    void addRootWatcher();
    void handleEvent(const inotify_event& event);
    void collectResources();

    int createDirWatcher(const char *dir_name);
    void deleteDirWatcher(int wd);
    void addSubdirWatchers(const std::filesystem::path& dir, int wd);
    void deleteSubdirWatchers(int wd);
    void dirAppears(const inotify_event& event);
    void dirDisappears(const inotify_event& event);

    struct Node;
    const Node& getNode(int wd);
    const Node& getNode(const std::filesystem::path& path);

    struct Node
    {
        int watcher;
        std::filesystem::path node_path;
        mutable std::vector<std::pair<std::string, int>> childs;
    };

    using watcher_storage = boost::multi_index::multi_index_container<
        Node, boost::multi_index::indexed_by<
                  boost::multi_index::hashed_unique<boost::multi_index::member<Node, int, &Node::watcher>>,
                  boost::multi_index::hashed_unique<
                      boost::multi_index::member<Node, std::filesystem::path, &Node::node_path>>>>;

    std::filesystem::path resource_dir;
    std::unordered_map<std::string, std::filesystem::path, StringViewHash, StringViewEqualTo> resource_index;
    watcher_storage dirs_wd;

    std::unordered_map<int, tracking_handler_t> external_tracking;
    int fd{-1};

    Logger logger{"ResourceObserver"};
};
