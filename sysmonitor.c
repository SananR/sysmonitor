#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>
#include <string.h> 
#include <stdbool.h>
#include <sys/utsname.h>
#include <utmp.h>
#include <sys/sysinfo.h>
#include <sys/resource.h>


/*
 * CONSTANTS
 */
//Divider bar 
const char *DIVIDER = "-------------------------------------------------";
//File path for stat
const char *STATS_PATH = "/proc/stat";
//File path for cpuinfo
const char *CPU_INFO_PATH = "/proc/cpuinfo";

//ANSI code to clear terminal
const char *CLEAR_ANSI = "\e[1;1H\e[2J";

//Number of samples to collect by default
const int SAMPLES_DEFAULT = 10;
//Seconds between each sample by default
const int DELAY_DEFAULT = 1;


/*
 * Display Functions
 */
void display_cpu(int cores, double usage, double usages[], int curr_samples, const char *model, bool graphics);
void display_system();
void display_users();
void display_memory(float usedphys[], float totalphys, float usedvir[], float totalvir, int curr_samples, bool graphics);

/*
 * Sampling Functions
 */
void sample_display(int samples, int delay, bool system, bool usage, bool graphics);
void sample_cpu(int *cores, double *usage, char *model);
void sample_memory(float *usedphys, float *totalphys, float *usedvir, float *totalvir);
void sample_self_memory(long int *memory);

/*
 * Helper Functions
 */
void create_bar_graphic(char *buff, float percent, int offset, float scale, char *bar, char *trail);
bool starts_with(const char *a, const char *b) ;
bool is_numeric(const char *in);


int main(int argc, char **argv) {

	//Initialize samples and delay to default values
	int samples = SAMPLES_DEFAULT, delay = DELAY_DEFAULT;

	bool graphics = false, system = false, user = false;

	/*
		User has given command line arguments
		Interpret them and set appropriate variable values
	*/
	if (argc > 1) {
		for (int i=1; i<argc; i++) {
			char *arg = argv[i];
			//System flag
			if (strcmp(arg, "--system") == 0 || strcmp(arg, "-system") == 0 || strcmp(arg, "-s") == 0) system = true;
			//User flag
			else if (strcmp(arg, "--user") == 0 || strcmp(arg, "-user") == 0 || strcmp(arg, "-u") == 0) user = true;
			//Graphics flag
			else if (strcmp(arg, "--graphics") == 0 || strcmp(arg, "-graphics") == 0 || strcmp(arg, "-g") == 0) graphics = true;
			//Samples flag
			else if (starts_with(arg, "--samples=") || starts_with(arg, "-samples=")) {
				strtok(arg, "=");
				char *in = strtok(NULL, "=");
				if (!is_numeric(in)) {
					printf("Invalid amount given for samples.\nUsage: TODO\n");
					return 0;
				}
				samples = strtol(in, NULL, 10);
			}
			//Delay flag
			else if (starts_with(arg, "--tdelay=") || starts_with(arg, "-tdelay=")) {
				strtok(arg, "=");
				char *in = strtok(NULL, "=");
				if (!is_numeric(in)) {
					printf("Invalid amount given for delay.\nUsage: TODO\n");
					return 0;
				}
				delay = strtol(in, NULL, 10);
			}
			//Positional arguments for samples
			else if (i == argc-2 && is_numeric(arg)) {
				samples = strtol(arg, NULL, 10);
			}
			//Positional arguments for delay
			else if (i == argc-1 && is_numeric(arg)) {
				delay = strtol(arg, NULL, 10);
			}
			//Invalid flag given
			else {
				printf("Invalid flags given!\n");
				return 0;
			}
		}
	}
	if (!system && !user) {
		sample_display(samples, delay, true, true, graphics);
		return 1;
	}
	sample_display(samples, delay, system, user, graphics);
}

/*
 * Function: sample_display
 * ----------------------------
 * Samples and displays all the relevent information to be shown
 * based on input parameters
 *
 * samples: the number of samples to take
 * delay: the delay in seconds between each sample
 * system: whether or not to show system information
 * users: whether or not to show user information
 * graphics: whether or not to show graphical representation
 */

