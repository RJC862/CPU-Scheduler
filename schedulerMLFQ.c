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
    list_t **queues;
    uint64_t levels;
    job_t *running_job;
    uint64_t running_job_start_time;
    job_priority_t running_job_priority;
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

    // dynamic list of queues allowing unlimited priority levels
    info->levels = 1;
    info->queues = malloc(info->levels * sizeof(list_t *));

    if (info->queues == NULL)
    {
        free(info);
        return NULL;
    }

    info->queues[0] = list_create(NULL);

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
    if (info->queues != NULL)
    {
        for (int i = 0; i < info->levels; i++)
        {
            list_destroy(info->queues[i]);
        }
    }
    free(info->queues);
    info->queues = NULL;

    free(info);
    info = NULL;
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
    if (info->running_job == NULL)
    {
        // run new job immediately
        info->running_job = job;
        info->running_job_priority = 0;
        info->running_job_start_time = currentTime;
        uint64_t run_time = 0;

        if (jobGetRemainingTime(job) > 0)
        {
            run_time = 1;
        }

        schedulerScheduleNextCompletion(scheduler, currentTime + run_time);
        return;
    }

    if (info->running_job_priority > 0)
    {
        // preempt the current running job

        schedulerCancelNextCompletion(scheduler); // preempt

        // update the remaining time
        uint64_t running_time = currentTime - info->running_job_start_time;
        uint64_t remaining_time = jobGetRemainingTime(info->running_job);

        if (running_time < remaining_time)
        {
            remaining_time -= running_time;
        }
        else
        {
            remaining_time = 0;
        }
        jobSetRemainingTime(info->running_job, remaining_time);

        // Put the preempted job in its respective queue
        if (remaining_time > 0)
        {
            if (running_time >= 1)
            {
                // demote
                if (info->running_job_priority >= info->levels - 1)
                {

                    // making room for the new priority level
                    list_t **temp = realloc(info->queues, (info->levels + 1) * sizeof(list_t *));

                    if (temp == NULL)
                    {
                        return;
                    }
                    info->queues = temp;

                    // create a new priority level, insert the running job, then put it the priority level in the list of queues
                    list_t *new_list = list_create(NULL);
                    list_insert(new_list, info->running_job);

                    // putting the priority level in the list of queues
                    info->queues[info->levels] = new_list;

                    // updating counters
                    info->running_job_priority += 1;
                    info->levels++;
                }
                else
                {
                    list_t *q = info->queues[info->running_job_priority + 1];
                    list_insert(q, info->running_job);
                    info->running_job_priority += 1;
                }
            }
            else
            {
                // insert back into its queue
                list_t *q = info->queues[info->running_job_priority];
                list_tail_insert(q, info->running_job);
            }
        }
        // run new job with priority 0

        info->running_job = job;
        info->running_job_priority = 0;
        info->running_job_start_time = currentTime;

        uint64_t run_time = 0;

        if (jobGetRemainingTime(job) > 0)
        {
            run_time = 1;
        }

        schedulerScheduleNextCompletion(scheduler, currentTime + run_time);
    }
    else
    {

        // make the new job wait
        list_t *q = info->queues[0];

        list_insert(q, job);
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

    if (running_job == NULL)
    {
        return NULL;
    }

    // update the requested completion job's remaining time
    uint64_t remaining_time = jobGetRemainingTime(running_job);

    uint64_t running_time = 1;

    if (running_time < remaining_time)
    {
        remaining_time -= running_time;
    }
    else
    {
        remaining_time = 0;
    }

    jobSetRemainingTime(running_job, remaining_time);

    // demote
    if (remaining_time > 0)
    {
        if (info->running_job_priority >= info->levels - 1)
        {

            // making room for the new priority level
            list_t **temp = realloc(info->queues, (info->levels + 1) * sizeof(list_t *));

            if (temp == NULL)
            {
                return NULL;
            }
            info->queues = temp;

            // create a new priority level, insert the running job, then put it the priority level in the list of queues
            list_t *new_list = list_create(NULL);
            list_insert(new_list, info->running_job);

            // putting the priority level in the list of queues
            info->queues[info->levels] = new_list;

            // updating counters
            info->running_job_priority += 1;
            info->levels++;
        }
        else
        {
            list_t *q = info->queues[info->running_job_priority + 1];
            list_insert(q, info->running_job);
            info->running_job_priority += 1;
        }
    }

    // run the next highest priority job
    job_t *next_job = NULL;

    // look for the next highest priority job
    for (int i = 0; i < info->levels; i++)
    {
        list_t *q = info->queues[i];

        // check the current queue's tail
        if (q != NULL)
        {
            list_node_t *tail = list_tail(q);

            if (tail != NULL)
            {
                next_job = (job_t *)tail->data;
                if (next_job != NULL)
                {
                    list_remove(q, list_tail(q));
                    info->running_job_priority = i;
                    break;
                }
            }
        }
    }
    info->running_job = next_job;
    info->running_job_start_time = currentTime;

    uint64_t run_time = 0;

    if (next_job != NULL)
    {

        if (jobGetRemainingTime(next_job) > 0)
        {
            run_time = 1;
        }

        schedulerScheduleNextCompletion(scheduler, currentTime + run_time);
    }

    // return completed job

    if (remaining_time == 0)
    {
        return running_job;
    }
    else
    {
        return NULL;
    }
}