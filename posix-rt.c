#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define NUM_CHAIRS 3
#define NUM_STUDENTS 10

pthread_mutex_t mutex;
sem_t students_sem, ta_sem, chairs_sem;

int num_waiting = 0; 
int ta_sleeping = 1; 

void* ta_thread(void* arg) {
    while (1) {
       
        sem_wait(&ta_sem);
        ta_sleeping = 0;

        
        while (num_waiting > 0) {
            
            sem_post(&chairs_sem);

            
            pthread_mutex_lock(&mutex);
            num_waiting--;
            printf("TA is helping a student (waiting students = %d)\n", num_waiting);
            pthread_mutex_unlock(&mutex);

            
            int help_time = rand() % 5 + 1;
            sleep(help_time);
        }

        
        ta_sleeping = 1;
    }

    return NULL;
}

void* student_thread(void* arg) {
    int id = *(int*) arg;
    int programming_time, help_time;

    while (1) {
        
        programming_time = rand() % 10 + 1; 
        printf("Student %d is programming for %d seconds\n", id, programming_time);
        sleep(programming_time);

        
        if (sem_trywait(&chairs_sem) == 0) {
            
            pthread_mutex_lock(&mutex);
            num_waiting++;
            printf("Student %d is waiting for the TA (waiting students = %d)\n", id, num_waiting);
            pthread_mutex_unlock(&mutex);

            
            sem_post(&ta_sem);

            
            sem_wait(&students_sem);

            
            sem_post(&chairs_sem);
        } else {
        
            printf("Student %d will try again later\n", id);
        }
    }

    return NULL;
}

int main() {
    pthread_t ta_tid, student_tid[NUM_STUDENTS];
    int student_id[NUM_STUDENTS];

    
    pthread_mutex_init(&mutex, NULL);
    sem_init(&students_sem, 0, 0);
    sem_init(&ta_sem, 0, 0);
    sem_init(&chairs_sem, 0, NUM_CHAIRS);

    
    pthread_create(&ta_tid, NULL, ta_thread, NULL);

    
    for (int i = 0; i < NUM_STUDENTS; i++) {
        student_id[i] = i + 1;
        pthread_create(&student_tid[i], NULL, student_thread, &student_id[i]);
    }


    pthread_join(ta_tid, NULL);
    for (int i = 0; i < NUM_STUDENTS; i++) {
        pthread_join(student_tid[i], NULL);
    }

    
    pthread_mutex_destroy(&mutex);
    sem_destroy(&students_sem);
    sem_destroy(&ta_sem);
    sem_destroy(&chairs_sem);

    return 0;
}
