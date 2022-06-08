/*
 * SPDX-FileCopyrightText: 2020 Amazon.com, Inc. or its affiliates
 *
 * SPDX-License-Identifier: MIT
 *
 * SPDX-FileContributor: 2016-2022 Espressif Systems (Shanghai) CO LTD
 */
/*
 * FreeRTOS Kernel V10.4.3
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * https://www.FreeRTOS.org
 * https://github.com/FreeRTOS
 *
 */

/* Standard includes. */
#include <stdlib.h>
#include <pthread.h>
#include "esp_err.h"
#include "event_groups.h"
#include "audio_mem.h"
#include "esp_log.h"

static const char *TAG = "EVENTGROUPS";

typedef struct EventGroupDef_t
{
    EventBits_t uxEventBits;
    pthread_cond_t  eventGroupCond[8];
    pthread_mutex_t eventGroupMux;
} EventGroup_t;

static BaseType_t prvTestWaitCondition( const EventBits_t uxCurrentEventBits,
                                        const EventBits_t uxBitsToWaitFor,
                                        const BaseType_t xWaitForAllBits );

static BaseType_t prvTestWaitCondition(const EventBits_t uxCurrentEventBits,
                                        const EventBits_t uxBitsToWaitFor,
                                        const BaseType_t xWaitForAllBits)
{
    BaseType_t xWaitConditionMet = pdFALSE;

    if(xWaitForAllBits == pdFALSE)
    {
        /* Task only has to wait for one bit within uxBitsToWaitFor to be
         * set.  Is one already set? */
        if((uxCurrentEventBits & uxBitsToWaitFor) != (EventBits_t) 0)
        {
            xWaitConditionMet = pdTRUE;
        }
    }
    else
    {
        /* Task has to wait for all the bits in uxBitsToWaitFor to be set.
         * Are they set already? */
        if((uxCurrentEventBits & uxBitsToWaitFor) == uxBitsToWaitFor)
        {
            xWaitConditionMet = pdTRUE;
        }
    }

    return xWaitConditionMet;
}

/*-----------------------------------------------------------*/
EventGroupHandle_t xEventGroupCreate(void)
{
    EventGroup_t *pxEventBits;
    int status = 0;
    pxEventBits = (EventGroup_t *) audio_malloc(sizeof(EventGroup_t)); /*lint !e9087 !e9079 see comment above. */

    if(pxEventBits != NULL)
    {
        pxEventBits->uxEventBits = 0;

          /* Initialize the mutex */
        status = pthread_mutex_init(&pxEventBits->eventGroupMux, NULL);
        if (status != 0)
        {
            ESP_LOGE(TAG, "xEventGroupCreate : pthread_mutex_init failed, status=%d", status);
        }

        /* Initialize the condition variable */
        for (size_t i = 0; i < 8; i++)
        {
            status = pthread_cond_init(&pxEventBits->eventGroupCond[i], NULL);
            if (status != 0)
            {
                ESP_LOGE(TAG, "ERROR xEventGroupCreate : pthread_cond_init failed status=%d", status);
            }
        }
    }
    else
    {
        /*assert null */
        assert(pxEventBits != NULL);
    }
    return pxEventBits;
}

