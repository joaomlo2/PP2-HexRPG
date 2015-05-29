#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>

typedef struct hexagono
{                                       /*																			EX:Se x=5 e y=5:		 ___		*/
	struct hexagono*NO;					/*	     2(N)		    	-Se for por 1, as coordenadas serão (x-1,y-1)	(5-1,5-1)=(4,4)		 ___/5,3\___	*/
	struct hexagono*N;					/* 		 ___		    	-"	"	"	2, "  "			  "		(x,y-2)		(5,5-2)=(5,3)		/4,4\___/6,4\	*/
	struct hexagono*NE;					/* 1(NO)/x,y\3(NE)	    	-"	"	"	3, "  "           "		(x+1,y-1)	(5+1,5-1)=(6,4)		\___/5,5\___/	*/
	struct hexagono*SO;					/* 6(SO)\_0_/4(SE)			-"	"	"	4, "  "           "		(x+1,y+1)	(5+1,5+1)=(6,6)		/4,6\___/6,6\	*/
	struct hexagono*S;					/*		 5(S)				-"	"	"	5, "  "           "		(x,y+2)		(5,5+2)=(5,7)		\___/5,7\___/	*/
	struct hexagono*SE;					/*							-"	"	"	6, "  "           "     (x-1,y+1)	(5-1,5+1)=(4,6)			\___/		*/
	int x;
	int y;
	int Facção; //0-Cruzados 1-Mouros
	int Tipo_de_Terreno; //0-Planície 1-Subida/Descida Suave 2-Montanha Íngreme
	int Tipo_de_Bioma; //0-Floresta/Campo Normal 1-Tundra Gelada 2-Deserto
	int Aldeia; //0-Não 1-Sim
	int Acampamento; //(Militar)/ 0-Não 1-Sim
	int Capital;
}*HEX;

typedef struct unidade
{
	int Tipo;
	int Nivel;
	int Comida_Necessaria;
	struct exercito*Prox;
}*Unidade;

typedef struct jogador
{
	int Ouro;
	int Comida;
	int X;
	int Y;
	int Capitais_Conquistadas;
	struct unidade*Exercito;
	struct jogador*Prox;
}*Jogador;

//Funções para as estruturas das unidades

void Print_Exercito(Unidade exercito)
{
	Unidade aux = exercito;
	int i = 1;
	while (aux != NULL)
	{
		printf("\nPosicao %d-\nNivel:%d\nTipo:%d\nSustento Necessario:%d\n\n", i, aux->Nivel, aux->Tipo,aux->Comida_Necessaria);
		i++;
		aux = aux->Prox;
	}
}

void Fome(Unidade exercito)
{
	Unidade aux=exercito;
	while (aux != NULL)
	{
		if (exercito->Nivel > 0)
		{
			exercito->Nivel--;
		}
		aux = aux->Prox;
	}
}

int Comida_Necessaria(Unidade exercito)
{
	Unidade aux = exercito;
	int comida=0;
	while (aux != NULL)
	{
		comida = comida + (aux->Comida_Necessaria);
		aux = aux->Prox;
	}
	return comida;
}

Unidade Iniciar_Exercito(Unidade jogador)
{
	Unidade aux = jogador;
	jogador->Nivel = 0;
	jogador->Tipo = -1;
	jogador->Comida_Necessaria = 0;
	jogador->Prox = (Unidade)malloc(sizeof(struct unidade));
	aux = aux->Prox;
	aux->Nivel = 0;
	aux->Tipo = -1;
	aux->Comida_Necessaria = 0;
	aux->Prox = (Unidade)malloc(sizeof(struct unidade));
	aux = aux->Prox;
	aux->Nivel = 0;
	aux->Tipo = -1;
	aux->Comida_Necessaria = 0;
	aux->Prox = NULL;
	return jogador;
}

int Ja_Tem_Unidades_Desse_Tipo(Unidade exercito,int tipo)
{
	Unidade aux = exercito;
	int tem=0;
	while (aux != NULL)
	{
		if (aux->Tipo == tipo)
		{
			tem = 1;
		}
		aux = aux->Prox;
	}
	return tem;
}

Unidade Adicionar_ou_Melhorar_Unidade(Unidade exercito,int tipo)
{
	if (exercito->Tipo == -1)
	{
		exercito->Tipo = tipo;
		exercito->Nivel = 1;
		exercito->Comida_Necessaria++;
	}
	else
	{
		if (Ja_Tem_Unidades_Desse_Tipo(exercito,tipo) == 1)
		{
			Unidade aux = exercito;
			while (aux->Tipo != tipo && aux != NULL)
			{
				aux = aux->Prox;
			}
			aux->Nivel+=10;
			aux->Comida_Necessaria++;
			return exercito;
		}
		else
		{
			Unidade aux = exercito;
			while (aux->Nivel != -1 && aux->Tipo != -1 && aux != NULL)
			{
				aux = aux->Prox;
			}
			aux->Nivel = 10;
			aux->Tipo = tipo;
			exercito->Comida_Necessaria=1;
		}
	}
	return exercito;
}

