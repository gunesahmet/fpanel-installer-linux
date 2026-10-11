#include <boost/program_options.hpp>
#include <Logger/Logger.h>
#include <Config/Config.h>
#include <Install/Install.h>

#include <execinfo.h>
//#include <signal.h>
//#include <stdlib.h>
//#include <unistd.h>

Logger logger;
Config *config;
Helpers _helpers;
bool debug = false;
bool cloud_main = false;
bool cloud_client = false;
bool fast = false;
std::string main_ip = "127.0.0.1";

std::string log_file = "/var/log/fpanel/installer.log";

std::map<int, std::string> words = std::map<int, std::string>();

void handler(int sig) {
    void *array[10];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 10);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}

void initLogs() {
    plog::init(plog::info, log_file.c_str(), 104857600);              // Initialize the default logger instance.
}

boost::program_options::variables_map commandLine(int argc, char **argv) {
    boost::program_options::variables_map vm;

    try {
        bool t_given = false;

        boost::program_options::options_description desc("Allowed options");
        desc.add_options()
                ("help", boost::program_options::bool_switch(&t_given), "Print usage message")
                ("verbose", boost::program_options::bool_switch(&t_given), "Verbose mode")
                ("standalone", boost::program_options::bool_switch(&t_given), "Standalone mode")
                ("cloud-main", boost::program_options::bool_switch(&t_given), "Cloud Main Mode")
                ("cloud", boost::program_options::bool_switch(&t_given), "Cloud client mode")
                ("fast", boost::program_options::bool_switch(&t_given), "Fast Install Mode (Default false)")
                ("main-ip", boost::program_options::value<std::string>(), "Cloud Main Server Address")
                ("apache-nginx", boost::program_options::bool_switch(&t_given),
                 "Select Apache Http Server With Nginx Reverse Proxy (Default false)")
                ("apache", boost::program_options::bool_switch(&t_given), "Select Apache Http Server (Default true)")
                ("perl", boost::program_options::bool_switch(&t_given), "Install Perl (Default true)")
                ("python", boost::program_options::bool_switch(&t_given), "Install Python (Default true)")
                ("php56", boost::program_options::bool_switch(&t_given), "Install PHP 5.6 (Default false)")
                ("php72", boost::program_options::bool_switch(&t_given), "Install PHP 7.2 (Default true)")
                ("php74", boost::program_options::bool_switch(&t_given), "Install PHP 7.4 (Default false)")
                ("mysql", boost::program_options::bool_switch(&t_given), "Install MySQL Database Server (Default true)")
                ("mariadb", boost::program_options::bool_switch(&t_given),
                 "Install MariaDB Database Server (Default false)")
                ("postgresql", boost::program_options::bool_switch(&t_given),
                 "Install PostgreSQL Database Server (Default false)")
                ("bind", boost::program_options::bool_switch(&t_given), "Install BIND DNS Server (Default true)")
                ("postfix", boost::program_options::bool_switch(&t_given), "Install Postfix MTA Server (Default true)")
                ("proftpd", boost::program_options::bool_switch(&t_given), "Install Proftpd FTP Server (Default true)")
                ("dovecot", boost::program_options::bool_switch(&t_given), "Install Dovecot Service (Default true)")
                ("spamassassin", boost::program_options::bool_switch(&t_given),
                 "Install Spamassassin Service (Default false)")
                ("mailman", boost::program_options::bool_switch(&t_given), "Install Mailman Service (Default false)")
                ("roundcube", boost::program_options::bool_switch(&t_given), "Install Roundcube Webmail (Default true)")
                ("phpmyadmin", boost::program_options::bool_switch(&t_given), "Install PHP MyAdmin (Default true)")
                ("awstats", boost::program_options::bool_switch(&t_given),
                 "Install Awstats Log Analyzer (Default true)")
                ("webalizer", boost::program_options::bool_switch(&t_given),
                 "Install Webalizer Log Analyzer (Default true)")
                ("hostname", boost::program_options::value<std::string>(), "Hostname");

        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
        // ./installer --fast --standalone --apache --php72 --perl --python --mysql --bind --postgresql --postfix --proftpd --dovecot --spamassassin --roundcube --phpmyadmin --awstats --webalizer --hostname=test.debian.fpanel.net
        if (vm.count("help") && (vm["help"].as<bool>())) {
            std::cout << "FPanel Installer Ver 1.0.2, for Linux (amd64)" << std::endl;
            std::cout << "Copyright (c) 2021 Kovengo Sotware Limited. All rights reserved." << std::endl;
            std::cout << "" << std::endl;
            std::cout
                    << "FPanel is a registered trademark of Kovengo Software Limited. Other names may be trademarks of their respective owners."
                    << std::endl;
            std::cout << "" << std::endl;
            std::cout << "Allowed options:\n"
                         "  --help                Print usage message\n"
                         "  --fast                Fast Install Mode (Default false)\n"
                         "  --standalone          Standalone Install Mode (Default true)\n"
                         "  --cloud-main          Cloud Main Install Mode (Default false)\n"
                         "  --cloud               Cloud Client Install Mode (Default false)\n"
                         "  --main-ip             Cloud Main Server IP\n"
                         "  --apache              Select Apache Http Server (Default true)\n"
                         "  --apache-nginx        Select Apache Http Server with Nginx Reverse Proxy Support (Default false)\n"
                         "  --perl                Install Perl (Default true)\n"
                         "  --python              Install Python (Default true)\n"
                         "  --php56               Install PHP 5.6 (Default false)\n"
                         "  --php72               Install PHP 7.2 (Default true)\n"
                         "  --php74               Install PHP 7.4 (Default false)\n"
                         "  --mysql               Install MySQL Database Server (Default true)\n"
                         "  --mariadb             Install MariaDB Database Server (Default false)\n"
                         "  --postgresql          Install PostgreSQL Database Server (Default false)\n"
                         "  --bind                Install BIND DNS Server (Default true)\n"
                         "  --postfix             Install Postfix MTA Server (Default true)\n"
                         "  --proftpd             Install Proftpd FTP Server (Default true)\n"
                         "  --dovecot             Install Dovecot Service (Default true)\n"
                         "  --spamassassin        Install Spamassassin Service (Default false)\n"
                         "  --mailman             Install Mailman Service (Default false)\n"
                         "  --roundcube           Install Roundcube Webmail (Default true)\n"
                         "  --phpmyadmin          Install PHP MyAdmin (Default true)\n"
                         "  --awstats             Install Awstats Log Analyzer (Default true)\n"
                         "  --webalizer           Install Webalizer Log Analyzer (Default true)\n"
                         "  --hostname=<value>    Hostname" << std::endl;
            exit(1);
        }
    }
    catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        exit(1);
    }
    return vm;
}

