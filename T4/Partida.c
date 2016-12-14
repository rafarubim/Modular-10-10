#include "Partida.h"

#include "Peao.h"
#include "ListaC.h"
#include "Lista.h"
#include "Tabuleiro.h"
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <time.h>

#define NUM_PEOES 4
	/* Número de peões que cada jogador possui */

#define MAX_REPETICOES_TURNO 3
	/* Quantidade máxima de turnos consecutivos que um jogador pode jogar */

/*******************************************************************************************************************************
*	Dados encapsulados no módulo:
*******************************************************************************************************************************/

typedef struct PART_tgJogador {
    
    LIS_tppLista pLstPeoes;
    DEF_tpCor Cor;

} PART_tpJogador;

static LSTC_tppListaC lstJogadores = NULL;
	/* Lista circular de jogadores. O corrente é aquele cujo turno está em andamento. */
	/* Inicialmente NULL para representar nenhuma partida em andamento */

static int iQtdJogadores;
	/* Quantidade de jogadores participando da partida. */

static PART_tpJogador* pUltimoJogador;
	/* Jogador que foi o último a jogar (cujo turno acabou de passar) - NULL se ninguém houver jogado */

static int iNumJogadasRepetidas;
	/* Número de jogadas repetidas que o jogador de turno atual já jogou. Ele jogará outra jogada repetida caso tire 6 ou coma
	outro peão em sua vez. Se esse número passar de MAX_REPETICOES_TURNO-1, a vez é passada para o próximo jogador */

/*******************************************************************************************************************************
*	Protótipos de funções encapsuladas no módulo:
*******************************************************************************************************************************/

static void ExcluirPeao(void* pVoid);

static void ExcluirJogador(void* pVoid);

static PART_tpCondRet PART_ChecarPeoesDisponiveis(PART_tpJogador* jogadorVez, int dado, LIS_tppLista peoesDisponiveis);

static PART_tpCondRet PART_Escolher(LIS_tppLista peoesDisponiveis, PEAO_tppPeao* peaoEscolhidoRet);

static PART_tpCondRet PART_ImprimirCor(DEF_tpCor cor);

/*******************************************************************************************************************************
*	Código de funções exportadas pelo módulo:
*******************************************************************************************************************************/

/*******************************************************************************************************************************
*	Função: PART_CriarPartida
*/
PART_tpCondRet PART_CriarPartida(int n) {
	PEAO_tpCondRet debugPeao;
	LSTC_tpCondRet debugLstC;
	LIS_tpCondRet debugLista;
	TAB_tpCondRet debugTabuleiro;
	int i;

	/* Número mínimo e máximo de jogadores de um jogo de ludo */
	if (n < 2 || n > 4)
		return PART_CondRetNumInvalido;
	iQtdJogadores = n;

	/* Lista de jogadores é circular */
	debugLstC = LSTC_CriarListaC(&lstJogadores, ExcluirJogador);
	/* Se não retornou OK, erro */
	if (debugLstC)	return PART_CondRetErroListaC;

	/* Nenhum jogador jogou ainda */
	pUltimoJogador = NULL;

	/* Número de jogadas repetidas ainda é zero */
	iNumJogadasRepetidas = 0;

	/* Criar tabuleiro */
	debugTabuleiro = TAB_CriarTabuleiro();
	/* Se não retornou OK, erro */
	if (debugTabuleiro)	return PART_CondRetErroTabuleiro;

	/* Criar cada jogador e inseri-lo em lista: */
	for (i = 0; i < n; i++) {
		DEF_tpCor cor;
		PART_tpJogador* jog;
		switch(i) {
			case 0:	cor = AZUL;		break;
			case 1: cor = VERMELHO;	break;
			case 2:	cor = VERDE;	break;
			case 3:	cor = AMARELO;	break;
		}
		jog = (PART_tpJogador*)malloc(sizeof(PART_tpJogador));
		if (!jog)	return PART_CondRetSemMemoria;
		jog->Cor = cor;
		debugLista = LIS_CriarLista(&jog->pLstPeoes, ExcluirPeao);
		if (debugLista)	return PART_CondRetErroLista;
		/* Criar cada peão do jogador: */
		for (i = 0; i < NUM_PEOES; i++) {
			PEAO_tppPeao tempPeao;
			debugPeao = PEAO_CriarPeao(&tempPeao, cor, i+1);
			if (debugPeao)	return PART_CondRetErroPeao;
			debugLista = LIS_InserirElementoApos(jog->pLstPeoes, tempPeao);
			if (debugLista)	return PART_CondRetErroListaC;
		}	/* for */
	}	/* for */
	return PART_CondRetOK;
} /* Fim Função PART_CriarPartida */

