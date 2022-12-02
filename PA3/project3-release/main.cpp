/*
Main program for the virtual memory project.
Make all of your modifications to this file.
You may add or rearrange any code or data as you need.
The header files page_table.h and disk.h explain
how to use the page table and disk interfaces.
*/

#include "page_table.h"
#include "disk.h"
#include "program.h"

#include <cassert>
#include <iostream>
#include <string.h>
#include <queue>
#include <vector>
#include <algorithm>
#include <map>
#include <time.h>

using namespace std;

// Prototype for test program
typedef void (*program_f)(char *data, int length);


// Number of physical frames
int nframes;

// frameTabel generation:
int *frameTable;
//int frameTable[nframes] = {}

// Pointer to disk for access from handlers
struct disk *disk = nullptr;

queue<int> free_frames;    //queue of free frames
map<int, int> frame_table; //will give page number of frame
//FIFO
queue<int> q; //queue to keep track of entries order of insertion

//stats
int num_faults;
int num_reads;
int num_writes;

//"CLEAN FIFO"
vector<int> readQ;
vector<int> readWriteQ;

//HELPERS FOR VECTORS
int removeFromQ(int page, vector<int>& Q)
{
	if(find(Q.begin(), Q.end(), page) != Q.end())
	{
		for (vector<int>::iterator iter = Q.begin(); iter != Q.end(); ++iter)
		{
			if (*iter == page)
			{
				Q.erase(iter);
				return true;
			}
		}
	}
	return false;
}

void addToQ(int page, vector<int>& Q)
{
    Q.insert(Q.end(), page);
}

// Simple handler for pages == frames
void page_fault_handler_example(struct page_table *pt, int page) {
	//cout << "page fault on page #" << page << endl;

	// Print the page table contents
	//cout << "Before ---------------------------" << endl;
	page_table_print(pt);
	//cout << "----------------------------------" << endl;

	// Map the page to the same frame number and set to read/write
	// TODO - Disable exit and enable page table update for example
	//exit(1);
	page_table_set_entry(pt, page, page, PROT_READ | PROT_WRITE);

	// Print the page table contents
	//cout << "After ----------------------------" << endl;
	//page_table_print(pt);
	//cout << "----------------------------------" << endl;
}

void page_fault_handler_fifo(struct page_table *pt, int page) {
	//cout << "page fault on page #" << page << endl;

	// Print the page table contents
	//cout << "Before ---------------------------" << endl;
	//page_table_print(pt);
	//cout << "----------------------------------" << endl;

	// Map the page to the same frame number and set to read/write
	int* retFrame = new int;
	int* retBits = new int;
	page_table_get_entry(pt, page, retFrame, retBits);

	//page does not have a frame
	if(*retBits == PROT_NONE)
	{
		num_faults++;
		//cout << "Page has no frame." << endl;

		//assign a frame and read in from disk
		//we need a victim
		if(free_frames.empty())
		{
			//cout << "Victim needed." << endl;

			//find a victim and replace it
			int victim_frame = q.front();
			q.pop();

			int victimPage = frame_table[victim_frame];
			int* victimRetFrame = new int;
			int* victimBits = new int;
			page_table_get_entry(pt, victimPage, victimRetFrame, victimBits);

			assert(victim_frame == *victimRetFrame);

			//check dirty bit and write to disk if required
			if(*victimBits & PROT_WRITE)
			{
				num_writes++;
				disk_write(disk, victimPage, page_table_get_physmem(pt) + victim_frame * PAGE_SIZE);
			}
			page_table_set_entry(pt, victimPage, 0, 0);

			//cout << "Setting page " << page << " to frame " << victim_frame << endl;
			num_reads++;
			disk_read(disk, page, page_table_get_physmem(pt) + victim_frame * PAGE_SIZE);
			page_table_set_entry(pt, page, victim_frame, PROT_READ);

			frame_table[victim_frame] = page;
			q.push(victim_frame);

			delete victimRetFrame;
			delete victimBits;
		}
		//assign the page to the free frame
		else
		{
			//cout << "Free frame found." << endl;
			int free_frame = free_frames.front();
			free_frames.pop();


			q.push(free_frame);
			frame_table[free_frame] = page;

			page_table_set_entry(pt, page, free_frame, PROT_READ);
			num_reads++;
			disk_read(disk, page, page_table_get_physmem(pt) + (free_frame * PAGE_SIZE));
		}
	}
	else if(*retBits & PROT_READ)
	{
		//cout << "Page need write access." << endl;
		page_table_set_entry(pt, page, *retFrame, PROT_READ|PROT_WRITE);
	}
	else
	{
		cerr << "ERROR: page had unexpected permission bits values" << endl;
		exit(1);
	}

	// Print the page table contents
	//cout << "After ----------------------------" << endl;
	//page_table_print(pt);
	//cout << "----------------------------------" << endl;

	delete retFrame;
	delete retBits;
}