std::string detectSystem() {
    _helpers.Println("Please wait...", _helpers.CYAN);
    struct utsname name;
    if (uname(&name)) {
        exit(-1);
    }
    std::string system_type;
    std::string version = std::string(name.version);
    _helpers.ToLower(version);
    if (_helpers.IfStringContains(version, "debian")) {
        system_type = "debian";
    } else {
        system_type = "centos";
    }
    if ((system_type.compare("debian") == 0) || system_type.compare("ubuntu") == 0) {
        _helpers.RunCmd("apt-get update", logger.debug);
        _helpers.RunCmd("apt-get install -y lsb-release", logger.debug);
    } else {
        _helpers.RunCmd("yum clean all", logger.debug);
        _helpers.MilliSleep(1000);
        _helpers.RunCmd("dnf --disablerepo '*' --enablerepo=extras swap centos-linux-repos centos-stream-repos -y",
                        logger.debug);
//        _helpers.RunCmd("dnf distro-sync -y", true);
        _helpers.RunCmd("yum install -y redhat-lsb-core.x86_64", logger.debug);
        _helpers.RunCmd("yum install -y redhat-lsb-core.x86_64", logger.debug);
        _helpers.RunCmd("yum install -y lsb-core", logger.debug);
    }
    std::string output;
    if (_helpers.IsFileExists("/usr/bin/lsb_release")) {
        output = _helpers.CmdOutput("lsb_release -is");
        _helpers.ToLower(output);
    } else {
        _helpers.Println("OS Cant detected", _helpers.RED);
        exit(0);
    }
    return output;
}

