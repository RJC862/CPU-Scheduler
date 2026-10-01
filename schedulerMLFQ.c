#include <stdint.h>
#include <stdlib.h>
#include "scheduler.h"
#include "job.h"
#include "linked_list.h"

typedef int job_priority_t;

// MLFQ scheduler info
typedef struct
{
    list_t *queues[4];
    job_t *running_job;
    job_priority_t running_job_priority;
    scheduler_info_create_fn create;
    scheduler_info_destroy_fn destroy;
    schedule_job_fn scheduleJob;
    complete_job_fn completeJob;
} scheduler_MLFQ_t;

void schedulerMLFQRunHighestPriorityJob(scheduler_MLFQ_t *info);

// Creates and returns scheduler specific info
void *schedulerMLFQCreate()
{
    scheduler_MLFQ_t *info = malloc(sizeof(scheduler_MLFQ_t));
    if (info == NULL)
    {
        return NULL;
    }

    for (int i = 0; i < 4; i++)
    {
        info->queues[i] = list_create(NULL);
    }
    info->running_job = NULL;
    info->running_job_priority = 0;
    info->create = schedulerMLFQCreate;
    info->destroy = schedulerMLFQDestroy;
    info->scheduleJob = schedulerMLFQScheduleJob;
    info->completeJob = schedulerMLFQCompleteJob;

    return info;
}

// Destroys scheduler specific info
void schedulerMLFQDestroy(void *schedulerInfo)
{
    scheduler_MLFQ_t *info = (scheduler_MLFQ_t *)schedulerInfo;
    for (int i = 0; i < 4; i++)
    {
        if (info->queues[i] != NULL)
        {
            free(info->queues[i]);
        }
    }
    free(info);
}

// Called to schedule a new job in the queue
// schedulerInfo - scheduler specific info from create function
// scheduler - used to call schedulerScheduleNextCompletion and schedulerCancelNextCompletion
// job - new job being added to the queue
// currentTime - the current simulated time
void schedulerMLFQScheduleJob(void *schedulerInfo, scheduler_t *scheduler, job_t *job, uint64_t currentTime)
{
    if (schedulerInfo == NULL)
    {
        return;
    }
    scheduler_MLFQ_t *info = (scheduler_MLFQ_t *)schedulerInfo;

    uint64_t job_time = (jobGetRemainingTime(job) > 0) ? 1 : 0;

    if (info->running_job == NULL)
    {
        info->running_job = job;
        info->running_job_priority = 0;

        schedulerScheduleNextCompletion(scheduler, currentTime + job_time);
    }
    else if (info->running_job_priority != 0)
    {
        schedulerCancelNextCompletion(scheduler); // preempt

        list_tail_insert(info->queues[info->running_job_priority], info->running_job);

        info->running_job = job;
        info->running_job_priority = 0;

        schedulerScheduleNextCompletion(scheduler, currentTime + job_time);
    }
    else
    {
        list_insert(info->queues[0], job);
    }
}

// Called to complete a job in response to an earlier call to schedulerScheduleNextCompletion
// schedulerInfo - scheduler specific info from create function
// scheduler - used to call schedulerScheduleNextCompletion and schedulerCancelNextCompletion
// currentTime - the current simulated time
// Returns the job that is being completed
job_t *schedulerMLFQCompleteJob(void *schedulerInfo, scheduler_t *scheduler, uint64_t currentTime)
{

    scheduler_MLFQ_t *info = (scheduler_MLFQ_t *)schedulerInfo;

    job_t *running_job = info->running_job;
    int job_priority = info->running_job_priority;

    if (running_job == NULL)
    {
        return NULL;
    }

    uint64_t remaining_time = jobGetRemainingTime(running_job);

    if (remaining_time > 0)
    {
        remaining_time -= 1;
        jobSetRemainingTime(running_job, remaining_time);
    }

    if (remaining_time > 0)
    {
        // demote one level; the bottom level stays where it is
        int next = job_priority;
        if (next < 4 - 1)
        {
            next++;
        }
        list_insert(info->queues[next], running_job);
    }

    schedulerMLFQRunHighestPriorityJob(info);

    if (info->running_job != NULL)
    {
        uint64_t job_time = (jobGetRemainingTime(info->running_job) > 0) ? 1 : 0;
        schedulerScheduleNextCompletion(scheduler, currentTime + job_time);
    }

    return (remaining_time == 0) ? running_job : NULL;
}

void schedulerMLFQRunHighestPriorityJob(scheduler_MLFQ_t *info)
{

    if (info == NULL)
    {
        return;
    }

    for (int i = 0; i < 4; i++)
    {
        list_node_t *tail = list_tail(info->queues[i]);
        if (tail != NULL)
        {
            job_t *job = tail->data;
            list_remove(info->queues[i], tail);
            info->running_job = job;
            return;
        }
    }

    info->running_job = NULL;
}