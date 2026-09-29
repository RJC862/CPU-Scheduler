#include <stdint.h>
#include <stdlib.h>
#include "scheduler.h"
#include "job.h"
#include "linked_list.h"

// SJF scheduler info
typedef struct
{
    list_t *queue;
    job_t *running_job;
    scheduler_info_create_fn create;
    scheduler_info_destroy_fn destroy;
    schedule_job_fn scheduleJob;
    complete_job_fn completeJob;

} scheduler_SJF_t;

// Creates and returns scheduler specific info
void *schedulerSJFCreate()
{
    scheduler_SJF_t *info = malloc(sizeof(scheduler_SJF_t));
    if (info == NULL)
    {
        return NULL;
    }
    info->queue = list_create(sjf_compare_fn);
    info->running_job = NULL;
    info->create = schedulerSJFCreate;
    info->destroy = schedulerSJFDestroy;
    info->scheduleJob = schedulerSJFDestroy;
    info->completeJob = schedulerSJFCompleteJob;
    return info;
}

// -1 if data1 goes before data2
// 0 if data1 is equivalent to data2
// 1 if data1 goes after data2
int sjf_compare_fn(void *data1, void *data2)
{
    job_t *dataItem1 = (job_t *)data1;
    job_t *dataItem2 = (job_t *)data2;

    if (jobGetJobTime(dataItem1) == jobGetJobTime(dataItem2))
    {

        // equivalently sized jobs are ordered by ID
        if (jobGetId(dataItem1) < jobGetId(dataItem2))
        {
            return -1;
        }
        else
        {
            return 1;
        }
    }
    else if (jobGetJobTime(dataItem1) > jobGetJobTime(dataItem2))
    {
        return 1;
    }

    return -1;
}

// Destroys scheduler specific info
void schedulerSJFDestroy(void *schedulerInfo)
{
    if (schedulerInfo == NULL)
    {
        return;
    }
    scheduler_SJF_t *info = (scheduler_SJF_t *)schedulerInfo;

    if (info->queue != NULL)
    {
        list_destroy(info->queue);
        info->queue = NULL;
    }

    free(info);
}

// Called to schedule a new job in the queue
// schedulerInfo - scheduler specific info from create function
// scheduler - used to call schedulerScheduleNextCompletion and schedulerCancelNextCompletion
// job - new job being added to the queue
// currentTime - the current simulated time
void schedulerSJFScheduleJob(void *schedulerInfo, scheduler_t *scheduler, job_t *job, uint64_t currentTime)
{
    if (schedulerInfo == NULL)
    {
        return;
    }

    scheduler_SJF_t *info = (scheduler_SJF_t *)schedulerInfo;

    if (info->running_job == NULL)
    {
        info->running_job = job;
        schedulerScheduleNextCompletion(scheduler, currentTime + jobGetJobTime(job));
    }
    else
    {
        list_insert(info->queue, job);
    }
}

// Called to complete a job in response to an earlier call to schedulerScheduleNextCompletion
// schedulerInfo - scheduler specific info from create function
// scheduler - used to call schedulerScheduleNextCompletion and schedulerCancelNextCompletion
// currentTime - the current simulated time
// Returns the job that is being completed
job_t *schedulerSJFCompleteJob(void *schedulerInfo, scheduler_t *scheduler, uint64_t currentTime)
{
    scheduler_SJF_t *info = (scheduler_SJF_t *)schedulerInfo;
    list_node_t *nextJobNode = list_head(info->queue);

    job_t *nextJob = (job_t *)nextJobNode->data;

    if (nextJob != NULL)
    {
        info->running_job = nextJob;
        schedulerScheduleNextCompletion(scheduler, currentTime + jobGetJobTime(nextJob));
    }
    return NULL;
}
