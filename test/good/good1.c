/*
Je suis un programme de C qui doit archer, je lance mon int main(void){} quand on m'appelle
*/

int max(int a, int b){ //valeur max entre a et b en int
  if(a > b || a == b){
    return a;
  } else return b;
}

int main(void){
  int a = 0;
  int b = 1 + 3;
  int c = max(a, b);
  return 1;
}