Unidade Reorganizar_Exercito(Unidade exercito)
{
	int acabou = 0;
	Unidade aux = NULL,aux2=NULL;
	while (acabou == 0)
	{
		int decisao,decisao2,tipo_a,tipo_b,nivel_a,nivel_b,comida_a,comida_b;
		printf("Qual a posição que pretende mover?(1(Frente)-3(Tras) para escolher, ou outro numero qualquer para não reorganizar)");
		Print_Exercito(exercito);
		scanf("%d",&decisao);
		switch (decisao)
		{
		case 1:
			aux = exercito;
			nivel_a = exercito->Nivel;
			tipo_a = exercito->Tipo;
			comida_a = exercito->Comida_Necessaria;
			printf("Onde pretende inserir a unidade?\n");
			scanf("%d",&decisao2);
			switch (decisao2)
			{
			case 1:
				printf("Unidade Inserida!\n");
				break;
			case 2:
				aux = aux->Prox;
				tipo_b = aux->Nivel;
				nivel_b = aux->Tipo;
				comida_b = aux->Comida_Necessaria;
				aux->Nivel = nivel_a;
				aux->Tipo = tipo_a;
				aux->Comida_Necessaria = comida_a;
				exercito->Nivel = nivel_b;
				exercito->Tipo = tipo_b;
				exercito->Comida_Necessaria = comida_b;
				printf("Unidade Inserida!\n");
				break;
			case 3:
				aux = aux->Prox;
				aux = aux->Prox;
				tipo_b = aux->Nivel;
				nivel_b = aux->Tipo;
				comida_b = aux->Comida_Necessaria;
				aux->Nivel = nivel_a;
				aux->Tipo = tipo_a;
				aux->Comida_Necessaria = comida_a;
				exercito->Nivel = nivel_b;
				exercito->Tipo = tipo_b;
				exercito->Comida_Necessaria = comida_b;
				printf("Unidade Inserida!\n");
				break;
			}
			break;
		case 2:
			aux = exercito->Prox;
			tipo_a = aux->Tipo;
			nivel_a = aux->Nivel;
			comida_a = aux->Comida_Necessaria;
			printf("Onde pretende inserir a unidade?\n");
			scanf("%d",&decisao2);
			switch (decisao2)
			{
			case 1:
				tipo_b = exercito->Tipo;
				nivel_b = exercito->Nivel;
				comida_b = exercito->Comida_Necessaria;
				exercito->Tipo = tipo_a;
				exercito->Nivel = nivel_a;
				exercito->Comida_Necessaria = comida_a;
				aux->Nivel = nivel_b;
				aux->Tipo = tipo_b;
				aux->Comida_Necessaria = comida_b;
				printf("Unidade Inserida!\n");
				break;
			case 2:
				printf("Unidade Inserida!\n");
				break;
			case 3:
				aux2 = aux->Prox;
				tipo_b = aux2->Tipo;
				nivel_b = aux2->Nivel;
				comida_b = aux2->Comida_Necessaria;
				aux2->Tipo = tipo_a;
				aux2->Nivel = nivel_a;
				aux2->Comida_Necessaria = comida_a;
				aux->Nivel = nivel_b;
				aux->Tipo = tipo_b;
				aux->Comida_Necessaria = comida_b;
				printf("Unidade Inserida!\n");
				break;
			}
			break;
		case 3:
			aux = exercito->Prox;
			aux = aux->Prox;
			tipo_a = aux->Tipo;
			nivel_a = aux->Nivel;
			comida_a = aux->Comida_Necessaria;
			printf("Onde pretende inserir a unidade?\n");
			scanf("%d", &decisao2);
			switch (decisao2)
			{
			case 1:
				tipo_b = exercito->Tipo;
				nivel_b = exercito->Nivel;
				comida_b = exercito->Comida_Necessaria;
				exercito->Tipo = tipo_a;
				exercito->Nivel = nivel_a;
				exercito->Comida_Necessaria = comida_a;
				aux->Nivel = nivel_b;
				aux->Tipo = tipo_b;
				aux->Comida_Necessaria = comida_b;
				printf("Unidade Inserida!\n");
				break;
			case 2:
				aux2 = exercito->Prox;
				tipo_b = aux2->Tipo;
				nivel_b = aux2->Nivel;
				comida_b = aux2->Comida_Necessaria;
				aux2->Tipo = tipo_a;
				aux2->Nivel = nivel_a;
				aux2->Comida_Necessaria = comida_a;
				aux->Nivel = nivel_b;
				aux->Tipo = tipo_b;
				aux->Comida_Necessaria = comida_b;
				printf("Unidade Inserida!\n");
				break;
			case 3:
				printf("Unidade Inserida!\n");;
				break;
			}
			break;
		default:
			acabou = 1;
			break;
		}
	}
	return exercito;
}

//Funções para células HEX


void Movimentos_Possiveis(HEX Jogador)
{
	printf("Onde Pode ir:\n");
	if (Jogador->N != NULL)
		printf("    8    \n");
	else
		printf("    #    \n");
	if (Jogador->NO != NULL)
		printf("8       ");
	else
		printf("#       ");
	if (Jogador->NE != NULL)
		printf("9\n");
	else
		printf("#\n");
	if (Jogador->SO != NULL)
		printf("1       ");
	else
		printf("#       ");
	if (Jogador->SE != NULL)
		printf("3\n");
	else
		printf("#\n");
	if (Jogador->S != NULL)
		printf("    2    \n");
	else
		printf("    #    \n");
}

