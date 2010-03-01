/*
www.sourceforge.net/projects/dfhack
Copyright (c) 2009 Petr Mrázek (peterix)

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/

/**
 * This is the source for the DF <-> dfhack shm bridge, server protocol part
 */
#include <stdio.h>
#include "../library/integers.h"
#include <stdlib.h>
#include <string.h>
#include <string>
//#include <unistd.h>
#include "shms.h"
#include <semaphore.h>
// various crud
extern int errorstate;
extern char *shm;
extern sem_t *mutex;
extern int shmid;

void SHM_Act (void)
{
    uint32_t numwaits = 0;
    bool suspended = 0;
    uint32_t length;
    uint32_t address;
    std::string * myStringPtr;
    if(errorstate)
    {
        return;
    }
    do {
        suspended = true;
        sem_wait(mutex);
        // we check the command contained in the shared memory
        switch (((shm_cmd *)shm)->pingpong)
        {
            // we are suspended, waiting for input
            case DFPP_RET_VERSION:
            case DFPP_RET_DATA:
            case DFPP_RET_DWORD:
            case DFPP_RET_WORD:
            case DFPP_RET_BYTE:
            case DFPP_RET_STRING:
            case DFPP_SUSPENDED:
            case DFPP_RET_PID:
            case DFPP_SV_ERROR:
                break;
                
            // suspending
            case DFPP_SUSPEND:
                full_barrier
                ((shm_cmd *)shm)->pingpong = DFPP_SUSPENDED;
                break;
                
            // client requests our process ID
            case DFPP_PID:
                ((shm_retval *)shm)->value = getPID();
                full_barrier
                ((shm_retval *)shm)->pingpong = DFPP_RET_PID;
                break;
            
            // client requests SHM bridge version
            case DFPP_VERSION:
                ((shm_retval *)shm)->value = PINGPONG_VERSION;
                full_barrier
                ((shm_retval *)shm)->pingpong = DFPP_RET_VERSION;
                break;
            
            // client wants to do a raw memory read
            case DFPP_READ:
                length = ((shm_read *)shm)->length;
                address = ((shm_read *)shm)->address;
                memcpy(shm + SHM_HEADER, (void *) address,length);
                full_barrier
                ((shm_cmd *)shm)->pingpong = DFPP_RET_DATA;
                break;
                
            // read of a DWORD
            case DFPP_READ_DWORD:
                address = ((shm_read_small *)shm)->address;
                ((shm_retval *)shm)->value = *((uint32_t*) address);
                full_barrier
                ((shm_retval *)shm)->pingpong = DFPP_RET_DWORD;
                break;
                
            // read of a WORD
            case DFPP_READ_WORD:
                address = ((shm_read_small *)shm)->address;
                ((shm_retval *)shm)->value = *((uint16_t*) address);
                full_barrier
                ((shm_retval *)shm)->pingpong = DFPP_RET_WORD;
                break;
            
            // read of a BYTE (WHAT A WASTE!)
            case DFPP_READ_BYTE:
                address = ((shm_read_small *)shm)->address;
                ((shm_retval *)shm)->value = *((uint8_t*) address);
                full_barrier
                ((shm_retval *)shm)->pingpong = DFPP_RET_BYTE;
                break;
            
            // raw write of a buffer
            case DFPP_WRITE:
                address = ((shm_write *)shm)->address;
                length = ((shm_write *)shm)->length;
                memcpy((void *)address, shm + SHM_HEADER,length);
                full_barrier
                ((shm_cmd *)shm)->pingpong = DFPP_SUSPENDED;
                break;
            
            // write of a DWORD
            case DFPP_WRITE_DWORD:
                (*(uint32_t*)((shm_write_small *)shm)->address) = ((shm_write_small *)shm)->value;
                full_barrier
                ((shm_cmd *)shm)->pingpong = DFPP_SUSPENDED;
                break;
                
            // write of a WORD
            case DFPP_WRITE_WORD:
                (*(uint16_t*)((shm_write_small *)shm)->address) = ((shm_write_small *)shm)->value;
                full_barrier
                ((shm_cmd *)shm)->pingpong = DFPP_SUSPENDED;
                break;
                
            // write of a BYTE
            case DFPP_WRITE_BYTE:
                (*(uint8_t*)((shm_write_small *)shm)->address) = ((shm_write_small *)shm)->value;
                full_barrier
                ((shm_cmd *)shm)->pingpong = DFPP_SUSPENDED;
                break;
                
            // next please.
            case DFPP_CL_ERROR:
            case DFPP_RUNNING:
                suspended = false;
                break;
            
            // we read an STL string
            case DFPP_READ_STL_STRING:
                myStringPtr = (std::string *) ((shm_read_small *)shm)->address;
                ((shm_retval *)shm)->value = myStringPtr->length();
                strncpy(shm+SHM_HEADER,myStringPtr->c_str(),myStringPtr->length()+1);// length + 1 for the null terminator
                full_barrier
                ((shm_retval *)shm)->pingpong = DFPP_RET_STRING;
                break;

            // we write an STL string
            case DFPP_WRITE_STL_STRING:
                myStringPtr = (std::string *) ((shm_write *)shm)->address;
                myStringPtr->assign((const char *) (shm + SHM_HEADER));
                full_barrier
                ((shm_cmd *)shm)->pingpong = DFPP_SUSPENDED;
                break;
            default:
                ((shm_retval *)shm)->value = DFEE_INVALID_COMMAND;
                full_barrier
                ((shm_retval *)shm)->pingpong = DFPP_SV_ERROR;
                break;
        }
        sem_post(mutex);
        if(suspended)
        {
            SCHED_YIELD;
        }
    } while (suspended);
    /*
    if(lock_held)
    {
        // wait on lock and timeout
        if(timeout)
        {
            check if SHM still valid
            if(!shm valid)
                reset lock
        }
        // acquire lock
        switch(cmd)
        {
            // blah, do stuff
        }
        // release lock
    }
    else
    {
        return;
    }
    */
    /*
    check_again: // goto target!!!
    SCHED_YIELD // yield the CPU, valid only on single-core CPUs
    if(numwaits == 10000)
    {
        // this tests if there's a process on the other side
        if(isValidSHM())
        {
            numwaits = 0;
        }
        else
        {
            full_barrier
            ((shm_cmd *)shm)->pingpong = DFPP_RUNNING;
            fprintf(stderr,"dfhack: Broke out of loop, other process disappeared.\n");
            //MessageBox(0,"Broke out of loop, other process disappeared.","FUN", MB_OK);
        }
    }
    switch (((shm_cmd *)shm)->pingpong)
    {
        case DFPP_RET_VERSION:
        case DFPP_RET_DATA:
        case DFPP_RET_DWORD:
        case DFPP_RET_WORD:
        case DFPP_RET_BYTE:
        case DFPP_RET_STRING:
        case DFPP_SUSPENDED:
        case DFPP_RET_PID:
        case DFPP_SV_ERROR:
            numwaits++;
            break;
        case DFPP_SUSPEND:
            full_barrier
            ((shm_cmd *)shm)->pingpong = DFPP_SUSPENDED;
            break;

        case DFPP_PID:
            ((shm_retval *)shm)->value = getPID();
            full_barrier
            ((shm_retval *)shm)->pingpong = DFPP_RET_PID;
            break;
            
        case DFPP_VERSION:
            ((shm_retval *)shm)->value = PINGPONG_VERSION;
            full_barrier
            ((shm_retval *)shm)->pingpong = DFPP_RET_VERSION;
            break;
            
        case DFPP_READ:
            length = ((shm_read *)shm)->length;
            address = ((shm_read *)shm)->address;
            memcpy(shm + SHM_HEADER, (void *) address,length);
            full_barrier
            ((shm_cmd *)shm)->pingpong = DFPP_RET_DATA;
            break;
            
        case DFPP_READ_DWORD:
            address = ((shm_read_small *)shm)->address;
            ((shm_retval *)shm)->value = *((uint32_t*) address);
            full_barrier
            ((shm_retval *)shm)->pingpong = DFPP_RET_DWORD;
            break;

        case DFPP_READ_WORD:
            address = ((shm_read_small *)shm)->address;
            ((shm_retval *)shm)->value = *((uint16_t*) address);
            full_barrier
            ((shm_retval *)shm)->pingpong = DFPP_RET_WORD;
            break;
            
        case DFPP_READ_BYTE:
            address = ((shm_read_small *)shm)->address;
            ((shm_retval *)shm)->value = *((uint8_t*) address);
            full_barrier
            ((shm_retval *)shm)->pingpong = DFPP_RET_BYTE;
            break;
            
        case DFPP_WRITE:
            address = ((shm_write *)shm)->address;
            length = ((shm_write *)shm)->length;
            memcpy((void *)address, shm + SHM_HEADER,length);
            full_barrier
            ((shm_cmd *)shm)->pingpong = DFPP_SUSPENDED;
            break;
            
        case DFPP_WRITE_DWORD:
            (*(uint32_t*)((shm_write_small *)shm)->address) = ((shm_write_small *)shm)->value;
            full_barrier
            ((shm_cmd *)shm)->pingpong = DFPP_SUSPENDED;
            break;

        case DFPP_WRITE_WORD:
            (*(uint16_t*)((shm_write_small *)shm)->address) = ((shm_write_small *)shm)->value;
            full_barrier
            ((shm_cmd *)shm)->pingpong = DFPP_SUSPENDED;
            break;
        
        case DFPP_WRITE_BYTE:
            (*(uint8_t*)((shm_write_small *)shm)->address) = ((shm_write_small *)shm)->value;
            full_barrier
            ((shm_cmd *)shm)->pingpong = DFPP_SUSPENDED;
            break;
            
        case DFPP_CL_ERROR:
        case DFPP_RUNNING:
            //fprintf(stderr, "no. of waits: %d\n", numwaits);
            //MessageBox(0,"Broke out of loop properly","FUN", MB_OK);
            break;

        case DFPP_READ_STL_STRING:
            myStringPtr = (std::string *) ((shm_read_small *)shm)->address;
            ((shm_retval *)shm)->value = myStringPtr->length();
            strncpy(shm+SHM_HEADER,myStringPtr->c_str(),myStringPtr->length()+1);// length + 1 for the null terminator
            full_barrier
            ((shm_retval *)shm)->pingpong = DFPP_RET_STRING;
            break;

        case DFPP_WRITE_STL_STRING:
            myStringPtr = (std::string *) ((shm_write *)shm)->address;
            myStringPtr->assign((const char *) (shm + SHM_HEADER));
            full_barrier
            ((shm_cmd *)shm)->pingpong = DFPP_SUSPENDED;
            break;


        default:
            ((shm_retval *)shm)->value = DFEE_INVALID_COMMAND;
            full_barrier
            ((shm_retval *)shm)->pingpong = DFPP_SV_ERROR;
            break;
    }
    */
}