/*******************************************************************************************************************************
*	Função: PART_Jogar
*/
PART_tpCondRet PART_Jogar() {
	LSTC_tpCondRet debugListaC;
	PART_tpCondRet debugPartida;
	LIS_tpCondRet debugLista;
	PEAO_tpCondRet debugPeao;
	PART_tpJogador* jogAtual;
	LIS_tppLista lstPeoesDisponiveis;
	PEAO_tppPeao peaoEscolhido;
	DEF_tpBool peaoEstaBase;
	int numDado, qtdPeoesDisponiveis, jogarNovamente = 0;

	if (lstJogadores == NULL)	return PART_CondRetPartidaInexistente;

	/* Pegar jogador atual */
	debugListaC = LSTC_ObterElemento(lstJogadores, 0, (void**)&jogAtual);
	/* Se não retornou OK, erro */
	if (debugListaC)	return PART_CondRetErroListaC;

	printf("Jogada do jogador ");
	/* Imprimir cor do jogador */
	debugPartida = PART_ImprimirCor(jogAtual->Cor);
	/* Se não retornou OK, erro */
	if (debugPartida)	return debugPartida;
	printf(":\n\n");

	printf("Pressione qualquer tecla para jogar o dado...");

	getch();

	srand(time(NULL));
	numDado = (rand() % 6) + 1;

	printf("\nO dado rola... Voce sorteou um %d!\n", numDado);

	if (numDado == 6)
		jogarNovamente = 1;

	/* Criar lista vazia temporária para colocar peões disponíveis (sem função de exclusão pois a lista é de ponteiros temporários) */
	debugLista = LIS_CriarLista(&lstPeoesDisponiveis, NULL);
	/* Se não retornou OK, erro */
	if (debugLista)	return PART_CondRetErroLista;

	/* Colocar peões disponíveis para movimentação na lista */
	debugPartida = PART_ChecarPeoesDisponiveis(jogAtual, numDado, lstPeoesDisponiveis);
	/* Se não retornou OK, erro */
	if (debugPartida)	return debugPartida;

	/* Pegar quantos peões estão disponíveis */
	debugLista = LIS_ObterTamanhoLista(lstPeoesDisponiveis, &qtdPeoesDisponiveis);
	/* Se não retornou OK, erro */
	if (debugLista)	return PART_CondRetErroLista;

	if (qtdPeoesDisponiveis == 0) {
		printf("Nenhum de seus peoes pode andar esse numero de casas!\nFim de turno...\n\n");

		/* Destruir lista temporária */
		debugLista = LIS_DestruirLista(lstPeoesDisponiveis);
		/* Se não retornou OK, erro */
		if (debugLista)	return PART_CondRetErroLista;

		if (!jogarNovamente || iNumJogadasRepetidas >= MAX_REPETICOES_TURNO-1) {
				/* Se o jogador atual não jogará novamente ou já jogou 3 vezes seguidas (2 repetidas) */

			LSTC_MoverCorrente(lstJogadores, 1);
			iNumJogadasRepetidas = 0;
		} else {
				/* Caso que o jogador atual jogará novamente */

			iNumJogadasRepetidas++;
		}	/* if */

		/* Marca o último a jogar */
		pUltimoJogador = jogAtual;

		/* Encerrar jogada */
		return PART_CondRetOK;
	}	/* if */
		/* Caso em que há peões disponíveis para movimentar, movimentação obrigatória */

	/* Pedir para jogador escolher o peão que deseja movimentar */
	debugPartida = PART_Escolher(lstPeoesDisponiveis, &peaoEscolhido);
	/* Se não retornou OK, erro */
	if (debugPartida)	return debugPartida;

	/* Ver se peão está na base */
	debugPeao = PEAO_EstaPeaoBase(peaoEscolhido, &peaoEstaBase);
	/* Se não retornou OK, erro */
	if (debugPeao)	return PART_CondRetErroPeao;

	if (peaoEstaBase) {
		DEF_tpBool podeAndar;
		DEF_tpCor corAComer;

		/* Se está na base e estava disponível, então o jogador tem que ter tirado 6 */
		if (numDado != 6)
			return PART_CondRetInconsistencia;

		/* Pegar possível cor de um peão que será comido na realização do movimento */
		debugPeao = PEAO_ChecarMovimentoDisponivelPeao(peaoEscolhido, numDado, &podeAndar, &corAComer);
		/* Se não retornou OK, erro */
		if (debugPeao)	return PART_CondRetErroPeao;

		//PEAO_

		/* Deve ser possível andar, se peão já estava disponível */
		if (!podeAndar)
			return PART_CondRetInconsistencia;

		if 
	}



	/* Marca o último a jogar */
	pUltimoJogador = jogAtual;

	/* Destruir lista temporária */
	debugLista = LIS_DestruirLista(lstPeoesDisponiveis);
	/* Se não retornou OK, erro */
	if (debugLista)	return PART_CondRetErroLista;

	return PART_CondRetOK;
}	/* Fim Função PART_Jogar */