void page_fault_handler_rand(struct page_table *pt, int page) {
	//cout << "page fault on page #" << page << endl;

	// Print the page table contents
	//cout << "Before ---------------------------" << endl;
	//page_table_print(pt);
	//cout << "----------------------------------" << endl;

	// Map the page to the same frame number and set to read/write
	int* retFrame = new int;
	int* retBits = new int;
	page_table_get_entry(pt, page, retFrame, retBits);

	//page does not have a frame
	if(*retBits == PROT_NONE)
	{
		num_faults++;
		//cout << "Page has no frame." << endl;

		//assign a frame and read in from disk
		//we need a victim
		if(free_frames.empty())
		{
			//cout << "Victim needed." << endl;

			//find a victim and replace it
			srand(time(0));
			int victim_frame = rand() % nframes;
			

			int victimPage = frame_table[victim_frame];
			int* victimRetFrame = new int;
			int* victimBits = new int;
			page_table_get_entry(pt, victimPage, victimRetFrame, victimBits);

			assert(victim_frame == *victimRetFrame);

			//check dirty bit and write to disk if required
			if(*victimBits & PROT_WRITE)
			{
				num_writes++;
				disk_write(disk, victimPage, page_table_get_physmem(pt) + victim_frame * PAGE_SIZE);
			}
			page_table_set_entry(pt, victimPage, 0, 0);

			//cout << "Setting page " << page << " to frame " << victim_frame << endl;
			num_reads++;
			disk_read(disk, page, page_table_get_physmem(pt) + victim_frame * PAGE_SIZE);
			page_table_set_entry(pt, page, victim_frame, PROT_READ);

			frame_table[victim_frame] = page;
			

			delete victimRetFrame;
			delete victimBits;
		}
		//assign the page to the free frame
		else
		{
			//cout << "Free frame found." << endl;
			int free_frame = free_frames.front();
			free_frames.pop();

			frame_table[free_frame] = page;

			page_table_set_entry(pt, page, free_frame, PROT_READ);
			num_reads++;
			disk_read(disk, page, page_table_get_physmem(pt) + (free_frame * PAGE_SIZE));
		}
	}
	else if(*retBits & PROT_READ)
	{

		page_table_set_entry(pt, page, *retFrame, PROT_READ|PROT_WRITE);
	}
	else
	{
		cerr << "ERROR: page had unexpected permission bits values" << endl;
		exit(1);
	}

	// Print the page table contents
	//cout << "After ----------------------------" << endl;
	//page_table_print(pt);
	//cout << "----------------------------------" << endl;

	delete retFrame;
	delete retBits;
}

void page_fault_handler_cleanFIFO(struct page_table *pt, int page) {
	//cout << "page fault on page #" << page << endl;

	// Print the page table contents
	//cout << "Before ---------------------------" << endl;
	//page_table_print(pt);
	//cout << "----------------------------------" << endl;

	// Map the page to the same frame number and set to read/write
	int* retFrame = new int;
	int* retBits = new int;
	page_table_get_entry(pt, page, retFrame, retBits);

	//page does not have a frame
	if(*retBits == PROT_NONE)
	{
		num_faults++;
		//cout << "Page has no frame." << endl;

		//assign a frame and read in from disk
		//we need a victim
		if(free_frames.empty())
		{
			//cout << "Victim needed." << endl;
			bool useRQ = !(readQ.empty());
			bool useRWQ = !(readWriteQ.empty());
			assert(useRQ || useRWQ);

			//find a victim and replace it
			int victim_page;
			if(useRQ)
			{
				victim_page = readQ[0];
				readQ.erase(readQ.begin());
			}
			else if(useRWQ)
			{
				victim_page = readWriteQ[0];
				readWriteQ.erase(readWriteQ.begin());
			}
			else
			{
				cerr << "ERROR: handler could not get a page from either FIFO queues" << endl;
				exit(1);
			}

			int* victimRetFrame = new int;
			int* victimBits = new int;
			page_table_get_entry(pt, victim_page, victimRetFrame, victimBits);

			assert(frame_table[*victimRetFrame] == victim_page);

			//check dirty bit and write to disk if required
			//this should only trigger if we pulled a victim from the readWriteQueue
			if(*victimBits & PROT_WRITE)
			{
				assert((!useRQ) && useRWQ);
				num_writes++;
				disk_write(disk, victim_page, page_table_get_physmem(pt) + *victimRetFrame * PAGE_SIZE);
			}
			page_table_set_entry(pt, victim_page, 0, 0);

			//cout << "Setting page " << page << " to frame " << victim_frame << endl;
			num_reads++;
			disk_read(disk, page, page_table_get_physmem(pt) + *victimRetFrame * PAGE_SIZE);
			page_table_set_entry(pt, page, *victimRetFrame, PROT_READ);

			frame_table[*victimRetFrame] = page;
			addToQ(page, readQ);

			delete victimRetFrame;
			delete victimBits;
		}
		//assign the page to the free frame
		else
		{
			//cout << "Free frame found." << endl;
			int free_frame = free_frames.front();
			free_frames.pop();

			addToQ(page, readQ);
			frame_table[free_frame] = page;

			page_table_set_entry(pt, page, free_frame, PROT_READ);
			num_reads++;
			disk_read(disk, page, page_table_get_physmem(pt) + (free_frame * PAGE_SIZE));
		}
	}
	else if(*retBits & PROT_READ)
	{
		//cout << "Page need write access." << endl;
		bool removed = removeFromQ(page, readQ);
		if(removed)
		{
			addToQ(page, readWriteQ);
			page_table_set_entry(pt, page, *retFrame, PROT_READ|PROT_WRITE);
		}
		else
		{
			cerr << "ERROR: could not remove page from read queue" << endl;
			exit(1);
		}
	}
	else
	{
		cerr << "ERROR: page had unexpected permission bits values" << endl;
		exit(1);
	}

	// Print the page table contents
	//cout << "After ----------------------------" << endl;
	//page_table_print(pt);
	//cout << "----------------------------------" << endl;

	delete retFrame;
	delete retBits;
}

