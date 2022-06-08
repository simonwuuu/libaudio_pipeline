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

#ifndef EVENT_GROUPS_H
#define EVENT_GROUPS_H

#include "portmacro.h"
#include <semaphore.h>
#include <time.h>

/**
 * @name Preprocessor aliases for individual bit positions
 *      Bits are defined here only if they are not already defined.
 * @{
 */

#ifndef BIT0

#define BIT0   0x00000001  /**< preprocessor alias for 32-bit value with bit 0 set, used to specify this single bit */
#define BIT1   0x00000002  /**< preprocessor alias for 32-bit value with bit 1 set, used to specify this single bit */
#define BIT2   0x00000004  /**< preprocessor alias for 32-bit value with bit 2 set, used to specify this single bit */
#define BIT3   0x00000008  /**< preprocessor alias for 32-bit value with bit 3 set, used to specify this single bit */
#define BIT4   0x00000010  /**< preprocessor alias for 32-bit value with bit 4 set, used to specify this single bit */
#define BIT5   0x00000020  /**< preprocessor alias for 32-bit value with bit 5 set, used to specify this single bit */
#define BIT6   0x00000040  /**< preprocessor alias for 32-bit value with bit 6 set, used to specify this single bit */
#define BIT7   0x00000080  /**< preprocessor alias for 32-bit value with bit 7 set, used to specify this single bit */
#define BIT8   0x00000100  /**< preprocessor alias for 32-bit value with bit 8 set, used to specify this single bit */
#define BIT9   0x00000200  /**< preprocessor alias for 32-bit value with bit 9 set, used to specify this single bit */
#define BIT10  0x00000400  /**< preprocessor alias for 32-bit value with bit 10 set, used to specify this single bit */
#define BIT11  0x00000800  /**< preprocessor alias for 32-bit value with bit 11 set, used to specify this single bit */
#define BIT12  0x00001000  /**< preprocessor alias for 32-bit value with bit 12 set, used to specify this single bit */
#define BIT13  0x00002000  /**< preprocessor alias for 32-bit value with bit 13 set, used to specify this single bit */
#define BIT14  0x00004000  /**< preprocessor alias for 32-bit value with bit 14 set, used to specify this single bit */
#define BIT15  0x00008000  /**< preprocessor alias for 32-bit value with bit 15 set, used to specify this single bit */
#define BIT16  0x00010000  /**< preprocessor alias for 32-bit value with bit 16 set, used to specify this single bit */
#define BIT17  0x00020000  /**< preprocessor alias for 32-bit value with bit 17 set, used to specify this single bit */
#define BIT18  0x00040000  /**< preprocessor alias for 32-bit value with bit 18 set, used to specify this single bit */
#define BIT19  0x00080000  /**< preprocessor alias for 32-bit value with bit 19 set, used to specify this single bit */
#define BIT20  0x00100000  /**< preprocessor alias for 32-bit value with bit 20 set, used to specify this single bit */
#define BIT21  0x00200000  /**< preprocessor alias for 32-bit value with bit 21 set, used to specify this single bit */
#define BIT22  0x00400000  /**< preprocessor alias for 32-bit value with bit 22 set, used to specify this single bit */
#define BIT23  0x00800000  /**< preprocessor alias for 32-bit value with bit 23 set, used to specify this single bit */
#define BIT24  0x01000000  /**< preprocessor alias for 32-bit value with bit 24 set, used to specify this single bit */
#define BIT25  0x02000000  /**< preprocessor alias for 32-bit value with bit 25 set, used to specify this single bit */
#define BIT26  0x04000000  /**< preprocessor alias for 32-bit value with bit 26 set, used to specify this single bit */
#define BIT27  0x08000000  /**< preprocessor alias for 32-bit value with bit 27 set, used to specify this single bit */
#define BIT28  0x10000000  /**< preprocessor alias for 32-bit value with bit 28 set, used to specify this single bit */
#define BIT29  0x20000000  /**< preprocessor alias for 32-bit value with bit 29 set, used to specify this single bit */
#define BIT30  0x40000000  /**< preprocessor alias for 32-bit value with bit 30 set, used to specify this single bit */
#define BIT31  0x80000000  /**< preprocessor alias for 32-bit value with bit 31 set, used to specify this single bit */

#endif  /* BIT0 et al */