void Actualizar_Interface(HEX Mapa,Jogador j)
{
	system("cls");
	HEX aux = Mapa;
	printf_s("================= MAPA ==================\n");
	if (aux == NULL)
		printf("Essa Celula nao existe!");
	else
	{
		/* N - Norte */
		if (aux->N != NULL)
		{
			printf_s("       _____\n");
			printf_s(" _____/%2d,%2d\\_____\n", aux->N->x, aux->N->y);
		}
		else printf("    NOPE\n");
		/* NO - Noroeste */
		if (aux->NO != NULL)
			printf_s("/%2d,%2d\\", aux->NO->x, aux->NO->y);
		else printf("NOPE	");
		/* NE - Nordeste */
		if (aux->NE != NULL)
			printf("_____/%2d,%2d\\\n", aux->NE->x, aux->NE->y);
		else printf("NOPE\n");

		/* Centro */
		printf("\\_____/%2d,%2d\\_____/\n", aux->x, aux->y);

		/* SO - Sudoeste */
		if (aux->SO != NULL)
			printf("/%2d,%2d\\", aux->SO->x, aux->SO->y);
		else printf("NOPE	");
		/* SE - Sudeste */
		if (aux->SE != NULL)
			printf("_____/%2d,%2d\\\n", aux->SE->x, aux->SE->y);
		else printf("NOPE\n");
		/* S - Sul */
		if (aux->S != NULL)
		{
			printf("\\_____/%2d,%2d\\_____/\n", aux->S->x, aux->S->y);
			printf_s("      \\_____/\n");
		}
		else printf("    NOPE\n\n");
	}
	printf("Voce encontra-se nas coordenadas (%d,%d).\n\n", Mapa->x, Mapa->y);

	printf_s("====== Atributos de <%d, %d> ======\n", Mapa->x, Mapa->y);
	switch (aux->Acampamento)
	{
	case 0:
		printf("Encontrou um acampamento!\n");
		break;
	case 1:
		printf("Nao tem acampamento.\n");
	default:
		break;
	}
	switch (aux->Aldeia)
	{
	case 0:
		printf("Nao tem aldeia.\n");
		break;
	case 1:
		printf("Tem aldeia.\n");
		break;
	default:
		break;
	}
	switch (aux->Facção)
	{
	case 0:
		printf("Aliados\n");
		break;
	case 1:
		printf("Inimigos\n");
		break;
	default:
		printf("VAI PARA DEFAULT!\n");
		break;
	}
	switch (aux->Tipo_de_Terreno)
	{
	case 0:
		printf("Planície\n");
		break;
	case 1:
		printf("Floresta\n");
		break;
	case 2:
		printf("Montanha\n");
	}
	switch (aux->Tipo_de_Bioma)
	{
	case 0:
		printf("Normal\n");
		break;
	case 1:
		printf("Tundra\n");
		break;
	case 2:
		printf("Deserto\n");
		break;
	default:
		break;
	}
	printf_s("=========================================\nOURO:%d	COMIDA:%d\n",j->Ouro,j->Comida);
	printf_s("=========================================\n\n");

	Movimentos_Possiveis(Mapa);
}

void Actualizar_Ligaçoes(HEX Centro)
{
	if (Centro == NULL)
	{
		printf("Esta celula nao existe");
	}
	else
	{
		if (Centro->NO == NULL && Centro->N == NULL && Centro->NE == NULL && Centro->S == NULL && Centro->SE == NULL&& Centro->SO == NULL)
			printf("Nao Existem outras celulas para ligar!");
		if (Centro->NO != NULL)
		{
			if (Centro->N != NULL)
			{
				Centro->NO->NE = Centro->N;
				Centro->N->SO = Centro->NO;
			}
			if (Centro->SO != NULL)
			{
				Centro->NO->S = Centro->SO;
				Centro->SO->N = Centro->NO;
			}
		}
		if (Centro->N != NULL)
		{
			if (Centro->NO != NULL)
			{
				Centro->NO->NE = Centro->N;
				Centro->N->SO = Centro->NO;
			}
			if (Centro->NE != NULL)
			{
				Centro->NE->NO = Centro->N;
				Centro->N->SE = Centro->NE;
			}
		}
		if (Centro->NE != NULL)
		{
			if (Centro->N != NULL)
			{
				Centro->NE->NO = Centro->N;
				Centro->N->SE = Centro->NE;
			}
			if (Centro->SE != NULL)
			{
				Centro->SE->N = Centro->NE;
				Centro->NE->S = Centro->SE;
			}
		}
		if (Centro->SE != NULL)
		{
			if (Centro->NE != NULL)
			{
				Centro->SE->N = Centro->NE;
				Centro->NE->S = Centro->SE;
			}
			if (Centro->S != NULL)
			{
				Centro->S->NE = Centro->SE;
				Centro->SE->SO = Centro->S;
			}
		}
		if (Centro->S != NULL)
		{
			if (Centro->SE != NULL)
			{
				Centro->S->NE = Centro->SE;
				Centro->SE->SO = Centro->S;
			}
			if (Centro->SO != NULL)
			{
				Centro->SO->SE = Centro->S;
				Centro->S->NO = Centro->SO;
			}
		}
		if (Centro->SO != NULL)
		{
			if (Centro->S != NULL)
			{
				Centro->SO->SE = Centro->S;
				Centro->S->NO = Centro->SO;
			}
			if (Centro->NO != NULL)
			{
				Centro->NO->S = Centro->SO;
				Centro->SO->N = Centro->NO;
			}
		}
	}
}

