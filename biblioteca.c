#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LIVROS 100
#define TAM_TEXTO 101
#define TAM_CAMINHO 256
#define ARQUIVO_DADOS "dados_biblioteca.txt"
#define ARQUIVO_TEMP "dados_biblioteca.tmp"

typedef struct livro {
    int codigo;
    char titulo[TAM_TEXTO];
    char autor[TAM_TEXTO];
    char caminho[TAM_CAMINHO];
} Livro;

void mostrarLista(const Livro *livro);
int lerNumero(const char *pergunta);
int carregarLivros(Livro livros[]);
void cadastrarLivro(Livro livros[], int *quantidade);
void editarLivro(Livro livros[], int quantidade);
void listarLivros(Livro livros[], int quantidade);
void pesquisarLivros(Livro livros[], int quantidade);

int main(void) {
    Livro livros[MAX_LIVROS];
    int quantidade, opcao;
    /*carrega os livros do arquivo dados_biblioteca 4 linhas por livro*/
    quantidade = carregarLivros(livros);

    do {
        printf("\n===== BIBLIOTECA =====\n");
        printf("1 - Cadastrar livro\n"
               "2 - Editar cadastro\n"
               "3 - Listar livros\n"
               "4 - Pesquisar livro\n"
               "0 - Sair\n");
        opcao = lerNumero("Escolha: ");
        switch (opcao) {
            case 1: cadastrarLivro(livros, &quantidade); break;
            case 2: editarLivro(livros, quantidade); break;
            case 3: listarLivros(livros, quantidade); break;
            case 4: pesquisarLivros(livros, quantidade); break;
            case 0: printf("Tchau!\n"); break;
            default: printf("Opção inválida.\n");
        }
    } while (opcao != 0);
    return 0;
}

/* le todo fgets e tira /n e coloca um \0 */
int lerLinha(FILE *arquivo, char texto[], int tamanho) {
    char linha[TAM_CAMINHO + 2];
    if (fgets(linha, sizeof linha, arquivo) == NULL) return 0;
    linha[strcspn(linha, "\r\n")] = '\0';
    linha[tamanho - 1] = '\0';
    strcpy(texto, linha);
    return 1;
}

int lerNumero(const char *pergunta) {
    char texto[64];
    int numero;
    do {
        printf("%s", pergunta);
        if (!lerLinha(stdin, texto, sizeof texto)) exit(0);
    } while (sscanf(texto, "%d", &numero) != 1);
    return numero;
}

void lerCampo(const char *pergunta, char campo[], int tamanho) {
    char texto[TAM_CAMINHO];
    printf("%s", pergunta);
    if (!lerLinha(stdin, texto, tamanho)) exit(0);
    if (texto[0] != '\0') strcpy(campo, texto);
}

int indicePorCodigo(const Livro livros[], int quantidade, int codigo) {
    int i;
    for (i = 0; i < quantidade; i++) {
        if (livros[i].codigo == codigo) return i;
    }
    return quantidade;
}

/* código, título, autor e caminho. */
int carregarLivros(Livro livros[]) {
    FILE *arquivo = fopen(ARQUIVO_DADOS, "r");
    char linha[64];
    Livro livro = {0};
    int quantidade = 0;

    if (arquivo == NULL) {
        arquivo = fopen(ARQUIVO_DADOS, "wx");
        if (arquivo == NULL) {
            printf("Não foi possível abrir ou criar %s.\n", ARQUIVO_DADOS);
            exit(1);
        }
        fclose(arquivo);
        return 0;
    }
    while (quantidade < MAX_LIVROS &&
           lerLinha(arquivo, linha, sizeof linha) &&
           lerLinha(arquivo, livro.titulo, sizeof livro.titulo) &&
           lerLinha(arquivo, livro.autor, sizeof livro.autor) &&
           lerLinha(arquivo, livro.caminho, sizeof livro.caminho)) {
        livro.codigo = atoi(linha);
        livros[quantidade] = livro;
        quantidade++;
    }
    fclose(arquivo);
    return quantidade;
}

