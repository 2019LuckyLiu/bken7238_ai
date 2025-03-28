#ifndef __RTOS_PUB__
#define __RTOS_PUB__


/** @brief Enter a critical session, all interrupts are disabled
*
 *  @param void
 *
 *  @return  
 *      - void
 */
void rtos_enter_critical( void);

/** @brief Exit a critical session, all interrupts are enabled
 *
 *  @param void
 *
 *  @return  
 *      - void
  */
void rtos_exit_critical( void);


/** @brief disable os scheduling
*
 *  @param void
 *
 *  @return  
 *      - void
 */
void rtos_lock_scheduling( void );


/** @brief enable os scheduling
 *
 *  @param void
 *
 *  @return  
 *      - void
 */
void rtos_unlock_scheduling( void );


/** @brief   Get system time value in milliseconds
  *
  * @param   time_ptr     : the pointer of time value in milliseconds
  *
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
OSStatus beken_time_get_time(beken_time_t* time_ptr);




/** @defgroup BEKEN_RTOS_Thread _BK_ RTOS Thread Management Functions
 *  @brief Provide thread creation, delete, suspend, resume, and other RTOS management API
 *  @verbatim
 *   _BK_ thread priority table
 *
 * +----------+-----------------+
 * | Priority |      Thread     |
 * |----------|-----------------|
 * |     0    |      _BK_       |   Highest priority
 * |     1    |     Network     |
 * |     2    |                 |
 * |     3    | Network worker  |
 * |     4    |                 |
 * |     5    | Default Library |
 * |          | Default worker  |
 * |     6    |                 |
 * |     7    |   Application   |
 * |     8    |                 |
 * |     9    |      Idle       |   Lowest priority
 * +----------+-----------------+
 *  @endverbatim
 * @{
 */


/** @brief Creates and starts a new thread
  *
  * @param thread     : Pointer to variable that will receive the thread handle (can be null)
  * @param priority   : A priority number. (0:Highest priority, 9:Lowest priority)
  * @param name       : a text name for the thread (can be null)
  * @param function   : the main thread function
  * @param stack_size : stack size for this thread
  * @param arg        : argument which will be passed to thread function (can be null)
  *
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
OSStatus rtos_create_thread( beken_thread_t* thread, uint8_t priority, const char* name, beken_thread_function_t function, uint32_t stack_size, beken_thread_arg_t arg);



/** @brief   Deletes a terminated thread
  *
  * @param   thread     : the handle of the thread to delete, NULL is the current thread
  *
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
OSStatus rtos_delete_thread( beken_thread_t* thread);



/** @brief   Set thread priority
  *
  * @param   thread     : the handle of the thread
  *
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
OSStatus rtos_thread_set_priority(beken_thread_t *thread, int priority);


/** @brief    Suspend a thread
  *
  * @param    thread     : the handle of the thread to suspend, NULL is the current thread
  *
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
void rtos_suspend_thread(beken_thread_t* thread);


/** @brief    Resume a thread
  *
  * @param    thread     : the handle of the thread to resume, NULL is the current thread
  *
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
void rtos_resume_thread(beken_thread_t *thread);


/** @brief    Sleeps until another thread has terminated
  *
  * @Details  Causes the current thread to sleep until the specified other thread
  *           has terminated. If the processor is heavily loaded with higher priority
  *           tasks, this thread may not wake until significantly after the thread termination.
  *
  * @param    thread : the handle of the other thread which will terminate
  *
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
OSStatus rtos_thread_join( beken_thread_t* thread);


/** @brief    Forcibly wakes another thread
  *
  * @Details  Causes the specified thread to wake from suspension. This will usually
  *           cause an error or timeout in that thread, since the task it was waiting on
  *           is not complete.
  *
  * @param    thread : the handle of the other thread which will be woken
  *
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
OSStatus rtos_thread_force_awake( beken_thread_t* thread);


/** @brief    Checks if a thread is the current thread
  *
  * @Details  Checks if a specified thread is the currently running thread
  *
  * @param    thread : the handle of the other thread against which the current thread
  *                    will be compared
  *
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
BOOL rtos_is_current_thread( beken_thread_t* thread);


/** @brief    Get current thread handler
  *
  * @return   Current RTOS thread handler
  */