HEX Inicializar(HEX Mapa)
{
	if (Mapa == NULL)
	{
		srand(time(NULL));
		Mapa = (HEX)malloc(sizeof(struct hexagono));
		Mapa->x = 0;
		Mapa->y = 0;
		Mapa->Acampamento = rand() % 2;
		Mapa->Facção = rand() % 3;
		if (Mapa->Acampamento = 1)
			Mapa->Aldeia = 0;
		else
			Mapa->Aldeia = rand() % 2;
		Mapa->Tipo_de_Bioma = rand() % 4;
		Mapa->Tipo_de_Terreno = rand() % 4;
		Mapa->N = NULL;
		Mapa->NE = NULL;
		Mapa->NO = NULL;
		Mapa->S = NULL;
		Mapa->SE = NULL;
		Mapa->SO = NULL;
	}
}

void Add_Hex(HEX apt,int dir,int ncapitais)
{
	HEX aux = apt;
	if (apt == NULL)
	{
		return NULL;
	}
	if (dir == 1)
	{
		if (aux->NO == NULL)
		{
			aux = aux->NO;
			aux = (HEX)malloc(sizeof(struct hexagono));
			aux->x=(apt->x) - 1;
			aux->y=(apt->y) - 1;
			aux->N = NULL;
			aux->NO = NULL;
			aux->SO = NULL;
			aux->S = NULL;
			aux->NE = NULL;
			aux->Acampamento = rand() % 2;
			aux->Facção = rand() % 2;
			if (aux->Acampamento = 1)
			{
				aux->Aldeia = 0;
				aux->Capital = 0;
			}
			else
			{
				aux->Aldeia = rand() % 2;
				if (aux->Aldeia == 1)
				{
					aux->Capital = 0;
				}
				else
				{
					if (ncapitais < 6)
					{
						aux->Capital = 1;
						ncapitais++;
					}
				}
			}
			aux->Tipo_de_Bioma = rand() % 3;
			aux->Tipo_de_Terreno = rand() % 3;
			apt->NO = aux;
			aux->SE = apt;
		}
		else
		{
			//Este "else" será apenas para actualizar informação numa célula já existente
		}
	}
	if (dir == 2)
	{
		if (aux->N == NULL)
		{
			aux = aux->N;
			aux = (HEX)malloc(sizeof(struct hexagono));
			aux->x = apt->x;
			aux->y = (apt->y) - 2;
			aux->N = NULL;
			aux->NO = NULL;
			aux->NE = NULL;
			aux->SE = NULL;
			aux->SO = NULL;
			aux->Acampamento = rand() % 2;
			aux->Facção = rand() % 2;
			if (aux->Acampamento = 1)
			{
				aux->Aldeia = 0;
				aux->Capital = 0;
			}
			else
			{
				aux->Aldeia = rand() % 2;
				if (aux->Aldeia == 1)
				{
					aux->Capital = 0;
				}
				else
				{
					if (ncapitais < 6)
					{
						aux->Capital = 1;
						ncapitais++;
					}
				}
			}
			aux->Tipo_de_Bioma = rand() % 3;
			aux->Tipo_de_Terreno = rand() % 3;
			apt->N = aux;
			aux->S = apt;
		}
		else
		{
			//Este "else" será apenas para actualizar informação numa célula já existente
		}
	}
	if (dir == 3)
	{
		if (aux->NE == NULL)
		{
			aux = aux->NE;
			aux = (HEX)malloc(sizeof(struct hexagono));
			aux->x = (apt->x) + 1;
			aux->y = (apt->y) - 1;
			aux->N = NULL;
			aux->NE = NULL;
			aux->SE = NULL;
			aux->S = NULL;
			aux->NO = NULL;
			aux->Acampamento = rand() % 2;
			aux->Facção = rand() % 2;
			if (aux->Acampamento = 1)
			{
				aux->Aldeia = 0;
				aux->Capital = 0;
			}
			else
			{
				aux->Aldeia = rand() % 2;
				if (aux->Aldeia == 1)
				{
					aux->Capital = 0;
				}
				else
				{
					if (ncapitais < 6)
					{
						aux->Capital = 1;
						ncapitais++;
					}
				}
			}
			aux->Tipo_de_Bioma = rand() % 3;
			aux->Tipo_de_Terreno = rand() % 3;
			apt->NE = aux;
			aux->SO = apt;
		}
		else
		{
			//Este "else" será apenas para actualizar informação numa célula já existente
		}
	}
	if (dir == 4)
	{
		if (aux->SE == NULL)
		{
			aux = aux->SE;
			aux = (HEX)malloc(sizeof(struct hexagono));
			aux->x = (apt->x) + 1;
			aux->y = (apt->y) + 1;
			aux->N = NULL;
			aux->S = NULL;
			aux->SE = NULL;
			aux->SO = NULL;
			aux->NE = NULL;
			aux->Acampamento = rand() % 2;
			aux->Facção = rand() % 2;
			if (aux->Acampamento = 1)
			{
				aux->Aldeia = 0;
				aux->Capital = 0;
			}
			else
			{
				aux->Aldeia = rand() % 2;
				if (aux->Aldeia == 1)
				{
					aux->Capital = 0;
				}
				else
				{
					if (ncapitais < 6)
					{
						aux->Capital = 1;
						ncapitais++;
					}
				}
			}
			aux->Tipo_de_Bioma = rand() % 3;
			aux->Tipo_de_Terreno = rand() % 3;
			apt->SE = aux;
			aux->NO = apt;
		}
		else
		{
			//Este "else" será apenas para actualizar informação numa célula já existente
		}
	}
	if (dir == 5)
	{
		if (aux->S == NULL)
		{
			aux = aux->S;
			aux = (HEX)malloc(sizeof(struct hexagono));
			aux->x = (apt->x);
			aux->y = (apt->y) + 2;
			aux->NO = NULL;
			aux->S = NULL;
			aux->SE = NULL;
			aux->SO = NULL;
			aux->NE = NULL;
			aux->Acampamento = rand() % 2;
			aux->Facção = rand() % 2;
			if (aux->Acampamento = 1)
			{
				aux->Aldeia = 0;
				aux->Capital = 0;
			}
			else
			{
				aux->Aldeia = rand() % 2;
				if (aux->Aldeia == 1)
				{
					aux->Capital = 0;
				}
				else
				{
					if (ncapitais < 6)
					{
						aux->Capital = 1;
						ncapitais++;
					}
				}
			}
			aux->Tipo_de_Bioma = rand() % 3;
			aux->Tipo_de_Terreno = rand() % 3;
			apt->S = aux;
			aux->N = apt;
		}
		else
		{
			//Este "else" será apenas para actualizar informação numa célula já existente
		}
	}
	if (dir == 6)
	{
		if (aux->SO == NULL)
		{
			aux = aux->SO;
			aux = (HEX)malloc(sizeof(struct hexagono));
			aux->x = (apt->x) - 1;
			aux->y = (apt->y) + 1;
			aux->N = NULL;
			aux->S = NULL;
			aux->SE = NULL;
			aux->SO = NULL;
			aux->NO = NULL;
			aux->Acampamento = rand() % 2;
			aux->Facção = rand() % 2;
			if (aux->Acampamento = 1)
			{
				aux->Aldeia = 0;
				aux->Capital = 0;
			}
			else
			{
				aux->Aldeia = rand() % 2;
				if (aux->Aldeia == 1)
				{
					aux->Capital = 0;
				}
				else
				{
					if (ncapitais < 6)
					{
						aux->Capital = 1;
						ncapitais++;
					}
				}
			}
			aux->Tipo_de_Bioma = rand() % 3;
			aux->Tipo_de_Terreno = rand() % 3;
			apt->SO = aux;
			aux->NE = apt;
		}
		else
		{
			//Este "else" será apenas para actualizar informação numa célula já existente
		}
	}
	Actualizar_Ligaçoes(apt);
}

