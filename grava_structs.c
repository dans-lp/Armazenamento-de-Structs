/* Daniel_Lages_de_Altavila_Peralta 1811442 3WA */
#include "grava_structs.h"

void first_byte(int nstructs,FILE *pf); 
void second_byte(char *campos, char ord, int * n, FILE *pf);
void third_byte(char *campos, int *sbIndex, FILE *pf);
void grava_Little(int n, int a, void *valores, int *sizeArray, int *paddingArray, FILE *pf);
void grava_Big(int n, int a, int total, void *valores, int *sizeArray, int *paddingArray, FILE *pf);
void decodifica(int *array, unsigned int a);


void first_byte(int nstructs,FILE * pf){
  unsigned char byte = nstructs;
  fputc(byte,pf);
  return;
}

void second_byte(char *campos, char ord, int * n, FILE *pf){
  unsigned char byte;
  
  while (*campos != '\0')
  {
    campos++;
    (*n)++;
  } 
  byte = *n;
  if (ord == 'L')
  {
    byte = byte | 0x80;
  }
  fputc(byte,pf);
  return;
}

void third_byte(char *campos, int *sbIndex, FILE *pf){
  
  int a = 0;
  unsigned char byte = 0x00;
    
  while (*campos != '\0')
  {
    switch (*campos)
    {
      case 'c':
        *sbIndex = 1;
        break;

      case 's':
        *sbIndex = 2; 
        byte = (0x40 >> a*2) | byte;
        break;

      case 'i':
        *sbIndex = 4;
        byte = (0x80 >> a*2) | byte;
        break;
      
      case 'l':
        *sbIndex = 8;
        byte = (0xC0 >> a*2) | byte;
        break;

      default: 
        printf("\n!!! campo invalido !!!\n");
        return;
    }
    
    a++;

    if ((a % 4 == 0) && a >= 4)
    {
      a = 0;
      fputc(byte,pf);
      int newByte = 0x00;
      byte = newByte;
    }

    sbIndex++;
    campos++;  
  }
  fputc(byte,pf);
  return;
}


void grava_Little(int n, int a, void *valores, int *sizeArray, int *paddingArray, FILE *pf){
  unsigned char *p = valores;
  
  for (int i = 0; i < n; i++)
  {
    for (int j = 0; j < a; j++,sizeArray++,paddingArray++)
    {
      for (int k = 0; k < *sizeArray; k++, p++) 
      {
        fputc(*p,pf);  
      }
      p += *paddingArray;
    }
    sizeArray -= a;
    paddingArray -=a;
  }
}

void grava_Big(int n, int a, int total, void *valores, int *sizeArray, int *paddingArray , FILE *pf){
  unsigned char *p = valores;
  unsigned char *end;
  for (int i = 0; i < n; i++)
  {
    p += total;
    end = p;
    for (int j = 0; j < a; j++,sizeArray--,paddingArray--)
    {  
      p -= *paddingArray;
      for (int k = 1; k <= *sizeArray; k++,p--)
      { 
        fputc(*p,pf);
      }
    }

    p = end;
    sizeArray += a ;
    paddingArray += a;
  }
}


int grava_structs (int nstructs, void *valores, char *campos, char ord, char *arquivo){

  FILE *f = fopen(arquivo, "wb");
  if(f == NULL)
  {
    printf("\n!! arquivo não pode ser aberto !!\n");
    return(-1);
  }
  int nCampos = 0;
  int *pointer_nc = &nCampos;

  first_byte(nstructs,f);
  second_byte(campos, ord, pointer_nc, f);

  int sizeBytes[nCampos] ,sizePadding[nCampos];

  third_byte(campos, sizeBytes,f);
  
  int k = 0, totalSize = 0, clock = 1;
  while (k < nCampos-1)
  {
    if (sizeBytes[k]> clock) clock = sizeBytes[k];
    
    if (sizeBytes[k]<sizeBytes[k+1]) 
      sizePadding[k] = (sizeBytes[k+1] - sizeBytes[k]);
    else sizePadding[k] = 0;
    
    totalSize += (sizePadding[k]+sizeBytes[k]);  
    k++;
  }

  if (totalSize%clock != 0)
  {
    sizePadding[k] = totalSize/clock;
    totalSize += (sizeBytes[k] + sizePadding[k]);
  }
  
  if(ord == 'L') 
    grava_Little(nstructs, nCampos, valores, sizeBytes, sizePadding, f);
  else if (ord == 'B')
    grava_Big(nstructs, nCampos, totalSize-1, valores, &sizeBytes[nCampos-1], &sizePadding[nCampos-1], f);   
  else return printf("\n!!!!!!! TIPO DE ORDENAÇÃO INVÁLIDA !!!!!\n");

  fclose(f);
  return 0;
}


void decodifica(int *array, unsigned int a){
  switch (a)
  {
    case 0:
      *array = 1;
      break;

      case 1:
      *array = 2;
      break;

      case 2:
      *array = 4;
      break;
      
    case 3:
      *array = 8;
      break;

    default: 
      printf("\n--> NÃO FOI POSSIVEL DECODIFICAR O VALOR <---\n");
      return;
  }
}

void dump_structs (char *arquivo){
  FILE *f = fopen(arquivo, "rb");
  if(f == NULL)
  {
    printf("\n!! arquivo não pode ser aberto !!\n");
  }
  
  unsigned int qtdS = fgetc(f);
  
  signed int sb = fgetc(f);
  unsigned int qtdC = sb & 0x7F;
  signed int ord = (sb & 0x80) << 24;

  unsigned int campo = fgetc(f), campoShift;
  int sizeCampos[qtdC];
  for (int i = 0, j = 0; i < qtdC; i++, j++)
  { 
    if (i%4 == 0 && i > 0)
    {
      campo = fgetc(f);
      j = 0;
    }
    campoShift = campo & (0x03 << (6 - j*2));
    decodifica(&sizeCampos[i], campoShift >> (6 - j*2));
  }

  if (ord >= 0) printf("B\n");
  else printf("L\n");

  for (int i = 0; i < qtdS; i++)
  {
    printf("*\n");
    if (ord >= 0 )
    { 
      for (int k = qtdC-1; k >= 0; k--)
      {
        for (int r = sizeCampos[k]; r > 0; r--)
        {
          printf("%02X ",fgetc(f));
        }
        printf("\n");
      }
    }
    else
    {
      for (int j = 0; j < qtdC; j++)
      {
        for (int q = sizeCampos[j]; q > 0; q--)
        {
          printf("%02X ",fgetc(f));
        }
        printf("\n");
      }  
    }
  }

  printf("\n");
  fclose(f);
}