beken_thread_t* rtos_get_current_thread( void);


/** @brief    Suspend current thread for a specific time
  *
  * @param    seconds : A time interval (Unit: seconds)
  *
  * @return   None.
  */
void rtos_thread_sleep(uint32_t seconds);


/** @brief    Suspend current thread for a specific time
 *
 * @param     milliseconds : A time interval (Unit: millisecond)
 *
 * @return    None.
 */
void rtos_thread_msleep(uint32_t milliseconds);


/** @brief    Suspend current thread for a specific time
 *
 * @param     num_ms : A time interval (Unit: millisecond)
 *
 * @return    
 *       - kNoErr        : on success.
 *       - kGeneralErr   : if an error occurred with any step
 */
OSStatus rtos_delay_milliseconds( uint32_t num_ms);


/** @brief    Print Thread status into buffer
  *
  * @param    buffer, point to buffer to store thread status
  * @param    length, length of the buffer
  *
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
OSStatus rtos_print_thread_status( char* buffer, int length);


/**
  * @}
  */


/** @defgroup BEKEN_RTOS_SEM _BK_ RTOS Semaphore Functions
  * @brief Provide management APIs for semaphore such as init,set,get and dinit.
  * @{
  */

/** @brief    Initialises a counting semaphore and set count to 0
  *
  * @param    semaphore : a pointer to the semaphore handle to be initialised
  * @param    maxCount  : the max count number of this semaphore
  *
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
OSStatus rtos_init_semaphore( beken_semaphore_t* semaphore, int maxCount);


/** @brief    Initialises a counting semaphore and set count to init count
  *
  * @param    semaphore : a pointer to the semaphore handle to be initialised
  * @param    maxCount  : the max count number of this semaphore
  * @param    init_count  : the init count number of this semaphore
  *
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
OSStatus rtos_init_semaphore_adv( beken_semaphore_t* semaphore, int maxCount, int init_count);


/** @brief    Set (post/put/increment) a semaphore
  *
  * @param    semaphore : a pointer to the semaphore handle to be set
  *
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
OSStatus rtos_set_semaphore( beken_semaphore_t* semaphore);


/** @brief    Get (wait/decrement) a semaphore
  *
  * @Details  Attempts to get (wait/decrement) a semaphore. If semaphore is at zero already,
  *           then the calling thread will be suspended until another thread sets the
  *           semaphore with @ref rtos_set_semaphore
  *
  * @param    semaphore : a pointer to the semaphore handle
  * @param    timeout_ms: the number of milliseconds to wait before returning
  *
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
OSStatus rtos_get_semaphore( beken_semaphore_t* semaphore, uint32_t timeout_ms);



/** @brief    Get (wait/decrement)  semaphore number
  *
  *
  * @param    semaphore : a pointer to the semaphore handle
  *
  * @return   the number of semaphore
  */
int rtos_get_sema_count( beken_semaphore_t* semaphore);


/** @brief    De-initialise a semaphore
  *
  * @Details  Deletes a semaphore created with @ref rtos_init_semaphore
  *
  * @param    semaphore : a pointer to the semaphore handle
  *
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
OSStatus rtos_deinit_semaphore( beken_semaphore_t* semaphore);



/**
  * @}
  */

/** @defgroup BEKEN_RTOS_MUTEX _BK_ RTOS Mutex Functions
  * @brief Provide management APIs for Mutex such as init,lock,unlock and dinit.
  * @{
  */

