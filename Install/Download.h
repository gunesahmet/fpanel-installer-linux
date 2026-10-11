#pragma once
#ifndef INSTALLER_DOWNLOAD_H
#define INSTALLER_DOWNLOAD_H

#include <Helpers/Helpers.h>
#include <Config/Config.h>
#include <Logger/Logger.h>
#include <Install/Items.h>
#include <boost/asio.hpp>
#include <map>
#include <boost/iostreams/device/mapped_file.hpp>
#include <openssl/md5.h>
#include <sstream>
#include <iomanip>

using boost::asio::ip::tcp;
using namespace boost::asio;
class Download {
private:
    Helpers _helpers;
    Config *_config;
    Logger _logger;
    Items *_items;

    std::string getList()
    {
        std::string fpanel_repo_addr = _config->getValue("panel.http.server.address");
        std::string fpanel_repo_path = _config->getValue("panel.http.server.distro");
//        _helpers.Println("Downloading: " + fpanel_repo_path + "packages.list", Helpers::RED);
//        _helpers.Println("Repo Address: " + fpanel_repo_addr, Helpers::RED);
        std::string packages_list;
        try {
            boost::asio::io_service io_service;
            // Get a list of endpoints corresponding to the server name.
            tcp::resolver resolver(io_service);
            boost::asio::ip::tcp::resolver::query query(fpanel_repo_addr, "80");

            tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
            tcp::resolver::iterator end;
            // Try each endpoint until we successfully establish a connection.
            tcp::socket socket(io_service);
            boost::system::error_code error = boost::asio::error::host_not_found;
            while (error && endpoint_iterator != end) {
                socket.close();
                socket.connect(*endpoint_iterator++, error);
            }
            if (error)
                throw boost::system::system_error(error);
            // Form the request. We specify the "Connection: close" header so that the
            // server will close the socket after transmitting the response. This will
            // allow us to treat all data up until the EOF as the content.
            boost::asio::streambuf request;
            std::ostream request_stream(&request);
            request_stream << "GET " << fpanel_repo_path << "packages.list" << " HTTP/1.0\r\n";
            request_stream << "Host: " << fpanel_repo_addr  << "\r\n";
            request_stream << "Accept: */*\r\n";
            request_stream << "Connection: close\r\n\r\n";
            // Send the request.
            boost::asio::write(socket, request);
            // Read the response status line.
            boost::asio::streambuf response;
            boost::asio::read_until(socket, response, "\r\n");
            // Check that response is OK.
            std::istream response_stream(&response);
            std::string http_version;
            response_stream >> http_version;
            unsigned int status_code;
            response_stream >> status_code;
            std::string status_message;
            std::getline(response_stream, status_message);
            if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
                _logger.Error("Invalid response");
                return {};
            }
            if (status_code != 200) {
                _logger.Error("Response returned with status code " + std::to_string(status_code));
                return {};
            }

            // Read the response headers, which are terminated by a blank line.
            boost::asio::read_until(socket, response, "\r\n\r\n");

            std::string header;
            while (std::getline(response_stream, header) && header != "\r") {
            }

            if (response.size() > 0) {
                std::istreambuf_iterator<char> eosf;
                std::string s(std::istreambuf_iterator<char>(&response), eosf);
                packages_list += s;
            }
            // Read until EOF, writing data to output as we go.
            while (boost::asio::read(socket, response, boost::asio::transfer_at_least(1), error)) {
                std::istreambuf_iterator<char> eoss;
                std::string s(std::istreambuf_iterator<char>(&response), eoss);
                packages_list += s;
            }
        }
        catch (std::exception &e) {
            _logger.Error("Exception: " + std::string(e.what()));
        }

