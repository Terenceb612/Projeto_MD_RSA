#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

////////////////////////////////////////////////////
// PROTOTIPOS DAS FUNCOES EXPORTADAS (chamadas pelo Django via ctypes)
////////////////////////////////////////////////////

int gerarChavePub(long long primo1, long long primo2, long long expoente);
int encriptarMenu(char *mensagem, long long n, long long e);
int desencriptarMenu(long long p, long long q, long long e);


////////////////////////////////////////////////////
// FUNCOES MATEMATICAS
////////////////////////////////////////////////////



/*terence:
Se n tem um divisor, ele sempre vem em par: n = a * b
Se a > raiz de n, então obrigatoriamente b < raiz de n, o menor sempre fica abaixo da raiz
Logo, se nenhum número até √n divide n, não existe divisor — n é primo
Usar i*i <= n evita importar math.h e é equivalente a i <= raiz de n
*/
int primo(long long n)
{
    if (n < 2)
    {
        return 0;
    }
    for(long long i = 2; i * i <= n; i++)
    {
        if (n%i == 0)
        {
            return 0;
        }
    }
    return 1;
}

//cauet: mdc implementado
/*
A função verifica qual o maximo divisor comum entre dois numeros grandes, utilizando o algoritmo de euclides tradicional.
A função usa aritmetica modular para simplificar a expressão ao máximo.
Após a simplificação, encontra o mdc que é igual ao mdc da expressão original e retorna o resultado.
*/
long long mdc(long long n1,long long n2) //precisa ser long long
{
    while (n2 != 0)
    {
        long long resto = n1 % n2;
        n1 = n2;
        n2 = resto;
    }

    return n1;
}

/*
FUNCAO MOD POW

terence: vou fazer uma maracutaia pra fazer a exponenciação ser mais eficiente
ela se baseia no fato de que qualquer expoente pode ser escrito em binario, ex: 7 = 111 (binario)
a cada iteracao, "expoente/2" descarata o bit da iteracao atual(resultado vai ser usado na prox) e expoente%2 le o valor (0 ou 1) 
se for 0, nao inclui na soma
a base é elevada ao quadrado a cada passo, o mod em cada produto serve pra o numero nunca crescer demais
pq é melhor : se for elevando no seco fica O(n) multiplicacoes, essa solucao bacana ai faz com que seja O(logn), ideal pra quando o expoente crescer muito.
exemplo : testa com  o expoente sendo um milhao e ve.
fonte: achei em um video aleatorio de um cara mostrando o codigo dele pra o projeto. exempplo que pedi pro claude fazer:
seja 13 o expoente entao podemos reescrever ele assim
passo 1: 13 % 2 = 1  → inclui base^1    13 / 2 = 6
passo 2:  6 % 2 = 0  → não inclui       6  / 2 = 3
passo 3:  3 % 2 = 1  → inclui base^4    3  / 2 = 1
passo 4:  1 % 2 = 1  → inclui base^8    1  / 2 = 0 → para

resultado : base^1 × base^4 × base^8 = base^13
*/
//  to usando o int128 pra nao dar overflow caso seja maior que o tolerado por long long
long long mod_pow(long long base, long long exponent, long long modulus)
{
    long long resultado = 1; 
    base = base % modulus;

    while (exponent > 0)
    {
        if (exponent %2 == 1)
        {
            resultado =(__int128)resultado * base % modulus; //nao pode abreviar o operador de multiplicacao, testei e deu erro essa bomba
        }
        exponent /=2;
        base = (__int128)base * base % modulus; 
        
    }
    return resultado;
}

//cauet: função tot_euler feita, mas precisa ser revisada.
//Função necessária para calcular a chave pública e privada.
// terence: corrigido - tava dando overflow pra primos grandes (tipo 3bi+), mesma maracutaia do mod_pow
long long tot_euler(long long primo1, long long primo2) // tava chamando n sem usar ele.
{
    long long res = ((__int128)(primo1 - 1)) * (primo2 - 1); // <- sem o cast aqui o resultado fica errado silenciosamente

    return res;
}

