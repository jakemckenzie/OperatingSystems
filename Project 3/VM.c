#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <alloca.h>

/*
 * Jake McKenzie
 * Operating Systems
 * Project 3
 * 
 * 
 * My personal notes:
 *                      Hierarchical Paging
 * 2^m bytes physical memory space and 2^n bytes virtual memory space.
 * Size of physical address = m bits
 * Size of virtual address = n bits
 * Size of page = 2^d bytes
 * Page offset of a virtual or physical address = d bits (the least significant)
 * Page number of a virtual address = n - d bits (the most significant) 
 * Frame number of the physical address = m - d bits
 * A page table entry should fit a frame number and a valid bit, and it should
 * be a power of 2.
 * Size e of page table entry >= m - d + 1 bits 
 * Entries per page: 2^d/(e/8) = 2^d/(m/8)
 * Number of page table entrie = 2^p
 */

// Grabs all but page number
#define OFFSET_MASK  	    0xFF
// (Physical Address - Page Offset) / Frame Size
#define FRAME_NUMBER	    0x100
//  (Physical Address - Page Offset) / Frame Number
#define FRAME_SIZE	        0x100
// Assignment specifications
#define TLB_SIZE 	        0x10
// Assignment specifications
#define PAGE_TABLE_SIZE     0x100
// The fgets function reads at most one less than the number of characters 
// specified by LINE_SIZE from the stream pointed to by address_file stream 
// into the array pointed to by the address array in this case.
#define LINE_SIZE           0x07
// number of bytes to read
#define READ_SIZE		    0x100

int input_fifo              = 0;
int output_fifo             = 0;
char address_array[LINE_SIZE];
int virtual_address;
signed char current_context[READ_SIZE];
FILE *address_file;
FILE *backing_store;
// page fault statstics 
int stats_page_fault        = 0;
int stats_TLB_hits          = 0;
int stats_no_references     = 0;
// page tables
int pageTable[PAGE_TABLE_SIZE];
// the list of free frames
int buffer_pool[FRAME_NUMBER];
// the actual RAM of the system split up by frame number and it's size
int physicalMemory[FRAME_NUMBER][FRAME_SIZE];
// a table for pages split by page table entries and segment table entries
int TLB[TLB_SIZE][2];

void fetch_page(int address);
int backing_page(int pageNumber);
int in_TLB(int pageNumber);
void TLB_add(int pageNumber, int mapped_frame);
int put_fifo(int pageNumber, int mapped_frame);
int get_fifo();

int main(int argc, char *argv[]) {
    
    /*
     * Error handling on command line input
     */
    if (argc != 2) {
        fprintf(stderr,"Usage: ./VM.out [input file]\n");
        return -1;
    }
    
    /*
     * Opening of backing_store bin file
     */
    backing_store = fopen("BACKING_STORE.bin", "rb");

    /*
     * Error handling whether backing store is empty
     */
    if (backing_store == NULL) {
        fprintf(stderr, "The BACKING_STORE.bin is empty: %s\n","BACKING_STORE.bin");
        return -1;
    }
    /*
     * Opens the file that contains the logical dresses
     */
    
    address_file = fopen(argv[1], "r");

    /*
     * Error handling if address file is empty
     */
    if (address_file == NULL) {
        fprintf(stderr, "Error opening addresses.txt %s\n",argv[1]);
        return -1;
    }

    /*
     * Initalizes the buffer pool for free frames
     */
    for(int i = 0; i < FRAME_NUMBER; i++) {
        // free buffer pool entry is 1 by default
        buffer_pool[i] = 1; 
    }

    /*
     * Initalizes the page tables
     */
    for(int i = 0;i < PAGE_TABLE_SIZE; i++) {
        // page table entry is -1 by default
        pageTable[i] = -1;
    }

    /*
     * Initalizes the TLB
     */
    for(int i = 0;i<TLB_SIZE; i++){
        // page frame entry is -1 by default
    	TLB[i][0] = -1;
        // mapped frame entry is -1 by default
    	TLB[i][1] = -1;
    }

    /*
     * reads through the input file and output each logical adress
     */
    while (fgets(address_array, LINE_SIZE, address_file) != NULL) {
        // stores the virtual address for future processing
        virtual_address = atoi(address_array);
        // fetches the page
        fetch_page(virtual_address);
        // counts no references for statistics
	    stats_no_references++;
    }

    // closes the input file
    fclose(address_file);
    // closes the backing store
    fclose(backing_store);
    // first order statstics
    printf("Reference Number | Page Fault Number | TLB hit number | page fault rate | TLB hit rate\n");
    printf("%d\t%d\t%d\t%d\t%d\n");
    printf("Number of references = %d\n", stats_no_references);
    printf("Number of page faults = %d\n",stats_page_fault);
    printf("Number of TLB Hits = %d\n",stats_TLB_hits);
    printf("Page fault rate = %f %%\n",stats_page_fault / (float)stats_no_references);
    printf("TLB hit rate = %f %%\n",stats_TLB_hits / (float)stats_no_references);

    return 0;
}

