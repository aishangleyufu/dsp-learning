

  .def  _c_int00
      
RESET:  .long  _c_int00

 .text

;.global start

_c_int00:
	MOV @AR1,#9000 ;��ar1�Ĵ�����ֵ
LOOP:
	ADDB SP,#3 ;��ʱspָ��Ϊ403h��ַ
	MOV *-SP[1],#0f000h ;��������10 �ŵ�402��ַ��
	MOV *-SP[2],#0e000h ;��������1�ŵ�401��ַ��
	MOV AL,*-SP[2] ;��401 ��ַ�����ݶ����ŵ�AL�Ĵ�����
	ADDCU ACC,*-SP[1] ;��401 ��402��ַ���������ӷ����㣬�ѽ���ŵ�AL�Ĵ�����
	MOV *-SP[3],AL ;��AL��ֵ�ŵ�400h��ַ��
	nop ;��ָ��
	nop
	SUBB SP,#3 ;����spָ��Ϊ400h��ַ
	BANZ LOOP,AR1-- ;��������ת��ֻҪar1�е�ֵ��Ϊ0
	.end