EventBits_t xEventGroupWaitBits(EventGroupHandle_t xEventGroup,
                                 const EventBits_t uxBitsToWaitFor,
                                 const BaseType_t xClearOnExit,
                                 const BaseType_t xWaitForAllBits,
                                 TickType_t xTicksToWait)
{
    EventGroup_t * pxEventBits = xEventGroup;
    EventBits_t uxReturn;
    BaseType_t xWaitConditionMet;
    int status = 0;

    assert(xEventGroup);
    assert(uxBitsToWaitFor != 0);

    if((uxBitsToWaitFor & (uxBitsToWaitFor - 1)) != 0) {
        ESP_LOGE(TAG, "ERROR xEventGroupWaitBits :Wait For  Muti Bits Is Not Implement!");
        abort();
        return ESP_FAIL;
    }

    pthread_mutex_lock(&xEventGroup->eventGroupMux);
    {
        const EventBits_t uxCurrentEventBits = pxEventBits->uxEventBits;

        /* Check to see if the wait condition is already met or not. */
        xWaitConditionMet = prvTestWaitCondition(uxCurrentEventBits, uxBitsToWaitFor, xWaitForAllBits);

        if(xWaitConditionMet != pdFALSE)
        {
            /* The wait condition has already been met so there is no need to
             * block. */
            uxReturn = uxCurrentEventBits;
            xTicksToWait = (TickType_t) 0;
            /* Clear the wait bits if requested to do so. */
            if(xClearOnExit != pdFALSE)
            {
                pxEventBits->uxEventBits &= ~uxBitsToWaitFor;
            }
        }
        else
        {
            struct timespec ts;
            clock_gettime(CLOCK_REALTIME, &ts);
            ts.tv_sec   += xTicksToWait;

            for (int i = 0; i < 8; i++) {
                if ((uxBitsToWaitFor & (1<<i)) != 0)
                {
                    if((pxEventBits->uxEventBits & (1<<i)) != 0)
                    {
                        ESP_LOGW(TAG, "xEventGroupWaitBits : the Waiting Bit  has been set");
                    }
                    status = pthread_cond_timedwait(&pxEventBits->eventGroupCond[i], &pxEventBits->eventGroupMux, &ts);
                    if (status != 0)
                    {
                        ESP_LOGE(TAG, "pthread_cond_clockwait failed, status=%d", status);
                    }
                    break;
                }
            }

            if ((pxEventBits->uxEventBits & uxBitsToWaitFor) == 0)
            {
                ESP_LOGE(TAG, "xEventGroupWaitBits :the Waiting Bit  has been set");
            }

            uxReturn = pxEventBits->uxEventBits;
            /* Clear the wait bits if requested to do so. */
            if(xClearOnExit != pdFALSE)
            {
                pxEventBits->uxEventBits &= ~uxBitsToWaitFor;
            }
        }
    }
    pthread_mutex_unlock(&xEventGroup->eventGroupMux);
    return uxReturn;
}
/*-----------------------------------------------------------*/

EventBits_t xEventGroupClearBits(EventGroupHandle_t xEventGroup,
                                  const EventBits_t uxBitsToClear)
{
    EventGroup_t * pxEventBits = xEventGroup;
    EventBits_t uxReturn;

    /* Check the user is not attempting to clear the bits used by the kernel
     * itself. */
    assert(xEventGroup);
    pthread_mutex_lock(&xEventGroup->eventGroupMux);
    {
        /* The value returned is the event group value prior to the bits being
         * cleared. */
        uxReturn = pxEventBits->uxEventBits;

        /* Clear the bits. */
        pxEventBits->uxEventBits &= ~uxBitsToClear;
    }
    pthread_mutex_unlock(&xEventGroup->eventGroupMux);
    return uxReturn;
}

EventBits_t xEventGroupSetBits(EventGroupHandle_t xEventGroup,
                                const EventBits_t uxBitsToSet)
{
    EventGroup_t * pxEventBits = xEventGroup;

    assert(xEventGroup);
    assert(uxBitsToSet != 0);

    pthread_mutex_lock(&pxEventBits->eventGroupMux);
    /* set the bits. */
    pxEventBits->uxEventBits |= uxBitsToSet;
    for(int i=0; i<8; i++){
        if((uxBitsToSet & (1<<i)) != (EventBits_t)0)
        pthread_cond_signal(&pxEventBits->eventGroupCond[i]);
    }

    pthread_mutex_unlock(&pxEventBits->eventGroupMux);
    return pxEventBits->uxEventBits;
}
/*-----------------------------------------------------------*/

void vEventGroupDelete(EventGroupHandle_t xEventGroup)
{
    if(xEventGroup == NULL) {
        ESP_LOGE(TAG, "ERROR : xEventGroup is already NULL");
        return ;
    }
    EventGroup_t * pxEventBits = xEventGroup;
    int status;
    pthread_mutex_lock(&pxEventBits->eventGroupMux);
    {
        for(int i=0; i<8; i++) {
           status = pthread_cond_destroy(&pxEventBits->eventGroupCond[i]);
            if (status != 0)
            {
                ESP_LOGE(TAG, "ERROR : pthread_cond_destroy eventGroupCond[%d] failed, status=%d\n", i, status);
            }
        }
    }
    pthread_mutex_unlock(&pxEventBits->eventGroupMux);
    pthread_mutex_destroy(&pxEventBits->eventGroupMux);
    audio_free(pxEventBits);
}