        return packages_list;
    }

    std::string getRepoList()
    {
        std::string fpanel_repo_addr = _config->getValue("panel.http.server.address");
        if(_logger.debug) {
            fpanel_repo_addr = _config->getValue("panel.http.test.server.address");
        }
        std::string fpanel_repo_path = _config->getValue("panel.http.server.distro");
//        _logger.Info("Downloading: " + fpanel_repo_path + "repo.list");
        std::string packages_list;
        try {
            boost::asio::io_service io_service;
            // Get a list of endpoints corresponding to the server name.
            tcp::resolver resolver(io_service);
            tcp::resolver::query query(fpanel_repo_addr, "http");
            tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
            tcp::resolver::iterator end;
            // Try each endpoint until we successfully establish a connection.
            tcp::socket socket(io_service);
            boost::system::error_code error = boost::asio::error::host_not_found;
            while (error && endpoint_iterator != end) {
                socket.close();
                socket.connect(*endpoint_iterator++, error);
            }
            if (error)
                throw boost::system::system_error(error);
            // Form the request. We specify the "Connection: close" header so that the
            // server will close the socket after transmitting the response. This will
            // allow us to treat all data up until the EOF as the content.
            boost::asio::streambuf request;
            std::ostream request_stream(&request);
            request_stream << "GET " << fpanel_repo_path << "repo.list" << " HTTP/1.0\r\n";
            request_stream << "Host: " << fpanel_repo_addr  << "\r\n";
            request_stream << "Accept: */*\r\n";
            request_stream << "Connection: close\r\n\r\n";
            // Send the request.
            boost::asio::write(socket, request);
            // Read the response status line.
            boost::asio::streambuf response;
            boost::asio::read_until(socket, response, "\r\n");
            // Check that response is OK.
            std::istream response_stream(&response);
            std::string http_version;
            response_stream >> http_version;
            unsigned int status_code;
            response_stream >> status_code;
            std::string status_message;
            std::getline(response_stream, status_message);
            if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
                _logger.Error("Invalid response");
                return {};
            }
            if (status_code != 200) {
                _logger.Error("Response returned with status code " + std::to_string(status_code));
                return {};
            }

            // Read the response headers, which are terminated by a blank line.
            boost::asio::read_until(socket, response, "\r\n\r\n");

            std::string header;
            while (std::getline(response_stream, header) && header != "\r") {
            }

            if (response.size() > 0) {
                std::istreambuf_iterator<char> eosf;
                std::string s(std::istreambuf_iterator<char>(&response), eosf);
                packages_list += s;
            }
            // Read until EOF, writing data to output as we go.
            while (boost::asio::read(socket, response, boost::asio::transfer_at_least(1), error))
            {
                std::istreambuf_iterator<char> eoss;
                std::string s(std::istreambuf_iterator<char>(&response), eoss);
                packages_list += s;
            }

        }
        catch (std::exception &e) {
            _logger.Error("Exception: " + std::string(e.what()));
        }

        return packages_list;
    }

    const std::string md5_from_file(const std::string& path)
    {
        unsigned char result[MD5_DIGEST_LENGTH];
        boost::iostreams::mapped_file_source src(path);
        MD5((unsigned char*)src.data(), src.size(), result);

        std::ostringstream sout;
        sout<<std::hex<<std::setfill('0');
        for(auto c: result) sout<<std::setw(2)<<(int)c;

        return sout.str();
    }

    bool downloadFile(const std::string &package, const std::string &file_md5_sum)
    {
        std::string file_download_path = _config->getValue("panel.tmp.directory") + package;
        std::ofstream outFile(file_download_path, std::ofstream::out|std::ofstream::binary);
        std::string fpanel_repo_addr = _config->getValue("panel.http.server.address");
        std::string fpanel_repo_path = _config->getValue("panel.http.server.distro");
        boost::asio::io_service io_service;

        // Get a list of endpoints corresponding to the server name.
        tcp::resolver resolver(io_service);
        tcp::resolver::query query(fpanel_repo_addr, "http");
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
        tcp::resolver::iterator end;

        // Try each endpoint until we successfully establish a connection.
        tcp::socket socket(io_service);
        boost::system::error_code error = boost::asio::error::host_not_found;
        while (error && endpoint_iterator != end) {
            socket.close();
            socket.connect(*endpoint_iterator++, error);
        }

        if (error)
            throw boost::system::system_error(error);

        boost::asio::streambuf request;
        std::ostream request_stream(&request);

        request_stream << "GET " << fpanel_repo_path << package << " HTTP/1.0\r\n";
        request_stream << "Host: " << fpanel_repo_addr << "\r\n";
        request_stream << "Accept: */*\r\n";
        request_stream << "Connection: close\r\n\r\n";

        // Send the request.
        boost::asio::write(socket, request);

        // Read the response status line.
        boost::asio::streambuf response;
        boost::asio::read_until(socket, response, "\r\n");

        // Check that response is OK.
        std::istream response_stream(&response);
        std::string http_version;
        response_stream >> http_version;
        unsigned int status_code;
        response_stream >> status_code;
        std::string status_message;
        std::getline(response_stream, status_message);


        // Read the response headers, which are terminated by a blank line.
        boost::asio::read_until(socket, response, "\r\n\r\n");

        // Process the response headers.
        std::string header;
        while (std::getline(response_stream, header) && header != "\r") {
        }

        // Write whatever content we already have to output.
        if (response.size() > 0) {
            outFile << &response;
        }
        // Read until EOF, writing data to output as we go.
        while (boost::asio::read(socket, response, boost::asio::transfer_at_least(1), error)) {
            if (error) {
                _logger.Error("Read downloaded package error: " + std::string(error.message()));
                return false;
            }
            outFile << &response;
        }

        outFile.close();

        //Calculate MD5 hash
        std::string md5sum =  md5_from_file(file_download_path);
        if(file_md5_sum.compare(md5sum) != 0) {
            _helpers.Println("File download failed. System will trying again download as correctly. File is : " + package, _helpers.RED);
            _logger.Error("File download failed. System will trying again download as correctly. File is : " + package);
            return downloadFile(package, file_md5_sum);
        }

        return true;
    }

    void downloadPanelPackages(std::map<std::string, std::string> files)
    {
        // Main Packages
        std::string pkg_extension = _config->getValue("system.package.extension");

        for (std::map<std::string, std::string>::iterator it = files.begin(); it != files.end(); it++) {
            downloadFile(it->first + pkg_extension, it->second);
        }
        return;
    }

    void initRepoList(std::map<std::string, std::string> repo_list)
    {
        _helpers.Println("Repositories configuring...", _helpers.YELLOW);
        if(_config->getValue("system.os").compare("centos") == 0) {
            _helpers.RunCmdWithOutput("sed -i 's/enabled=1/enabled=0/g' /etc/yum/pluginconf.d/fastestmirror.conf");
            _helpers.RunCmd(_config->getValue("system.install.command") + " lsb-core.x86_64", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " bc", _logger.debug);
        }
        else if(_config->getValue("system.os").compare("debian") == 0) {
            _helpers.RunCmd("apt-get update", _logger.debug);
            _helpers.RunCmd("apt-get install multiarch-support -y", _logger.debug);
            _helpers.RunCmd("apt-get install zip unzip curl less dirmngr -y", _logger.debug);
            _helpers.RunCmd("apt-get install libc-client2007e -y", _logger.debug);
            _helpers.RunCmd("apt-get install apt-transport-https ca-certificates -y", _logger.debug);
            _helpers.Println("Certificates downloading...", _helpers.YELLOW);
            _helpers.RunCmd("apt-get clean", _logger.debug);
            _helpers.Println("System cleaning...", _helpers.YELLOW);
            _helpers.UpdateSystem(_logger.debug);
            _helpers.Println("System updating...", _helpers.YELLOW);
            _helpers.RunCmd(_config->getValue("system.install.command") + " lsb-release", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " aptitude", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " ftp", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " bc", _logger.debug);
            _helpers.RunCmd(_config->getValue("system.install.command") + " libnet-xwhois-perl", _logger.debug);
            _helpers.Println("Required libraries downloading....", _helpers.YELLOW);
            _helpers.RunCmd("apt-get install apt-transport-https -y", _logger.debug);
        }
        else if(_config->getValue("system.os").compare("ubuntu") == 0) {

            _helpers.Println("Disabling auto update...", _helpers.YELLOW);
            _logger.Info("Disabling auto update...");
            /**
             *
             */
             _helpers.RemoveFile("/etc/apt/apt.conf.d/20auto-upgrades");

             std::string auto_upgrade = "APT::Periodic::Update-Package-Lists \"0\";\n"
                                        "APT::Periodic::Download-Upgradeable-Packages \"0\";\n"
                                        "APT::Periodic::AutocleanInterval \"0\";\n"
                                        "APT::Periodic::Unattended-Upgrade \"0\";";
            _helpers.writeNewFile("/etc/apt/apt.conf.d/20auto-upgrades", auto_upgrade);
            _helpers.KillPids();
            _helpers.MilliSleep(1000);
            _helpers.KillPids();

            for (;;) {
                std::string apt_pids = _helpers.CmdOutput("ps aux | grep -i apt | grep -v grep | awk '{print $2}' | tr '\\n' ' '");
                if(apt_pids.empty()) {
//                    _helpers.Println("All pids killed...", _helpers.YELLOW);
                    break;
                }

                _helpers.RunCmd("rm -rf /var/lib/apt/lists/lock", false);
                _helpers.RunCmd("rm -rf /var/cache/apt/archives/lock", false);
                _helpers.RunCmd("rm -rf /var/lib/dpkg/lock*", false);

                _helpers.RunCmd("kill -9 " + apt_pids, false);
//                _helpers.Println("Killed Pids" + apt_pids, _helpers.YELLOW);
                _helpers.MilliSleep(1000);
            }

            _helpers.RunCmd("dpkg --configure -a", _logger.debug);

            _helpers.RunCmd("apt-get update", _logger.debug);
            _helpers.RunCmd("apt-get install -y multiarch-support", _logger.debug);
            _helpers.RunCmd("apt-get install -y multiarch-support", _logger.debug);
            // E: Unable to locate package multiarch-support
            _helpers.RunCmd(_config->getValue("system.install.command") + " --force-yes aptitude", _logger.debug);
            _helpers.RunCmd("apt-get install dirmngr -y", _logger.debug);
            _helpers.RunCmd("apt-get install libc-client2007e libltdl7 -y", _logger.debug);
            _helpers.RunCmd("apt-get install apt-transport-https ca-certificates -y", _logger.debug);
            _helpers.Println("Certificates downloading...", _helpers.YELLOW);
            _helpers.RunCmd("apt-get clean", _logger.debug);
            _helpers.Println("System cleaning...", _helpers.YELLOW);
            _helpers.UpdateSystem(_logger.debug);
            _helpers.Println("System updating...", _helpers.YELLOW);
            _helpers.RunCmd(_config->getValue("system.install.command") + " lsb-release ftp bc libnet-xwhois-perl", _logger.debug);
            _helpers.Println("Required libraries downloading....", _helpers.YELLOW);
            _helpers.RunCmd("apt-get install apt-transport-https -y", _logger.debug);
        }

        _helpers.Println("Repositories configuring...", _helpers.YELLOW);
        for( std::map<std::string, std::string>::iterator it = repo_list.begin(); it != repo_list.end(); ++it ) {
            std::string for_repo = it->first;
            std::string repo_addr = it->second;

            if(_config->getValue("system.os").compare("centos") == 0) {
                _helpers.RunCmdWithOutput("sed -i 's/enabled=1/enabled=0/g' /etc/yum/pluginconf.d/fastestmirror.conf");
                _helpers.writeNewFile("/etc/yum.repos.d/"+for_repo+".repo", repo_addr);
            }
            else if(_config->getValue("system.os").compare("debian") == 0) {
                _helpers.writeNewFile("/etc/apt/sources.list.d/"+for_repo+".list", repo_addr);
            }
            else if(_config->getValue("system.os").compare("ubuntu") == 0) {
                _helpers.writeNewFile("/etc/apt/sources.list.d/"+for_repo+".list", repo_addr);
            }
        }

        if((_config->getValue("system.os").compare("debian") == 0) || (_config->getValue("system.os").compare("ubuntu") == 0)) {
            _helpers.Println("PHP repositories configuring...", _helpers.YELLOW);
            _helpers.RunCmd("wget -qO - https://packages.sury.org/php/apt.gpg | apt-key add -", _logger.debug);
            _helpers.writeNewFile("/etc/apt/apt.conf.d/sury", "Acquire::https::packages.sury.org::Verify-Peer \"false\";");
            _helpers.UpdateSystem(_logger.debug);
        }
        else {
            _helpers.RunCmd("yum clean all", _logger.debug);
        }
    }
