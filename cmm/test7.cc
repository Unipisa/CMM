#include "../../Basic/Basic.H"
#include "../../Arith/BigNum/Integer.H"
#include "cmm.h"

int main()
{
GcArray<Integer> &t = * new(900) GcArray<Integer>(0);
int i,j,k=1;

for (i=0;i<30;i++){
   for (j=0;j<30;j++){
        t[30*i+j]=Integer(i+j);
        cout << t[30*i+j];
   }
}

for (k=25;k<30;i++){
  for (i=10;i<30;i++){
    for (j=10;j<30;j++){
        t[30*i+j]=(t[30*k]*t[30*i+j])-(t[i*30]*t[30*k+j]);
        cout << t[(i-10)*30 + (j-10)];
    }
  }
}
}
