#include <stdint.h>
#include <stdlib.h>
#include "scheduler.h"
#include "job.h"
#include "linked_list.h"

// MLFQ scheduler info
typedef struct
{
    list_t *q0;
    list_t *q1;
    list_t *q2;
    job_t *job0;
    job_t *job1;
    job_t *job2;
    scheduler_info_create_fn create;
    scheduler_info_destroy_fn destroy;
    schedule_job_fn scheduleJob;
    complete_job_fn completeJob;
} scheduler_MLFQ_t;

// Creates and returns scheduler specific info
void *schedulerMLFQCreate()
{
    scheduler_MLFQ_t *info = malloc(sizeof(scheduler_MLFQ_t));
    if (info == NULL)
    {
        return NULL;
    }

    info->q0 = list_create(NULL);
    info->q1 = list_create(NULL);
    info->q2 = list_create(NULL);
    info->job0 = NULL;
    info->job1 = NULL;
    info->job2 = NULL;
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
    if (info->q0 != NULL)
    {
        list_destroy(info->q0);
        info->q0 = NULL;
    }
    if (info->q1 != NULL)
    {
        list_destroy(info->q0);
        info->q1 = NULL;
    }
    if (info->q2 != NULL)
    {
        list_destroy(info->q0);
        info->q2 = NULL;
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

    if (info->job0 == NULL)
    {
        info->job0 = job;
        schedulerCancelNextCompletion(scheduler);
        schedulerScheduleNextCompletion(scheduler, currentTime + 1); // timeslice of 1
    }
    else
    {
        list_insert(info->q0, job);
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
    job_t *job0 = info->job0;
    job_t *job1 = info->job1;
    job_t *job2 = info->job2;

    // queues will always have an assigned job to run if not empty
    if (job0 != NULL)
    {

        // if job0 in q0 is not finished, demote it
        if (jobGetRemainingTime(job0) > 0)
        {
            list_insert(info->q1, job0);

            job_t *next_job = (job_t *)list_tail(info->q0)->data;
            info->job0 = next_job;

            if (next_job != NULL)
            {
                schedulerScheduleNextCompletion(scheduler, currentTime + 1);
            }

            list_remove(info->q0, list_tail(info->q0));
        }
        else
        {
            return job0;
        }
    }
    else if (job1 != NULL) // demote running job in q1 to q2
    {
        if (jobGetRemainingTime(job1) > 0)
        {
            list_insert(info->q2, job0);

            job_t *next_job = (job_t *)list_tail(info->q1)->data;
            info->job0 = next_job;

            if (next_job != NULL)
            {
                schedulerScheduleNextCompletion(scheduler, currentTime + 1);
            }

            list_remove(info->q1, list_tail(info->q1));
        }
    }
    else if (job2 != NULL)
    {

        if (jobGetRemainingTime(job2) > 0)
        {
            job_t *next_job = (job_t *)list_tail(info->q2)->data;
            info->job2 = next_job;
            if (next_job != NULL)
            {
                schedulerScheduleNextCompletion(scheduler, currentTime + 1);
            }
        }
    }
    return NULL;
}
