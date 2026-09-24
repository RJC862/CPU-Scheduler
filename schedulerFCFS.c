#include <stdint.h>
#include <stdlib.h>
#include "scheduler.h"
#include "job.h"
#include "linked_list.h"

// FCFS scheduler info
typedef struct
{

    list_t *queue;
    job_t *running_job;
    scheduler_info_create_fn create;
    scheduler_info_destroy_fn destroy;
    schedule_job_fn scheduleJob;
    complete_job_fn completeJob;

} scheduler_FCFS_t;

// Creates and returns scheduler specific info
void *schedulerFCFSCreate()
{
    scheduler_FCFS_t *info = malloc(sizeof(scheduler_FCFS_t));
    if (info == NULL)
    {
        return NULL;
    }

    info->queue = list_create(NULL);
    info->running_job = NULL;
    info->create = schedulerFCFSCreate;
    info->destroy = schedulerFCFSDestroy;
    info->completeJob = schedulerFCFSCompleteJob;
    info->scheduleJob = schedulerFCFSScheduleJob;

    return info;
}

// Destroys scheduler specific info
void schedulerFCFSDestroy(void *schedulerInfo)
{

    scheduler_FCFS_t *info = (scheduler_FCFS_t *)schedulerInfo;
}

// Called to schedule a new job in the queue
// schedulerInfo - scheduler specific info from create function
// scheduler - used to call schedulerScheduleNextCompletion and schedulerCancelNextCompletion
// job - new job being added to the queue
// currentTime - the current simulated time
void schedulerFCFSScheduleJob(void *schedulerInfo, scheduler_t *scheduler, job_t *job, uint64_t currentTime)
{
    scheduler_FCFS_t *info = (scheduler_FCFS_t *)schedulerInfo;
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
job_t *schedulerFCFSCompleteJob(void *schedulerInfo, scheduler_t *scheduler, uint64_t currentTime)
{
    scheduler_FCFS_t *info = (scheduler_FCFS_t *)schedulerInfo;

    job_t *currJob = info->running_job;

    list_node_t *job_node = list_tail(info->queue);

    // Tail of the queue is the next running job
    job_t *next_job = (job_t *)job_node->data;
    if (next_job != NULL)
    {
        schedulerScheduleNextCompletion(scheduler, currentTime + jobGetJobTime(next_job));
    }

    list_remove(info->queue, job_node);
    return currJob;
}