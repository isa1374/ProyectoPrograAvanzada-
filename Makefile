CFLAGS = -g
ALL = server client execphp

ALL:${ALL}

clean : 
	\rm -rf ${ALL}
