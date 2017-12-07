/******************************************************************
 * The Main program with the two functions. A simple
 * example of creating and using a thread is provided.
 ******************************************************************/

#include "helper.h"

const int NUM_OF_INPUT_ARGUMENTS = 5;
const int TIMEOUT_WAIT_SEC = 20;
const int NUM_OF_SEMAPHORES = 3;
const int INITIAL_ARRAY_LENGTH = 500;

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

/* It is undestood that this way of creation of the array and then
 * the usage of only a small part of it is inefficient. It could have
 * been declared as a pointer and then created on the heap. However, it 
 * is believed that this is not in the scope of this exercise */

// The circular queue which will be accessed by both producers and consumers
int shared_buffer[INITIAL_ARRAY_LENGTH];

int main (int argc, char **argv)
{

  // Handling invalid number of command line arguments
  if(argc != NUM_OF_INPUT_ARGUMENTS) {
    printf("Invalid number of input parameters; Yous should give exactly 5 parameters: \n"
           "program's name, the queue's size, "
           "the number of jobs per producer, the number of producers and the number of "
           "consumers \n");
    return -1;
  }

  int queue_size = check_arg(argv[1]);
  int number_of_jobs_per_producer = check_arg(argv[2]);
  int number_of_producers = check_arg(argv[3]);
  int number_of_consumers = check_arg(argv[4]);

  // Handling invalid input values in arguments
  if(queue_size < 0) {
    printf("Invalid value in the first parameter; queue size should be a non-negative integer \n");
    return -1;
  } else if(number_of_jobs_per_producer < 0) {
    printf("Invalid value in the second parameter; number of jobs per producer should be a non-negative integer \n");
    return -1;
  } else if(number_of_producers < 0) {
    printf("Invalid value in the third parameter; number of producers should be a non-negative integer \n");
    return -1;
  } else if(number_of_consumers < 0) {
    printf("Invalid value in the fourth parameter; number of consumers should be a non-negative integer \n");
    return -1;
  }

  // Pass time as a parameter in order for srand to produce (pseudo-)random values.
  srand(time(NULL));

  // Initialize the queue with 0, representing the empty values
  for(int i = 0; i < queue_size; i++) {
    shared_buffer[i] = 0;
  }

  // Initialize indexes in order to keep track of the next item in each case
  int producer_index = 0;
  int consumer_index = 0;
  
  // Number of each created semaphore
  int mutex = 0;
  int full = 1;
  int empty = 2;

  // Creation of the 3 Semaphores that will be needed
  int sem_id;
  sem_id = sem_create(SEM_KEY, NUM_OF_SEMAPHORES);

  if(sem_id == -1) {
    printf("Error occured while creating the semaphore set");
    return -1;
  }

  // Initialization of the Semaphores
  int init1 = sem_init(sem_id, mutex, 1);
  int init2 = sem_init(sem_id, full, 0);
  int init3 = sem_init(sem_id, empty, queue_size);

  if((init1 == 0) && (init2 == 0) && (init3 == 0)) {
    //printf("Semaphores successfully created \n");
  } else {
    printf("Error occured during the semaphore's initialization \n");
    exit(-1);
  }

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

  // Ensure that all threds are done
  for(int i = 0; i < number_of_producers; i++) {
    pthread_join(producersid[i], NULL);
  }

  for(int i = 0; i < number_of_consumers; i++) {
    pthread_join(consumersid[i], NULL);
  }

  // Close the semaphores
  int closed = sem_close(sem_id);

  if(!closed) {
    //printf("Semaphores successfully destroyed \n");
  } else {
    printf("An error occured while trying to close the semaphores \n");
  }

  pthread_exit(0);

  return 0;
}

int produce_job() {
  return (rand() % 10 + 1);
}

void *producer(void *parameter)
{
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

    // Consumer producers job
    job_dur = produce_job();

    errno = sem_time_wait(sem_id, empty, TIMEOUT_WAIT_SEC);

    // If the queue is full even after the waiting time of 20", quit.
    if(errno == -1) {
      printf("Producer (%i): The time interval of 20 seconds was exceeded without"
	     "any new job being consumed \n", thread_id + 1); 
      break;
    }

    sem_wait(sem_id, mutex);

    shared_buffer[*producer_index] = job_dur;
 
    printf("Producer(%i): Job id %i duration %i \n", (thread_id + 1), *producer_index,
	   shared_buffer[*producer_index]);

    (*producer_index)++;

    if(*producer_index == queue_size) {
      *producer_index = 0;
    }

    jobs_produced++;

    sem_signal(sem_id, mutex);
    sem_signal(sem_id, full);

    next_job_produced_in = rand() % 5 + 1;

    // Each job is added every 1 to 5 seconds
    sleep(next_job_produced_in);
  }

  if((errno != -1) && (jobs_produced == num_of_jobs_per_producer)) {
    printf("Producer(%i): No more jobs to generate. \n", thread_id + 1);
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
  int id_just_finished;
  int errno;

  while(true) {

    errno = sem_time_wait(sem_id, full, TIMEOUT_WAIT_SEC);

    if(errno == -1) {
      printf("Consumer(%i): No more jobs left \n", thread_id + 1);
      break;
    }

    sem_wait(sem_id, mutex);

    duration = shared_buffer[*consumer_index];

    // If there is no job in this location (duration = 0), move to the next
    while(!duration)
      duration = shared_buffer[++*consumer_index];

    // Job removed and 0 (signal of no existing jobs) was placed for its duration
    shared_buffer[*consumer_index] = 0;
    printf("Consumer(%i): Job id %i executing sleep duration %i \n", (thread_id + 1),
	   *consumer_index, duration);
    id_just_finished = *consumer_index;

    (*consumer_index)++;

    if(*consumer_index == queue_size)
      *consumer_index = 0;

    sem_signal(sem_id, mutex);
    sem_signal(sem_id, empty);

    // This has the meaning of "consume the removed item"
    sleep(duration);
    printf("Consumer(%i): Job id %i completed \n", (thread_id + 1), id_just_finished);
  }

  pthread_exit (0);
}

