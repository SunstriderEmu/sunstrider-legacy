
#ifndef CONFIG_H
#define CONFIG_H

#include "Define.h"

#include <string>
#include <list>
#include <vector>
#include <mutex>
#include <boost/property_tree/ptree.hpp>

class TC_COMMON_API ConfigMgr
{
public:
    ConfigMgr() = default;
    ~ConfigMgr() = default;

    /// Method used only for loading main configuration files (authserver.conf and worldserver.conf)
    bool LoadInitial(std::string const& file, std::vector<std::string> args, std::string& error);

    static ConfigMgr* instance();

    bool Reload(std::string& error);

    std::string GetStringDefault(std::string const& name, const std::string& def) const;
    bool GetBoolDefault(std::string const& name, bool def) const;
    int GetIntDefault(std::string const& name, int def) const;
    float GetFloatDefault(std::string const& name, float def) const;

    std::string const& GetFilename();
	std::vector<std::string> const& GetArguments() const { return _args; }
    std::list<std::string> GetKeysByString(std::string const& name);

private:
    std::string _filename;
	std::vector<std::string> _args;
    boost::property_tree::ptree _config;
    std::mutex _configLock;

    ConfigMgr& operator=(ConfigMgr const&) = delete;
	ConfigMgr(ConfigMgr const&) = delete;

	template<class T>
	T GetValueDefault(std::string const& name, T def) const;
};

#define sConfigMgr ConfigMgr::instance()

#endif
