/*
 *  audio_queue.h
 *
 *  Create on: 2017.02.25
 *     Author: minhua huang
 */

#ifndef AUDIO_QUEUE_H
#define AUDIO_QUEUE_H

#include <pthread.h>

typedef struct audio_buffer_t {
    /*The data buffer*/
    void            *data;
    /*Size of data in bytes*/
    unsigned int    size;
    /*Next audio_buffer pointer*/
    struct audio_buffer_t *next;
}audio_buffer;

typedef struct audio_queue_t {
    audio_buffer       *first_buf;      /*First audio buffer*/
    audio_buffer       *last_buf;       /*Last audio buffer*/
    unsigned int       count;           /*Count number of audio_buffer in the queue*/
    unsigned int       size;            /*Size of queue in bytes*/
    pthread_mutex_t    mutex;
    pthread_cond_t     cond;
}audio_queue;


int audio_queue_init(audio_queue *);

int audio_queue_put(audio_queue *, void *data, unsigned int size);

int audio_queue_get(audio_queue *, audio_buffer *);

void audio_data_free(audio_buffer *);

void audio_queue_flush(audio_queue *);

unsigned int audio_queue_count(audio_queue *);

unsigned int audio_queue_size(audio_queue *);

void audio_queue_free(audio_queue *);

#endif
