// Generated by mkhdr

#ifndef HEADER_NET_RTL8139
#define HEADER_NET_RTL8139


    // 関数宣言 ---------------------------------------------------------------

void rtl8139_init ( void );
void rtl8139_handler ( int *esp );
void rtl8139_receive ( void );
void rtl8139_send ( void *p  ,  int len );
void rtl8139_dbg ( void );


#endif