void Del_Hex(HEX apt)
{
	if (apt == NULL)
	{
		printf("Esse HEX ja foi removido!");
	}
	else
	{
		if (apt->NO != NULL)
		{
			if (apt->N != NULL)
				apt->N->S = NULL;
			if (apt->NE != NULL)
				apt->NE->SO = NULL;
			if (apt->S != NULL)
				apt->S->N = NULL;
			if (apt->SE != NULL)
				apt->SE->NO=NULL;
			if (apt->SO != NULL)
				apt->SO->NE = NULL;
		}
		if (apt->N != NULL)
		{
			if (apt->NO != NULL)
				apt->NO->SE = NULL;
			if (apt->NE != NULL)
				apt->NE->SO = NULL;
			if (apt->S != NULL)
				apt->S->N = NULL;
			if (apt->SE != NULL)
				apt->SE->NO = NULL;
			if (apt->SO != NULL)
				apt->SO->NE = NULL;
		}
		if (apt->NE != NULL)
		{
			if (apt->NO != NULL)
				apt->NO->SE = NULL;
			if (apt->N != NULL)
				apt->N->S = NULL;
			if (apt->S != NULL)
				apt->S->N = NULL;
			if (apt->SE != NULL)
				apt->SE->NO = NULL;
			if (apt->SO != NULL)
				apt->SO->NE = NULL;
		}
		if (apt->SO!=NULL)
		{
			if (apt->NE != NULL)
				apt->NE->SO = NULL;
			if (apt->NO != NULL)
				apt->NO->SE = NULL;
			if (apt->N != NULL)
				apt->N->S = NULL;
			if (apt->S != NULL)
				apt->S->N = NULL;
			if (apt->SE != NULL)
				apt->SE->NO = NULL;
		}
		if (apt->S != NULL)
		{
			if (apt->NE != NULL)
				apt->NE->SO = NULL;
			if (apt->NO != NULL)
				apt->NO->SE = NULL;
			if (apt->N != NULL)
				apt->N->S = NULL;
			if (apt->SE != NULL)
				apt->SE->NO = NULL;
			if (apt->SO != NULL)
				apt->SO->NE = NULL;
		}
		if (apt->SE != NULL)
		{
			if (apt->NE != NULL)
				apt->NE->SO = NULL;
			if (apt->NO != NULL)
				apt->NO->SE = NULL;
			if (apt->N != NULL)
				apt->N->S = NULL;
			if (apt->SO != NULL)
				apt->SO->NE = NULL;
			if (apt->S != NULL)
				apt->S->N = NULL;
		}
	}
}

//Funções para a estrutura do jogador
Jogador Iniciar_Jogador(Jogador j)
{
	j->Ouro = 50;
	j->Comida = 50;
	j->Capitais_Conquistadas = 0;
	j->Prox = NULL;
	j->Exercito = (Unidade)malloc(sizeof(struct unidade));
	j->Exercito=Iniciar_Exercito(j->Exercito);
	return j;
}

//Funções para o jogo em si