/*******************************************************************************************************************************
*	Função: PART_ChecarVitoria
*/
PART_tpCondRet PART_ChecarVitoria(DEF_tpBool* BoolRet, DEF_tpCor* CorVencedorRet) {
	int i, qtdPeoes, peoesFinal = 0, qtdJog;
	PART_tpJogador* jog;
	PEAO_tppPeao peao;
	LIS_tppLista lstPeoes;
	LSTC_tpCondRet debugListaC;
	LIS_tpCondRet debugLista;

	if (lstJogadores == NULL)	return PART_CondRetPartidaInexistente;

	if (!pUltimoJogador)	return PART_CondRetNinguemJogou;

	jog = pUltimoJogador;

	/* Pega quantidade de peões */
	lstPeoes = jog->pLstPeoes;
	debugLista = LIS_ObterTamanhoLista(lstPeoes, &qtdPeoes);
	/* Se não retornou OK, erro */
	if (debugLista)	return PART_CondRetErroLista;

	/* Vai para início da lista de peões do jogador pego */
	debugLista = LIS_IrInicioLista(lstPeoes);
	/* Se não retornou OK, erro */
	if (debugLista)	return PART_CondRetErroLista;

	/* i conta a posição atual na lista de peões */
	i = 0;

	/* Percorre a lista de peões */
	while(1) {
		PEAO_tpCondRet debugPeao;
		PEAO_tppPeao peao;
		DEF_tpBool final;

		/* Obtém peão */
		debugLista = LIS_ObterValor(lstPeoes, (void**)&peao);
		/* Se não retornou OK, erro */
		if (debugLista)	return PART_CondRetErroLista;

		/* Testa se ele está no final */
		debugPeao = PEAO_EstaPeaoFinal(peao, &final);
		/* Se não retornou OK, erro */
		if (debugPeao)	return PART_CondRetErroPeao;

		/* Se o peão estiver no final, conta-o */
		if (final)
			peoesFinal++;

		/* Se está no último peão da lista, não avança a lista e para de percorrê-la */
		if (i >= qtdPeoes-1)
			break;

		/* Vai para o próximo peão */
		debugLista = LIS_AvancarElementoCorrente(jog->pLstPeoes, 1);
		/* Se não retornou OK, erro */
		if (debugLista)	return PART_CondRetErroLista;

		i++;
	}	/* while */

	/* Se todos os peões do jogador estão no final, ele ganhou! */
	if (peoesFinal == qtdPeoes)
		*BoolRet = True;
	else
		*BoolRet = False;
	*CorVencedorRet = jog->Cor;
	return PART_CondRetOK;
} /* Fim Função PART_ChecarVitoria */

