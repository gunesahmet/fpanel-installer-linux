#pragma once
#ifndef INSTALLER_PROGRESSBAR_H
#define INSTALLER_PROGRESSBAR_H
#include <iostream>
#include <string>
#include <sys/ioctl.h>
#include <Helpers/Helpers.h>
#include <boost/thread.hpp>
#include <Logger/Logger.h>
#include <tsl/ordered_map.h>
#include <Jobs/ProgressJob.h>
#include <mutex>

namespace FPanel {

    class ProgressBar {
    private:
        unsigned int total;
        unsigned int progress;
        unsigned int colwidth;
        std::string message;
        Logger logger;
        ProgressJob *_progress_job;
    public:
        bool _status;
        Helpers _helpers;

        ProgressBar(unsigned int total_, std::string message_) : total(total_), progress(0), message(message_)
        {
            _status = false;
            _progress_job = new ProgressJob();
            struct winsize w;
            ioctl(0, TIOCGWINSZ, &w);
            colwidth = w.ws_col;
            show();
        }

        ~ProgressBar()
        {
            progress = total;
//            show();
            std::cout << std::endl << std::flush;
        }

        void update(unsigned int from_, unsigned int to_)
        {
            std::map<unsigned int, unsigned int> job = std::map<unsigned int, unsigned int>();
            job.insert({from_, to_});
            _progress_job->addJob(job);
        }

        void startUpdate()
        {
            while(true) {
                _status = startUpdateWorker();
                if(_status) {
                    break;
                }
                _helpers.MilliSleep(250);
            }
        }

        bool startUpdateWorker()
        {
            std::map<unsigned int, unsigned int> job = _progress_job->getJob();
            if(job.size() > 0) {
                unsigned int from_ = job.find(0)->first;
                unsigned int to_ = job.find(0)->second;
                unsigned int diff = to_ - from_;
                for(int i = 0; i < diff; i++) {
                    progress = from_ + i -1;
                    show();
                    _helpers.MilliSleep(1550);
//                    _helpers.MilliSleep(150);
                    if(progress == 100)  {
                        return true;
                        break;
                    }
                }
            }
            else
                _helpers.MilliSleep(1000);
            return false;
        }

        void show() {
            std::string prefix = message
                                 + std::string{" "} + std::to_string(progress)
                                 + std::string{"/"} + std::to_string(total);

            // 4 Additional spaces for space, braces and carriage return in " [...]\r"
            int prefix_length = prefix.length() + 4;
            int progress_length = colwidth - prefix_length;
            int completed = progress * progress_length / total;
            int remaining = progress_length - completed;

            std::string hashes = std::string(completed, '#');
            std::string period = std::string(progress_length - completed, '.');

            std::cout
                    << _helpers.colors[_helpers.RED] << message << " "
                    << _helpers.colors[_helpers.YELLOW] << progress
                    << _helpers.colors[_helpers.RESET] << "/"
                    << _helpers.colors[_helpers.MAGENTA] << total
                    << _helpers.colors[_helpers.WHITE] << _helpers.bg_colors[_helpers.WHITE] << " " << hashes << _helpers.bg_colors[_helpers.RESET]
                    << _helpers.colors[_helpers.DARKGRAY] << _helpers.bg_colors[_helpers.DARKGRAY] << period << _helpers.bg_colors[_helpers.RESET]
                    << _helpers.colors[_helpers.GREEN] << "\r"
                    << _helpers.colors[_helpers.RESET] << std::flush;
        }
    };
}
#endif //INSTALLER_PROGRESSBAR_H
