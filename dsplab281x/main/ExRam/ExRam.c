//功能：/*实现对外部存储空间的读写操作*/
#include "DSP281x_Device.h"
void	InitExRam(Uint16	Start);
void	InitExRam1(Uint16	Start);
void	RamRead(Uint16	Start);

unsigned  	int  *ExRamStart = (unsigned  int *)0x100000;

void main(void)
{

	/*初始化系统*/
	InitSysCtrl();

	/*关中断*/
	DINT;
	IER = 0x0000;
	IFR = 0x0000;
   
	InitExRam(0);
	RamRead(0x4000);
	InitExRam1(0x0);		//在这里设置断点	
	for(;;);

} 	


void	InitExRam(Uint16	Start)
		{
			Uint16	i;
			for	(i=0;i<0x4000;i++)		*(ExRamStart + Start + i) = i;
		}
void	InitExRam1(Uint16	Start)
		{
			Uint16	i;
			for	(i=0;i<0x4000;i++)		*(ExRamStart + Start + i) = 0;
		}
void	RamRead(Uint16	Start)
		{
			Uint16	i;
			for	(i=0;i<0x4000;i++)		*(ExRamStart + Start + i) = *(ExRamStart +i);
		}


