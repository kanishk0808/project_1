//  CITS2002 Project 1 2023
//  Student1:   23201336   Chunchignorov Buyanjargal

#include <stdio.h>
#include <stdlib.h> 
#include <ctype.h>
#include <string.h>
// other header to be added

/*
 * Constants for maximum sizes for 
 * device and command data structures.
 */
#define MAX_DEVICES                     4
#define MAX_DEVICE_NAME                 20

#define MAX_COMMANDS                    10
#define MAX_COMMAND_NAME                20
/*
 * This is constant for max processes
 * a single command has to help with
 * storing data into pointer arrays
 * in the command data structure.
 */
#define MAX_PROCESS_COUNT               20 

#define MAX_SYSCALLS_PER_PROCESS        40
#define MAX_RUNNING_PROCESSES           50

//  NOTE THAT DEVICE DATA-TRANSFER-RATES ARE MEASURED IN BYTES/SECOND, THAT ALL TIMES ARE MEASURED IN MICROSECONDS (usecs),

#define DEFAULT_TIME_QUANTUM            100
#define TIME_CONTEXT_SWITCH             5
#define TIME_CORE_STATE_TRANSITIONS     10
#define TIME_ACQUIRE_BUS                20

#define CHAR_COMMENT                    '#'

/*
 * Device structure to store data of devices.
 * Storing the name, readspeed and writespeed
 * of devices.
 */ 
struct device {
    char name[MAX_DEVICE_NAME];
    int readspeed;
    int writespeed;
}; 

/*
 * Command structure to store data of commands.
 * It stores the command name, load_time, process
 * name, the device it uses and the last element
 * which I said variable(time or bytes).
 */ 
struct command {
    char name[MAX_COMMAND_NAME];
    int load_time[MAX_PROCESS_COUNT];
    char *process[MAX_PROCESS_COUNT];
    char *commandDev[MAX_PROCESS_COUNT];
    int variable[MAX_PROCESS_COUNT];
};

/*
 * Make device and command structs 
 * global variables so that functions 
 * can acces them.
 */
struct device devices[MAX_DEVICES];
struct command commands[MAX_COMMANDS];

/*
 * Read sys_config file and put in
 * data into device structure to
 * be processed later.
 */
void read_sysconfig(char argv0[], char filename[]){
    
    FILE *f_sysconfig = fopen(filename,"r");
    if(f_sysconfig != NULL){
        /*
         * Read device file and put in
         * data into device structure to
         * be processed later.
         */
        char str_device[1000];
        int dev_len = sizeof(str_device);
        int device_count = 0;
        while (fgets(str_device,dev_len,f_sysconfig) != NULL) {
            /*
             * If line starts with '#' then
             * continue the program. But I probably don't
             * need to use this due to else if condition under.
             */
            if(str_device[0] == CHAR_COMMENT){     
                continue;                          
            }

            /*
             * If first 6 letters of str_device is
             * equal to "device" then process that line.
             */
            else if(strncmp(str_device, "device", 6) == 0){ // If str_device line starts with "device" then ...
                char dev_name[MAX_DEVICE_NAME]; 
                char dev_reads[20];
                char dev_writes[20];
                sscanf(str_device, "%*s %s %s %s", dev_name, dev_reads, dev_writes);

                strcpy(devices[device_count].name, dev_name);

                int read_len = strlen(dev_reads);
                int write_len = strlen(dev_writes);   

                /*
                 * Delete the last 3 chars of readspeed and writespeed
                 * to delete Bps and to use atoi() function
                 * later for calculating microseconds. 
                 */ 
                dev_reads[read_len - 3] = '\0';
                dev_writes[write_len - 3] = '\0';
                
                devices[device_count].readspeed = atoi(dev_reads);
                devices[device_count].writespeed = atoi(dev_writes);

                // print data to check program success.
                printf(" name:%s \t| readspeed:%i \t| writespeed=%i\n",
                      devices[device_count].name, devices[device_count].readspeed, devices[device_count].writespeed);


                device_count++;
            }
        }
    }
    fclose(f_sysconfig);


}

int command_count = 0; // Global variable to track how many commands are in commands file.

/*
 * Read commands file and put in
 * data into device structure to
 * be processed later.
 */ 