void sample_display(int samples, int delay, bool system, bool users, bool graphics) {
	int cores;
	double usage, usage_sample_total;
	double usages[samples];

	float phys_samples[samples];
	float vir_samples[samples];

	char model[100];

	//Sample CPU
	//TODO sample memory
	//TODO Program memory usage
	for (int i=0; i<samples; i++) {
		//Sample self memory usage
		long int self_memory;
		sample_self_memory(&self_memory);

		printf("%s", CLEAR_ANSI);
		printf("Number of samples: %i -- every %i seconds\n", samples, delay);
		printf("Memory Usage: %ld kb\n", self_memory);
		printf("Displaying sample %i / %i\n", i+1, samples);
		printf("%s\n", DIVIDER);
		//Display memory

		if (system) {
			float usedphys, totalphys, usedvir, totalvir;
			sample_memory(&usedphys, &totalphys, &usedvir, &totalvir);
			phys_samples[i] = usedphys;		
			vir_samples[i] = usedvir;
			display_memory(phys_samples, totalphys, vir_samples, totalvir, i+1, graphics);
			printf("%s\n", DIVIDER);
		}
		//User Information
		if (users) {
			display_users();
			printf("%s\n", DIVIDER);
		}
		//Sample and display CPU
		if (system) {
			sample_cpu(&cores, &usage, model);
			usage_sample_total += usage;
			usages[i] = usage;
			display_cpu(cores, usage_sample_total/(i+1), usages, i+1, model, graphics);
			printf("%s\n", DIVIDER);
		}
		//System Information
		display_system();
		printf("%s\n", DIVIDER);
		sleep(delay);
	}
}

/*
 * Function: display_system
 * ----------------------------
 * Gets and displays the system information to the screen
 */
void display_system() {
	//Creates utsname buffer object to store system information
	struct utsname buff;
	//Populates buffer with system information
	if (uname(&buff) == -1) {
		perror("Failed to get system information: ");
		return;
	}
	//Printing information to the terminal
	printf("### System Information ###\n");
	printf("System Name: %s\n", buff.sysname);
	printf("Machine Name: %s\n", buff.nodename);
	printf("Version: %s\n", buff.version);
	printf("Release: %s\n", buff.release);
	printf("Architecture: %s\n", buff.machine);
	return;
}

/*
 * Function: display_cpu
 * ----------------------------
 * Displays the cpu information to the screen based on input parameters
 *
 * cores: the number of cores the cpu has
 * usage: the current average cpu usage
 * usages[]: usages of each sample taken so far
 * curr_samples: the current sample being taken
 * model: the model of the cpu
 * graphics: whether or not to show graphical representation
 */
void display_cpu(int cores, double usage, double usages[], int curr_samples, const char *model, bool graphics) {
	printf("### CPU ###\n");
	printf("Model: %s\n", model);
	printf("Number of cores: %d\n", cores);
	printf("CPU Usage: %.2f%% (average over %d samples)\n", usage, curr_samples);

	//100 bars possible for 100% cpu usage + 3 offset bars + 1 for terminating string char
	char buff[104] = "";

	for (int i=0; i<curr_samples; i++) {
		if (graphics) create_bar_graphic(buff, usages[i], 3, 1.0, "|", "");
		printf("\tSample %d: %s %.2f%%\n", i+1, buff, usages[i]);
		strcpy(buff, "");
	}
}

/*
 * Function: display_users
 * ----------------------------
 * Gets all the connected users to the system and displays
 * them to the screen.
 *
 */
void display_users() {
	printf("### Sessions/Users ###\n");
	FILE *fp;
	fp = fopen(_PATH_UTMP, "r");
	struct utmp usr;

	while (fread(&usr, sizeof(struct utmp), 1, fp)) {
		if (strcmp(usr.ut_user, "") != 0 && strcmp(usr.ut_user, "LOGIN") != 0 && strcmp(usr.ut_id, "~~") != 0) {
			printf("%s\t%s (%s)\n", usr.ut_user, usr.ut_line, usr.ut_host);
		}
	}
}

/*
 * Function: display_memory
 * ----------------------------
 * Displays memory information to the screen based on input values
 *
 * usedphys[]: array containing i floats with the ith value corresponding to physical memory usage of sample i+1 
 * totalphys: total amount of physical memory
 * usedvir[]: array containing i floats with the ith value corresponding to virtual memory usage of sample i+1 
 * totalvir: total amount of virtual memory
 * curr_samples: the current sample being displayed
 * graphics: whether or not to show graphical representation
 *
 */
void display_memory(float usedphys[], float totalphys, float usedvir[], float totalvir, int curr_samples, bool graphics) {
	printf("### Memory (Physical Used/Total -- Virtual Used/Total) ###\n");
	for (int i=0; i<curr_samples; i++) {
		char gfx[100] = "|";
		float change;
		char *bar, *trail;
		if (i == 0 && graphics) {
			create_bar_graphic(gfx, 0.0, 1, 1.0, "", "o");
		}
		else {
			change = (usedphys[i] + usedvir[i]) - (usedphys[i-1] + usedvir[i-1]);
			//Only 'flip' chars when change is significant enough
			if (change == 0) {
				bar = "";
				trail = "o";
			}
			else if (change < 0 && fabs(change) < 0.01) {
				bar = "o";
				trail = "-";
			} 
			else if (change > 0 && fabs(change) < 0.01) {
				bar = "o";
				trail = "+";
			}
			else if (change > 0 && fabs(change) >= 0.01) {
				bar = "#";
				trail = "*";
			}
			else {
				bar = ":";
				trail = "@";
			}
			if (graphics) create_bar_graphic(gfx, fabs(change), 1, 100.0, bar, trail);
		}
		printf("%.2f GB / %.2f GB  --  %.2f GB / %.2f GB\t%s\n", usedphys[i], totalphys, usedvir[i], totalvir, gfx);
	}
}

