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
  int sem_id;
  int mutex;
  int full;
  int empty;
};

struct consumer_thread_data {
  int thread_id;
  int queue_size;
  int sem_id;
  int* mutex;
  int* full;
  int* empty;
};

struct job_data {
  int id;
  int duration;
};

// How can I improve it?
int shared_buffer[100];


int main (int argc, char **argv)
{

  // 1(a)
  int queue_size = check_arg(argv[1]);
  int number_of_jobs_per_producer = check_arg(argv[2]);
  int number_of_producers = check_arg(argv[3]);
  int number_of_consumers = check_arg(argv[4]);

  // 1(b)
  //int shared_buffer[queue_size];

  // 1(c)
  
  // Num of each created semaphore
  int mutex = 0;
  int full = 1;
  int empty = 2;
  
  // Initial value of each created semaphore used for initialization
  //int mutex = 1;
  //int full = 0;
  //int empty = queue_size;
  
  //sem_t mutex;
  //sem_t item;
  //sem_t empty;
  
  // Creation of the 3 Semaphores that will be needed
  int sem_id;
  sem_id = sem_create(SEM_KEY, 3);
  cout << "Semaphore array created " << sem_id << endl;
  // Initialization of the Semaphores
  
  int init1 = sem_init(sem_id, mutex, 1);
  cout << "Semaphore initialized " << init1 << endl;
  int init2 = sem_init(sem_id, full, 0);
  cout << "Semaphore initialized " << init2 << endl;  
  int init3 = sem_init(sem_id, empty, queue_size);
  cout << "Semaphore initialized " << init3 << endl;

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
    producer_data[i].sem_id = sem_id;
    producer_data[i].mutex = mutex;
    producer_data[i].full = full;
    producer_data[i].empty = empty;
  
    pthread_create(&producersid[i], NULL, producer, (void*) &producer_data[i]);
  }

  pthread_join(producersid[0], NULL);

  pthread_t consumersid[number_of_consumers];

  struct consumer_thread_data consumer_data[number_of_consumers];

  for(int i = 0; i < number_of_consumers; i++) {
    consumer_data[i].thread_id = i;
    consumer_data[i].queue_size = queue_size;
    consumer_data[i].sem_id = sem_id;
    consumer_data[i].mutex = &mutex;
    consumer_data[i].full = &full;
    consumer_data[i].empty = &empty;
  
    pthread_create(&consumersid[i], NULL, consumer, (void*) &consumer_data[i]);
  }


  // 1(e)

  // Close the semaphores
  int closed = sem_close(sem_id);

  // To be deleted
  if(!closed) {
    cout << "Semaphores successfully closed" << endl;
  }

  // Ensure that all threds are done
  for(int i = 0; i < number_of_producers; i++) {
    pthread_join(producersid[i], NULL);
  }
  
  for(int i = 0; i < number_of_consumers; i++) {
    pthread_join(consumersid[i], NULL);
  }
  
  pthread_exit(0);

  //pthread_t producerid;
  //int parameter = 5;

  //pthread_create (&producerid, NULL, producer, (void *) &parameter);

  //pthread_join (producerid, NULL);

  return 0;
}

void *producer(void *parameter)
{

  //int *param = (int *) parameter;

  int jobs_produced = 0;

  struct producer_thread_data *received_data;

  received_data = (struct producer_thread_data *) parameter;

  int thread_id = received_data->thread_id;
  int num_of_jobs_per_producer = received_data->number_of_jobs_per_producer;
  int queue_size = received_data->queue_size;
  int sem_id = received_data->sem_id;
  int mutex = received_data->mutex;
  int full = received_data->full;
  int empty = received_data->empty;

  int job_dur = 2;

  while(jobs_produced < num_of_jobs_per_producer) {
    sem_wait(sem_id, empty);
    sem_wait(sem_id, mutex);
    shared_buffer[jobs_produced] = job_dur;
    jobs_produced++;
    cout << "producing... " << jobs_produced << "job " << endl;
    sem_signal(sem_id, mutex);
    sem_signal(sem_id, full);
  }

  cout << "shared buffer[0] " << shared_buffer[0] << endl;
  cout << "jobs produced " << jobs_produced << endl;
  
  pthread_exit(0);
}

void *consumer (void *id)
{
    // TODO

  pthread_exit (0);

}