/** @brief    Initialises a mutex
  *
  * @Details  A mutex is different to a semaphore in that a thread that already holds
  *           the lock on the mutex can request the lock again (nested) without causing
  *           it to be suspended.
  *
  * @param    mutex : a pointer to the mutex handle to be initialised
  *
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
OSStatus rtos_init_mutex( beken_mutex_t* mutex);


/** @brief    Obtains the lock on a mutex
  *
  * @Details  Attempts to obtain the lock on a mutex. If the lock is already held
  *           by another thead, the calling thread will be suspended until the mutex
  *           lock is released by the other thread.
  *
  * @param    mutex : a pointer to the mutex handle to be locked
  *
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
OSStatus rtos_lock_mutex( beken_mutex_t* mutex);



/** @brief    Obtains the lock on a mutex
  *
  * @Details  Attempts to obtain the lock on a mutex. If the lock is already held
  *           by another thead, the calling thread will not be suspended until the mutex
  *           lock is released by the other thread.
  *
  * @param    mutex : a pointer to the mutex handle to be locked
  *
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
OSStatus rtos_trylock_mutex( beken_mutex_t* mutex);


/** @brief    Releases the lock on a mutex
  *
  * @Details  Releases a currently held lock on a mutex. If another thread
  *           is waiting on the mutex lock, then it will be resumed.
  *
  * @param    mutex : a pointer to the mutex handle to be unlocked
  *
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
OSStatus rtos_unlock_mutex( beken_mutex_t* mutex);




/** @brief    De-initialise a mutex
  *
  * @Details  Deletes a mutex created with @ref rtos_init_mutex
  *
  * @param    mutex : a pointer to the mutex handle
  *
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
OSStatus rtos_deinit_mutex( beken_mutex_t* mutex);

/**
  * @}
  */


/** @defgroup BEKEN_RTOS_MUTEX _BK_ RTOS Mutex Functions
  * @brief Provide management APIs for Mutex such as init,lock,unlock and dinit.
  * @{
  */

/** @brief    Initialises a recursive mutex
  *
  * @Details  A mutex is different to a semaphore in that a thread that already holds
  *           the lock on the mutex can request the lock again (nested) without causing
  *           it to be suspended.
  *
  * @param    mutex : a pointer to the mutex handle to be initialised
  *
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
OSStatus rtos_init_recursive_mutex(beken_mutex_t *mutex);



/** @brief    Obtains the lock on a recursive mutex
  *
  * @Details  Attempts to obtain the lock on a mutex. If the lock is already held
  *           by another thead, the calling thread will be suspended until the mutex
  *           lock is released by the other thread.
  *
  * @param    mutex : a pointer to the mutex handle to be locked
  *         - timeout: timeout for wait mutex
  *
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
OSStatus rtos_lock_recursive_mutex(beken_mutex_t *mutex, uint32_t timeout);





/** @brief    Releases the lock on a recursive mutex
  *
  * @Details  Releases a currently held lock on a mutex. If another thread
  *           is waiting on the mutex lock, then it will be resumed.
  *
  * @param    mutex : a pointer to the mutex handle to be unlocked
  *
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
OSStatus rtos_unlock_recursive_mutex(beken_mutex_t *mutex);


/**
  * @}
  */


/** @defgroup BEKEN_RTOS_QUEUE _BK_ RTOS FIFO Queue Functions
  * @brief Provide management APIs for FIFO such as init,push,pop and dinit.
  * @{
  */