/*******************************************************************************************************************************
*	Função: PART_DestruirPartida
*/
PART_tpCondRet PART_DestruirPartida() {
	LSTC_tpCondRet debugListaC;
	TAB_tpCondRet debugTabuleiro;

	if (lstJogadores == NULL)	return PART_CondRetPartidaInexistente;

	debugTabuleiro = TAB_DestruirTabuleiro();
	/* Se não retornou OK, erro */
	if (debugTabuleiro)	return PART_CondRetErroTabuleiro;

	/* Libera lista de jogadores */
	debugListaC = LSTC_DestruirListaC(lstJogadores);
	/* Se não retornou OK, erro */
	if (debugListaC)	return PART_CondRetErroListaC;

	lstJogadores = NULL;

	return PART_CondRetOK;
} /* Fim Função PART_DestruirPartida */

/*******************************************************************************************************************************
*	Código de funções encapsuladas no módulo:
*******************************************************************************************************************************/

/*******************************************************************************************************************************
*	$FC Função: ExcluirPeao
*
*	$ED Descrição da função:
*		Função de exclusão de peões de uma lista.
*
*	$EP Parâmetros:
*		$P pVoid			-	parâmetro que receberá ponteiro para o peão em formato genérico e o liberará da memória.
*******************************************************************************************************************************/
static void ExcluirPeao(void* pVoid) {
	PEAO_tppPeao pPeao = (PEAO_tppPeao) pVoid;

	/* Libera peão */
	PEAO_DestruirPeao(pPeao);
}	/* Fim Função ExcluirPeao */

/*******************************************************************************************************************************
*	$FC Função: ExcluirPeao
*
*	$ED Descrição da função:
*		Função de exclusão de jogadores de uma lista.
*
*	$EP Parâmetros:
*		$P pVoid			-	parâmetro que receberá ponteiro para o jogador em formato genérico e o liberará da memória.
*******************************************************************************************************************************/
static void ExcluirJogador(void* pVoid) {
	PART_tpJogador* pJog = (PART_tpJogador*) pVoid;

	/* Libera lista de peões */
	LIS_DestruirLista(pJog->pLstPeoes);
	/* Libera jogador */
	free(pJog);
}	/* Fim Função ExcluirJogador */