int salvarLivros(const Livro livros[], int quantidade) {
    FILE *arquivo = fopen(ARQUIVO_TEMP, "w");
    int i, gravou = 1;
    if (arquivo == NULL) return 0;

    for (i = 0; i < quantidade; i++) {
        if (fprintf(arquivo, "%d\n%s\n%s\n%s\n", livros[i].codigo,
                    livros[i].titulo, livros[i].autor, livros[i].caminho) < 0) {
            gravou = 0;
            break;
        }
    }
    /* Troca o catálogo depois de terminar a gravação. */
    if (fclose(arquivo) == 0 && gravou && rename(ARQUIVO_TEMP, ARQUIVO_DADOS) == 0) return 1;
    remove(ARQUIVO_TEMP);
    return 0;
}

void mostrarLivro(const Livro *livro) {
    printf("\nCódigo: %d | Título: %s | Autor: %s\nArquivo: %s\n",
           livro->codigo, livro->titulo, livro->autor, livro->caminho);
}

void lerDadosLivro(Livro *livro) {
    lerCampo("Título: ", livro->titulo, sizeof livro->titulo);
    lerCampo("Autor: ", livro->autor, sizeof livro->autor);
    lerCampo("Arquivo .txt: ", livro->caminho, sizeof livro->caminho);
}

void cadastrarLivro(Livro livros[], int *quantidade) {
    Livro novo = {0};
    if (*quantidade == MAX_LIVROS) {
        printf("Biblioteca cheia.\n");
        return;
    }

    novo.codigo = lerNumero("Código do livro (0 volta): ");
    if (novo.codigo == 0) return;
    if (novo.codigo < 0 || indicePorCodigo(livros, *quantidade, novo.codigo) < *quantidade) {
        printf("Código inválido ou já cadastrado.\n");
        return;
    }

    lerDadosLivro(&novo);
    if (!novo.titulo[0] || !novo.autor[0] || !novo.caminho[0]) {
        printf("Preencha os três campos do livro.\n");
        return;
    }
    livros[*quantidade] = novo;
    if (salvarLivros(livros, *quantidade + 1)) {
        (*quantidade)++;
        printf("Livro cadastrado.\n");
    } else {
        printf("Falha ao salvar o cadastro.\n");
    }
}

void editarLivro(Livro livros[], int quantidade) {
    int codigo, indice;
    Livro anterior;
    codigo = lerNumero("Código do livro (0 volta): ");
    if (codigo == 0) return;
    indice = indicePorCodigo(livros, quantidade, codigo);
    if (indice == quantidade) {
        printf("Livro não encontrado.\n");
        return;
    }

    mostrarLivro(&livros[indice]);
    printf("Pressione Enter para manter o valor de um campo.\n");
    anterior = livros[indice];
    lerDadosLivro(&livros[indice]);
    if (salvarLivros(livros, quantidade)) {
        printf("Cadastro atualizado.\n");
    } else {
        livros[indice] = anterior;
        printf("Falha ao salvar. Edição cancelada.\n");
    }
}
/*negativo antes do pivo; positivo dps dp pivo*/
int compararCodigo(const void *a, const void *b) {
    const Livro *primeiro = a, *segundo = b;
    return primeiro->codigo - segundo->codigo;
}

int compararLivros(const Livro *a, const Livro *b, int criterio) {
    if (criterio == 2) return strcmp(a->titulo, b->titulo);
    if (criterio == 3) return strcmp(a->autor, b->autor);
    return compararCodigo(a, b);
}

