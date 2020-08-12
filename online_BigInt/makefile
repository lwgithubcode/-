httpd : httpd.c
	gcc -lpthread -o httpd -g httpd.c

install:
	g++ -o htdocs/BigIntSer.cgi -g BigInt_serve/BigIntSer.c

remove:
	rm  htdocs/BigIntSer.cgi
clean:
	rm httpd