/*******************************************************************************************************************************
*	$FC Função: PART_ChecarPeoesDisponiveis
*
*	$ED Descrição da função:
*		Recebe um jogador, o número de casas que ele deseja andar com um de seus peões e uma lista vazia. A função encherá a
*		lista com os peões do jogador que podem ser escolhidos para serem movimentados (peões cujo movimento é válido). A
*		função de exclusão de elementos dessa lista NÃO deve excluir os peões, uma vez que a lista é apenas de ponteiros
*		temporários para peões que existem na duração da partida.
*
*	$EP Parâmetros:
*		$P jogadorVez			-	jogador em questão
*		$P iNumDado				-	número tirado no dado (quantidade de casas que se deseja andar)
*		$P peoesDisponiveis		-	lista que deve estar inicialmente vazia
*
*	$FV Valor retornado:
*		PART_CondRetOK
*		PART_CondRetInconsistencia
*		PART_CondRetSemMemoria
*		PART_CondRetErroLista
*		PART_CondRetErroPeao
*******************************************************************************************************************************/
static PART_tpCondRet PART_ChecarPeoesDisponiveis(PART_tpJogador* jogadorVez, int iNumDado, LIS_tppLista peoesDisponiveis) {
	LIS_tpCondRet debugLista;
	LIS_tppLista lstPeoes;
	int tamLst, i;

	/* Pegar tamanho da lista passada: */
	debugLista = LIS_ObterTamanhoLista(peoesDisponiveis, &tamLst);
	/* Se não retornou OK, erro */
	if (debugLista)	return PART_CondRetErroLista;
	/* Se a lista passada é não vazia, erro */
	if (tamLst != 0)	return PART_CondRetInconsistencia;

	lstPeoes = jogadorVez->pLstPeoes;
		/* Lista de peões do jogador */

	/* Vai para o primeiro peão */
	debugLista = LIS_IrInicioLista(lstPeoes);
	/* Se não retornou OK, erro */
	if (debugLista)	return PART_CondRetErroLista;

	i = 0;
		/* Posição na lista */
	/* Percorrer lista de peões: */
	while(1) {
		PEAO_tppPeao tempPeao;
		DEF_tpBool movimentoDisp;
		DEF_tpCor corAFrente;
		PEAO_tpCondRet debugPeao;

		/* Pegar peão */
		debugLista = LIS_ObterValor(lstPeoes, (void**)&tempPeao);
		/* Se não retornou OK, erro */
		if (debugLista)	return PART_CondRetErroLista;

		/* Ver se ele pode andar */
		debugPeao = PEAO_ChecarMovimentoDisponivelPeao(tempPeao, iNumDado, &movimentoDisp, &corAFrente);
		/* Se não retornou OK, erro */
		if (debugPeao)	return PART_CondRetErroPeao;

		/* Se puder andar */
		if (movimentoDisp) {
			/* Adicionar peão à lista de peões disponíveis */
			debugLista = LIS_InserirElementoApos(peoesDisponiveis, tempPeao);
			/* Se não retornou OK, erro */
			if (debugLista)	return PART_CondRetErroLista;
		}	/* if */

		/* Se chegou em último elemento da lista, para de percorrer */
		if (i >= tamLst - 1)
			break;

		/* Avança em lista */
		debugLista = LIS_AvancarElementoCorrente(lstPeoes, 1);
		/* Se não retornou OK, erro */
		if (debugLista)	return PART_CondRetErroLista;

		i++;
	}	/* while */

	return PART_CondRetOK;
}	/* Fim Função PART_ChecarPeoesDisponiveis */