int main(int argc, char *argv[]) {
	// Check argument count
	if (argc != 5) {
		cerr << "Usage: virtmem <npages> <nframes> <rand|fifo|lru> <sort|scan|focus>" << endl;
		exit(1);
	}

	// Parse command line arguments
	int npages = atoi(argv[1]);
	nframes = atoi(argv[2]);
	const char *algorithm = argv[3];
	const char *program_name = argv[4];

	//frameTable = new sizeOf(nframes*int);

	// Validate the algorithm specified
	if ((strcmp(algorithm, "rand") != 0) &&
	    (strcmp(algorithm, "fifo") != 0) &&
	    (strcmp(algorithm, "clean") != 0)) {
		cerr << "ERROR: Unknown algorithm: " << algorithm << endl;
		exit(1);
	}

	// Validate the program specified
	program_f program = NULL;
	if (!strcmp(program_name, "sort")) {
		if (nframes < 2) {
			cerr << "ERROR: nFrames >= 2 for sort program" << endl;
			exit(1);
		}

		program = sort_program;
	}
	else if (!strcmp(program_name, "scan")) {
		program = scan_program;
	}
	else if (!strcmp(program_name, "focus")) {
		program = focus_program;
	}
	else {
		cerr << "ERROR: Unknown program: " << program_name << endl;
		exit(1);
	}

	// TODO - Any init needed
	num_faults = 0;
	num_reads = 0;
	num_writes = 0;

	struct page_table *pt = nullptr;

	for(int i = 0; i < nframes; i++)
	{
		free_frames.push(i);
	}

	// Create a virtual disk
	disk = disk_open("myvirtualdisk", npages);
	if (!disk) {
		cerr << "ERROR: Couldn't create virtual disk: " << strerror(errno) << endl;
		return 1;
	}

	if(!strcmp(algorithm, "fifo"))
	{

		pt = page_table_create(npages, nframes, page_fault_handler_fifo);
		if (!pt) {
			cerr << "ERROR: Couldn't create page table: " << strerror(errno) << endl;
			return 1;
		}

	}
	else if(!strcmp(algorithm, "rand"))
	{

		pt = page_table_create(npages, nframes, page_fault_handler_rand);
		if (!pt) {
			cerr << "ERROR: Couldn't create page table: " << strerror(errno) << endl;
			return 1;
		}

	} 
	else if(!strcmp(algorithm, "clean"))
	{

		pt = page_table_create(npages, nframes, page_fault_handler_cleanFIFO);
		if (!pt) {
			cerr << "ERROR: Couldn't create page table: " << strerror(errno) << endl;
			return 1;
		}

	} 
	else 
	{

		// Create a page table
		pt = page_table_create(npages, nframes, page_fault_handler_example /* TODO - Replace with your handler(s)*/);
		if (!pt) {
			cerr << "ERROR: Couldn't create page table: " << strerror(errno) << endl;
			return 1;
		}

	}

	// Run the specified program
	char *virtmem = page_table_get_virtmem(pt);
	program(virtmem, npages * PAGE_SIZE);

	cout << "Number of page faults: " << num_faults << endl;
	cout << "Number of disk reads: " << num_reads << endl;
	cout << "Number of disk writes: " << num_writes << endl;

	// Clean up the page table and disk
	page_table_delete(pt);
	disk_close(disk);

	return 0;
}
