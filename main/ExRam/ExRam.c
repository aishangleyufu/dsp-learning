



//用ExRamprj打开

//功能：/*实现对外部存储空间的读写操作*/
#include "DSP281x_Device.h"
#include "math.h"
void	InitExRam(Uint16	Start);
void	InitExRam1(Uint16	Start);
void	RamRead(Uint16	Start);

int  *ExRamStart = (int *)0x100000;

void main(void)
{

	/*初始化系统*/
	InitSysCtrl();

	/*关中断*/
	DINT;
	IER = 0x0000;
	IFR = 0x0000;
   
	InitExRam(0);
 //   RamRead(0x4000);
   InitExRam1(0x4000);		//在这里设置断点	
	for(;;);

} 	


void	InitExRam(Uint16	Start)//0x100000~0x104000 正弦波
		{
			Uint16	i;
			for	(i=0;i<0x4000;i++)		*(ExRamStart + Start + i) =100*sin(3.14159*i/1000);
		}
void	InitExRam1(Uint16	Start)//0x104000~0x108000 锯齿波
		{
			Uint16	i;
			for	(i=0;i<0x4000;i++)		*(ExRamStart + Start + i) =100*i;
		}
		
void	RamRead(Uint16	Start)
		{
			Uint16	i;
			for	(i=0;i<0x4000;i++)		*(ExRamStart + Start + i) = *(ExRamStart +i);
		}