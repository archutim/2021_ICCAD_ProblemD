#include "io.h"
#include "macro.h"
#include <iostream>
#include <unistd.h>
#include <signal.h>

extern bool do_SA;
extern IoData* IO_data_ptr;
extern vector<Macro*> macros_best;

void sigalrm_handler(int sig)
{
    // This gets called when the timer runs out.  Try not to do too much here;
    // the recommended practice is to set a flag (of type sig_atomic_t), and have
    // code elsewhere check that flag (e.g. in the main loop of your program)
	if(do_SA){
		do_SA = false;
		alarm(5);
	}
	else{
		IO_data_ptr->macros = macros_best;
		IO_data_ptr->output();
		exit(0);
	}
}

void sigint_handler(int sig)
{
 	cout<<"ctrl C interrupt !"<<endl;
 	IO_data_ptr->macros = macros_best;
 	IO_data_ptr->output();
 	exit(sig);
}