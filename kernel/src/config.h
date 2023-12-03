/*
    Gallium Operating System kernel configuration header
*/

//
// Scheduler
//
#define SCHEDULER_DESIRED_FREQUENCY 100 /* the frequency at which the preemption timer runs */

////
//// Task Scheduler
////

#define ROUND_ROBIN 1
#define SCHEDULER_DESIRED_TASK_SCHEDULER ROUND_ROBIN /* available types of task schedulers: ROUND_ROBIN */