public:
    Download() {

    }

    ~Download() {

    }

    void SetConfig(Config *config)
    {
        _config = config;
    }

    void SetLogger(Logger logger)
    {
        _logger = logger;
    }

    void SetItems(Items *items)
    {
        _items = items;
    }

    void Start()
    {
        // First get package list

        std::map<std::string, std::string> files;
        std::string packages_response = getList();
        std::stringstream ssp(packages_response.c_str());
        std::string line;
        while(std::getline(ssp,line,'\n')) {
            if(!line.empty()) {
                std::vector<std::string> pkg_arr;
                _helpers.Split(pkg_arr, line, " ");
                files.insert({ pkg_arr[0], pkg_arr[2] });
            }
        }
        if((_config->getValue("system.os").compare("debian") == 0) || (_config->getValue("system.os").compare("ubuntu") == 0)) {
            std::string repo_response = getRepoList();
            std::map<std::string, std::string> repo_list;
            std::stringstream ssr(repo_response.c_str());
            while(std::getline(ssr,line,'\n')) {
                if(!line.empty()) {
                    std::vector<std::string> repo_arr;
                    _helpers.Split(repo_arr, line, " ");
                    std::string repo = line;
                    std::string repo_name = repo_arr[0] + " ";
                    _helpers.ReplaceWithString(repo, repo_name, "");
                    _helpers.trim(repo);
                    repo_list.insert({ repo_arr[0], repo });
                }
            }
            initRepoList(repo_list);
        }


        _helpers.Println("Packages downloading...", _helpers.CYAN);
        downloadPanelPackages(files);
    }
};

#endif //INSTALLER_DOWNLOAD_H
