#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

// Time for actions
#define RUNTIME 30
#define THINK 2
#define EAT 1

// No. of philosophers
#define N 5
// For readibility
#define LEFT (i+N-1)%N
#define RIGHT (i+1)%N

#define THINKING 0
#define HUNGRY 1
#define EATING 2

// Synchronization
void grab_forks(int i);
void put_away_forks(int i);
void test(int i);

// Philosopher Thread
void* philosopher(void* num);

// Mutexes and shared variables
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t s[N] = PTHREAD_MUTEX_INITIALIZER;
int state[N];
int meals[N];

int main(int argc, char** argv)
{
	// Start by initializing mutexes to locked state
	for(int i=0; i<N; i++)
	{
		pthread_mutex_lock(s+i);
	}
	int* num = (int*) malloc(N*sizeof(int));

	// Start Threads
	pthread_t philo[N];
	for(int i=0; i<N; i++)
	{
		num[i] = i;
		if(pthread_create(&philo[i], NULL, philosopher, (void*) (num+i)))
		{
			perror("Thread failed");
			exit(1);
		}	
	}

	// Let the simulation run
	sleep(RUNTIME);
	printf("Finishing threads and showing results\n");
	for(int i=0; i<N; i++)
	{
		// Cancel all philosophers and wait for them to exit
		pthread_cancel(philo[i]);
		pthread_join(philo[i], NULL);
		printf("Philosopher %d has left the table.\n", i);
	}
	// Print results
	pthread_mutex_lock(&m);
		printf("Meals eaten:\n");
		for(int i=0; i<N; i++)
		{
			printf("Philosopher %d: %d\n", i, meals[i]);
		}
	pthread_mutex_unlock(&m);

	free(num);

	return 0;
}

void* philosopher(void* num)
{
	int* i = (int*) num;
	printf("Philosopher %d has joined the table.\n", *i);
	while(1)
	{
		// Think
		printf("Philosopher %d is thinking...\n", *i);
		sleep(THINK);
		// Hungry
		grab_forks(*i);
		// Eating
		printf("Philosopher %d is eating...\n", *i);
		sleep(EAT);
		// Think again
		put_away_forks(*i);
	}
	return NULL;
}

void grab_forks(int i)
{
	// Become hungry and check if can eat
	pthread_mutex_lock(&m);
		state[i] = HUNGRY;
		printf("Philosopher %d is hungry and trying to take forks.\n", i);
		test(i);
	pthread_mutex_unlock(&m);

	// Take forks
	pthread_mutex_lock(&s[i]);
}

void put_away_forks(int i)
{
	pthread_mutex_lock(&m);
		// Increase meal count and start thinking
		++meals[i];
		state[i] = THINKING;
		printf("Philosopher %d has finished eating and put away his forks.\n", i);
		// Let neighbours eat
		test(LEFT);
		test(RIGHT);
	pthread_mutex_unlock(&m);
}

void test(int i)
{
	if(state[i] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING)
	{
		state[i] = EATING;
		pthread_mutex_unlock(&s[i]);
	}
}
