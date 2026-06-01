

// FALTA IMPLEMENTAR ESSAS 3 fif
int gerarChavePub(long long primo1, long long primo2, long long expoente);
int encriptarMenu(char *mensagem, long long n, long long e);
int desencriptarMenu(long long p, long long q, int e);

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

void limparBufferEntrada()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

//FALTA IMPLEMENTAR fif
int primo(int n)
{
    //Implementar função para verificar se um número é primo
}

//FALTA IMPLEMENTAR fif
int mdc(int n1, int n2)
{
    //Implementar função para calcular o máximo divisor comum
    libelula
}


void criarChavePub(long long n, long long e)
{
    FILE *file;

    file = fopen("chavePub.txt", "w");
    fprintf(file, "%lld %lld", n, e);
    fclose(file);
    return;
}

//FALTA IMPLEMENTAR ESSA fif, o n ta dando erro por isso
int gerarChavePub(long long primo1, long long primo2, long long expoente)
{
    //Implementar lógica para ler a chave pública
    //Lembrar de não aceitar valores pequenos de tal forma que p*q < 256
    criarChavePub(n, expoente);
    return 0;
}

// ------------------------------- ENCRYPT


int salvarEmArquivo(long long mensagemencriptada[], int tamanho)
{
    FILE *file;
    file = fopen("textEncript.txt", "w");
    if (file == NULL){
        return 1;
    }

    for (int i = 0; i < tamanho; i++){
        fprintf(file, "%lld ", mensagemencriptada[i]);
    }

    fclose(file);
    return 0;
}

//FALTA IMPLEMENTAR fif
long long mod_pow(long long base, long long exponent, long long modulus)
{
    //implementar função de exponenciação modular
}


// REVISAR ESSA PORRA, NAO ENTENDI A LOGICA 
void encriptar(char *mensagem, long long mensagemencriptada[], long long n, long long e)
{
    for (int i = 0; mensagem[i] != '\0'; i++)
    {
        mensagemencriptada[i] = mensagem[i];
        mensagemencriptada[i] = mod_pow(mensagemencriptada[i], e, n);
    }
}

// ---------------------- DESENCRIPTY


int salvarEmArquivoD(char mensagemdesencriptada[], int tamanho)
{
    FILE *file;
    file = fopen("textDencript.txt", "w");

    if (file == NULL)
    {
        perror("Erro ao abrir o arquivo");
        return 1;
    }

    for (int i = 0; i < tamanho; i++)
    {
        if(mensagemdesencriptada[i] != ' ') fprintf(file, "%c", mensagemdesencriptada[i]);
        else fprintf(file," ");
    }

    fclose(file);
    return 0;
}


void descriptografar(long long mensagemencriptada[], int tamanho, long long d, long long n)
{
    for (int i = 0; i < tamanho; i++)
    {
        mensagemencriptada[i] = mod_pow(mensagemencriptada[i], d, n);
    }
}


void convertascii(char mensagem[], long long mensagemencriptada[], int tamanho)
{
    for (int i = 0; i < tamanho; i++)
    {
        mensagem[i] = (char)mensagemencriptada[i];
    }
}

//FALTA IMPLEMENTAR fif
long long encotrarD(long long e, long long p, long long q)
{
    //Implementar função para encontrar o valor de D
}


void converterParaLongLong(char *str, long long a[], long long *tamanho)
{
    char *token = strtok(str, " ");
    long long i = 0;
    while (token != NULL)
    {
        a[i] = atoll(token);
        i++;
        token = strtok(NULL, " ");
    }
    *tamanho = i;
}



int encriptarMenu(char* mensagem, long long n, long long e)
{
    int tamanho = (int)strlen(mensagem);
    long long mensagemencriptada[tamanho];
    encriptar(mensagem, mensagemencriptada, n, e);
    salvarEmArquivo(mensagemencriptada, tamanho);
    return 0;
}



//REVISAR, ACHO QUE TEM PROBLEMA AQUI 
int desencriptarMenu(long long p, long long q, int e)
{
    long long mensagemenc[100000], D, tamanho;
    char mensagem[10000];
    char linha[10000];
    int i = 0;

    FILE *file;
    file = fopen("textEncript.txt", "r");
    

    if (file == NULL) {
        perror("Erro ao abrir o arquivo");
        return 1;
    }

    fgets(mensagem, sizeof(mensagem), file);
    if (mensagem[strlen(mensagem) - 1] == '\n'){
        mensagem[strlen(mensagem) - 1] = '\0';
    }

    fclose(file);

    converterParaLongLong(mensagem, mensagemenc, &tamanho);
    
    long long n = p * q;

    D = encotrarD(e, p, q);
    descriptografar(mensagemenc, tamanho, D, n);
    convertascii(mensagem, mensagemenc, tamanho);
    salvarEmArquivoD(mensagem, tamanho);
    return 0;
}