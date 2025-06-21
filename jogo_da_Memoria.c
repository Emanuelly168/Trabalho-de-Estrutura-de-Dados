#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include <ctype.h>

#define TAMANHO_TABULEIRO 4
#define NUM_PARES 8
#define MAX_NOME 50
#define ARQUIVO_PLACAR "placar.txt"
#define MAX_TENTATIVAS 15

typedef enum {
    COPAS,
    OUROS,
    ESPADAS,
    PAUS,
    SEM_NAIPE
} Naipe;

typedef enum {
    EM_ANDAMENTO,
    VITORIA,
    DERROTA,
    CANCELADO
} EstadoJogo;

typedef struct Carta {
    int valor;
    Naipe naipe;
    bool virada;
    bool encontrada;
    struct Carta *proxima;
} Carta;

const char* naipeParaString(Naipe naipe);

typedef struct Jogador {
    char nome[MAX_NOME];
    int pontuacao;
    int tentativas;
    struct Jogador *proximo;
} Jogador;

typedef struct {
    Carta *cartas[TAMANHO_TABULEIRO][TAMANHO_TABULEIRO];
    Jogador *jogadores;
    int paresEncontrados;
    EstadoJogo estado;
} JogoMemoria;

unsigned long calcularChecksum(const char *str) {
    unsigned long checksum = 0;
    while (*str) {
        checksum += (unsigned char)(*str);
        str++;
    }
    return checksum;
}

Carta* criarCarta(int valor, Naipe naipe) {
    Carta *novaCarta = (Carta*)malloc(sizeof(Carta));
    if (novaCarta == NULL) {
        perror("Erro ao alocar memoria para carta");
        return NULL;
    }
    novaCarta->valor = valor;
    novaCarta->naipe = naipe;
    novaCarta->virada = false;
    novaCarta->encontrada = false;
    novaCarta->proxima = NULL;
    return novaCarta;
}

bool adicionarJogador(Jogador **lista, const char *nome) {
    if (nome == NULL || strlen(nome) == 0) {
        printf("Nome do jogador invalido.\n");
        return false;
    }
    Jogador *novo = (Jogador*)malloc(sizeof(Jogador));
    if (novo == NULL) {
        perror("Erro ao alocar memoria para jogador");
        return false;
    }
    strncpy(novo->nome, nome, MAX_NOME - 1);
    novo->nome[MAX_NOME - 1] = '\0';
    novo->pontuacao = 0;
    novo->tentativas = 0;
    novo->proximo = *lista;
    *lista = novo;
    return true;
}

bool validarNome(const char *nome) {
    if (nome == NULL || strlen(nome) == 0) {
        printf("Nome nao pode ser vazio.\n");
        return false;
    }
    for (size_t i = 0; i < strlen(nome); i++) {
        if (!isalnum(nome[i]) && nome[i] != ' ') {
            printf("Nome contem caracteres invalidos.\n");
            return false;
        }
    }
    return true;
}

bool inicializarJogo(JogoMemoria *jogo, const char *nomeJogador) {
    if (jogo == NULL || nomeJogador == NULL) {
        printf("Parametros invalidos para inicializar jogo.\n");
        return false;
    }
    int valores[NUM_PARES * 2];
    Naipe naipes[NUM_PARES * 2];
    for (int i = 0; i < NUM_PARES; i++) {
        valores[2 * i] = i + 1;
        valores[2 * i + 1] = i + 1;
        Naipe naipe = (Naipe)(i % 4); 
        naipes[2 * i] = naipe;
        naipes[2 * i + 1] = naipe;
    }
    srand(time(NULL));
    for (int i = 0; i < NUM_PARES * 2; i++) {
        int j = rand() % (NUM_PARES * 2);
       
        int tempVal = valores[i];
        valores[i] = valores[j];
        valores[j] = tempVal;
        Naipe tempNaipe = naipes[i];
        naipes[i] = naipes[j];
        naipes[j] = tempNaipe;
    }
    int index = 0;
    for (int i = 0; i < TAMANHO_TABULEIRO; i++) {
        for (int j = 0; j < TAMANHO_TABULEIRO; j++) {
            if (index < NUM_PARES * 2) {
                jogo->cartas[i][j] = criarCarta(valores[index], naipes[index]);
                if (jogo->cartas[i][j] == NULL) {
                    printf("Erro ao criar carta na posicao %d,%d\n", i, j);
                    return false;
                }
                index++;
            } else {
                jogo->cartas[i][j] = NULL;
            }
        }
    }
    jogo->jogadores = NULL;
    if (!adicionarJogador(&jogo->jogadores, nomeJogador)) {
        printf("Erro ao adicionar jogador.\n");
        return false;
    }
    jogo->paresEncontrados = 0;
    jogo->estado = EM_ANDAMENTO;
    return true;
}