void quicksort(Livro livros[], int inicio, int fim, int criterio) {
    int esquerda = inicio, direita = fim;
    Livro pivo, temporario;

    if (inicio >= fim) return;
    pivo = livros[(inicio + fim) / 2];
    while (esquerda <= direita) {
        while (compararLivros(&livros[esquerda], &pivo, criterio) < 0) esquerda++;
        while (compararLivros(&livros[direita], &pivo, criterio) > 0) direita--;
        if (esquerda <= direita) {
            temporario = livros[esquerda];
            livros[esquerda] = livros[direita];
            livros[direita] = temporario;
            esquerda++;
            direita--;
        }
    }
    /* Ordena as duas partes do vetor. */
    quicksort(livros, inicio, direita, criterio); // esquerda
    quicksort(livros, esquerda, fim, criterio);  // direita
}

void abrirLivro(const Livro *livro) {
    FILE *arquivo = fopen(livro->caminho, "r");
    char linha[TAM_CAMINHO];
    if (arquivo == NULL) {
        printf("Não foi possível abrir %s.\n", livro->caminho);
        return;
    }
    printf("\n");
    while (fgets(linha, sizeof linha, arquivo) != NULL) {
        printf("%s", linha);
    }
    fclose(arquivo);
}
void mostrarLista(const Livro *livro) {
    printf("\nCódigo: %d\nTítulo: %s\nAutor: %s\nArquivo: %s\n",
           livro->codigo, livro->titulo, livro->autor, livro->caminho);
}
void mostrarResultados(const Livro *resultados[], int quantidade) {
    int i, codigo;
    if (quantidade == 0) {
        printf("Nenhum livro encontrado.\n");
        return;
    }
    for (i = 0; i < quantidade; i++) {
        mostrarLista(resultados[i]);
    }

    codigo = lerNumero("\nCódigo para abrir (0 volta): ");
    if (codigo == 0) return;
    for (i = 0; i < quantidade; i++) {
        if (resultados[i]->codigo == codigo) {
            abrirLivro(resultados[i]);
            return;
        }
    }
    printf("Código não encontrado nesta lista.\n");
}

int escolherCriterio(const char *titulo, int quantidade) {
    int criterio;
    if (quantidade == 0) {
        printf("Nenhum livro cadastrado.\n");
        return 0;
    }
    printf("%s\n1 - Código | 2 - Título | 3 - Autor | 0 - Voltar\n", titulo);
    criterio = lerNumero("Escolha: ");
    if (criterio < 0 || criterio > 3) {
        printf("Opção inválida.\n");
        return 0;
    }
    return criterio;
}

void listarLivros(Livro livros[], int quantidade) {
    const Livro *resultados[MAX_LIVROS];
    int i, criterio = escolherCriterio("Ordenar por", quantidade);
    if (criterio == 0) return;

    quicksort(livros, 0, quantidade - 1, criterio);
    for (i = 0; i < quantidade; i++) {
        resultados[i] = &livros[i];
    }
    mostrarResultados(resultados, quantidade);
}

void pesquisarLivros(Livro livros[], int quantidade) {
    const Livro *resultados[MAX_LIVROS], *livro;
    char termo[TAM_TEXTO] = "";
    Livro chave = {0};
    int i, encontrados = 0;
    int criterio = escolherCriterio("Pesquisar por", quantidade);

    if (criterio == 0) return;
    if (criterio == 1) {
        chave.codigo = lerNumero("Código: ");
        if (chave.codigo <= 0) return;
        /* A busca binária exige que os códigos estejam em ordem. */
        quicksort(livros, 0, quantidade - 1, 1);
        livro = bsearch(&chave, livros, (size_t)quantidade, sizeof(Livro), compararCodigo);
        if (livro != NULL) {
            resultados[0] = livro;
            encontrados = 1;
        }
    } else {
        lerCampo("Texto procurado: ", termo, sizeof termo);
        if (termo[0] == '\0') return;
        for (i = 0; i < quantidade; i++) {
            const char *campo = criterio == 2 ? livros[i].titulo : livros[i].autor;
            if (strstr(campo, termo) != NULL) {
                resultados[encontrados] = &livros[i];
                encontrados++;
            }
        }
    }
    mostrarResultados(resultados, encontrados);
}
