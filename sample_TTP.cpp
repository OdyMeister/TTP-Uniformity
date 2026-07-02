#include <fstream>
#include "TTP_helpers.hpp"
#include <chrono>
#include <algorithm>
#include <random>
#include <mpi.h>
#include <math.h>
#include <iomanip>

/* The maximum value for a long long int. */
#define MAX_COUNT 9223372036854775807

/* Global variable to keep track of the number of schedules generated
 * and recursive calls made per valid schedule. */
long long int count = 0;
long long int recs = 0;
long long int cum_recs = 0;

/* Global variable to enable pure random sampling and keep track of
 * the number of sampling attempts which have been made. */
bool rand_sample = false;
long long int attempts = 0;
long long int placements = 0;

/* Adds the current time to the start of each output file. */
void add_timestamp(int num_teams, int world_rank) {
    std::ofstream output_file_node("out_" + std::to_string(num_teams) + "/out_node_" + std::to_string(world_rank) + ".txt", std::ios_base::app);
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm = *std::localtime(&now_time);
    output_file_node << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S") << "." << std::setfill('0') << std::setw(3) << ms.count() << "\n";
    output_file_node.close();
}

/* Prints the schedule to the console, for deugging */
void print_schedule(std::vector<matchup> &schedule, int num_teams) {
    printf("Schedule:\n");
    int counter = 0;
    
    for (matchup m : schedule) {
        printf("%d vs %d", m.team1, m.team2);
        if (counter % (num_teams / 2) == (num_teams / 2) - 1) {
            printf("\n");
        } else {
            printf(" | ");
        }
        counter++;
    }
}

/* Writes schedules to a file */
void write_schedule_to_file(std::vector<matchup> &schedule, int world_rank, int num_teams) {
    std::ofstream output_file_node("out_" + std::to_string(num_teams) + "/out_node_" + std::to_string(world_rank) + ".txt", std::ios_base::app);
    
    // Add timestamp for each schedule
    auto now = std::chrono::system_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm local_tm = *std::localtime(&now_time);
    output_file_node << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S") << "." << std::setfill('0') << std::setw(3) << ms.count() << " " << recs;

    if (rand_sample) {
        output_file_node << " " << attempts << "\n";
    } else {
        output_file_node << "\n";
    }
    
    // Skip the first num_teams/2 matchups (normalized first round)
    // Saves space since first round is fixed anyways
    int skip = num_teams / 2;
    for (size_t i = skip; i < schedule.size(); ++i) {
        auto &m = schedule[i];
        output_file_node << m.team1 << "," << m.team2 << " ";
    }
    output_file_node << "\n";
    output_file_node.close();
}

/* DFS traverses the search tree until a schedule is found. */
void generate_schedule(work_item item, std::vector<bool> &found, int num_teams, int num_schedules, int world_rank) {
    /* If there are no more matchups, the schedule is complete */
    if (found[world_rank] == true) {
        return;
    } else if (item.matchups.size() == 0) {
        found[world_rank] = true;
        //print_schedule(item.schedule, num_teams);
        write_schedule_to_file(item.schedule, world_rank, num_teams);
        count++;
        attempts++;
        recs = 0;
        return;
    } else if (count >= num_schedules) {
        return;
    }

    recs++;
    cum_recs++;

    /* For each round still possible, generate a new schedule */
    for (matchup m : item.matchups) {
        placements++;
        
        /* Check validity of the schedule with constraints */
        if (check_constraints_opt(item.schedule, item.matchups, item.streaks, num_teams, m)) {
            if (rand_sample) {
                found[world_rank] = true;
                attempts++;
                return;
            }
            continue;
        }

        /* Mark the matchup as processed and push to schedule */
        item.schedule.push_back(m);
        item.matchups.erase(std::remove(item.matchups.begin(), item.matchups.end(), m), item.matchups.end());

        /* Update the streaks for the two teams in the current matchup */
        streak old_streak1 = item.streaks[m.team1];
        streak old_streak2 = item.streaks[m.team2];
        update_streaks(item.streaks, m);

        /* Recursive call */
        generate_schedule(item, found, num_teams, num_schedules, world_rank);

        /* Undo streak mod */
        item.streaks[m.team1] = old_streak1;
        item.streaks[m.team2] = old_streak2;

        /* Undo matchup placement */
        item.schedule.pop_back();
        item.matchups.insert(item.matchups.begin(), m);
    }
}

