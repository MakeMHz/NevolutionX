#include "config.hpp"

#include <fstream>
#include <iostream>
#include <iomanip>

#ifdef NXDK
#define SEPARATOR "\\"
#define HOME "A:" SEPARATOR
#else
#define SEPARATOR "/"
#define HOME "." SEPARATOR
#endif

/*============================================================================*/
/*                                      FTP                                   */
/*============================================================================*/
ftpConfig::ftpConfig() {
  enable = true;
  username = "xbox";
  password = "xbox";
  port = 21;
}

void to_json(nlohmann::json& j, ftpConfig const& f) {
  j = nlohmann::json{{"enable", f.getEnabled()},
                     {"username", f.getUser()}, {"password", f.getPassword()},
                     {"port", f.getPort()}};
}

void from_json(nlohmann::json const& j, ftpConfig& f) {
  if (j.contains("enable")) {
    if (j["enable"].is_boolean()) {
      f.setEnabled(j["enable"]);
    }
  }
  if (j.contains("username")) {
    if (j["username"].is_string()) {
      f.setUser(j["username"]);
    }
  }
  if (j.contains("password")) {
    if (j["password"].is_string()) {
      f.setPassword(j["password"]);
    }
  }
  if (j.contains("port")) {
    if (j["port"].is_number()) {
      f.setPort(j["port"]);
    }
  }
}

void ftpConfig::setEnabled(bool val) {
  if (enable != val) {
    enable = val;
  }
}

void ftpConfig::setUser(std::string const& user) {
  if (username.compare(user)) {
    username = user;
  }
}

void ftpConfig::setPassword(std::string const& pwd) {
  if (password.compare(pwd)) {
    password = pwd;
  }
}

void ftpConfig::setPort(int p) {
  if (port != p) {
    port = p;
  }
}

/*============================================================================*/
/*                                      SNTP                                  */
/*============================================================================*/
sntpConfig::sntpConfig() {
  enable = false;
  address = "no";
  port = 123;
}

void to_json(nlohmann::json& j, sntpConfig const& s) {
  j = nlohmann::json{{"enable", s.getEnabled()},
                     {"server", s.getAddress()},
                     {"port", s.getPort()}};
}

void from_json(nlohmann::json const& j, sntpConfig& s) {
  if (j.contains("enable")) {
    if (j["enable"].is_boolean()) {
      s.setEnabled(j["enable"]);
    }
  }
  if (j.contains("server")) {
    if (j["server"].is_string()) {
      s.setAddress(j["server"]);
    }
  }
  if (j.contains("port")) {
    if (j["port"].is_number()) {
      s.setPort(j["port"]);
    }
  }
}

void sntpConfig::setEnabled(bool val) {
  if (enable != val) {
    enable = val;
  }
}

void sntpConfig::setAddress(std::string const& addr) {
  if (address.compare(addr)) {
    address = addr;
  }
}

void sntpConfig::setPort(int p) {
  if (port != p) {
    port = p;
  }
}

/*============================================================================*/
/*                                Settings                                    */
/*============================================================================*/
void to_json(nlohmann::json& j, Settings const& s) {
  j = nlohmann::json{{"ftp", s.ftp}, {"sntp", s.sntp}};
}

void from_json(nlohmann::json const& j, Settings& s) {
  if (j.contains("ftp")) {
    s.ftp = j["ftp"].get<ftpConfig>();
  }
  if (j.contains("sntp")) {
    s.sntp = j["sntp"].get<sntpConfig>();
  }
}

/*============================================================================*/
/*                                 Config                                     */
/*============================================================================*/
Config::Config() {
  std::ifstream configFile(HOME "config.json");
  nlohmann::json json;
  // FIXME: Once nxdk supports C++ Exceptions, this needs to be put in a try-catch block!
  configFile >> json;
  if (json.contains("settings")) {
    settings = json["settings"].get<Settings>();
  } else {
    changed = true;
  }
  if (!json.contains("menu")) {
    json["menu"] = R"([{"label": "Games", "type": "scan", "path": "F:\\Games\\"},
        {"label": "Applications", "type": "scan", "path": "F:\\Apps"},
        {"label": "Launch DVD", "type": "launch", "path": "D:\\default.xbe"},
        {"label": "Settings", "type": "settings"},
        {"label": "Reboot", "type": "reboot"}])"_json;
    changed = true;
  }
  menu = json["menu"];
  configFile.close();
}

void Config::setChanged() {
  changed = true;
}

void Config::storeToDisk() {
  if (changed) {
    std::ofstream configFile(HOME "config.json");
    nlohmann::json json;
    to_json(json, *this);
    configFile << std::setw(4) << json << std::endl;
    configFile.close();
  }
}