/** @brief    Initialises a FIFO queue
  *
  * @param    queue : a pointer to the queue handle to be initialised
  * @param    name  : a text string name for the queue (NULL is allowed)
  * @param    message_size : size in bytes of objects that will be held in the queue
  * @param    number_of_messages : depth of the queue - i.e. max number of objects in the queue
  *
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
OSStatus rtos_init_queue( beken_queue_t* queue, const char* name, uint32_t message_size, uint32_t number_of_messages);






/** @brief    Pushes an object onto a queue (to the back)
  *
  * @param    queue : a pointer to the queue handle
  * @param    message : the object to be added to the queue. Size is assumed to be
  *                  the size specified in @ref rtos_init_queue
  * @param    timeout_ms: the number of milliseconds to wait before returning
  *
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
OSStatus rtos_push_to_queue( beken_queue_t* queue, void* message, uint32_t timeout_ms);



/** @brief    Pushes an object to front of the queue
  *
  * @param    queue : a pointer to the queue handle
  * @param    message : the object to be added to the queue. Size is assumed to be
  *                  the size specified in @ref rtos_init_queue
  * @param    timeout_ms: the number of milliseconds to wait before returning
  *
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
OSStatus rtos_push_to_queue_front(beken_queue_t* queue, void* message, uint32_t timeout_ms);


/** @brief    Pops an object off a queue
  *
  * @param    queue : a pointer to the queue handle
  * @param    message : pointer to a buffer that will receive the object being
  *                     popped off the queue. Size is assumed to be
  *                     the size specified in @ref rtos_init_queue , hence
  *                     you must ensure the buffer is long enough or memory
  *                     corruption will result
  * @param    timeout_ms: the number of milliseconds to wait before returning
  *
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
OSStatus rtos_pop_from_queue( beken_queue_t* queue, void* message, uint32_t timeout_ms);





/** @brief    De-initialise a queue created with @ref rtos_init_queue
  *
  * @param    queue : a pointer to the queue handle
  *
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
OSStatus rtos_deinit_queue( beken_queue_t* queue);







/** @brief    Check if a queue is empty
  *
  * @param    queue : a pointer to the queue handle
  *
  * @return   
  *     - true  : queue is empty.
  *     - false : queue is not empty.
  */
BOOL rtos_is_queue_empty( beken_queue_t* queue);







/** @brief    Check if a queue is full
  *
  * @param    queue : a pointer to the queue handle
  *
  * @return   
  *     - true  : queue is empty.
  *     - false : queue is not empty.
  */
BOOL rtos_is_queue_full( beken_queue_t* queue);

/**
  * @}
  */


/** @defgroup BEKEN_RTOS_TIMER _BK_ RTOS Timer Functions
  * @brief Provide management APIs for timer such as init,start,stop,reload and dinit.
  * @{
  */

/**
  * @brief    Gets time in miiliseconds since RTOS start
  *
  * @attention:    Since this is only 32 bits, it will roll over every 49 days, 17 hours.
  *
  * @returns  Time in milliseconds since RTOS started.
  */
uint32_t rtos_get_time(void);



/**
  * @brief    Gets time in microsecond since RTOS start
  *
  * @attention:    Since this is only 32 bits, it will roll over every 49 days, 17 hours.
  *
  * @returns  Time in milliseconds since RTOS started.
  */
uint64_t rtos_get_time_us( void );



