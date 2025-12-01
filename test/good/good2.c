void affiche_le_premier_C(char* tank){
  int i = 0;
  while((char assaut = tank[i]) != EOF){
    if(assaut == 'c'){
      printf("%d\n", i);
      return;
    }
    i = i + 1;
  }
}

int main(void){
  char* tank= "salut c'est michel";
  affiche_le_premier_C(tank);
}