void Main_Menu()
{
	char a;
	printf("=================================================================================                                   HEX RPG                                    =================================================================================");
	printf("\n \n Bem vindo ao HEX RPG! \n Ande pelo mapa! \n Recrute tropas! \n Combata contra os seus inimigos! \n Conquiste-os!!  \n\n1-Comecar\n2-Sair\n");
	while (!_kbhit())
	{
	}
	switch (_getch())
	{
	case 49:
		system("cls");
		printf("Novo Jogo\n");
		break;
	case 50:
		system("cls");
		printf("Acabar Jogo\n");
		exit(0);
	}
}

HEX Movimento(HEX Jogador)
{
	HEX aux=NULL;
	switch (_getch())
	{
	case 55:
		if (Jogador->NO != NULL)
			aux = Jogador->NO;
		else
			printf("Essa celula nao existe!");
		break;
	case 56:
		if (Jogador->N != NULL)
			aux = Jogador->N;
		else
			printf("Essa celula nao existe!");
		break;
	case 57:
		if (Jogador->NE != NULL)
			aux = Jogador->NE;
		else
			printf("Essa celula nao existe!");
		break;
	case 49:
		if (Jogador->SO != NULL)
			aux = Jogador->SO;
		else
			printf("Essa celula nao existe!");
		break;
	case 50:
		if (Jogador->S != NULL)
			aux = Jogador->S;
		else
			printf("Essa celula nao existe!");
		break;
	case 51:
		if (Jogador->SE != NULL)
			aux = Jogador->SE;
		else
			printf("Essa celula nao existe!");
		break;
	default:
		printf("Essa Jogada nao e valida|");
		break;
	}
	return aux;
}

int Calcular_Vantagem(HEX pos, int tipo_de_unidade,int nivel)
{
	int vantagem = 5;
	if (pos->Tipo_de_Bioma == 0)
	{
		switch (tipo_de_unidade)
		{
		case 1:
			break;
		case 2:
			vantagem += 2;
			break;
		case 3:
			vantagem -= 2;
			break;
		}
	}
	if (pos->Tipo_de_Bioma == 1)
	{
		switch (tipo_de_unidade)
		{
		case 1:
			vantagem +=2;
			break;
		case 2:
			break;
		case 3:
			break;
		}
	}
	if (pos->Tipo_de_Bioma == 2)
	{
		switch (tipo_de_unidade)
		{
		case 1:
			vantagem -= 2;
			break;
		case 2:
			vantagem += 2;
			break;
		case 3:
			vantagem -= 2;
			break;
		}
	}
	if (pos->Tipo_de_Terreno == 0)
	{
		switch (tipo_de_unidade)
		{
		case 1:
			vantagem += 2;
			break;
		case 2:
			vantagem += 2;
			break;
		case 3:
			vantagem += 2;
			break;
		}
	}
	if (pos->Tipo_de_Terreno == 1)
	{
		switch (tipo_de_unidade)
		{
		case 1:
			break;
		case 2:
			vantagem -= 2;
			break;
		case 3:
			break;
		}
	}
	if (pos->Tipo_de_Terreno == 2)
	{
		switch (tipo_de_unidade)
		{
		case 1:
			vantagem -= 2;
			break;
		case 2:
			break;
		case 3:
			vantagem -= 2;
			break;
		}
	}
	vantagem = vantagem + nivel;
	return vantagem;
}

void Conversao(HEX pos,Jogador j)
{
	pos->Facção = 0;
	j->Ouro += rand()%501;
	j->Comida += rand() % 101;
}

int Perdeu_Jogo(Jogador j)
{
	int resultado = 0;
	Unidade aux = j->Exercito;
	while (aux != NULL)
	{
		if (aux->Nivel > 0)
		{
			resultado = 1;
		}
		aux = aux->Prox;
	}
	if (resultado = 1)
		return 0;
	else
		return 1;
}

