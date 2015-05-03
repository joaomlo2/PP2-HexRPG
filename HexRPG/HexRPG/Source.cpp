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
	int Acampamento; //(Inimigo) 0-Não 1-Sim
}*HEX;

int Encontrou_Celula(HEX apt, int X, int Y)
{
	if (apt->x == X&&apt->y == Y)
		return 1;
	else
		return 0;
}

HEX Encontrar_Celula(HEX Mapa, int X, int Y)
{
	if (Mapa == NULL)
	{
		return NULL;
	}
	else
	{
		if (Encontrou_Celula(Mapa, X, Y) == 1)
		{
			return Mapa;
		}
		else
		{
			if (Mapa->N != NULL)
			{
				Encontrar_Celula(Mapa->N, X, Y);
			}
			if (Mapa->NE != NULL)
			{
				Encontrar_Celula(Mapa->NE, X, Y);
			}
			if (Mapa->NO != NULL)
			{
				Encontrar_Celula(Mapa->NO, X, Y);
			}
			if (Mapa->S != NULL)
			{
				Encontrar_Celula(Mapa->S, X, Y);
			}
			if (Mapa->SE != NULL)
			{
				Encontrar_Celula(Mapa->SE, X, Y);
			}
			if (Mapa->SO != NULL)
			{
				Encontrar_Celula(Mapa->SO, X, Y);
			}
		}
	}
}

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

void Actualizar_Interface(HEX Mapa)
{
	system("cls");
	HEX aux = Mapa;
	if (aux == NULL)
		printf("Essa Celula nao existe!");
	else
	{
		if (aux->N != NULL)
			printf("    %d,%d\n", aux->N->x, aux->N->y);
		else
			printf("    NOPE\n");
		if (aux->NO != NULL)
			printf("%d,%d	", aux->NO->x, aux->NO->y);
		else
			printf("NOPE	");
		if (aux->NE != NULL)
			printf("%d,%d\n", aux->NE->x, aux->NE->y);
		else
			printf("NOPE\n");
		printf("    %d,%d    	\n", aux->x, aux->y);
		if (aux->SO != NULL)
			printf("%d,%d     ",aux->SO->x, aux->SO->y);
		else
			printf("NOPE	");
		if (aux->SE != NULL)
			printf("%d,%d\n", aux->SE->x, aux->SE->y);
		else
			printf("NOPE\n");
		if (aux->S != NULL)
			printf("    %d,%d\n", aux->S->x, aux->S->y);
		else
			printf("    NOPE\n\n");
	}
	printf("Este Hexagono encontra-se nas coordenadas (%d,%d).\n",Mapa->x,Mapa->y);
	Movimentos_Possiveis(Mapa);
	printf("%d\n", aux->Acampamento);
	printf("%d\n", aux->Aldeia);
	printf("%d\n", aux->Facção);
	printf("%d\n", aux->Tipo_de_Bioma);
	printf("%d\n", aux->Tipo_de_Terreno);
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
		Mapa = (HEX)malloc(sizeof(struct hexagono));
		Mapa->x = 0;
		Mapa->y = 0;
		Mapa->Acampamento = rand() % 10;
		Mapa->Facção = 0;
		Mapa->Aldeia = 1;
		Mapa->Tipo_de_Bioma = 0;
		Mapa->Tipo_de_Terreno = 0;
		Mapa->N = NULL;
		Mapa->NE = NULL;
		Mapa->NO = NULL;
		Mapa->S = NULL;
		Mapa->SE = NULL;
		Mapa->SO = NULL;
	}
}

void Add_Hex(HEX apt,int dir)
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
			aux->Acampamento = 0;
			aux->Aldeia = 0;
			aux->Facção = 0;
			aux->Tipo_de_Bioma = 0;
			aux->Tipo_de_Terreno = 0;
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
			aux->Acampamento = 0;
			aux->Aldeia = 0;
			aux->Facção = 0;
			aux->Tipo_de_Bioma = 0;
			aux->Tipo_de_Terreno = 0;
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
			aux->Acampamento = 0;
			aux->Aldeia = 0;
			aux->Facção = 0;
			aux->Tipo_de_Bioma = 0;
			aux->Tipo_de_Terreno = 0;
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
			aux->Acampamento = 0;
			aux->Aldeia = 0;
			aux->Facção = 0;
			aux->Tipo_de_Bioma = 0;
			aux->Tipo_de_Terreno = 0;
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
			aux->Acampamento = 0;
			aux->Aldeia = 0;
			aux->Facção = 0;
			aux->Tipo_de_Bioma = 0;
			aux->Tipo_de_Terreno = 0;
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
			aux->Acampamento = 0;
			aux->Aldeia = 0;
			aux->Facção = 0;
			aux->Tipo_de_Bioma = 0;
			aux->Tipo_de_Terreno = 0;
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

void Main_Menu()
{
	char a;
	printf("=================================================================================                                   HEX RPG                                    =================================================================================\n1-Comecar\n2-Continuar\n3-Sair\n");
	while (!_kbhit())
	{}
	switch (_getch())
	{
	case 49:
		system("cls");
		printf("Novo Jogo\n");
		break;
	case 50:
		system("cls");
		printf("Continuar Jogo\n");
		break;
	case 51:
		system("cls");
		printf("Acabar Jogo\n");
	}
}

HEX Jogada(HEX Jogador)
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

void main()
{
	srand(time(NULL));
	int Perdeu = 0;
	HEX Mapa = NULL,Jogador;
	Mapa = Inicializar(Mapa);
	Jogador = Mapa;
	Add_Hex(Mapa, 1);
	Add_Hex(Mapa, 2);
	Add_Hex(Mapa, 3);
	Add_Hex(Mapa, 4);
	Add_Hex(Mapa, 5);
	Add_Hex(Mapa, 6);
	Add_Hex(Mapa->N, 1);
	Add_Hex(Mapa->N, 2);
	Add_Hex(Mapa->N, 3);
	Add_Hex(Mapa->NO, 1);
	Add_Hex(Mapa->NO, 6);
	Add_Hex(Mapa->SO, 6);
	Add_Hex(Mapa->S, 6);
	Add_Hex(Mapa->S, 5);
	Add_Hex(Mapa->S, 4);
	Add_Hex(Mapa->SE, 4);
	Add_Hex(Mapa->NE, 3);
	Add_Hex(Mapa->NE, 4);
	Actualizar_Interface(Jogador);
	while (Perdeu==0)
	{
		Jogador=Jogada(Jogador);
		Actualizar_Ligaçoes(Jogador);
		Actualizar_Interface(Jogador);
	}
	system("pause");
	free(Mapa);
}