void liberarJogo(JogoMemoria *jogo) {
    if (jogo == NULL) return;
    for (int i = 0; i < TAMANHO_TABULEIRO; i++) {
        for (int j = 0; j < TAMANHO_TABULEIRO; j++) {
            if (jogo->cartas[i][j] != NULL) {
                free(jogo->cartas[i][j]);
            }
        }
    }
    Jogador *atual = jogo->jogadores;
    while (atual != NULL) {
        Jogador *temp = atual;
        atual = atual->proximo;
        free(temp);
    }
}

void exibirTabuleiro(JogoMemoria *jogo) {
    if (jogo == NULL) {
        printf("Jogo invalido.\n");
        return;
    }
    printf("\n   ");
    for (int j = 0; j < TAMANHO_TABULEIRO; j++) {
        printf("%2d ", j + 1);
    }
    printf("\n");
    for (int i = 0; i < TAMANHO_TABULEIRO; i++) {
        printf("%2d ", i + 1);
        for (int j = 0; j < TAMANHO_TABULEIRO; j++) {
            if (jogo->cartas[i][j] == NULL) {
                printf("   ");
            } else if (jogo->cartas[i][j]->encontrada || jogo->cartas[i][j]->virada) {
                printf("%2d ", jogo->cartas[i][j]->valor);
            } else {
                printf(" * ");
            }
        }
        printf("\n");
    }
    printf("\n");
}

bool verificarFimJogo(JogoMemoria *jogo) {
    if (jogo == NULL) return false;
    if (jogo->paresEncontrados >= NUM_PARES) {
        jogo->estado = VITORIA;
        return true;
    }
    if (jogo->jogadores != NULL && jogo->jogadores->tentativas >= MAX_TENTATIVAS) {
        jogo->estado = DERROTA;
        return true;
    }
    return false;
}

bool salvarPlacar(JogoMemoria *jogo) {
    if (jogo == NULL || jogo->jogadores == NULL) {
        printf("Dados invalidos para salvar placar.\n");
        return false;
    }
    FILE *arquivo = fopen(ARQUIVO_PLACAR, "a");
    if (arquivo == NULL) {
        perror("Erro ao abrir arquivo de placar");
        return false;
    }
    char buffer[512] = {0};
    time_t now;
    time(&now);
    struct tm *local = localtime(&now);
    snprintf(buffer, sizeof(buffer), "Data: %02d/%02d/%d %02d:%02d\n",
             local->tm_mday, local->tm_mon + 1, local->tm_year + 1900,
             local->tm_hour, local->tm_min);
    Jogador *j = jogo->jogadores;
    while (j != NULL) {
        char linha[256];
        snprintf(linha, sizeof(linha), "Jogador: %s - Pontuacao: %d - Tentativas: %d/%d\n",
                 j->nome, j->pontuacao, j->tentativas, MAX_TENTATIVAS);
        strncat(buffer, linha, sizeof(buffer) - strlen(buffer) - 1);
        j = j->proximo;
    }
    unsigned long checksum = calcularChecksum(buffer);
    fprintf(arquivo, "%s", buffer);
    fprintf(arquivo, "CHECKSUM:%lu\n", checksum);
    fprintf(arquivo, "----------------------------\n");
    if (fclose(arquivo)) {
        perror("Erro ao fechar arquivo de placar");
        return false;
    }
    return true;
}