void Batalha(HEX pos, Jogador j)
{
	int tipo_inimigo,nivel_inimigo,vantagem_jog,vantagem_inim,dec;
	if (pos->Tipo_de_Bioma == 0)
	{
		printf("Está um dia agradavel na ");
	}
	if (pos->Tipo_de_Bioma == 1)
	{
		printf("A neve e o gelo arrefecem a ");
	}
	if (pos->Tipo_de_Bioma == 2)
	{
		printf("O sol e a areia quente queimam a ");
	}
	if (pos->Tipo_de_Terreno == 0)
	{
		printf("planicie.\n");
	}
	if (pos->Tipo_de_Terreno == 1)
	{
		printf("descida.\n");
	}
	if (pos->Tipo_de_Terreno == 2)
	{
		printf("montanha\n");
	}
	tipo_inimigo = rand() % 3;
	nivel_inimigo = rand() % 11;
	if (nivel_inimigo <= 5)
	{
		nivel_inimigo = j->Exercito->Nivel - nivel_inimigo;
	}
	else
	{
		nivel_inimigo = j->Exercito->Nivel + (nivel_inimigo-5);
	}
	vantagem_jog = Calcular_Vantagem(pos, j->Exercito->Tipo, j->Exercito->Nivel);
	vantagem_inim = Calcular_Vantagem(pos,tipo_inimigo,nivel_inimigo);
	switch (tipo_inimigo)
	{
	case 0:
		printf("Do lado dos inimigos estao %d soldados de Infantaria\n",nivel_inimigo);
		break;
	case 1:
		printf("Do lado dos inimigos estao %d Arqueiros\n",nivel_inimigo);
		break;
	case 2:
		printf("Do lado dos inimigos estao %d Cavaleiros\n",nivel_inimigo);
		break;
	}
	int ganhou = 0;
	while (Perdeu_Jogo != 1 && ganhou==0)
	{
		Print_Exercito(j->Exercito);
		printf("Deseja mudar a formacao?\n1-Sim\n2-Nao\n");
		scanf("%d", &dec);
		if (dec == 1)
		{
			j->Exercito = Reorganizar_Exercito(j->Exercito);
		}
		system("cls");
		if (vantagem_jog < vantagem_inim)
		{
			int d;
			printf("O jogador esta em desvantagem.\nDeseja fazer retirada?\n1-Sim\n2-Atacar na mesma\n");
			scanf("%d", &d);
			if (d != 1)
			{
				printf("O jogador manda atacar a unidade mais a frente.\n");
				int res = rand() % 3;
				if (res == 1)
				{
					printf("O jogador ganhou!\n");
					Conversao(pos, j);
					if (pos->Capital=1)
						j->Capitais_Conquistadas++;
					ganhou = 1;
				}
				else
				{
					printf("O jogador perdeu a unidade!\n");
				}
			}
			else
			{
				printf("O jogador fugiu!\n");
				ganhou = 1;
			}
		}
		else
		{
			if (vantagem_jog > vantagem_inim)
			{
				int d;
				printf("O jogador tem a vantagem.\nAtacar?\n1-Sim\n2-Retirada\n");
				scanf("%d", &d);
				if (d == 1)
				{
					printf("O jogador manda atacar a unidade mais a frente.\n");
					int res = rand() % 6;
					if (res != 1)
					{
						printf("O jogador ganhou!\n");
						Conversao(pos, j);
						if (pos->Capital = 1)
							j->Capitais_Conquistadas++;
						ganhou = 1;
					}
					else
					{
						printf("O jogador perdeu a unidade!\n");
					}
				}
				else
				{
					printf("O jogador fugiu!\n");
					ganhou = 1;
				}
			}
			else
			{
				int d;
				printf("Ambos os exercitos estao equilibrados.\n1-Atacar\n2-Retirada\n");
				scanf("%d", &d);
				if (d == 1)
				{
					printf("O jogador manda atacar a unidade mais a frente.\n");
					int res = rand() % 2;
					if (res != 1)
					{
						printf("O jogador ganhou!\n");
						Conversao(pos, j);
						if (pos->Capital = 1)
							j->Capitais_Conquistadas++;
						ganhou = 1;
					}
					else
					{
						printf("O jogador perdeu a unidade!\n");
					}
				}
				else
				{
					printf("O jogador fugiu!\n");
					ganhou = 1;
				}
			}
		}
	}
	system("pause");
}

