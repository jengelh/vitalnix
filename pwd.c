#include <stdio.h>
#include <string.h>

int main (int Anzahl, char *Argument[])
{
  int i,l,a,b,c;
  char s[20],t[]="0123456";
 
  if (Anzahl<2) {
    printf("Parameter fehlt (Aufruf: pwd benutzer)\n");
  }
  else {
    strcpy(s,Argument[1]);
    strcat(s,Argument[1]); 
    l=strlen(s)-1;
    for (i=0;i<7;i++) {
      a= (int) s[l] + (int) s[i];
      b= (int) s[l] + (int) s[i+1];
      c= 97 + (a+b) % 26;
      t[i]=(char) c;
    }
  }
  printf("%s:%s\n",Argument[1],t);
  return 0;
}