#define configSUPPORT_DYNAMIC_ALLOCATION 1
#define pdFALSE                                  ( ( BaseType_t ) 0 )
#define pdTRUE                                   ( ( BaseType_t ) 1 )
/* *INDENT-OFF* */
#ifdef __cplusplus
    extern "C" {
#endif
/* *INDENT-ON* */

/**
 * An event group is a collection of bits to which an application can assign a
 * meaning.  For example, an application may create an event group to convey
 * the status of various CAN bus related events in which bit 0 might mean "A CAN
 * message has been received and is ready for processing", bit 1 might mean "The
 * application has queued a message that is ready for sending onto the CAN
 * network", and bit 2 might mean "It is time to send a SYNC message onto the
 * CAN network" etc.  A task can then test the bit values to see which events
 * are active, and optionally enter the Blocked state to wait for a specified
 * bit or a group of specified bits to be active.  To continue the CAN bus
 * example, a CAN controlling task can enter the Blocked state (and therefore
 * not consume any processing time) until either bit 0, bit 1 or bit 2 are
 * active, at which time the bit that was actually active would inform the task
 * which action it had to take (process a received message, send a message, or
 * send a SYNC).
 *
 * The event groups implementation contains intelligence to avoid race
 * conditions that would otherwise occur were an application to use a simple
 * variable for the same purpose.  This is particularly important with respect
 * to when a bit within an event group is to be cleared, and when bits have to
 * be set and then tested atomically - as is the case where event groups are
 * used to create a synchronisation point between multiple tasks (a
 * 'rendezvous').
 *
 * @cond !DOC_SINGLE_GROUP
 * \defgroup EventGroup EventGroup
 * @endcond
 */



/**
 * event_groups.h
 *
 * Type by which event groups are referenced.  For example, a call to
 * xEventGroupCreate() returns an EventGroupHandle_t variable that can then
 * be used as a parameter to other event group functions.
 *
 * @cond !DOC_SINGLE_GROUP
 * \defgroup EventGroupHandle_t EventGroupHandle_t
 * @endcond
 * \ingroup EventGroup
 */
struct EventGroupDef_t;
typedef struct EventGroupDef_t   * EventGroupHandle_t;

/*
 * The type that holds event bits always matches TickType_t - therefore the
 * number of bits it holds is set by configUSE_16_BIT_TICKS (16 bits if set to 1,
 * 32 bits if set to 0.
 *
 * @cond !DOC_SINGLE_GROUP
 * \defgroup EventBits_t EventBits_t
 * @endcond
 * \ingroup EventGroup
 */
typedef TickType_t               EventBits_t;

/**
 * @cond !DOC_EXCLUDE_HEADER_SECTION
 * event_groups.h
 * @code{c}
 * EventGroupHandle_t xEventGroupCreate( void );
 * @endcode
 * @endcond
 *
 * Create a new event group.
 *
 * Internally, within the FreeRTOS implementation, event groups use a [small]
 * block of memory, in which the event group's structure is stored.  If an event
 * groups is created using xEventGroupCreate() then the required memory is
 * automatically dynamically allocated inside the xEventGroupCreate() function.
 * (see https://www.FreeRTOS.org/a00111.html).  If an event group is created
 * using xEventGroupCreateStatic() then the application writer must instead
 * provide the memory that will get used by the event group.
 * xEventGroupCreateStatic() therefore allows an event group to be created
 * without using any dynamic memory allocation.
 *
 * Although event groups are not related to ticks, for internal implementation
 * reasons the number of bits available for use in an event group is dependent
 * on the configUSE_16_BIT_TICKS setting in FreeRTOSConfig.h.  If
 * configUSE_16_BIT_TICKS is 1 then each event group contains 8 usable bits (bit
 * 0 to bit 7).  If configUSE_16_BIT_TICKS is set to 0 then each event group has
 * 24 usable bits (bit 0 to bit 23).  The EventBits_t type is used to store
 * event bits within an event group.
 *
 * @return If the event group was created then a handle to the event group is
 * returned.  If there was insufficient FreeRTOS heap available to create the
 * event group then NULL is returned.  See https://www.FreeRTOS.org/a00111.html
 *
 * Example usage:
 * @code{c}
 *  // Declare a variable to hold the created event group.
 *  EventGroupHandle_t xCreatedEventGroup;
 *
 *  // Attempt to create the event group.
 *  xCreatedEventGroup = xEventGroupCreate();
 *
 *  // Was the event group created successfully?
 *  if( xCreatedEventGroup == NULL )
 *  {
 *      // The event group was not created because there was insufficient
 *      // FreeRTOS heap available.
 *  }
 *  else
 *  {
 *      // The event group was created.
 *  }
 * @endcode
 * @cond !DOC_SINGLE_GROUP
 * \defgroup xEventGroupCreate xEventGroupCreate
 * @endcond
 * \ingroup EventGroup
 */

EventGroupHandle_t xEventGroupCreate( void );

/**
 * @cond !DOC_EXCLUDE_HEADER_SECTION
 * event_groups.h
 * @code{c}
 *  EventBits_t xEventGroupWaitBits(    EventGroupHandle_t xEventGroup,
 *                                      const EventBits_t uxBitsToWaitFor,
 *                                      const BaseType_t xClearOnExit,
 *                                      const BaseType_t xWaitForAllBits,
 *                                      const TickType_t xTicksToWait );
 * @endcode
 * @endcond
 *
 * [Potentially] block to wait for one or more bits to be set within a
 * previously created event group.
 *
 * This function cannot be called from an interrupt.
 *
 * @param xEventGroup The event group in which the bits are being tested.  The
 * event group must have previously been created using a call to
 * xEventGroupCreate().
 *
 * @param uxBitsToWaitFor A bitwise value that indicates the bit or bits to test
 * inside the event group.  For example, to wait for bit 0 and/or bit 2 set
 * uxBitsToWaitFor to 0x05.  To wait for bits 0 and/or bit 1 and/or bit 2 set
 * uxBitsToWaitFor to 0x07.  Etc.
 *
 * @param xClearOnExit If xClearOnExit is set to pdTRUE then any bits within
 * uxBitsToWaitFor that are set within the event group will be cleared before
 * xEventGroupWaitBits() returns if the wait condition was met (if the function
 * returns for a reason other than a timeout).  If xClearOnExit is set to
 * pdFALSE then the bits set in the event group are not altered when the call to
 * xEventGroupWaitBits() returns.
 *
 * @param xWaitForAllBits If xWaitForAllBits is set to pdTRUE then
 * xEventGroupWaitBits() will return when either all the bits in uxBitsToWaitFor
 * are set or the specified block time expires.  If xWaitForAllBits is set to
 * pdFALSE then xEventGroupWaitBits() will return when any one of the bits set
 * in uxBitsToWaitFor is set or the specified block time expires.  The block
 * time is specified by the xTicksToWait parameter.
 *
 * @param xTicksToWait The maximum amount of time (specified in 'ticks') to wait
 * for one/all (depending on the xWaitForAllBits value) of the bits specified by
 * uxBitsToWaitFor to become set.
 *
 * @return The value of the event group at the time either the bits being waited
 * for became set, or the block time expired.  Test the return value to know
 * which bits were set.  If xEventGroupWaitBits() returned because its timeout
 * expired then not all the bits being waited for will be set.  If
 * xEventGroupWaitBits() returned because the bits it was waiting for were set
 * then the returned value is the event group value before any bits were
 * automatically cleared in the case that xClearOnExit parameter was set to
 * pdTRUE.
 *
 * Example usage:
 * @code{c}
 * #define BIT_0 ( 1 << 0 )
 * #define BIT_4 ( 1 << 4 )
 *
 * void aFunction( EventGroupHandle_t xEventGroup )
 * {
 * EventBits_t uxBits;
 * const TickType_t xTicksToWait = 100 / portTICK_PERIOD_MS;
 *
 *      // Wait a maximum of 100ms for either bit 0 or bit 4 to be set within
 *      // the event group.  Clear the bits before exiting.
 *      uxBits = xEventGroupWaitBits(
 *                  xEventGroup,    // The event group being tested.
 *                  BIT_0 | BIT_4,  // The bits within the event group to wait for.
 *                  pdTRUE,         // BIT_0 and BIT_4 should be cleared before returning.
 *                  pdFALSE,        // Don't wait for both bits, either bit will do.
 *                  xTicksToWait ); // Wait a maximum of 100ms for either bit to be set.
 *
 *      if( ( uxBits & ( BIT_0 | BIT_4 ) ) == ( BIT_0 | BIT_4 ) )
 *      {
 *          // xEventGroupWaitBits() returned because both bits were set.
 *      }
 *      else if( ( uxBits & BIT_0 ) != 0 )
 *      {
 *          // xEventGroupWaitBits() returned because just BIT_0 was set.
 *      }
 *      else if( ( uxBits & BIT_4 ) != 0 )
 *      {
 *          // xEventGroupWaitBits() returned because just BIT_4 was set.
 *      }
 *      else
 *      {
 *          // xEventGroupWaitBits() returned because xTicksToWait ticks passed
 *          // without either BIT_0 or BIT_4 becoming set.
 *      }
 * }
 * @endcode
 * @cond !DOC_SINGLE_GROUP
 * \defgroup xEventGroupWaitBits xEventGroupWaitBits
 * @endcond
 * \ingroup EventGroup
 */
EventBits_t xEventGroupWaitBits( EventGroupHandle_t xEventGroup,
                                 const EventBits_t uxBitsToWaitFor,
                                 const BaseType_t xClearOnExit,
                                 const BaseType_t xWaitForAllBits,
                                 TickType_t xTicksToWait );

/**
 * @cond !DOC_EXCLUDE_HEADER_SECTION
 * event_groups.h
 * @code{c}
 *  EventBits_t xEventGroupClearBits( EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToClear );
 * @endcode
 * @endcond
 *
 * Clear bits within an event group.  This function cannot be called from an
 * interrupt.
 *
 * @param xEventGroup The event group in which the bits are to be cleared.
 *
 * @param uxBitsToClear A bitwise value that indicates the bit or bits to clear
 * in the event group.  For example, to clear bit 3 only, set uxBitsToClear to
 * 0x08.  To clear bit 3 and bit 0 set uxBitsToClear to 0x09.
 *
 * @return The value of the event group before the specified bits were cleared.
 *
 * Example usage:
 * @code{c}
 * #define BIT_0 ( 1 << 0 )
 * #define BIT_4 ( 1 << 4 )
 *
 * void aFunction( EventGroupHandle_t xEventGroup )
 * {
 * EventBits_t uxBits;
 *
 *      // Clear bit 0 and bit 4 in xEventGroup.
 *      uxBits = xEventGroupClearBits(
 *                              xEventGroup,    // The event group being updated.
 *                              BIT_0 | BIT_4 );// The bits being cleared.
 *
 *      if( ( uxBits & ( BIT_0 | BIT_4 ) ) == ( BIT_0 | BIT_4 ) )
 *      {
 *          // Both bit 0 and bit 4 were set before xEventGroupClearBits() was
 *          // called.  Both will now be clear (not set).
 *      }
 *      else if( ( uxBits & BIT_0 ) != 0 )
 *      {
 *          // Bit 0 was set before xEventGroupClearBits() was called.  It will
 *          // now be clear.
 *      }
 *      else if( ( uxBits & BIT_4 ) != 0 )
 *      {
 *          // Bit 4 was set before xEventGroupClearBits() was called.  It will
 *          // now be clear.
 *      }
 *      else
 *      {
 *          // Neither bit 0 nor bit 4 were set in the first place.
 *      }
 * }
 * @endcode
 * @cond !DOC_SINGLE_GROUP
 * \defgroup xEventGroupClearBits xEventGroupClearBits
 * @endcond
 * \ingroup EventGroup
 */
EventBits_t xEventGroupClearBits( EventGroupHandle_t xEventGroup,
                                  const EventBits_t uxBitsToClear );

/**
 * @cond !DOC_EXCLUDE_HEADER_SECTION
 * event_groups.h
 * @code{c}
 *  EventBits_t xEventGroupSetBits( EventGroupHandle_t xEventGroup, const EventBits_t uxBitsToSet );
 * @endcode
 * @endcond
 *
 * Set bits within an event group.
 * This function cannot be called from an interrupt.  xEventGroupSetBitsFromISR()
 * is a version that can be called from an interrupt.
 *
 * Setting bits in an event group will automatically unblock tasks that are
 * blocked waiting for the bits.
 *
 * @param xEventGroup The event group in which the bits are to be set.
 *
 * @param uxBitsToSet A bitwise value that indicates the bit or bits to set.
 * For example, to set bit 3 only, set uxBitsToSet to 0x08.  To set bit 3
 * and bit 0 set uxBitsToSet to 0x09.
 *
 * @return The value of the event group at the time the call to
 * xEventGroupSetBits() returns.  There are two reasons why the returned value
 * might have the bits specified by the uxBitsToSet parameter cleared.  First,
 * if setting a bit results in a task that was waiting for the bit leaving the
 * blocked state then it is possible the bit will be cleared automatically
 * (see the xClearBitOnExit parameter of xEventGroupWaitBits()).  Second, any
 * unblocked (or otherwise Ready state) task that has a priority above that of
 * the task that called xEventGroupSetBits() will execute and may change the
 * event group value before the call to xEventGroupSetBits() returns.
 *
 * Example usage:
 * @code{c}
 * #define BIT_0 ( 1 << 0 )
 * #define BIT_4 ( 1 << 4 )
 *
 * void aFunction( EventGroupHandle_t xEventGroup )
 * {
 * EventBits_t uxBits;
 *
 *      // Set bit 0 and bit 4 in xEventGroup.
 *      uxBits = xEventGroupSetBits(
 *                          xEventGroup,    // The event group being updated.
 *                          BIT_0 | BIT_4 );// The bits being set.
 *
 *      if( ( uxBits & ( BIT_0 | BIT_4 ) ) == ( BIT_0 | BIT_4 ) )
 *      {
 *          // Both bit 0 and bit 4 remained set when the function returned.
 *      }
 *      else if( ( uxBits & BIT_0 ) != 0 )
 *      {
 *          // Bit 0 remained set when the function returned, but bit 4 was
 *          // cleared.  It might be that bit 4 was cleared automatically as a
 *          // task that was waiting for bit 4 was removed from the Blocked
 *          // state.
 *      }
 *      else if( ( uxBits & BIT_4 ) != 0 )
 *      {
 *          // Bit 4 remained set when the function returned, but bit 0 was
 *          // cleared.  It might be that bit 0 was cleared automatically as a
 *          // task that was waiting for bit 0 was removed from the Blocked
 *          // state.
 *      }
 *      else
 *      {
 *          // Neither bit 0 nor bit 4 remained set.  It might be that a task
 *          // was waiting for both of the bits to be set, and the bits were
 *          // cleared as the task left the Blocked state.
 *      }
 * }
 * @endcode
 * @cond !DOC_SINGLE_GROUP
 * \defgroup xEventGroupSetBits xEventGroupSetBits
 * @endcond
 * \ingroup EventGroup
 */
EventBits_t xEventGroupSetBits( EventGroupHandle_t xEventGroup,
                                const EventBits_t uxBitsToSet );


/**
 * @cond !DOC_EXCLUDE_HEADER_SECTION
 * event_groups.h
 * @code{c}
 *  EventBits_t xEventGroupGetBits( EventGroupHandle_t xEventGroup );
 * @endcode
 * @endcond
 *
 * Returns the current value of the bits in an event group.  This function
 * cannot be used from an interrupt.
 *
 * @param xEventGroup The event group being queried.
 *
 * @return The event group bits at the time xEventGroupGetBits() was called.
 *
 * @cond !DOC_SINGLE_GROUP
 * \defgroup xEventGroupGetBits xEventGroupGetBits
 * @endcond
 * \ingroup EventGroup
 */
#define xEventGroupGetBits( xEventGroup )    xEventGroupClearBits( xEventGroup, 0 )

/**
 * @cond !DOC_EXCLUDE_HEADER_SECTION
 * event_groups.h
 * @code{c}
 *  void xEventGroupDelete( EventGroupHandle_t xEventGroup );
 * @endcode
 * @endcond
 *
 * Delete an event group that was previously created by a call to
 * xEventGroupCreate().  Tasks that are blocked on the event group will be
 * unblocked and obtain 0 as the event group's value.
 *
 * @param xEventGroup The event group being deleted.
 */
void vEventGroupDelete( EventGroupHandle_t xEventGroup );

/** @endcond */

/* *INDENT-OFF* */
#ifdef __cplusplus
    }
#endif
/* *INDENT-ON* */

#endif /* EVENT_GROUPS_H */
