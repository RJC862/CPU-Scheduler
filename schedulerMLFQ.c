#include <stdint.h>
#include <stdlib.h>
#include "scheduler.h"
#include "job.h"
#include "linked_list.h"

typedef int job_priority_t;

// MLFQ scheduler info
typedef struct
{
    list_t *q0;
    list_t *q1;
    list_t *q2;
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

    info->q0 = list_create(NULL);
    info->q1 = list_create(NULL);
    info->q2 = list_create(NULL);
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
    if (info->q0 != NULL)
    {
        list_destroy(info->q0);
        info->q0 = NULL;
    }
    if (info->q1 != NULL)
    {
        list_destroy(info->q1);
        info->q1 = NULL;
    }
    if (info->q2 != NULL)
    {
        list_destroy(info->q2);
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

    // next completion event happens at most 1 second after a job is scheduled
    uint64_t job_time = jobGetJobTime(job);
    if (job_time > 0)
    {
        job_time = 1;
    }

    if (info->running_job == NULL)
    {
        info->running_job = job;
        info->running_job_priority = 0;

        schedulerScheduleNextCompletion(scheduler, currentTime + job_time);
    }
    else if (info->running_job_priority != 0)
    {

        schedulerCancelNextCompletion(scheduler); // preempt

        if (info->running_job_priority == 1) {
            list_insert(info->q1, info->running_job);
        } else {
            list_insert(info->q2, info->running_job);
        }
        
        info->running_job = job;
        info->running_job_priority = 0;

        schedulerScheduleNextCompletion(scheduler, currentTime + job_time);
    } else {
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

    if (schedulerInfo == NULL) {
        return NULL;
    }
    scheduler_MLFQ_t *info = (scheduler_MLFQ_t *)schedulerInfo;

    job_t *running_job = info->running_job;
    job_priority_t job_priority = info->running_job_priority;

    uint64_t remaining_time = jobGetRemainingTime(running_job) - 1;
    jobSetRemainingTime(running_job, remaining_time); // update remaining time

    if (remaining_time > 0)
    {
        // demote running job
        if (job_priority == 0) {
            list_insert(info->q1, running_job);
        } else {
            list_insert(info->q2, running_job);
        }
    }

    schedulerMLFQRunHighestPriorityJob(info);

    if (info->running_job != NULL) {
        uint64_t job_time = (jobGetRemainingTime(info->running_job) > 0) ? 1 : 0;
        schedulerScheduleNextCompletion(scheduler, currentTime + job_time);
    }

    if (remaining_time <= 0) {
        return running_job;
    } else {
        return NULL;
    }
}


void schedulerMLFQRunHighestPriorityJob(scheduler_MLFQ_t *info){
    
    if (info == NULL) {
        return;
    }

    list_node_t *q0_tail = list_tail(info->q0);

    list_node_t *q1_tail = list_tail(info->q1);

    list_node_t *q2_tail = list_tail(info->q2);

    if (q0_tail != NULL) {

        job_t *job = q0_tail->data;
        list_remove(info->q0, q0_tail);

        info->running_job_priority = 0;
        info->running_job = job;
    } else if (q1_tail != NULL) {

        job_t *job = q1_tail->data;
        list_remove(info->q1, q1_tail);

        info->running_job_priority = 1;
        info->running_job = job;
    } else if (q2_tail != NULL){

        job_t *job = q2_tail->data;
        list_remove(info->q2, q2_tail);

        info->running_job_priority = 2;
        info->running_job = job;
    } else {
        info->running_job = NULL;
    }
}