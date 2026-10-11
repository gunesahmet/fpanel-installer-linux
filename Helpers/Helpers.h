#pragma once
#ifndef INSTALLER_HELPERS_H
#define INSTALLER_HELPERS_H

#include <iostream>
#include <sys/stat.h>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <sys/timeb.h>
#include <algorithm>
#include <sstream>
#include <string>
#include <cstring>
#include <initializer_list>
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <dirent.h>
#include <sys/types.h>
#include <vector>
#include <iomanip>
#include <netdb.h>
#include <arpa/inet.h>
#include <fstream>
#include <unistd.h>
#include <regex>
#include <unordered_map>
#include <sys/socket.h>
#include <thread>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/filesystem.hpp>
#include <boost/asio.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <random>

#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.
#include <boost/lexical_cast.hpp>
#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <iconv.h>
#include <stdarg.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <sys/utsname.h>
#include <sys/wait.h>

#define MAXCOM 1000 // max number of letters to be supported
#define MAXLIST 100 // max number of commands to be supported
#define DELIM "."
#define gotoxy(x,y) printf("\033[%d;%dH", (y), (x))

using namespace boost::asio;
using namespace boost::asio::ip;
class Helpers {
private:
public:
    enum colour {
        RESET = 0,
        RED = 1,
        GREEN = 2,
        YELLOW = 3,
        BLUE = 4,
        MAGENTA = 5,
        CYAN = 6,
        WHITE = 7,
        BLACK = 8,
        DARKGRAY = 9,
        BRIGHTWHITE = 10
    };

    std::vector<std::string> colors = {"\033[0m", "\033[31m", "\033[32m", "\033[33m", "\033[34m", "\033[35m", "\033[36m", "\033[37m", "\033[30m", "\033[90m", "\u001b[37;1m"};
    std::vector<std::string> bg_colors = {"\033[0m", "\033[41m", "\033[42m", "\033[43m", "\033[44m", "\033[45m", "\033[46m", "\033[47m", "\033[40m", "\033[100m", "\u001b[47;1m"};

