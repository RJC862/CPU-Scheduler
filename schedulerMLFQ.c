#include <stdint.h>
#include <stdlib.h>
#include "scheduler.h"
#include "job.h"
#include "linked_list.h"
#include <stdio.h>

typedef int job_priority_t;

// MLFQ scheduler info
typedef struct
{
    list_t *q0;
    list_t *q1;
    list_t *q2;
    job_t *running_job;
    uint64_t running_job_start_time;
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
    info->running_job_start_time = 0;
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

    //printf("new job ID: %ld Arrival Time: %ld Running Job Priority: %d\n", jobGetId(job), jobGetArrivalTime(job), info->running_job_priority);
    if (info->running_job == NULL)
    {
        // run new job immediately
        info->running_job = job;
        info->running_job_priority = 0;

        //printf("schedule job ID: %ld Remaining time: %ld\n", jobGetId(job), jobGetRemainingTime(job));

        schedulerScheduleNextCompletion(scheduler, currentTime + jobGetRemainingTime(job));
    }
    else
    {

        // update the running job's remaining time
        uint64_t running_time = currentTime - jobGetArrivalTime(info->running_job);
        uint64_t rem_time = jobGetRemainingTime(info->running_job);

        int priority = info->running_job_priority;

        // first insert the job into its apprioriate priority queue
        if (running_time > 2) {
            
            list_insert(info->q2, info->running_job);
            priority = 2;

        } else if (running_time == 1) {

            if (priority == 0) {
                list_insert(info->q1, info->running_job);
                priority = 1;
            } else {
                list_insert(info->q2, info->running_job);
                priority = 2;
            }

        } else {
            if (priority == 1) {
                list_insert(info->q1, info->running_job);
            } else if (priority == 2) {
                list_insert(info->q2, info->running_job);
            }
        }

        // now update its remaining time
        // Note: The running time will never be more than the remaining time, since its scheduled completion event is according to the remaining time
        jobSetRemainingTime(info->running_job, rem_time - running_time);

        // preempt the current running job only if it does not have highest priority

        if (priority != 0) {
            printf("preempted job ID: %ld Remaining time: %ld Current Time: %ld Priority: %d\n", jobGetId(info->running_job), jobGetRemainingTime(info->running_job), currentTime, priority);

            schedulerCancelNextCompletion(scheduler); // preempt

            // run the new job
            info->running_job = job;
            info->running_job_priority = 0;

            printf("Newly running job after preemption: %ld\n", jobGetId(info->running_job));
            schedulerScheduleNextCompletion(scheduler, currentTime + jobGetRemainingTime(job));
        } else {

            // make the new job wait
            list_insert(info->q0, job);
        }
    }
}

// Called to complete a job in response to an earlier call to schedulerScheduleNextCompletion
// schedulerInfo - scheduler specific info from create function
// scheduler - used to call schedulerScheduleNextCompletion and schedulerCancelNextCompletion
// currentTime - the current simulated time
// Returns the job that is being completed
job_t *schedulerMLFQCompleteJob(void *schedulerInfo, scheduler_t *scheduler, uint64_t currentTime)
{

    if (schedulerInfo == NULL)
    {
        return NULL;
    }
    scheduler_MLFQ_t *info = (scheduler_MLFQ_t *)schedulerInfo;

    job_t *running_job = info->running_job;
    printf("job ID: %ld Remaining Time: %ld Priority: %d current time: %ld\n", jobGetId(running_job), jobGetRemainingTime(running_job), info->running_job_priority, currentTime);

    // run the next highest priority job
    list_node_t *q0_tail = list_tail(info->q0);
    list_node_t *q1_tail = list_tail(info->q1);
    list_node_t *q2_tail = list_tail(info->q2);


    if (q0_tail != NULL) {
       // printf("q0 tail: %ld; \n", jobGetId((job_t *) q0_tail->data));
    }
    if (q1_tail != NULL) {
       // printf("q1 tail: %ld; \n", jobGetId((job_t *) q1_tail->data));
    }
    if (q2_tail != NULL) {
        //printf("q2 tail: %ld\n ", jobGetId((job_t *) q2_tail->data));
    }

    job_t *next_job = NULL;

    if (q0_tail != NULL)
    {
        next_job = (job_t *) q0_tail->data;
        list_remove(info->q0, q0_tail);

        info->running_job_priority = 0;
    }
    else if (q1_tail != NULL)
    {
        next_job = (job_t *) q1_tail->data;
        list_remove(info->q1, q1_tail);

        info->running_job_priority = 1;
    }
    else if (q2_tail != NULL)
    {
        next_job = (job_t *) q2_tail->data;
        list_remove(info->q2, q2_tail);

        info->running_job_priority = 2;
    }

    info->running_job = next_job;

    return running_job;
}