

  .def  _c_int00
      
RESET:  .long  _c_int00

 .text

;.global start

_c_int00:
	MOV @AR1,#9000 ;给ar1寄存器赋值
LOOP:
	ADDB SP,#3 ;此时sp指针为403h地址
	MOV *-SP[1],#10 ;把立即数10 放到402地址上
	MOV *-SP[2],#1 ;把立即数1放到401地址上
	MOV AL,*-SP[2] ;把401 地址上数据读出放到AL寄存器中
	ADDCU ACC,*-SP[1] ;把401 和402地址中数据做加法运算，把结果放到AL寄存器中
	MOV *-SP[3],AL ;把AL中值放到400h地址中
	nop ;空指令
	nop
	SUBB SP,#3 ;设置sp指针为400h地址
	BANZ LOOP,AR1-- ;有条件跳转，只要ar1中的值不为0
	.end

