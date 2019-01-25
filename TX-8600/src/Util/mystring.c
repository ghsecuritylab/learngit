#include "mystring.h"

wchar *wstrcat(wchar *wdst, const wchar*wsrc)
{
    wchar *p=wdst;  
    while(*wdst!=0)wdst++;
    while(*wsrc!=0)*wdst++=*wsrc++;
    *wdst=0;
    return p; 
}
int wstrcmp(const wchar *wdst, const wchar*wsrc)
{
    if((0 == wdst) || (0 == wsrc)) return 1;
    
    while (*wdst && *wsrc && (*wdst == *wsrc))
    {
        wdst++;
        wsrc++;
    }
    return *wdst - *wsrc;
}

int wstrlen(const wchar * wstr) 
{
    const wchar *cp =  wstr;
    while (*cp++);
    return (cp - wstr - 1);
	
}


wchar * wstrcpy(wchar *wdst, const wchar *wsrc)  
{
    if(wdst == 0 || wsrc == 0) return 0;   
    wchar *ret = wdst; 
    while(*wsrc != 0)
    {
        *ret++ = *wsrc++;
    }
    *ret = 0;
    return ret;
}


void ascii_to_unicode(wchar *wdst, const char *wsrc)
{
    while(*wsrc != 0)
    {
        *wdst++ = *wsrc++;
    }
    *wdst = 0;
}


/*************************************************************************************************
* ��UTF8����ת����Unicode��UCS-2LE������  �͵�ַ���λ�ֽ�
* ������
*    char* p_in     �����ַ���
*    char*p_out   ����ַ���
* ����ֵ��ת�����Unicode�ַ������ֽ�������������򷵻�-1
**************************************************************************************************/
//utf8תunicode
int utf8_to_unicode(const char* p_in, char* p_out)
{
    char high;
    char low;
    char middle;
	int output_size = 0; //��¼ת�����Unicode�ַ������ֽ���
 
	while (*p_in)
	{
		if (*p_in > 0x00 && *p_in <= 0x7F) //�����ֽ�UTF8�ַ���Ӣ����ĸ�����֣�
		{
			*p_out = *p_in;
			 p_out++;
			*p_out = 0; //С�˷���ʾ���ڸߵ�ַ�0
		}
		else if (((*p_in) & 0xE0) == 0xC0) //����˫�ֽ�UTF8�ַ�
		{
			high = *p_in;
			p_in++;
			low = *p_in;
			if ((low & 0xC0) != 0x80)  //����Ƿ�Ϊ�Ϸ���UTF8�ַ���ʾ
			{
                output_size = -1;
			    goto end;//��������򱨴�
			}
 
			*p_out = (high << 6) + (low & 0x3F);
			p_out++;
			*p_out = (high >> 2) & 0x07;
		}
		else if (((*p_in) & 0xF0) == 0xE0) //�������ֽ�UTF8�ַ�
		{
			high = *p_in;
			p_in++;
			middle = *p_in;
			p_in++;
			low = *p_in;
			if (((middle & 0xC0) != 0x80) || ((low & 0xC0) != 0x80))
			{
                output_size = -1;
                goto end;
			}
			*p_out = (middle << 6) + (low & 0x3F);//ȡ��middle�ĵ���λ��low�ĵ�6λ����ϳ�unicode�ַ��ĵ�8λ
			p_out++;
			*p_out = (high << 4) + ((middle >> 2) & 0x0F); //ȡ��high�ĵ���λ��middle���м���λ����ϳ�unicode�ַ��ĸ�8λ
		}
		else //���������ֽ�����UTF8�ַ������д���
		{
            output_size = -1;
			goto end;
		}
		p_in ++;//������һ��utf8�ַ�
		p_out ++;
		output_size += 2;
	}
end:    
	//unicode�ַ������棬������\0
	*p_out = 0;
	 p_out++;
	*p_out = 0;
	return output_size;
}
#include "debug_print.h"
void string_test()
{
    const char utf8[] = "123中文123";
    char output[50] = {0};
    int size = utf8_to_unicode(utf8, output);
    debug_printf("string_test %d:\n", wstrlen((const wchar*)output));
    for(int i=0; i<size; i++)
    {
        debug_printf("%x ", output[i]);
    }
    debug_printf("\n\n");
}

