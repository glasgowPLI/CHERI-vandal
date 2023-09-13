/*
 * Copyright (c) 2023 University of Glasgow
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program. If not, see
 * <https://www.gnu.org/licenses/>.
 */

#ifndef INCLUDE_VANDAL_H
#define INCLUDE_VANDAL_H

/* Attack implicitly via all registers (except c16-c18).
 *
 * If there is a valid capability in registers c16, c17, or c18, the value -1 is returned immediaty.
 *
 * Otherwise, the attack is performed and the value 0 is returned.
 */
extern int vandalise(void);

/* Attack via an array of capabilites.
 * The 'protected' stack region (`BASE(CSP)` to `stack_guard`) will remain unaffected.
 *
 * The attack is performed and the value 0 is returned.
 *
 * targets     - array of target capabilities 
 * n           - number of target capabilities
 * stack_guard - end of 'protected' stack region */
extern int vandalise_arr(void ** targets, int n, vaddr stack_guard);

#endif