void exibirPlacar() {
    printf("\n=== PLACAR ===\n");
    FILE *arquivo = fopen(ARQUIVO_PLACAR, "r");
    if (arquivo == NULL) {
        printf("Nenhum placar registrado ainda.\n");
        return;
    }
    char bloco[1024];
    char linha[256];
    int blocoPos = 0;
    int valido = 1;
    while (fgets(linha, sizeof(linha), arquivo)) {
        if (strncmp(linha, "----------------------------", 28) == 0) {
            bloco[blocoPos] = '\0';
            char *ptr = strstr(bloco, "CHECKSUM:");
            if (ptr) {
                unsigned long checksumArquivo = strtoul(ptr + 9, NULL, 10);
                *ptr = '\0';
                unsigned long checksumCalc = calcularChecksum(bloco);
                if (checksumArquivo != checksumCalc) {
                    printf("[AVISO] Placar adulterado ou corrompido!\n");
                    valido = 0;
                }
            }
            printf("%s----------------------------\n", bloco);
            blocoPos = 0;
            bloco[0] = '\0';
        } else {
            strncpy(bloco + blocoPos, linha, sizeof(bloco) - blocoPos - 1);
            blocoPos += strlen(linha);
        }
    }
    if (ferror(arquivo)) {
        perror("Erro ao ler arquivo de placar");
    }
    fclose(arquivo);
    printf("\n");
}

bool lerCoordenadas(int *linha, int *coluna) {
    char buffer[100];
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        printf("Erro ao ler entrada.\n");
        return false;
    }
    if (sscanf(buffer, "%d %d", linha, coluna) != 2) {
        printf("Formato invalido. Digite dois numeros separados por espaco.\n");
        return false;
    }
    return true;
}

bool lerCoordenadasOuSair(int *linha, int *coluna) {
    char buffer[100];
    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        printf("Erro ao ler entrada.\n");
        return false;
    }
    if (strncmp(buffer, "sair", 4) == 0 || strncmp(buffer, "q", 1) == 0) {
        return false;
    }
    if (sscanf(buffer, "%d %d", linha, coluna) != 2) {
        printf("Formato invalido. Digite dois numeros separados por espaco ou 'sair'.\n");
        return false;
    }
    return true;
}