    unsigned int RandomChar()
    {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, 255);
        return dis(gen);
    }

    std::string GenerateHex(const unsigned int len)
    {
        std::stringstream ss;
        for (auto i = 0; i < len; i++) {
            const auto rc = RandomChar();
            std::stringstream hexstream;
            hexstream << std::hex << rc;
            auto hex = hexstream.str();
            ss << (hex.length() < 2 ? '0' + hex : hex);
        }
        return ss.str();
    }

    std::string& ltrim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
    {
        str.erase(0, str.find_first_not_of(chars));
        return str;
    }

    std::string& rtrim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
    {
        str.erase(str.find_last_not_of(chars) + 1);
        return str;
    }

    std::string& trim(std::string& str, const std::string& chars = "\t\n\v\f\r ")
    {
        return ltrim(rtrim(str, chars), chars);
    }

    bool ReplaceWithString(std::string& str, std::string oldstr, std::string newstr)
    {
        boost::replace_all(str, oldstr, newstr);
        return true;
    }

    bool CmdOptionExists(char **begin, char **end, const std::string& option)
    {
        return find(begin, end, option) != end;
    }

    bool IsDirExist(const std::string& path)
    {
        struct stat info;
        if (stat(path.c_str(), &info) != 0) {
            return false;
        }
        return (info.st_mode & S_IFDIR) != 0;
    }

    long GetTimeStamp()
    {
        struct timeb timer_msec;
        long timestamp_msec; /* timestamp in millisecond. */
        if (!ftime(&timer_msec)) {
            timestamp_msec = ((long) timer_msec.time) * 1000ll + (long) timer_msec.millitm;
        }
        else {
            timestamp_msec = -1;
        }
        return timestamp_msec;
    }

    std::vector<std::string> strSplit(std::string strToSplit, char delimeter)
    {
        std::stringstream ss(strToSplit);
        std::string item;
        std::vector<std::string> splittedStrings;
        while (std::getline(ss, item, delimeter)) {
            splittedStrings.push_back(std::string(item));
        }
        return splittedStrings;
    }

    long TimestampFromString(std::string datetime)
    {
        char delimeter = '.';
        std::vector<std::string> datetime_arr = strSplit(datetime, delimeter);
        struct tm tm;
        time_t ts = 0;
        memset(&tm, 0, sizeof(tm));

        strptime(std::string(datetime_arr[0]).c_str(), "%Y-%m-%d %H:%M:%S", &tm);
        ts = mktime(&tm);
        std::string ts_str = std::to_string(ts);
        return std::atol(ts_str.c_str());
    }

    uint64_t Milliseconds()
    {
        return std::chrono::duration_cast<std::chrono::milliseconds>
                (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    }

    uint64_t Microseconds()
    {
        return std::chrono::duration_cast<std::chrono::microseconds>
                (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    }

    uint64_t Nanoseconds()
    {
        return std::chrono::duration_cast<std::chrono::nanoseconds>
                (std::chrono::high_resolution_clock::now().time_since_epoch()).count();
    }

    void NanoSleep(int nanosec)
    {
        if(nanosec == 0) {
            nanosec = 1;
        }

        struct timespec req = {0};
        req.tv_sec = 0;
        req.tv_nsec = nanosec * 1L; // 1 nanosecond
        nanosleep(&req, (struct timespec *)NULL);
    }

    void MicroSleep(int micro)
    {
        if(micro == 0) {
            micro = 1;
        }

        struct timespec req = {0};
        req.tv_sec = 0;
        req.tv_nsec = micro * 1000L; // 1 microsecond
        nanosleep(&req, (struct timespec *)NULL);
    }

    void MilliSleep(int millis)
    {
        if(millis == 0) {
            millis = 1;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(millis));
    }

    void SecSleep(int seconds)
    {
        if(seconds == 0) {
            seconds = 1;
        }
        seconds = seconds * 1000;
        std::this_thread::sleep_for(std::chrono::milliseconds(seconds));
    }

    void Println(int t, const int& color = RESET)
    {
        std::cout << colors[color] << t  << colors[RESET] <<std::endl;
    }

    void Println(char t, const int& color = RESET)
    {
        std::cout << colors[color] << t  << colors[RESET] <<std::endl;
    }

    void Println(std::string t, const int& color = RESET)
    {
        std::cout << colors[color] << t  << colors[RESET] <<std::endl;
    }

    void Print(int t, const int& color = RESET)
    {
        std::cout << colors[color] << t  << colors[RESET];
    }

    void Print(char t, const int& color = RESET)
    {
        std::cout << colors[color] << t  << colors[RESET];
    }

    void Print(std::string t, const int& color = RESET)
    {
        std::cout << colors[color] << t  << colors[RESET];
    }

    size_t Write(char* ptr, size_t size, size_t nmemb, void *f)
    {
        FILE *file = (FILE *)f;
        return fwrite(ptr, size, nmemb, file);
    };

    void ListFiles(const char *path, std::vector<std::string> *file_list)
    {
        struct dirent *entry;
        DIR *dir = opendir(path);

        if (dir == NULL) {
            return;
        }
        while ((entry = readdir(dir)) != NULL) {
            std::string fname = std::string(entry->d_name);
            if(!boost::filesystem::is_directory(fname.c_str())) {
                file_list->push_back(fname);
            }
        }
        closedir(dir);
    }

    void Split(std::vector<std::string> &tokens, std::string str, std::string delimiter)
    {
        boost::split(tokens, str, boost::is_any_of(delimiter.c_str()));
    }

    void RemoveLastChar(std::string &str)
    {
        trim(str, "\t\n\v\f\r ");
        str = str.substr(0, str.size()-1);
    }

    void ToLower(std::string &str)
    {
        boost::algorithm::to_lower(str);
    }

    const char* GenerateUniqueKey()
    {
        size_t length = 4;
        auto randchar = []() -> char {
            const char charset[] =
                    "0123456789"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "abcdefghijklmnopqrstuvwxyz";
            const size_t max_index = (sizeof(charset) - 1);
            return charset[ rand() % max_index ];
        };
        uint64_t current_nanosecond = Nanoseconds();
        std::string str(length,0);
        std::generate_n( str.begin(), length, randchar );
        return str.append(std::to_string(current_nanosecond)).c_str();
    }

    int RandomInteger(int min, int max)
    {
        return min + (rand() % static_cast<int>(max - min + 1));
    }

    int valid_digit(char *ip_str)
    {
        while (*ip_str) {
            if (*ip_str >= '0' && *ip_str <= '9')
                ++ip_str;
            else
                return 0;
        }
        return 1;
    }

    int IsV4(char *ip_str)
    {
        int i, num, dots = 0;
        char *ptr;

        if (ip_str == NULL)
            return 0;

        // See following link for strtok()
        // http://pubs.opengroup.org/onlinepubs/009695399/functions/strtok_r.html
        ptr = strtok(ip_str, DELIM);

        if (ptr == NULL)
            return 0;

        while (ptr) {

            /* after parsing string, it must contain only digits */
            if (!valid_digit(ptr))
                return 0;

            num = atoi(ptr);

            /* check for valid IP */
            if (num >= 0 && num <= 255) {
                /* parse remaining string */
                ptr = strtok(NULL, DELIM);
                if (ptr != NULL)
                    ++dots;
            } else
                return 0;
        }

        /* valid IP string must contain 3 dots */
        if (dots != 3)
            return 0;
        return 1;
    }

    std::string GetIpAddressFromUrl(const std::string &host)
    {

//    std::string rand_dns = dns_server_list[(rand()%(dns_server_list.size()-0 + 1) + 0)];
//    return CmdOutput(std::string("dig A @" + rand_dns + " " + url + " +noall +answer +multiline | grep ^" + url + " | head -1 | awk '{print $5}' | tr -d '\n'"));
        std::string ip;
        try {
            boost::asio::io_service io_service;
            ip::tcp::resolver resolver(io_service);
            ip::tcp::resolver::query query(host, "");
            ip::tcp::resolver::iterator iter = resolver.resolve(query);
            ip::tcp::resolver::iterator end; // End marker.
            while (iter != end) {
                ip::tcp::endpoint endpoint = *iter++;
                ip = std::string(endpoint.address().to_string());
                ReplaceWithString(ip, ":80","");
                char cstr[ip.size() + 1];
                strcpy(cstr, ip.c_str());	// or pass &s[0]


                if(IsV4(cstr)){
                    break;
                }
            }
        }
        catch (...) {
            return {};
        }

        return ip;
    }

    void MemoryUsage(double& vm_usage, double& resident_set)
    {
        vm_usage     = 0.0;
        resident_set = 0.0;

        // the two fields we want
        unsigned long vsize;
        long rss;
        {
            std::string ignore;
            std::ifstream ifs("/proc/self/stat", std::ios_base::in);
            ifs >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
                >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore >> ignore
                >> ignore >> ignore >> vsize >> rss;
        }

        long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
        vm_usage = vsize / 1024.0;
        resident_set = rss * page_size_kb;
    }

    bool IsFileExists(std::string name)
    {
        std::ifstream f(name.c_str());
        return f.good();
    }

    bool IsValidDomain(std::string &domain)
    {
        //^([a-z0-9-]{1,63}|[a-z0-9][a-z0-9-]{0,61}[a-z0-9]-)$
        std::regex rx("^(([a-z0-9-]{1,63}|[a-z0-9-][a-z0-9-]{0,61}[a-z0-9-]))$");
        if (std::regex_match(domain, rx)) {
            return true;
        }
        return false;
    }

    std::string CurrentDate()
    {
        struct timeval tmnow;
        struct tm *tm;
        char buffer[30], usec_buf[6];
        gettimeofday(&tmnow, NULL);
        tm = localtime(&tmnow.tv_sec);
        strftime(buffer,30,"%Y-%m-%d", tm);
//    strcat(buffer,".");
//    sprintf(usec_buf,"%d",(int)tmnow.tv_usec);
//    strcat(buffer,usec_buf);
        std::string str(buffer);
        return str;
    }

    std::string CurrentDateTime()
    {
        struct timeval tmnow;
        struct tm *tm;
        char buffer[30], usec_buf[6];
        gettimeofday(&tmnow, NULL);
        tm = localtime(&tmnow.tv_sec);
        strftime(buffer,30,"%Y-%m-%d %H:%M:%S", tm);
        strcat(buffer,".");
        sprintf(usec_buf,"%d",(int)tmnow.tv_usec);
        strcat(buffer,usec_buf);
        std::string str(buffer);
        return str;
    }

    std::string CurrentTime()
    {
        struct timeval tmnow;
        struct tm *tm;
        char buffer[30], usec_buf[6];
        gettimeofday(&tmnow, NULL);
        tm = localtime(&tmnow.tv_sec);
        strftime(buffer,30,"%H:%M:%S", tm);
        strcat(buffer,".");
        sprintf(usec_buf,"%d",(int)tmnow.tv_usec);
        strcat(buffer,usec_buf);
        std::string str(buffer);
        return str;
    }

    std::string CurrentDateTimeWithMs()
    {
        struct timeval tmnow;
        struct tm *tm;
        char buffer[30], usec_buf[6];
        gettimeofday(&tmnow, NULL);
        tm = localtime(&tmnow.tv_sec);
        strftime(buffer,30,"%Y-%m-%d %H:%M:%S", tm);
        strcat(buffer,".");
        sprintf(usec_buf,"%d",(int)tmnow.tv_usec);
        strcat(buffer,usec_buf);
        std::string str(buffer);
        return str;
    }

    std::string CmdOutput(std::string cmd)
    {
        std::string data;
        FILE * stream;
        const int max_buffer = 8192;
        char buffer[max_buffer];
        // cmd.append(" 2>&1");

        stream = popen(cmd.c_str(), "r");
        if (stream) {
            while (!feof(stream)) {
                if (fgets(buffer, max_buffer, stream) != NULL) {
                    data.append(buffer);
                }
            }
            pclose(stream);
        }
        return data;
    }

    int LastModifiedOfFile(std::string &file_path)
    {
        std::string modified_time = CmdOutput(file_path + " | grep Modify | sed -e 's/Modify\\:\\s//g'");
        trim(modified_time);
        return int(TimestampFromString(modified_time));
    }

    bool MoveFile(std::string old_path, std::string new_path)
    {
        if(std::rename(old_path.c_str(), new_path.c_str()) < 0) {
            return false;
        }
        return true;
    }

    long GetFileSize(std::string filename)
    {
        struct stat stat_buf;
        int rc = stat(filename.c_str(), &stat_buf);
        return rc == 0 ? stat_buf.st_size : -1;
    }

    bool CopyFile(std::string old_path, std::string new_path)
    {
        std::ifstream  src(old_path.c_str(), std::ios::binary);
        std::ofstream  dst(new_path.c_str(),   std::ios::binary);
        dst << src.rdbuf();
    }

    bool RemoveFile(std::string filename)
    {
        bool rstatus = false;
        int status = remove(filename.c_str());
        if(status == 0) {
            rstatus = true;
        }
        return rstatus;
    }

    bool CreateDirectory(std::string path)
    {
        return boost::filesystem::create_directories(path);
    }

    std::string GenerateUuid()
    {
        boost::uuids::uuid uuid = boost::uuids::random_generator()();
        const std::string tmp = boost::lexical_cast<std::string>(uuid);
        return std::string(tmp);
    }

    std::string GetLocalIpAddress()
    {
        struct ifaddrs * ifAddrStruct=NULL;
        struct ifaddrs * ifa=NULL;
        void * tmpAddrPtr=NULL;

        getifaddrs(&ifAddrStruct);
        char addressBuffer[INET_ADDRSTRLEN];

        for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
            if (ifa ->ifa_addr->sa_family==AF_INET) { // check it is IP4
                // is a valid IP4 Address
                tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
                inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
//                printf("'%s': %s\n", ifa->ifa_name, addressBuffer);
            }
        }
        if (ifAddrStruct!=NULL)
            freeifaddrs(ifAddrStruct);//remember to free ifAddrStruct
        return std::string(addressBuffer);
    }

    bool isAlpha(std::string &str) {
        if(!isalpha(str[0]) || !isspace(str[0])) {
            return true;
        }
        return false;
    }

    bool is_number(const std::string& s)
    {
        std::string::const_iterator it = s.begin();
        while (it != s.end() && std::isdigit(*it)) ++it;
        return !s.empty() && it == s.end();
    }

    std::vector<std::string> readFile(std::string file_path)
    {
        std::ifstream file_content(file_path.c_str());
        std::string line;
        std::vector<std::string> line_result;

        while (getline(file_content, line)) {
            if(!line.empty()) {
                line_result.push_back(line);
            }
        }
        file_content.close();
        return line_result;
    }

    bool isContain(std::string str, std::string search)
    {
//        Println("Comparing " + str + " with " + search, RESET);
        if (str.find(search) == std::string::npos) {
            return false;
        }
        return true;
    }

    bool StartsWith(const char *a, const char *b) {
        if (strncmp(a, b, strlen(b)) == 0) return 1;
        return 0;
    }

    bool writeNewFile(std::string file_path, std::string str)
    {
        if(IsFileExists(file_path)) {
            RemoveFile(file_path);
        }
        std::ofstream outfile;
        outfile.open(file_path, std::ios::in|std::ios::out|std::ios::app);
        outfile << str << std::endl;
        outfile.close();
    }

    void getSizeOfTerminal(int *columns, int *rows) {
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        *columns = w.ws_col;
        *rows = w.ws_row;
    }

    void ClearScreen()
    {
        std::cout << "\033[2J\033[1;1H";
    }

    std::string GetHostname()
    {
        return CmdOutput("hostname -f");
    }

    void InitDService(std::string service, std::string process)
    {
        pid_t pid = fork();
        if (pid == 0) { // child process
            execl(std::string("/etc/init.d/" + service).c_str(), std::string("/etc/init.d/" + service).c_str(), process.c_str(), (char*) 0);
            perror("execl"); // only get here when exec fails
            exit(0);
        } else if (pid > 0) { // parent process
            wait(NULL); // wait for child to finish
        } else { // error
            perror("fork");
        }
    }

    void installMessage(int x, int y, const std::string& text)
    {
//        int columns, rows;
//        getSizeOfTerminal(&columns, &rows);
//        std::string head = std::string(columns-2, ' ');
//        printf("\033[%d;%dH%s\n", x, y, head.c_str());
////        printf("\033[%d;%dH%s\n", x, y, text.c_str());
//        std::cout << colors[CYAN];
//        printf("\033[%d;%dH%s\n", x, y, text.c_str());
//        std::cout << colors[RESET];
    }

    bool IfStringContains(std::string subject, std::string need)
    {
        if (subject.find(need) != std::string::npos) {
            return true;
        }
        return false;
    }

    void RunCmd(std::string cmd, bool debug)
    {
        if(!debug) {
            cmd = cmd + " >/dev/null 2>&1";
        }
        system(cmd.c_str());
    }

    void KillPids()
    {
        std::string apt_pids = CmdOutput("ps aux | grep -i apt | grep -v grep | awk '{print $2}' | tr '\\n' ' '");
        if(!apt_pids.empty()) {
            RunCmd("rm /var/lib/apt/lists/lock", false);
            RunCmd("rm /var/cache/apt/archives/lock", false);
            RunCmd("rm /var/lib/dpkg/lock*", false);
            RunCmd("kill -9 " + apt_pids, false);
        }
    }

    void RunCmdWithOutput(std::string cmd)
    {
        system(cmd.c_str());
    }

    void CreateFileSymLink(std::string source, std::string target)
    {
        try {
            RunCmd("ln -s " + source + " " + target, false);
        }
        catch (...) {

        }
    }

    void CreateDirectorySymLink(std::string source, std::string target)
    {
        try {
            boost::filesystem::create_directory_symlink(source, target);
        }
        catch (...) {

        }
    }

    void UpdateSystem(bool debug)
    {
        std::string cmd = "#!/bin/bash\n"
                          "RESULT=`apt update 2>&1 1>/dev/null | sed -ne 's/.*NO_PUBKEY //p'`\n"
                          "while read key\n"
                          "do\n"
                          "        if ! [[ ${keys[*]} =~ \"$key\" ]]; then\n"
                          "                apt-key adv --keyserver keyserver.ubuntu.com --recv-keys \"$key\"\n"
                          "                keys+=(\"$key\")\n"
                          "        fi\n"
                          "done <<< $RESULT\n"
                          "apt update 2>&1 1>/dev/null";
        writeNewFile("/tmp/.update.sh", cmd);
        RunCmdWithOutput("chmod 777 /tmp/.update.sh");
        if(debug) {
            RunCmdWithOutput("/tmp/.update.sh");
        }
        else {
            RunCmdWithOutput("/tmp/.update.sh > /dev/null 2>&1");
        }
        RemoveFile("/tmp/.update.sh");
    }
};


#endif //INSTALLER_HELPERS_H