void isFast(boost::program_options::variables_map vm) {
    if (fast) {
        config->setValue("apache", " ");
        config->setValue("apache-nginx", " ");
        config->setValue("perl", " ");
        config->setValue("python", " ");
        config->setValue("php56", " ");
        config->setValue("php72", " ");
        config->setValue("php74", " ");
        config->setValue("mysql", " ");
        config->setValue("proftpd", " ");
        config->setValue("mariadb", " ");
        config->setValue("postgresql", " ");
        config->setValue("bind", " ");
        config->setValue("postfix", " ");
        config->setValue("dovecot", " ");
        config->setValue("spamassassin", " ");
        config->setValue("mailman", " ");
        config->setValue("roundcube", " ");
        config->setValue("dbmyadmin", " ");
        config->setValue("awstats", " ");
        config->setValue("webalizer", " ");
        config->setValue("php", " ");
        if (!vm.count("hostname")) {
            _helpers.Println("You must set a hostname for fast installation.", _helpers.RED);
            exit(0);
        }
    }

    if (vm.count("apache") && (vm["apache"].as<bool>())) {
        config->setValue("apache", "X");
        config->setValue("apache-nginx", " ");
    }

    if (vm.count("apache-nginx") && (vm["apache-nginx"].as<bool>())) {
        config->setValue("apache-nginx", "X");
        config->setValue("apache", " ");
    }

    if (vm.count("perl") && (vm["perl"].as<bool>())) {
        config->setValue("perl", "X");
    }
    if (vm.count("python") && (vm["python"].as<bool>())) {
        config->setValue("python", "X");
    }

    if (vm.count("php56") && (vm["php56"].as<bool>())) {
        config->setValue("php56", "X");
        config->setValue("php", "X");
        if (vm.count("apache") && (vm["apache"].as<bool>())) {
            config->setValue("php72", " ");
            config->setValue("php74", " ");
        }
    }
    if (vm.count("php72") && (vm["php72"].as<bool>())) {
        config->setValue("php72", "X");
        config->setValue("php", "X");
        if (vm.count("apache") && (vm["apache"].as<bool>())) {
            config->setValue("php56", " ");
            config->setValue("php74", " ");
        }
    }
    if (vm.count("php74") && (vm["php74"].as<bool>())) {
        config->setValue("php74", "X");
        config->setValue("php", "X");
        if (vm.count("apache") && (vm["apache"].as<bool>())) {
            config->setValue("php56", " ");
            config->setValue("php72", " ");
        }
    }

    if (vm.count("mysql") && (vm["mysql"].as<bool>())) {
        config->setValue("mysql", "X");
        config->setValue("mariadb", " ");
    }

    if (vm.count("mariadb") && (vm["mariadb"].as<bool>())) {
        config->setValue("mariadb", "X");
        config->setValue("mysql", " ");
    }

    if (vm.count("postgresql") && (vm["postgresql"].as<bool>())) {
        config->setValue("postgresql", "X");
    }

    if (vm.count("bind") && (vm["bind"].as<bool>())) {
        config->setValue("bind", "X");
    }
    if (vm.count("proftpd") && (vm["proftpd"].as<bool>())) {
        config->setValue("proftpd", "X");
    }

    if (vm.count("postfix") && (vm["postfix"].as<bool>())) {
        config->setValue("postfix", "X");
        config->setValue("dovecot", "X");
    }
    if (vm.count("dovecot") && (vm["dovecot"].as<bool>())) {
        config->setValue("dovecot", "X");
        config->setValue("postfix", "X");
    }
    if (vm.count("spamassassin") && (vm["spamassassin"].as<bool>())) {
        config->setValue("postfix", "X");
        config->setValue("dovecot", "X");
        config->setValue("spamassassin", "X");
    }
    if (vm.count("mailman") && (vm["mailman"].as<bool>())) {
        config->setValue("postfix", "X");
        config->setValue("dovecot", "X");
        config->setValue("mailman", "X");
    }
    if (vm.count("roundcube") && (vm["roundcube"].as<bool>())) {
        config->setValue("postfix", "X");
        config->setValue("dovecot", "X");
        config->setValue("roundcube", "X");
    }

    if (vm.count("phpmyadmin") && (vm["phpmyadmin"].as<bool>())) {
        if (vm.count("mariadb") && (vm["mariadb"].as<bool>())) {
            config->setValue("mysql", "X");
        }
        config->setValue("dbmyadmin", "X");
    }

    if (vm.count("awstats") && (vm["awstats"].as<bool>())) {
        config->setValue("awstats", "X");
    }
    if (vm.count("webalizer") && (vm["webalizer"].as<bool>())) {
        config->setValue("webalizer", "X");
    }

    if (vm.count("hostname")) {
        config->setValue("hostname", std::string(vm["hostname"].as<std::string>()));
    }
}

