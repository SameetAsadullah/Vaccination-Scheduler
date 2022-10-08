#include <iostream>
#include <unistd.h>
#include <vector>
#include <string.h>
#include <fstream>
#include <pthread.h>
using namespace std;

// making structure
struct cust{
    string name;
    int arrival_time, time_to_vaccinate, after_time;
};

// initializing global variables
static int nVaccinated = 0, nHadToWait = 0, nFreeNurses = 0;
static pthread_mutex_t mylock;
pthread_cond_t clear = PTHREAD_COND_INITIALIZER;

// function to read from file
void readFile(string file_name, vector<cust> &customers)
{
	fstream file(file_name, ios::in);
	while (file)
	{
        cust c;
        string temp = "";
		file >> temp;
        if (temp != "") {
            c.name = temp;
            file >> temp;
            c.arrival_time = stoi(temp);
            file >> temp;
            c.time_to_vaccinate = stoi(temp);
            file >> temp;
            c.after_time = stoi(temp);
            customers.push_back(c);
        }
	}
}

// threaded function
void* customer(void *arg) {
    cust cData;
    cData = *((cust*)arg);

    // waiting for the shot
    pthread_mutex_lock(&mylock);
    cout << cData.name << " arrives at the center." << endl;
    if (nFreeNurses == 0) {
        nHadToWait++;
        pthread_cond_wait(&clear, &mylock);
    }
    nFreeNurses--;
    cout << cData.name << " is getting helped." << endl;
    pthread_mutex_unlock(&mylock);

    sleep(cData.time_to_vaccinate);
    
    // after getting the shot
    pthread_mutex_lock(&mylock);
    cout << cData.name << " got vaccinated." << endl;
    nVaccinated++;
    nFreeNurses++;
    pthread_cond_signal(&clear);
    pthread_mutex_unlock(&mylock);

    sleep(cData.after_time);
    cout << cData.name << " leaves the center." << endl;
    return 0;
}

int main (int argc, char *argv[]) {
	vector<cust> customers;
    pthread_mutex_init(&mylock, NULL);

    nFreeNurses = stoi(argv[1]);
    readFile("customers30.txt", customers);

    // creating threads
    pthread_t* thread_id = new pthread_t[customers.size()];
    for (int i = 0; i < customers.size(); ++i) {
        sleep(customers[i].arrival_time);
        pthread_create(&thread_id[i], NULL, customer, (void*)&customers[i]);
    }

    // joining threads
    for (int i = 0; i < customers.size(); ++i) {
        pthread_join(thread_id[i], NULL);
    }
    pthread_exit(NULL);
}