/*
 * Takes the virtual address and fetches the page
 */
void fetch_page(int virtual_address) {

    // grabs the page number
    int pageNumber = (virtual_address>>8);
    // grabs the offset
    int offset = (virtual_address & OFFSET_MASK);
    // Initalizes the mapped frame and value to zero
    int mapped_frame = 0;
    int value = 0;

   /*
    * Step 1: check to see if page number is in TLB
    * Step 2: If step 1 fails searchs to see if page number is in page table
    * Step 3: If step 2 fails searchs to see if page number is in backing store
    */
   if(in_TLB(pageNumber) != 0){
        // step 1
        mapped_frame = in_TLB(pageNumber);
	    printf("Found in TLB, page number = %d correspond to frame = %d \n",pageNumber,mapped_frame);
   } else {
	    if(pageTable[pageNumber] != -1){
		    // step 2
            mapped_frame = pageTable[pageNumber];
		    value = physicalMemory[mapped_frame][offset];
		    TLB_add(pageNumber, mapped_frame);
	    } else { 
		    // step 3
            mapped_frame = backing_page(pageNumber);
		    value = physicalMemory[mapped_frame][offset];
		    TLB_add(pageNumber, mapped_frame);
	    }
    }
    // for printing
    printf("Page Number \t| Frame Number \t| Offset \t:: Virtual Address \t| Physical Address \t| Value\n");
    printf("%d\t\t%d\t\t%d\t\t%d\t\t%d\t\t%d \n", pageNumber, mapped_frame, offset, virtual_address, (mapped_frame << 8) | offset, value);
}

/*
 * Takes the page number and backs them up into physical memory
 */
int backing_page(int pageNumber) {
    // initializes the available frame to zero
    int available_frame = 0;
    // checks from the beginning of the file in byte chunks
    if (fseek(backing_store, pageNumber * READ_SIZE, SEEK_SET) != 0) {
        fprintf(stderr, "Error seeking in backing store\n");
    }
    // read from backing_store file to current_context array
    if (fread(current_context, sizeof(signed char), READ_SIZE, backing_store) == 0) {
        fprintf(stderr, "Error reading from backing store\n");
    }
    // attempts to find an available frame
    for(int i = 0; i < FRAME_NUMBER;i++) {
        // checks the available frame buffer pool
        if(buffer_pool[i] == 1){
            // identifies the available frame
		    available_frame = i;
            // sets the entry to empty
		    buffer_pool[i] = 0;
            // creates a page table
		    pageTable[pageNumber] = available_frame;
            // stores page fault for statistics
		    stats_page_fault++;
            // break
		    break;
	    }
    }
    // if this line has been reached there was a page fault
    printf("Page Fault! Obtaining page from disk and assigning to frame %d\n",available_frame);

    // finds the first available frame and puts it into physical memory
    for(int i = 0; i < READ_SIZE; i++) {
        physicalMemory[available_frame][i] = current_context[i];
    }
    return available_frame;
}

//checks to see if page number is in the TLB
int in_TLB(int pageNumber){
	for(int i = 0; i < TLB_SIZE; i++){
		if(TLB[i][0] == pageNumber){
            // stores TLB hit
			stats_TLB_hits++;
            // returns the hit TLB if it exists
			return TLB[i][1];
		}
	}
    // hit it and missed
    return 0;
}
// implements a FIFO TLB
void TLB_add(int pageNumber, int mapped_frame){
    // puts into fifo queue, if there's no problems
	if(put_fifo(pageNumber, mapped_frame) != -1){
        // gets from circular fifo queue
		get_fifo();
        // puts into circular fifo queue
		put_fifo(pageNumber, mapped_frame);
	}
}

int get_fifo() {
    // if true queue is full...return for debugging purposes
    if(input_fifo == output_fifo) {
        return -1;
    }
    // circular queue
    output_fifo = (output_fifo + 1) % TLB_SIZE;
    // return for debugging purposes
    return 1;
}

int put_fifo(int pageNumber, int mapped_frame) {
    // return for debugging purposes
    if(input_fifo == ((TLB_SIZE + output_fifo - 1) % TLB_SIZE)) {
        return -1; /* Queue Full*/
    }
    // stores the page number into TLB
    TLB[input_fifo][0] = pageNumber;
    // stores the mapped frame into TLB
    TLB[input_fifo][1] = mapped_frame;
    // circular queue
    input_fifo = (input_fifo + 1) % TLB_SIZE;
    // return for debugging purposes
    return 0;
}

