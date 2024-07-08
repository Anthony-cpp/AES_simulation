#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_TRAINS_UP 48
#define NUM_TRAINS_DOWN 48
#define NUM_STATIONS 23
#define TRAIN_CAPACITY 2500
#define DEPARTURE_INTERVAL 15
 // 15 minutes
#define SIMULATION_STEPS 720


typedef struct {
    int position;    // Current position (index in the stations array)
    int destination; // Destination position (index in the stations array)
    int passengers;
    int status;      // -1: not started, -2: finished, 0: running, >0: stopped (remaining time to resume)
    double satisfaction;
    int departure_time; // The minute the train departs
} Train;

typedef struct {
    char name[40];
    int time_to_next; // Time to next station
    int passengers;   // Daily passengers
} Station;

void initialize_trains(Train trains[], int num_trains, int start_station, int end_station, int start_time) {
    for (int i = 0; i < num_trains; i++) {
        trains[i].position = start_station;
        trains[i].destination = end_station;
        trains[i].passengers = 0; // Start with no passengers
        trains[i].status = -1; // All trains start as not started
        trains[i].satisfaction = 100.0; // Initial satisfaction
        trains[i].departure_time = start_time + i * DEPARTURE_INTERVAL; // Set departure time with interval
    }
}

void simulate_step(Train trains[], int num_trains, Station stations[], int num_stations, int current_time) {
    for (int i = 0; i < num_trains; i++) {
        if (current_time >= trains[i].departure_time && trains[i].status == -1) {
            trains[i].status = 0; // Start the train
        }
        if (trains[i].status == 0) {
            if (rand() % 100 < 1) { // 1% chance of an accident
                trains[i].status = rand() % 100 + 5; // stop the train for 20+ minutes
                for(int k = 0; k < num_trains;k++) trains[k].status = trains[i].status;
            } else {
                int direction = (trains[i].position < trains[i].destination) ? 1 : -1;
                trains[i].position += direction;
                if (trains[i].position < 0) trains[i].position = 0;
                if (trains[i].position >= num_stations) trains[i].position = num_stations - 1;

                // Calculate passengers boarding and alighting
                int station_passengers = stations[trains[i].position].passengers / (NUM_TRAINS_UP + NUM_TRAINS_DOWN); // distribute passengers across all trains
                int boarding_passengers = station_passengers;
                trains[i].passengers += boarding_passengers;
                if (trains[i].position == trains[i].destination) {
                    trains[i].status = -2; // Train has reached its destination
                }
            }
        }
    }
}

void calculate_satisfaction(Train trains[], int num_trains) {
    for (int i = 0; i < num_trains; i++) {
        if (trains[i].status == -1 || trains[i].status == -2) {
            trains[i].satisfaction = 100.0;
        } else {
            double congestion = (double)trains[i].passengers / TRAIN_CAPACITY;
            double congestion_penalty = 150 * congestion;
            double stop_penalty = trains[i].status > 0 ? trains[i].status : 0;
            trains[i].satisfaction = 100.0 - congestion_penalty - stop_penalty;
        }
    }
}

void output_results(Train trains_up[], Train trains_down[], int num_trains, double avg_satisfaction[], int steps) {
    FILE *file = fopen("simulation_results.txt", "a+");
    if (file == NULL) {
        printf("Error opening file!\n");
        return;
    }
            double sum = 0;
    for (int i = 0; i < steps; i++) {
            sum += avg_satisfaction[i];
        
        /*fprintf(file, "Step %d: Average Satisfaction = %.2f\n", i, avg_satisfaction[i]);
        fprintf(file, "  Upward Trains:\n");
        for (int j = 0; j < num_trains; j++) {
            fprintf(file, "    Train %d: Satisfaction = %.2f, Passengers = %d, Position = %d, Status = %s\n", 
                j, trains_up[j].satisfaction, trains_up[j].passengers, trains_up[j].position, 
                trains_up[j].status == 0 ? "Running" : (trains_up[j].status > 0 ? "Stopped" : (trains_up[j].status == -1 ? "Not started" : "Finished")));
        }
        fprintf(file, "  Downward Trains:\n");
        for (int j = 0; j < num_trains; j++) {
            fprintf(file, "    Train %d: Satisfaction = %.2f, Passengers = %d, Position = %d, Status = %s\n", 
                j, trains_down[j].satisfaction, trains_down[j].passengers, trains_down[j].position, 
                trains_down[j].status == 0 ? "Running" : (trains_down[j].status > 0 ? "Stopped" : (trains_down[j].status == -1 ? "Not started" : "Finished")));
        }*/
    }
    fprintf(file,"%.2f\n",sum/steps);
    fclose(file);
}

int main() {
    srand(time(0));
    Train trains_up[NUM_TRAINS_UP];
    Train trains_down[NUM_TRAINS_DOWN];
    double avg_satisfaction[SIMULATION_STEPS];

    Station stations[NUM_STATIONS] = {
        {"宇都宮", 7, 31241}, {"雀宮", 6, 4089}, {"石橋", 4, 4059}, {"自治医大", 3, 2868}, {"小金井", 6, 3351}, 
        {"小山", 5, 18839}, {"間々田", 3, 3546}, {"野木", 4, 3606}, {"古河", 6, 10812}, {"栗橋", 5, 10451},
        {"東鷲宮", 2, 8118}, {"久喜", 3, 30225}, {"新白岡", 2, 6050}, {"白岡", 4, 11104}, {"蓮田", 3, 17628},
        {"東大宮", 4, 30234}, {"土呂", 3, 13881}, {"大宮", 2, 226249}, {"さいたま新都心", 5, 49332}, 
        {"浦和", 10, 84116}, {"赤羽", 5, 86518}, {"尾久", 7, 9485}, {"上野", 10, 147777}
    };

    for(int num = 0; num < 100; num++){

    initialize_trains(trains_up, NUM_TRAINS_UP, 0, NUM_STATIONS - 1, 0); // Upward from Utsunomiya to Ueno, starting at 00:00
    initialize_trains(trains_down, NUM_TRAINS_DOWN, NUM_STATIONS - 1, 0, 0); // Downward from Ueno to Utsunomiya, starting at 00:00

    for (int i = 0; i < SIMULATION_STEPS; i++) {
        simulate_step(trains_up, NUM_TRAINS_UP, stations, NUM_STATIONS, i);
        simulate_step(trains_down, NUM_TRAINS_DOWN, stations, NUM_STATIONS, i);
        calculate_satisfaction(trains_up, NUM_TRAINS_UP);
        calculate_satisfaction(trains_down, NUM_TRAINS_DOWN);
        
        double total_satisfaction = 0;
        for (int j = 0; j < NUM_TRAINS_UP; j++) {
            total_satisfaction += trains_up[j].satisfaction;
        }
        for (int j = 0; j < NUM_TRAINS_DOWN; j++) {
            total_satisfaction += trains_down[j].satisfaction;
        }
        avg_satisfaction[i] = total_satisfaction / (NUM_TRAINS_UP + NUM_TRAINS_DOWN);
    }

    output_results(trains_up, trains_down, NUM_TRAINS_UP, avg_satisfaction, SIMULATION_STEPS);
    }

    return 0;
}
