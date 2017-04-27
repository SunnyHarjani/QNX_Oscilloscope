#include <stdio.h>
#include <unistd.h>       /* for sleep() */
#include <stdint.h>       /* for uintptr_t */
#include <hw/inout.h>     /* for in*() and out*() functions */
#include <sys/neutrino.h> /* for ThreadCtl() */
#include <sys/mman.h>     /* for mmap_device_io() */
#include <cstdlib>
#include <iostream>
#include <pthread.h>
#include <ctime>
#include <queue>
#include <string>
#include <sys/mman.h>
#include <sys/time.h>

#define BASE 0x280
#define CONTROL_REGISTER BASE+11
#define INIT_BIT 0x00
#define PORT_LENGTH 1

bool checkstatus() // returns 0 if ok, -1 if error
	{
	int i;
	for (i = 0; i < 20000; i++)
	if (!(in8(BASE+3) & 0x80)) return(false); // conversion completed
	return(true); // conversion did not complete
	}

void QNXInit(){
	int privity_err;
	uintptr_t ctrl_handle;
	uintptr_t data_handle;

	/* Give this thread root permissions to access the hardware */
	privity_err = ThreadCtl( _NTO_TCTL_IO, NULL );
	if ( privity_err == -1 )
	{
		fprintf( stderr, "can't get root permissions\n" );
		return;
	}

	/* Get a handle to the parallel port's Control register */
	ctrl_handle = mmap_device_io( PORT_LENGTH, CONTROL_REGISTER );

	/* Initialise the parallel port */
	out8( ctrl_handle, INIT_BIT );

	out8(BASE+2, 0x22);// example, set channel range to fixed channel 2

	out8(BASE+3, 0x01); // example: set gain = 2

	out8(BASE+1, 2); // set page
	out8(BASE+13, 0x00); // example: sets bipolar, single-ended
}

void AnalogToDigitalRead(){
	while (in8(BASE+3) & 0x20); // wait for ADWAIT to go low, base+3 bit 5

	out8(BASE,0x80); // start the A/D conversion
	while(checkstatus()){};

	int8_t LSB = in8(BASE);
	int8_t MSB = in8(BASE+1);
	double ADdata = MSB * 256 + LSB; // combine the 2 bytes into a 16-bit value

	float InputVoltage = ADdata / 32768.0 * 5.0;

	std::cout << InputVoltage << std::endl;
}

/* ______________________________________________________________________ */
int main( )
{
	QNXInit();

	// wait for ADBUSY to go low, base+3 bit 7

while(1){
	AnalogToDigitalRead();
}
	return EXIT_SUCCESS;
}
