/******************************************************************
 * The Main program with the two functions. A simple
 * example of creating and using a thread is provided.
 ******************************************************************/

#include "helper.h"

void *producer (void *id);
void *consumer (void *id);

struct producer_thread_data {
  int thread_id;
  int number_of_jobs_per_producer;
  int queue_size;
  int* producer_index;
  int sem_id;
  int mutex;
  int full;
  int empty;
};

struct consumer_thread_data {
  int thread_id;
  int queue_size;
  int* consumer_index;
  int sem_id;
  int mutex;
  int full;
  int empty;
};

/*
struct job_data {
  int id;
  int duration;
};
*/

// See how I could improve it!
int shared_buffer[100];
int item_counter = 0;

int main (int argc, char **argv)
{

  // 1(a)
  int queue_size = check_arg(argv[1]);
  int number_of_jobs_per_producer = check_arg(argv[2]);
  int number_of_producers = check_arg(argv[3]);
  int number_of_consumers = check_arg(argv[4]);

  // 1(b)

  // initialize the queue with 0, representing the empty values
  for(int i = 0; i < queue_size; i++) {
    shared_buffer[i] = 0;
  }

  // Initialize indexes in order to keep track of the next item in each case
  int producer_index = 0;
  int consumer_index = 0;

  // 1(c)
  
  // Num of each created semaphore
  int mutex = 0;
  int full = 1;
  int empty = 2;
    
  // Creation of the 3 Semaphores that will be needed
  int sem_id;
  sem_id = sem_create(SEM_KEY, 3);
  //cout << "Semaphore array created " << sem_id << endl;
  
  // Initialization of the Semaphores
  
  int init1 = sem_init(sem_id, mutex, 1);
  //cout << "Semaphore initialized " << init1 << endl;
  int init2 = sem_init(sem_id, full, 0);
  //cout << "Semaphore initialized " << init2 << endl;  
  int init3 = sem_init(sem_id, empty, queue_size);
  //cout << "Semaphore initialized " << init3 << endl;

  // To be deleted
  if((init1 == 0) && (init2 == 0) && (init3 == 0)) {
    cout << "Semaphores successfully created" << endl;
  }

  // 1(d)
  pthread_t producersid[number_of_producers];

  struct producer_thread_data producer_data[number_of_producers];

  for(int i = 0; i < number_of_producers; i++) {
    producer_data[i].thread_id = i;
    producer_data[i].number_of_jobs_per_producer = number_of_jobs_per_producer; 
    producer_data[i].queue_size = queue_size;
    producer_data[i].producer_index = &producer_index;
    producer_data[i].sem_id = sem_id;
    producer_data[i].mutex = mutex;
    producer_data[i].full = full;
    producer_data[i].empty = empty;
  
    pthread_create(&producersid[i], NULL, producer, (void*) &producer_data[i]);
  }

  //pthread_join(producersid[0], NULL);

  pthread_t consumersid[number_of_consumers];

  struct consumer_thread_data consumer_data[number_of_consumers];

  for(int i = 0; i < number_of_consumers; i++) {
    consumer_data[i].thread_id = i;
    consumer_data[i].queue_size = queue_size;
    consumer_data[i].consumer_index = &consumer_index;
    consumer_data[i].sem_id = sem_id;
    consumer_data[i].mutex = mutex;
    consumer_data[i].full = full;
    consumer_data[i].empty = empty;
 
    pthread_create(&consumersid[i], NULL, consumer, (void*) &consumer_data[i]);
  }

  // 1(e)

  // Ensure that all threds are done
  for(int i = 0; i < number_of_producers; i++) {
    pthread_join(producersid[i], NULL);
  }
  
  for(int i = 0; i < number_of_consumers; i++) {
    pthread_join(consumersid[i], NULL);
  }

  // Close the semaphores
  int closed = sem_close(sem_id);

  // To be deleted
  if(!closed) {
    cout << "Semaphores successfully destructed" << endl;
  }

  pthread_exit(0);

  return 0;
}