/*******************************************************************************************************************************
*	$FC Função: PART_Escolher
*
*	$ED Descrição da função:
*		Recebe uma lista não vazia de peões e apresenta seus números identificadores, através da interface do console, ao
*		usuário. O usuário poderá escolher um deles através do teclado. A função retorna o peão escolhido.
*
*	$EP Parâmetros:
*		$P peoesDisponiveis	-	lista não vazia de peões
*		$P peaoEscolhidoRet	-	parâmetro que receberá peão escolhido.
*								Este parâmetro é passado como referência.
*
*	$FV Valor retornado:
*		PART_CondRetOK
*		PART_CondRetInconsistencia
*		PART_CondRetErroLista
*		PART_CondRetErroPeao
*******************************************************************************************************************************/
static PART_tpCondRet PART_Escolher(LIS_tppLista peoesDisponiveis, PEAO_tppPeao* peaoEscolhidoRet) {
	LIS_tpCondRet debugLista;
	int tamLst, i, numEscolhido;

	/* Pegar tamanho da lista */
	debugLista = LIS_ObterTamanhoLista(peoesDisponiveis, &tamLst);
	/* Se não retornou OK, erro */
	if (debugLista)	return PART_CondRetErroLista;
	/* Se a lista está vazia, erro */
	if (tamLst == 0)	return PART_CondRetInconsistencia;

	debugLista = LIS_IrInicioLista(peoesDisponiveis);
	/* Se não retornou OK, erro */
	if (debugLista)	return PART_CondRetErroLista;

	printf("Escolha um dos seguintes peoes para movimentar: ");

	i = 0;
		/* Posição atual na lista */

	while(1) {
		PEAO_tpCondRet debugPeao;
		PEAO_tppPeao tempPeao;
		int numPeao;

		/* Pegar peão da lista */
		debugLista = LIS_ObterValor(peoesDisponiveis, (void**)&tempPeao);
		/* Se não retornou OK, erro */
		if (debugLista)	return PART_CondRetErroLista;

		/* Pegar número do peão */
		debugPeao = PEAO_ObterNumeroPeao(tempPeao, &numPeao);
		/* Se não retornou OK, erro */
		if (debugPeao)	return PART_CondRetErroPeao;

		printf("%d", numPeao);

		/* Se chegou em último elemento da lista, para de percorrer */
		if (i >= tamLst - 1)
			break;

		if (i == tamLst - 2)
			printf(" ou ");
		else
			printf(", ");

		/* Avançar em lista */
		debugLista = LIS_AvancarElementoCorrente(peoesDisponiveis, 1);
		/* Se não retornou OK, erro */
		if (debugLista)	return PART_CondRetErroLista;

		i++;
	}	/* while */

	printf(".");

	/* Tentar obter um número válido do teclado (se for inválido, repete, pedindo o usuário para digitar outro) */
	while(1) {
		int flag = 0;

		/* scanf("%d", &numEscolhido); */
		numEscolhido = getch() - '0';

		i = 0;
			/* Posição atual na lista */

		while(1) {
			PEAO_tpCondRet debugPeao;
			PEAO_tppPeao tempPeao;
			int numPeao;

			/* Pegar peão da lista */
			debugLista = LIS_ObterValor(peoesDisponiveis, (void**)&tempPeao);
			/* Se não retornou OK, erro */
			if (debugLista)	return PART_CondRetErroLista;

			debugPeao = PEAO_ObterNumeroPeao(tempPeao, &numPeao);

			/* Se o número escolhido está na lista, marca uma flag, marca peão retornado e segue em frente */
			if (numEscolhido == numPeao) {
				*peaoEscolhidoRet = tempPeao;
				flag = 1;
				break;
			}

			/* Se chegou em último elemento da lista, para de percorrer */
			if (i >= tamLst - 1)
				break;

			/* Avançar em lista */
			debugLista = LIS_AvancarElementoCorrente(peoesDisponiveis, 1);
			/* Se não retornou OK, erro */
			if (debugLista)	return PART_CondRetErroLista;

			i++;
		}	/* while */

		/* Se a flag está marcada, o peão escolhido foi encontrado na lista e segue-se em frente */
		if (flag)
			break;
		/* Se não, o número escolhido foi inválido e pede-se outro número */

		printf("\nCaracter digitado inválido. Escolha um peão para movimentar.\n");

	}	/* while */

	return PART_CondRetOK;
}	/* Fim Função PART_Escolher */


/*******************************************************************************************************************************
*	$FC Função: PART_ImprimirCor
*
*	$ED Descrição da função:
*		Recebe uma cor e imprime seu nome na interface do console. Dá erro caso cor recebida seja SEM_COR.
*
*	$EP Parâmetros:
*		$P cor	-	cor a ser impressa
*
*	$FV Valor retornado:
*		PART_CondRetOK
*		PART_CondRetInconsistencia
*******************************************************************************************************************************/
static PART_tpCondRet PART_ImprimirCor(DEF_tpCor cor) {

	switch(cor) {
		case AZUL:
			printf("azul");
			break;
		case VERMELHO:
			printf("vermelho");
			break;
		case VERDE:
			printf("verde");
			break;
		case AMARELO:
			printf("amarelo");
			break;
		default:
			return PART_CondRetInconsistencia;
	}	/* switch */
	return PART_CondRetOK;
}	/* Fim Função PART_ImprimirCor */