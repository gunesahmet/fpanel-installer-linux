#pragma once
#ifndef INSTALLER_PROGRESSJOB_H
#define INSTALLER_PROGRESSJOB_H
#include <iostream>
#include <mutex>
#include <tuple>
#include <map>
#include <tsl/ordered_map.h>

class ProgressJob {
    tsl::ordered_map<int, std::map<unsigned int, unsigned int>> queue_jobs;
    std::mutex queue_job_mutex;
public:
    ProgressJob() {
        queue_jobs = tsl::ordered_map<int, std::map<unsigned int, unsigned int>>();
    }

    void addJob(std::map<unsigned int, unsigned int> job)
    {
        std::lock_guard<std::mutex> lockGuard(queue_job_mutex);
        int job_id = 1;
        if(queue_jobs.rbegin() != queue_jobs.rend()) {
            if(queue_jobs.size() > 0) {
                int last_job_id = queue_jobs.rbegin()->first;
                job_id = last_job_id + 1;
            }
        }
        queue_jobs.insert({ job_id , job });
    }

    std::map<unsigned int, unsigned int> getJob()
    {
        std::lock_guard<std::mutex> lockGuard(queue_job_mutex);
        std::map<unsigned int, unsigned int> job;
        if(queue_jobs.rbegin() != queue_jobs.rend()) {
            if(queue_jobs.size() > 0) {
                int job_id = queue_jobs.begin()->first;
                job = queue_jobs.begin()->second;
                queue_jobs.erase(job_id);
            }
        }
        return job;
    }
};

#endif //INSTALLER_PROGRESSJOB_H
