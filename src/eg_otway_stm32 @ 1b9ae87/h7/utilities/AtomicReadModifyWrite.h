/////////////////////////////////////////////////////////////////////////////////////////////
// (c) eg technology ltd, 2024.  All rights reserved.
//
// This software is the property of eg technology ltd. and may not be copied or reproduced
// otherwise than on to a single hard disk for backup or archival purposes. The source code 
// is confidential information and must not be disclosed to third parties or used without 
// the express written permission of eg technology ltd.
/////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <cstdint>
#include "stm32h7xx_hal.h"

namespace eg
{
	template<typename T, typename Operation>
	T atomic_read_modify_write(volatile T* addr, Operation operation) {
		static_assert(sizeof(T) == 4, "Atomic operations are only supported on 32-bit types.");

		uint32_t atomically_written;
		T current_value;
		T new_value;

		do {
			current_value = static_cast<T>(__LDREXW(reinterpret_cast<volatile uint32_t *>(addr))); // Load exclusive
			new_value = operation(current_value); // Apply operation
			atomically_written = __STREXW(static_cast<uint32_t>(new_value), reinterpret_cast<volatile uint32_t*>(addr)); // Store exclusive
	    } while (atomically_written != 0);  // Retry if the store failed
			
		return current_value; // return the value the operation was applied to. NB this may have changed if a non-atomic iteration occured in the do while loop
	}	

	template<typename T, typename Test, typename Operation>
	bool atomic_test_and_read_modify_write(volatile T* addr, Test test, Operation operation) {
		static_assert(sizeof(T) == 4, "Atomic operations are only supported on 32-bit types.");

		uint32_t atomically_written;
		bool actioned = false;
		T current_value;
		T new_value;

		do {
			current_value = static_cast<T>(__LDREXW(reinterpret_cast<volatile uint32_t *>(addr))); // Load exclusive
			if (test(current_value)) // Test if the operation should be performed
			{
				new_value = operation(current_value); // Apply operation					
				actioned = true;
			}
			else
			{
				// If the test failed, clear the exclusive write and return
				__CLREX();
				break;
			}
		    atomically_written = __STREXW(static_cast<uint32_t>(new_value), reinterpret_cast<volatile uint32_t*>(addr)); // Store exclusive
		} while (atomically_written != 0);  // Retry if the store failed
			
		return actioned;
	}	
}
