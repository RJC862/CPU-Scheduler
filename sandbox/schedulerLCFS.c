#include <stdint.h>
#include <stdlib.h>
#include "scheduler.h"
#include "job.h"
#include "linked_list.h"

// LCFS scheduler info
typedef struct
{
    list_t *queue;
    job_t *running_job;
    scheduler_info_create_fn create;
    scheduler_info_destroy_fn destroy;
    schedule_job_fn scheduleJob;
    complete_job_fn completeJob;
} scheduler_LCFS_t;

// Creates and returns scheduler specific info
void *schedulerLCFSCreate()
{
    scheduler_LCFS_t *info = malloc(sizeof(scheduler_LCFS_t));
    if (info == NULL)
    {
        return NULL;
    }
    info->queue = list_create(NULL);
    info->running_job = NULL;
    info->create = schedulerLCFSCreate;
    info->destroy = schedulerLCFSDestroy;
    info->scheduleJob = schedulerLCFSScheduleJob;
    info->completeJob = schedulerLCFSCompleteJob;

    return info;
}

// Destroys scheduler specific info
void schedulerLCFSDestroy(void *schedulerInfo)
{
    if (schedulerInfo == NULL)
    {
        return;
    }
    scheduler_LCFS_t *info = (scheduler_LCFS_t *)schedulerInfo;

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
void schedulerLCFSScheduleJob(void *schedulerInfo, scheduler_t *scheduler, job_t *job, uint64_t currentTime)
{
    scheduler_LCFS_t *info = (scheduler_LCFS_t *)schedulerInfo;

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
job_t *schedulerLCFSCompleteJob(void *schedulerInfo, scheduler_t *scheduler, uint64_t currentTime)
{
    scheduler_LCFS_t *info = (scheduler_LCFS_t *)schedulerInfo;
    job_t *curr_job = info->running_job;

    list_node_t *next_job_node = list_head(info->queue);

    if (next_job_node != NULL)
    {

        job_t *next_job = (job_t *)next_job_node->data;
        info->running_job = next_job;

        if (next_job != NULL)
        {
            schedulerScheduleNextCompletion(scheduler, currentTime + jobGetJobTime(next_job));
        }
        list_remove(info->queue, next_job_node);
    }
    else
    {
        info->running_job = NULL;
    }
    return curr_job;
}