void sample_normalized_schedules(work_item item, int num_teams, int num_schedules, int world_size, int world_rank) {
    std::vector<matchup> first_round = std::vector<matchup>();
    
    /* Create a list of bools sized to the number of MPI processes 
     * Keeps track of whether a schedules has been found for each node */
    std::vector<bool> found(world_size, false);

    /* Order the first round in the schedule to normalize it */
    for (int i = 0; i < num_teams; i += 2) {
        first_round.push_back({i, i + 1});
        matchup m = {i, i + 1};
        std::vector<matchup>::iterator position = std::find(item.matchups.begin(), item.matchups.end(), m);
        item.matchups.erase(position);
        update_streaks(item.streaks, {i, i + 1});
    }

    /* Copy the initial matchups */
    auto matchups_copy = item.matchups;

    /* Divide num_schedules by world_size */
    int schedules_per_node = ceil((double)num_schedules / world_size);

    /* Timestamp output file */
    add_timestamp(num_teams, world_rank);

    while (count < schedules_per_node) {
        static thread_local std::mt19937 rng(std::random_device{}());
        std::shuffle(item.matchups.begin(), item.matchups.end(), rng);
        found[world_rank] = false;
        recs = 0;
        generate_schedule({item.matchups, first_round, item.streaks}, found, num_teams, num_schedules, world_rank);
    }

    // printf("\nNode %d finished\n", world_rank);
}

int main(int argc, char* argv[]) {
    /* Initialize MPI */
    MPI_Init(NULL, NULL);

    int world_rank, world_size;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    
    /* Check if the correct number of arguments is given */
    if (argc < 2) {
        printf("Usage: ./sampler <NUM_TEAMS> <NUM_SCHEDULES>\n");
        return 1;
    }
    /* Parse the command line arguments */
    int NUM_TEAMS = atoi(argv[1]);
    int NUM_SCHEDULES = atoi(argv[2]);

    if (world_rank == 0) {
        printf("Sampling %d normalized schedules for %d teams using %d nodes.\n", NUM_SCHEDULES, NUM_TEAMS, world_size);
    }

    auto start_time = std::chrono::high_resolution_clock::now();

    /* Initialize the matchups, schedule, and streaks */
    auto matchups = std::vector<matchup>(NUM_TEAMS * (NUM_TEAMS - 1));
    auto schedule = std::vector<matchup>();
    auto streaks = std::vector<streak>(NUM_TEAMS);

    /* Generate the initial matchups */
    generate_matchups(matchups, NUM_TEAMS);

    /* Generate the initial streak count */
    generate_streak_count(streaks, NUM_TEAMS);

    /* Count the normalized schedules */
    sample_normalized_schedules({matchups, schedule, streaks}, NUM_TEAMS, NUM_SCHEDULES, world_size, world_rank);

    /* Calculate the elapsed time */
    auto end_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end_time - start_time;
    /* Print the time in HH:MM:SS:MS format */
    if (world_rank == 0) {
        printf("Time taken: %d:%d:%d:%d\n", (int)elapsed.count() / 3600, (int)elapsed.count() % 3600 / 60, (int)elapsed.count() % 60, (int)(elapsed.count() * 1000) % 1000);
    }
    // printf("Count: %lld\n", count);

    long long int total_count = 0;
    MPI_Reduce(&count, &total_count, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    if (world_rank == 0) {
        printf("Total count:\t\t %lld\n", total_count);
    }

    long long int total_recs = 0;
    MPI_Reduce(&cum_recs, &total_recs, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    if (world_rank == 0) {
        printf("Total recursive calls:\t %lld\n", total_recs);
    }

    long long int total_placements = 0;
    MPI_Reduce(&placements, &total_placements, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    if (world_rank == 0) {
        printf("Total placements:\t %lld\n", total_placements);
    }

    long long int total_attempts = 0;
    MPI_Reduce(&attempts, &total_attempts, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);
    if (world_rank == 0) {
        printf("Total attempts:\t\t %lld\n", total_attempts);
    }

    /* Finalize MPI */
    MPI_Finalize();
}