/*
 * Function: sample_self_memory
 * ----------------------------
 * Stores the current memory usage of the process
 * (in kilobytes) into input pointer
 *
 * memory: pointer to long int 
 *
 */
void sample_self_memory(long int *memory) {
	struct rusage r_usage;
	getrusage(RUSAGE_SELF, &r_usage);
	*memory = r_usage.ru_maxrss;
}
/*
 * Function: sample_memory
 * ----------------------------
 * Gets the current memory information from file and stores into input pointers
 *
 * usedphys: pointer to float
 * totalphys: pointer to float
 * usedvir: pointer to float
 * totalvir: pointer to float
 */
void sample_memory(float *usedphys, float *totalphys, float *usedvir, float *totalvir) {
	struct sysinfo buff;

	if (sysinfo(&buff) == -1) {
		perror("Failed to get memory information: ");
		return;
	}
	*usedphys = (buff.totalram - buff.freeram) / 1000000000.0;
	*totalphys = buff.totalram / 1000000000.0;

	*usedvir = ((buff.totalswap - buff.freeswap) / 1000000000.0) + *usedphys;
	*totalvir = (buff.totalswap / 1000000000.0) + *totalphys;

	return;
}

/*
 * Function: sample_cpu
 * ----------------------------
 * Gets the current cpu information from file and stores into input pointers
 *
 * cores: pointer to int
 * usage: pointer to double
 * model: pointer to string
 */
void sample_cpu(int *cores, double *usage, char *model) {
	FILE *fp;
	fp = fopen(STATS_PATH, "r");

	if (fp == NULL) {
		perror("Failed to open /proc/stat:");
		return;
	}
	unsigned long user, nice, system, idle, iowait, irq, softirq;

	fscanf(fp, "cpu %lu %lu %lu %lu %lu %lu %lu", &user, &nice, &system, &idle, &iowait, &irq, &softirq);
	unsigned long total = user + nice + system + idle + iowait + irq + softirq;
	unsigned long proc = total - idle;
	*usage = ((double)proc / total)*100;

	fclose(fp);
	fp = fopen(CPU_INFO_PATH, "r");

	char buff[255]="";

	while (fgets(buff, 255, fp)) {
		if (starts_with(buff, "model name")) {
			strtok(buff, ":");
			strcpy(model, strtok(NULL, ":"));
			model[strcspn(model, "\n")] = 0;
			break;
		}
	}
	while (fgets(buff, 255, fp)) {
		if (starts_with(buff, "cpu cores")) {
			strtok(buff, ":");
			*cores = strtol(strtok(NULL, ":"), NULL, 10);
			break;
		}
	}
	return;
}

/*
 * Function: create_bar_graphic
 * ----------------------------
 * Creates a bar graphical representation based on input parameters
 *
 * buff: pointer to a string
 * percent: the percent of the bar
 * offset: how many bars to always add (offset by)
 * scale: how many bars to add for each % 
 * bar: the char to use for bar
 * trail: the trailing character to add to the end
 */
void create_bar_graphic(char *buff, float percent, int offset, float scale, char* bar, char* trail) {
	for (int i=0; i<offset; i++) strcat(buff, bar);
	
	float t = percent * scale;
	for (int i=0; i<(int)t; i++) strcat(buff, bar);

	strcat(buff, trail);
}

/*
 * Function: starts_with
 * ----------------------------
 * Returns whether or not the first input string
 * starts with the second input string
 *
 * a: input string
 * b: input substring
 *
 * returns true if a starts with b, false otherwise
 */
bool starts_with(const char *a, const char *b) {
   if(strncmp(a, b, strlen(b)) == 0) return 1;
   return 0;
}

/*
 * Function: is_numeric
 * ----------------------------
 * Returns whether or not input is of numeric type
 * (consists only of digit characters)
 *
 * in: input string
 *
 * returns true if input is numeric, false otherwise
 */

bool is_numeric(const char *in) {
	if (in == NULL || strlen(in) == 0) return false;
	for (int i=0; i<strlen(in); i++) {
		if (!isdigit(in[i])) return false;
	}
	return true;
}