void isCloud(boost::program_options::variables_map vm) {
    if (vm.count("cloud") && (vm["cloud"].as<bool>())) {
        cloud_client = true;
        cloud_main = false;
        if (vm.count("main-ip") && !vm["main-ip"].as<std::string>().empty()) {
            config->setValue("main-ip", vm["main-ip"].as<std::string>());
            main_ip = vm["main-ip"].as<std::string>();
        } else {
            _helpers.Println("Please enter a cloud main server ip for fast installation", _helpers.RED);
            exit(1);
        }
    } else if (vm.count("cloud-main") && (vm["cloud-main"].as<bool>())) {
        cloud_main = true;
        cloud_client = false;
        config->setValue("apache", " ");
        config->setValue("apache-nginx", " ");
        config->setValue("perl", " ");
        config->setValue("python", " ");
        config->setValue("php56", " ");
        config->setValue("php72", " ");
        config->setValue("php74", " ");
        config->setValue("mysql", "X");
        config->setValue("proftpd", " ");
        config->setValue("mariadb", " ");
        config->setValue("postgresql", " ");
        config->setValue("bind", " ");
        config->setValue("postfix", " ");
        config->setValue("dovecot", " ");
        config->setValue("spamassassin", " ");
        config->setValue("mailman", " ");
        config->setValue("roundcube", " ");
        config->setValue("dbmyadmin", " ");
        config->setValue("awstats", " ");
        config->setValue("webalizer", " ");
        config->setValue("php", " ");
    }
}

void Start(int argc, char **argv) {
    boost::program_options::variables_map vm = commandLine(argc, argv);
    std::string system_type = detectSystem();
    config = new Config();
    // Initialize Logger
    initLogs();

    if (vm.count("verbose") && (vm["verbose"].as<bool>())) {
        debug = true;
    }

    logger.debug = debug;
    config->setLogger(logger);
    config->init(system_type);
    logger.Info("System install starting...");
    if (getuid() != 0) {
        logger.Error("Please run as root user.");
        exit(0);
    }

    if (vm.count("fast") && (vm["fast"].as<bool>())) {
        fast = true;
    }

    isFast(vm);
    isCloud(vm);

    Install install;
    install.SetConfig(config);
    install.SetCloudMain(cloud_main);
    install.SetCloudClient(cloud_client);
    install.SetLogger(logger);
    install.SetMainIp(main_ip);
    install.Start(fast);
}


int main(int argc, char **argv) {
    signal(SIGSEGV, handler);
    Start(argc, argv);
    return 0;
}