void read_commands(char argv0[], char filename[]){
    
    FILE *f_command = fopen(filename,"r");
    if(f_command != NULL){
        /*
         * Read command file and put in
         * data into device structure to
         * be processed later.
         */
        char str_command[1000];
        int process_count = 0;
        while (fgets(str_command,1000,f_command) != NULL) {
            
            /*
             * If line starts with '#' then
             * continue the program.
             */
            if(str_command[0] == CHAR_COMMENT){     
                continue;                          
            }
            else if(!isspace(str_command[0])){ // If line doesn't start with white space then process line for command name.
                char command_name[MAX_COMMAND_NAME];
                sscanf(str_command, "%s", command_name);
                strcpy(commands[command_count].name, command_name);
                command_count++;
                process_count = 0;
            }

            /*
             * If first char of the line is (\t)
             * a white space then process the strings
             * and put them into command data structure.
             */
            else if(isspace(str_command[0])){ 
                
                // chars to store command file line strings.
                char cmd_time[20];
                char cmd_process[20];
                char cmd_Dev[20];
                char cmd_var[20];

                int num_process = sscanf(str_command, "%s %s %s %s", cmd_time, cmd_process, cmd_Dev, cmd_var);
                
                // Delete "usecs" from load length to use atoi later.
                int load_length = strlen(cmd_time);
                cmd_time[load_length - 5] = '\0';

                /*
                 * variable either ends with "usecs" or
                 * "B" for bytes, so whichever it ends with
                 * find out and delete the chars for atoi
                 * function to process into ints.
                 */
                int var_length = strlen(cmd_var);
                if(var_length != 1 && cmd_var[var_length - 1] == 's'){
                    cmd_var[var_length - 5] = '\0';
                }
                else if(var_length != 1 && cmd_var[var_length - 1] == 'B'){
                    cmd_var[var_length - 1] = '\0';
                }

                // If command line has 4 strings.
                if(num_process == 4){
                    commands[command_count - 1].load_time[process_count] = atoi(cmd_time);
                    
                    commands[command_count - 1].process[process_count] = (char *)malloc(strlen(cmd_process) + 1);
                    strcpy(commands[command_count - 1].process[process_count], cmd_process);
                    
                    commands[command_count - 1].commandDev[process_count] = (char *)malloc(strlen(cmd_Dev) + 1);
                    strcpy(commands[command_count - 1].commandDev[process_count], cmd_Dev);
                    
                    commands[command_count - 1].variable[process_count] = atoi(cmd_var);

                }

                /*
                 * If command line has 3 strings either device 
                 * or variable is null. If 3rd string is a char
                 * that is alphabetic letter then its a device name.
                 */
                else if(num_process == 3 && (!isdigit(cmd_Dev[0]))){
                    commands[command_count - 1].load_time[process_count] = atoi(cmd_time);
                    
                    commands[command_count - 1].process[process_count] = (char *)malloc(strlen(cmd_process) + 1);
                    strcpy(commands[command_count - 1].process[process_count], cmd_process);
                    
                    commands[command_count - 1].commandDev[process_count] = (char *)malloc(strlen(cmd_Dev) + 1);
                    strcpy(commands[command_count - 1].commandDev[process_count], cmd_Dev);
                    
                    commands[command_count - 1].variable[process_count] = 0;
                }

                /*
                 * If 3rd string starts with a char
                 * that is a number then it is time in
                 * "usecs" or data in bytes("B").
                 */
                else if(num_process == 3 && (isdigit(cmd_Dev[0]))){
                    commands[command_count - 1].load_time[process_count] = atoi(cmd_time);
                    
                    commands[command_count - 1].process[process_count] = (char *)malloc(strlen(cmd_process) + 1);
                    strcpy(commands[command_count - 1].process[process_count], cmd_process);
                    
                    commands[command_count - 1].commandDev[process_count] = (char *)malloc(2);  // Allocate space for a single character
                    commands[command_count - 1].commandDev[process_count][0] = '0';  // Set the character to '0'
                    commands[command_count - 1].commandDev[process_count][1] = '\0'; // Null-terminate the string
                    
                    commands[command_count - 1].variable[process_count] = atoi(cmd_Dev);
                }

                /*
                 * Else both device used and variableS
                 * are null.
                 */
                else{
                    commands[command_count - 1].load_time[process_count] = atoi(cmd_time);
                    
                    commands[command_count - 1].process[process_count] = (char *)malloc(strlen(cmd_process) + 1);
                    strcpy(commands[command_count - 1].process[process_count], cmd_process);
                    
                    commands[command_count - 1].commandDev[process_count] = (char *)malloc(2);  // Allocate space for a single character
                    commands[command_count - 1].commandDev[process_count][0] = '0';  // Set the character to '0'
                    commands[command_count - 1].commandDev[process_count][1] = '\0'; // Null-terminate the string
                    
                    commands[command_count - 1].variable[process_count] = 0;
                }

                // print data to check program success.
                printf("command name:%s | load_time:%i | process:%s | device:%s | variable:%i\n",
                commands[command_count - 1].name,
                commands[command_count - 1].load_time[process_count],
                commands[command_count - 1].process[process_count],
                commands[command_count - 1].commandDev[process_count],
                commands[command_count - 1].variable[process_count]);
                
                process_count++;
            }
        }
    }
    fclose(f_command);
}

/*
 * In command structure for *process[]
 * and *commandDev[] I used malloc to allocate
 * memory dynamically. So after the data is 
 * finished processing then free up the memory
 * to prevent memory leaks.
 */
void free_commands_memory(void){
    for (int i = 0; i < command_count; i++) {
        for (int j = 0; j < 20 ; j++) {
            free(commands[i].process[j]);
            free(commands[i].commandDev[j]);
        }
    }
}

/*
 * execute_commands() function takes 
 * device and command structures to process
 * the time and percentage of time used
 * on work compared to time wasted on 
 * process switch and waiting.
 */


//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
void execute_commands(void){
    
}


//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
int main(int argc, char *argv[])
{
//  ENSURE THAT WE HAVE THE CORRECT NUMBER OF COMMAND-LINE ARGUMENTS
    if(argc != 3) {
        fprintf(stderr,"%s: Program expected 2 arguments but recieved %d.\n", argv[0], argc - 1);
        exit(EXIT_FAILURE);
    }
    
    read_sysconfig(argv[0], argv[1]); // READ THE SYSTEM CONFIGURATION FILE
    printf("\n");
    read_commands(argv[0], argv[2]); // READ THE COMMAND FILE
    
    execute_commands(); // EXECUTE COMMANDS, STARTING AT FIRST IN command-file, UNTIL NONE REMAIN
    
    // PRINT THE PROGRAM'S RESULTS
    int size_device = sizeof devices / sizeof devices[0];
    int size_command = sizeof commands / sizeof commands[0];

    free_commands_memory(); // Free pointer arrays' memory after data is processed.
    exit(EXIT_SUCCESS);
}