void Evento(HEX Pos, Jogador jogador)
{
	system("cls");
	if (Pos->Acampamento == 1)
	{
		if (Pos->Facção == 0)
		{
			int tipo_de_recrutas=0,decisao3;
			printf("Encontrou um acampamento aliado!\n");
			tipo_de_recrutas = rand() % 3;
			tipo_de_recrutas = 0;
			switch (tipo_de_recrutas)
			{
			case 0:
				printf("Uma dezena de soldados de Infantaria deseja-se alistar.\nDeseja Contratar(Comida necessaria por movimento: %d, tem:%d)?\n1-Sim\n2-Nao\n", (Comida_Necessaria(jogador->Exercito)) + 1, jogador->Comida);
				scanf("%d", &decisao3);
				if (decisao3 == 1)
				{
					system("cls");
					printf("O jogador aceita os soldados de Infantaria para o seu exercito.\nToda a ajuda e necessaria.\n");
					jogador->Exercito = Adicionar_ou_Melhorar_Unidade(jogador->Exercito, tipo_de_recrutas);
				}
				printf("O jogador abandona o acampamento.\n");
				break;
			case 1:
				printf("Uma dezena de Arqueiros deseja-se alistar.\nDeseja Contratar(Comida necessaria por movimento: %d, tem:%d)?\n1-Sim\n2-Nao\n", Comida_Necessaria(jogador->Exercito) + 1, jogador->Comida);
				scanf("%d", &decisao3);
				if (decisao3 == 1)
				{
					system("cls");
					printf("O jogador aceita os Arqueiros para o seu exercito.\nToda a ajuda e necessaria.\n");
					jogador->Exercito = Adicionar_ou_Melhorar_Unidade(jogador->Exercito, tipo_de_recrutas);
				}
				printf("O jogador abandona o acampamento.\n");
				break;
			case 2:
				printf("Uma dezena de Cavaleiros deseja-se alistar.\nDeseja Contratar(Comida necessaria por movimento: %d, tem:%d)?\n1-Sim\n2-Nao\n", Comida_Necessaria(jogador->Exercito) + 1, jogador->Comida);
				scanf("%d", &decisao3);
				if (decisao3 == 1)
				{
					system("cls");
					printf("O jogador aceita os Cavaleiros para o seu exercito.\nToda a ajuda e necessaria.\n");
					jogador->Exercito = Adicionar_ou_Melhorar_Unidade(jogador->Exercito, tipo_de_recrutas);
				}
				printf("O jogador abandona o acampamento.\n");
				break;
			}
		}
		else
		{
			printf("O jogador encontrou um acampamento inimigo!\n");
			Batalha(Pos, jogador);
		}
	}
	else
	{
		if (Pos->Aldeia == 1)
		{
			if (Pos->Facção == 0)
			{
				int decisao3=0;
				printf("O jogador encontrou uma aldeia aliada!\nOs precos serao baixos no mercado!\n");
				system("pause");
				while (decisao3 != 3)
				{
					system("cls");
					printf("O que deseja fazer?\n1-Comprar Comida\n2-Recrutar aldeões\n3-Ir Embora\n");
					scanf("%d", &decisao3);
					if (decisao3 == 1)
					{
						int quant;
						system("cls");
						printf("1 de Comida=1 de Ouro\nQuanta Comida quer?\n");
						scanf("%d", &quant);
						if (quant > jogador->Ouro||quant<0)
						{
							printf("Essa quantidade nao e valida");
						}
						else
						{
							jogador->Ouro = (jogador->Ouro) - quant;
							jogador->Comida = (jogador->Comida) + quant;
							printf("Foram compradas %d quantidades de Comida por %d de Ouro.\n", quant, quant);
						}
					}
				}
			}
			else
			{
				int decisao3=0;
				printf("O jogador encontrou uma aldeia inimiga!\nOs habitantes nao estao muito contentes.\n");
				system("pause");
				while (decisao3!=3)
				{
					system("cls");
					printf("O que deseja fazer?\n1-Comprar Comida\n2-Saquear\n3-Abandonar");
					scanf("%d",&decisao3);
					if (decisao3 == 1)
					{
						int quant = 0;
						system("cls");
						printf("1 de Comida=2 de Ouro\nQuanta Comida deseja comprar?\n");
							printf("Insira a quantidade de comida a comprar.\n");
							scanf("%d", &quant);
							if ((quant * 2) >= jogador->Ouro||quant<0)
								printf("Essa quantidade nao e valida!\n");
							else
							{
								jogador->Comida = (jogador->Comida) + quant;
								jogador->Ouro = (jogador->Ouro) - (quant * 2);
								printf("Comprou %d de Comida por %d de Ouro!\n",quant,quant*2);
							}
					}
					else
					{
						printf("O jogador decide saquear a aldeia!\n Os aldeoes tentam avisar tropas inimigas...\n");
						switch (rand()%2)
						{
						case 0:
							printf("...mas nao conseguem ir a tempo.\n");
							break;
						case 1:
							printf("...e conseguem!\n As tropas chegam para a batalha!\n");
							Batalha(Pos, jogador);
							break;
						}
					}
				}
			}
		}
		else
		{
			if (Pos->Facção == 0)
			{
				int bonus;
				printf("O jogador chegou a uma cidade capital.\n");
				bonus = rand() % 201;
				printf("Foram-lhe doados %d de ouro",bonus);
				jogador->Ouro = jogador->Ouro + bonus;
				bonus = rand() % 201;
				printf("e %d de comida.", bonus);
				jogador->Comida = jogador->Comida + bonus;
				system("pause");
			}
			else
			{
				printf("O jogador chegou a uma capital inimiga!\n");
				Batalha(Pos, jogador);
			}
		}
	}
}

void main()
{
	int ncap = 0;
	Jogador j = NULL;
	j = (Jogador)malloc(sizeof(struct jogador));
	j = Iniciar_Jogador(j);
	srand((unsigned)time(NULL));
	int Perdeu = 0;
	HEX Mapa = NULL,jogador;
	Mapa = Inicializar(Mapa);
	jogador = Mapa;
	Add_Hex(Mapa, 1,ncap);
	Add_Hex(Mapa, 2,ncap);
	Add_Hex(Mapa, 3,ncap);
	Add_Hex(Mapa, 4,ncap);
	Add_Hex(Mapa, 5,ncap);
	Add_Hex(Mapa, 6,ncap);
	Add_Hex(Mapa->N, 1,ncap);
	Add_Hex(Mapa->N, 2,ncap);
	Add_Hex(Mapa->N, 3,ncap);
	Add_Hex(Mapa->NO, 1,ncap);
	Add_Hex(Mapa->NO, 6,ncap);
	Add_Hex(Mapa->SO, 6,ncap);
	Add_Hex(Mapa->S, 6,ncap);
	Add_Hex(Mapa->S, 5,ncap);
	Add_Hex(Mapa->S, 4,ncap);
	Add_Hex(Mapa->SE, 4,ncap);
	Add_Hex(Mapa->NE, 3,ncap);
	Add_Hex(Mapa->NE, 4,ncap);
	jogador->Acampamento = 1;
	jogador->Aldeia = 0;
	jogador->Capital = 0;
	jogador->Facção = 0;
	Main_Menu();
	Actualizar_Interface(jogador,j);
	Actualizar_Ligaçoes(jogador);
	system("pause");
	while (Perdeu==0 && j->Capitais_Conquistadas<5)
	{
		Evento(jogador, j);
		Actualizar_Interface(jogador,j);
		jogador=Movimento(jogador);
		if (Comida_Necessaria(j->Exercito) < j->Comida)
			Fome(j->Exercito);
		else
			j->Comida -= Comida_Necessaria(j->Exercito);
		j->X = jogador->x;
		j->Y = jogador->y;
		Actualizar_Interface(jogador,j);
		Actualizar_Ligaçoes(jogador);
		Perdeu=Perdeu_Jogo(j);
	}
	system("cls");
	if (Perdeu == 0)
	{
		printf("Perdeu o Jogo!");
	}
	else
	{
		printf("Ganhou o jogo!");
	}
	system("pause");
	free(j);
}