//Função encontrarD implementada juntamente com o euclides estendido.
//A função encontrarD usa o euclides estendido para calcular o valor d, pois d é o inverso multiplicativo de 
long long euclidesEstendido(long long a, long long b, long long *s, long long *t)
{
    if (a == 0) 
    {
        *s = 0;
        *t = 1;
        return b;
    }
    
    long long s1, t1; // corrigido - o __int128 aqui tava causando ub, passava __int128* como long long* e corrompindo tudo
    long long resultado = euclidesEstendido(b % a, a, &s1, &t1);

    *s = (long long)(t1 - (__int128)(b / a) * s1); //quick fix 128int pra nao dar overflow no meio do calculo, igual tava no TESTE1.C
    *t = s1;
    
    return resultado;
}

long long encotrarD(long long e, long long p, long long q)// precisa ser long long, corrigido
{
    long long tot = tot_euler(p,q);

    long long j, k;

    long long resultado = euclidesEstendido(e,tot,&j,&k);

    if (resultado != 1) // Caso tenha algum erro (primos inválidos, expoente errado, etc)
    {
        printf("Erro: mdc != 1, não existe d!");
        return -1;
    }
    long long d = j % tot;
    if (d < 0)  d += tot; // <- Para a chave privada nunca ser negativa.
    return d;
}


////////////////////////////////////////////////////
// FUNCOES DE ARQUIVO
////////////////////////////////////////////////////


void criarChavePub(long long n, long long e)
{
    FILE *file;

    file = fopen("chavePub.txt", "w");
    fprintf(file, "%lld %lld", n, e);
    fclose(file);
    return;
}

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


////////////////////////////////////////////////////

// FUNCOES DE CRIPTOGRAFIA

////////////////////////////////////////////////////


// REVISAR ESSA PORRA, NAO ENTENDI A LOGICA 
void encriptar(char *mensagem, long long mensagemencriptada[], long long n, long long e)
{
    for (int i = 0; mensagem[i] != '\0'; i++)
    {
        mensagemencriptada[i] = mensagem[i]; // ← aqui, converte char pra ASCII
        mensagemencriptada[i] = mod_pow(mensagemencriptada[i], e, n); // ← aplica RSA
    }
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

void limparBufferEntrada()
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}


////////////////////////////////////////////////////
// FUNCOES EXPORTADAS (interface com o Django)
////////////////////////////////////////////////////


//implementacao completa e revisada
int gerarChavePub(long long primo1, long long primo2, long long expoente)
{
    if (primo(primo1) == 0)
    {
        return 1;
    }
    if (primo(primo2)== 0)
    {
        return 2;
    }

    long long n = primo1 * primo2;
    if (n <=255)
    {
        return 1;
    }
    
    long long phi = tot_euler(primo1,primo2);
    if (mdc(expoente,phi) != 1)
    {
        return 3; //expoente invalido
    }
    
    criarChavePub(n,expoente);
    return 0;
}

int encriptarMenu(char* mensagem, long long n, long long e)
{
    int tamanho = (int)strlen(mensagem);
    long long mensagemencriptada[tamanho];
    encriptar(mensagem, mensagemencriptada, n, e);
    salvarEmArquivo(mensagemencriptada, tamanho);
    return 0;
}

// terence: tinha problema aqui sim - buffer de 10000 chars truncava mensagens longas
// cada char encriptado vira um numero de ate 19 digitos no arquivo, entao ~500 chars ja estourava
// corrigido: agora pega o tamanho real do arquivo e le tudo de uma vez
int desencriptarMenu(long long p, long long q, long long int e)
{
    long long mensagemenc[100000], D, tamanho;
    char mensagem[100000]; // <- aumentei pra bater com o tamanho de mensagemenc
    int i = 0;

    FILE *file;
    file = fopen("textEncript.txt", "r");

    if (file == NULL) {
        perror("Erro ao abrir o arquivo");
        return 1;
    }

    // descobre o tamanho real do arquivo pra alocar certo
    fseek(file, 0, SEEK_END);
    long tamArquivo = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *bufArquivo = malloc(tamArquivo + 1); // +1 pro \0
    if (bufArquivo == NULL) {
        fclose(file);
        return 1;
    }

    fread(bufArquivo, 1, tamArquivo, file);
    bufArquivo[tamArquivo] = '\0';
    fclose(file);

    converterParaLongLong(bufArquivo, mensagemenc, &tamanho);
    free(bufArquivo);
    
    long long n = p * q;

    D = encotrarD(e, p, q);
    descriptografar(mensagemenc, tamanho, D, n);
    convertascii(mensagem, mensagemenc, tamanho);
    salvarEmArquivoD(mensagem, tamanho);
    return 0;
}