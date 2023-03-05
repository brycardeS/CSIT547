#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define NUM_CHAIRS 3
#define NUM_STUDENTS 10

pthread_mutex_t mutex;
sem_t students_sem, ta_sem, chairs_sem;

int num_waiting = 0; // number of students waiting for help
int ta_sleeping = 1; // initial state: TA is sleeping

void* ta_thread(void* arg) {
    while (1) {
        // wait for a student to wake up the TA
        sem_wait(&ta_sem);
        ta_sleeping = 0;

        // help each waiting student
        while (num_waiting > 0) {
            // signal that a chair is now available
            sem_post(&chairs_sem);

            // start helping the next waiting student
            pthread_mutex_lock(&mutex);
            num_waiting--;
            printf("TA is helping a student (waiting students = %d)\n", num_waiting);
            pthread_mutex_unlock(&mutex);

            // help the student for a random period of time
            int help_time = rand() % 5 + 1; // between 1 and 5 seconds
            sleep(help_time);
        }

        // if no students are waiting, go back to sleep
        ta_sleeping = 1;
    }

    return NULL;
}

void* student_thread(void* arg) {
    int id = *(int*) arg;
    int programming_time, help_time;

    while (1) {
        // program for a random period of time
        programming_time = rand() % 10 + 1; // between 1 and 10 seconds
        printf("Student %d is programming for %d seconds\n", id, programming_time);
        sleep(programming_time);

        // try to get a chair in the hallway
        if (sem_trywait(&chairs_sem) == 0) {
            // if a chair is available, wait for the TA to become available
            pthread_mutex_lock(&mutex);
            num_waiting++;
            printf("Student %d is waiting for the TA (waiting students = %d)\n", id, num_waiting);
            pthread_mutex_unlock(&mutex);

            // notify the TA that a student needs help
            sem_post(&ta_sem);

            // wait for the TA to help
            sem_wait(&students_sem);

            // release the chair
            sem_post(&chairs_sem);
        } else {
            // if no chair is available, keep programming and try again later
            printf("Student %d will try again later\n", id);
        }
    }

    return NULL;
}

int main() {
    pthread_t ta_tid, student_tid[NUM_STUDENTS];
    int student_id[NUM_STUDENTS];

    // initialize synchronization primitives
    pthread_mutex_init(&mutex, NULL);
    sem_init(&students_sem, 0, 0);
    sem_init(&ta_sem, 0, 0);
    sem_init(&chairs_sem, 0, NUM_CHAIRS);

    // create the TA thread
    pthread_create(&ta_tid, NULL, ta_thread, NULL);

    // create the student threads
    for (int i = 0; i < NUM_STUDENTS; i++) {
        student_id[i] = i + 1;
        pthread_create(&student_tid[i], NULL, student_thread, &student_id[i]);
    }


    pthread_join(ta_tid, NULL);
    for (int i = 0; i < NUM_STUDENTS; i++) {
        pthread_join(student_tid[i], NULL);
    }

    // destroy synchronization primitives
    pthread_mutex_destroy(&mutex);
    sem_destroy(&students_sem);
    sem_destroy(&ta_sem);
    sem_destroy(&chairs_sem);

    return 0;
}
