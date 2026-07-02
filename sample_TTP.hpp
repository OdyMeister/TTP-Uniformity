#ifndef SAMPLE_TTP_HPP
#define SAMPLE_TTP_HPP

#include <vector>
#include "TTP_helpers.hpp"

/*
 * Generate a single valid schedule given all possible matchups.
 * Stops when the first valid schedule is found.
 *
 * @param item: work_item struct containing the initial matchups, schedule, and streaks
 * @param NUM_TEAMS: number of teams in the schedule
 */
void generate_schedule(work_item item, int NUM_TEAMS);

/*
 * Generate all possible normalized schedules given all possible rounds.
 * Samples a given number of valid schedules.
 *
 * @param item: work_item struct containing the initial matchups, schedule, and streaks
 * @param NUM_TEAMS: number of teams in the schedule
 * @param NUM_SCHEDULES: number of valid schedules to sample
 */
void sample_normalized_schedules(work_item item, int NUM_TEAMS, int NUM_SCHEDULES);

/*
 * Print a given schedule to the console.
 *
 * @param schedule: vector of matchups representing the schedule
 * @param NUM_TEAMS: number of teams in the schedule
 */
void print_schedule(std::vector<matchup> &schedule, int NUM_TEAMS);
#endif