void jogarJogoMemoria() {
    JogoMemoria jogo;
    char nomeJogador[MAX_NOME];
    printf("Bem-vindo ao Jogo da Memoria Avancado!\n");
    while (true) {
        printf("Digite seu nome (max %d caracteres alfanumericos): ", MAX_NOME-1);
        if (fgets(nomeJogador, MAX_NOME, stdin) == NULL) {
            perror("Erro ao ler nome do jogador");
            return;
        }
        nomeJogador[strcspn(nomeJogador, "\n")] = '\0';
        if (validarNome(nomeJogador)) {
            break;
        }
    }
    if (!inicializarJogo(&jogo, nomeJogador)) {
        printf("Falha ao inicializar jogo.\n");
        return;
    }
    printf("\nEncontre todos os %d pares de cartas.\n", NUM_PARES);
    printf("Voce tem %d tentativas para errar pares.\n", MAX_TENTATIVAS);
    printf("Digite as coordenadas (linha coluna) para virar as cartas.\n");
    printf("Exemplo: '1 2' para linha 1, coluna 2\n");
    printf("Digite 'sair' ou 'q' a qualquer momento para sair e salvar seu progresso.\n");
    while (jogo.estado == EM_ANDAMENTO) {
        exibirTabuleiro(&jogo);
        int linha1, coluna1, linha2, coluna2;
        printf("Primeira carta (linha coluna): ");
        if (!lerCoordenadasOuSair(&linha1, &coluna1)) {
            printf("Saindo do jogo. Seu progresso sera salvo.\n");
            jogo.estado = CANCELADO;
            break;
        }
        linha1--; coluna1--;
        if (linha1 < 0 || linha1 >= TAMANHO_TABULEIRO ||
            coluna1 < 0 || coluna1 >= TAMANHO_TABULEIRO) {
            printf("Posicao invalida! Use valores entre 1 e %d.\n", TAMANHO_TABULEIRO);
            continue;
        }
        if (jogo.cartas[linha1][coluna1] == NULL) {
            printf("Erro interno: carta nula.\n");
            continue;
        }
        if (jogo.cartas[linha1][coluna1]->encontrada ||
            jogo.cartas[linha1][coluna1]->virada) {
            printf("Carta ja virada ou encontrada!\n");
            continue;
        }
        printf("Segunda carta (linha coluna): ");
        if (!lerCoordenadasOuSair(&linha2, &coluna2)) {
            printf("Saindo do jogo. Seu progresso sera salvo.\n");
            jogo.estado = CANCELADO;
            break;
        }
        linha2--; coluna2--;
        if (linha2 < 0 || linha2 >= TAMANHO_TABULEIRO ||
            coluna2 < 0 || coluna2 >= TAMANHO_TABULEIRO) {
            printf("Posicao invalida! Use valores entre 1 e %d.\n", TAMANHO_TABULEIRO);
            continue;
        }
        if (linha1 == linha2 && coluna1 == coluna2) {
            printf("Voce selecionou a mesma carta duas vezes!\n");
            continue;
        }
        if (jogo.cartas[linha2][coluna2] == NULL) {
            printf("Erro interno: carta nula.\n");
            continue;
        }
        if (jogo.cartas[linha2][coluna2]->encontrada ||
            jogo.cartas[linha2][coluna2]->virada) {
            printf("Carta ja virada ou encontrada!\n");
            continue;
        }
        jogo.cartas[linha1][coluna1]->virada = true;
        jogo.cartas[linha2][coluna2]->virada = true;
        exibirTabuleiro(&jogo);
        if (jogo.cartas[linha1][coluna1]->valor == jogo.cartas[linha2][coluna2]->valor) {
            printf("Par encontrado! +10 pontos\n");
            printf("Naipe do par: %s\n", naipeParaString(jogo.cartas[linha1][coluna1]->naipe));
            jogo.cartas[linha1][coluna1]->encontrada = true;
            jogo.cartas[linha2][coluna2]->encontrada = true;
            jogo.jogadores->pontuacao += 10;
            jogo.paresEncontrados++;
        } else {
            printf("Nao e um par. -1 ponto\n");
            jogo.cartas[linha1][coluna1]->virada = false;
            jogo.cartas[linha2][coluna2]->virada = false;
            jogo.jogadores->pontuacao -= 1;
            jogo.jogadores->tentativas++;
        }
        if (verificarFimJogo(&jogo)) {
            if (jogo.estado == VITORIA) {
                printf("\nParabens, %s! Voce encontrou todos os pares!\n", jogo.jogadores->nome);
                printf("Pontuacao final: %d\n", jogo.jogadores->pontuacao);
            } else {
                printf("\nFim de jogo! Voce esgotou suas %d tentativas.\n", MAX_TENTATIVAS);
                printf("Pontuacao final: %d\n", jogo.jogadores->pontuacao);
            }
            break;
        }
        printf("Pressione Enter para continuar...");
        while (getchar() != '\n');
    }
    if (!salvarPlacar(&jogo)) {
        printf("O placar nao foi salvo corretamente.\n");
    } else if (jogo.estado == CANCELADO) {
        printf("Seu progresso foi salvo como jogo cancelado.\n");
    }
    liberarJogo(&jogo);
}

void exibirMenu() {
    int opcao;
    do {
        printf("\n=== JOGO DA MEMORIA ===\n");
        printf("1. Iniciar novo jogo\n");
        printf("2. Ver placar\n");
        printf("3. Sair\n");
        printf("Escolha uma opcao: ");
        char buffer[100];
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
            perror("Erro ao ler opcao");
            break;
        }
        if (sscanf(buffer, "%d", &opcao) != 1) {
            printf("Opcao invalida!\n");
            continue;
        }
        switch (opcao) {
            case 1:
                jogarJogoMemoria();
                break;
            case 2:
                exibirPlacar();
                break;
            case 3:
                printf("Ate logo!\n");
                break;
            default:
                printf("Opcao invalida!\n");
        }
    } while (opcao != 3);
}

const char* naipeParaString(Naipe naipe) {
    switch (naipe) {
        case COPAS: return "Copas";
        case OUROS: return "Ouros";
        case ESPADAS: return "Espadas";
        case PAUS: return "Paus";
        default: return "Sem Naipe";
    }
}

int main() {
    exibirMenu();
    return 0;
}