/**
  * @brief     Deinitialization a RTOS oneshot timer
  *
  * @attention      Timer does not start running until @ref beken_start_timer is called
  *
  * @param     timer    : a pointer to the timer handle to be initialised
  * @param     time_ms  : Timer period in milliseconds
  * @param     function : the callback handler function that is called each time the
  *                       timer expires
  * @param     larg      : an argument that will be passed to the callback function
  * @param     rarg      : an argument that will be passed to the callback function
  *
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
OSStatus rtos_init_oneshot_timer( beken2_timer_t *timer,
									uint32_t time_ms,
									timer_2handler_t function,
									void* larg,
									void* rarg);




/**
  * @brief      De-initialise a RTOS oneshot timer
  *
  *
  * @param     timer    : a pointer to the timer handle to be initialised
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
OSStatus rtos_deinit_oneshot_timer( beken2_timer_t* timer);



/**
  * @brief      De-initialise a RTOS timer with block way
  *
  *
  * @param     timer    : a pointer to the timer handle to be initialised
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
OSStatus rtos_deinit_oneshot_timer_block( beken2_timer_t* timer);



/**
  * @brief     stop a RTOS oneshot timer 
  *
  *
  * @param     timer    : a pointer to the timer handle to be initialised
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
OSStatus rtos_stop_oneshot_timer( beken2_timer_t* timer);


/**
  * @brief     whether oneshot_timer is running
  *
  *
  * @param     timer    : a pointer to the timer handle to be initialised
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
BOOL rtos_is_oneshot_timer_running( beken2_timer_t* timer);


/**
  * @brief     start a RTOS oneshot timer 
  *
  *
  * @param     timer    : a pointer to the timer handle to be initialised
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
OSStatus rtos_start_oneshot_timer( beken2_timer_t* timer);


/**
  * @brief     whether oneshot_timer is inited
  *
  *
  * @param     timer    : a pointer to the timer handle to be initialised
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
BOOL rtos_is_oneshot_timer_init( beken2_timer_t* timer);


/**
  * @brief     Reloads oneshot_timer is inited that has expired
  *
  *
  * @param     timer    : a pointer to the timer handle to be initialised
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
OSStatus rtos_oneshot_reload_timer( beken2_timer_t* timer);


/**
  * @brief     change timeout for a RTOS oneshot timer  that has expired
  *
  *
  * @param     timer    : a pointer to the timer handle to be initialised
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
OSStatus rtos_change_period_1( beken2_timer_t* timer, uint32_t time_ms);


/**
  * @brief     Initialize a RTOS timer
  *
  * @attention      Timer does not start running until @ref beken_start_timer is called
  *
  * @param     timer    : a pointer to the timer handle to be initialised
  * @param     time_ms  : Timer period in milliseconds
  * @param     function : the callback handler function that is called each time the
  *                       timer expires
  * @param     arg      : an argument that will be passed to the callback function
  *
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
OSStatus rtos_init_timer( beken_timer_t* timer, uint32_t time_ms, timer_handler_t function, void* arg);


/** @brief    Starts a RTOS timer running
  *
  * @attention     Timer must have been previously initialised with @ref rtos_init_timer
  *
  * @param    timer    : a pointer to the timer handle to start
  *
  * @return   kNoErr        : on success.
  * @return   kGeneralErr   : if an error occurred
  */
OSStatus rtos_start_timer( beken_timer_t* timer);


/**
  * @brief     change timeout for a RTOS oneshot timer that has expired
  *
  *
  * @param     timer    : a pointer to the timer handle to be initialised
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
OSStatus rtos_change_period( beken_timer_t* timer, uint32_t time_ms);

/** @brief    Stops a running RTOS timer
  *
  * @attention     Timer must have been previously started with @ref rtos_init_timer
  *
  * @param    timer    : a pointer to the timer handle to stop
  *
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
OSStatus rtos_stop_timer( beken_timer_t* timer);


/** @brief    Reloads a RTOS timer that has expired
  *
  * @attention     This is usually called in the timer callback handler, to
  *           reschedule the timer for the next period.
  *
  * @param    timer    : a pointer to the timer handle to reload
  *
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
OSStatus rtos_reload_timer( beken_timer_t* timer);


/** @brief    De-initialise a RTOS timer
  *
  * @attention     Deletes a RTOS timer created with @ref rtos_init_timer
  *
  * @param    timer : a pointer to the RTOS timer handle
  *
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
OSStatus rtos_deinit_timer( beken_timer_t* timer);


/** @brief    De-initialise a RTOS timer with black way
  *
  * @attention     Deletes a RTOS timer created with @ref rtos_init_timer
  *
  * @param    timer : a pointer to the RTOS timer handle
  *
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
OSStatus rtos_deinit_timer_block(beken_timer_t *timer);


/** @brief    Check if an RTOS timer is running
  *
  * @param    timer : a pointer to the RTOS timer handle
  *
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
BOOL rtos_is_timer_init( beken_timer_t* timer);


/**
  * @brief     whether oneshot_timer is running
  *
  *
  * @param     timer    : a pointer to the timer handle to be initialised
  * @return    
  *       - kNoErr        : on success.
  *       - kGeneralErr   : if an error occurred with any step
  */
BOOL rtos_is_timer_running( beken_timer_t* timer);



#ifdef __cplusplus
}
#endif

#endif // __RTOS_PUB__
// EOF