int produce_job() {
  srand(time(NULL));

  return (rand() % 10 + 1);
}

void *producer(void *parameter)
{

  srand(time(NULL));
  //int *param = (int *) parameter;

  struct producer_thread_data *received_data;

  received_data = (struct producer_thread_data *) parameter;

  int thread_id = received_data->thread_id;
  int num_of_jobs_per_producer = received_data->number_of_jobs_per_producer;
  int queue_size = received_data->queue_size;
  int* producer_index = received_data->producer_index;
  int sem_id = received_data->sem_id;
  int mutex = received_data->mutex;
  int full = received_data->full;
  int empty = received_data->empty;

  int jobs_produced = 0;
  int errno;
  int job_dur;
  int next_job_produced_in;

  while(jobs_produced < num_of_jobs_per_producer) {

    job_dur = produce_job();

    errno = sem_time_wait(sem_id, empty, 20);

    if(errno == -1) {
      cout << "Producer(" << thread_id << ") The time inteval of 20 sec was "
       	   << "exceeded without any new job being consumed" << endl;
      break;
    }

    sem_wait(sem_id, mutex);
    shared_buffer[*producer_index] = job_dur;
 
    // cout << "Shared buffer current state: ";
    // for(int i = 0; i < queue_size; i++)
    //   cout << shared_buffer[i] << " ";
    // cout << endl;

    cout << "Producer(" << thread_id << ") Job id " << *producer_index
	 << " duration " << shared_buffer[*producer_index] << endl;
    (*producer_index)++;
    jobs_produced++;
    sem_signal(sem_id, mutex);
    sem_signal(sem_id, full);

    //cout << "Value of full in producer is " << semctl(sem_id, full, GETVAL) << endl;

    next_job_produced_in = rand() % 5 + 1;
    sleep(next_job_produced_in);
  }

  if(*producer_index == queue_size) {
    *producer_index = 0;
  }

  if((errno != -1) && (jobs_produced == num_of_jobs_per_producer)) {
    cout << "Producer(" << thread_id << "): No more jobs to generate." << endl;
  }

  pthread_exit(0);
}

void *consumer (void *parameter)
{
  struct consumer_thread_data *received_data;

  received_data = (struct consumer_thread_data *) parameter;

  int thread_id = received_data->thread_id;
  int queue_size = received_data->queue_size;
  int* consumer_index = received_data->consumer_index;
  int sem_id = received_data->sem_id;
  int mutex = received_data->mutex;
  int full = received_data->full;
  int empty = received_data->empty;

  int duration;
  int errno;

  while(true) {

    //sem_wait(sem_id, full);

    errno = sem_time_wait(sem_id, full, 20);
    if(errno == -1) {
      cout << "Consumer(" << thread_id << "): The time inteval of 20 sec was "
	   << "exceeded without any new job being produced" << endl;
      break;
    }

    sem_wait(sem_id, mutex);
    
    // AVOID INITIALIZING WITH 0
    duration = shared_buffer[*consumer_index];
    shared_buffer[*consumer_index] = 0;
    cout << "Consumer(" << thread_id << "): Job id "<< *consumer_index
	 << " executing sleep duration " << duration << endl;
    (*consumer_index)++;
    sem_signal(sem_id, mutex);
    sem_signal(sem_id, empty);

    if(*consumer_index == queue_size)
      *consumer_index = 0;

    // This has the meaning of "consume the removed item"
    sleep(duration);
  }

  // if the 20'' have passed withouth having any new job to consumer
  // if(errno == -1) {
  //   cout << "The time inteval of 20 sec was exceeded without any new job to be produced"
  // 	 << endl;
  // } else {
  if(errno != -1)
    cout << "Consumer (" << thread_id << "): No more jobs left" << endl;

  pthread_exit (0);
}

