#include <stdio.h>
#include <stdlib.h>

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
}*HEX;

HEX Encontrar_Celula(HEX Mapa, int X, int Y)
{
	if (Mapa == NULL)
	{
		return NULL;
	}
	else
	{
		if (Mapa->x == X)
		{
			if (Mapa->y == Y)
			{
				return Mapa;
			}
			else
			{
				if (Mapa->y > Y)
				{

				}
			}
		}
	}
}

void Mostrar_Celula(HEX Mapa)
{
	HEX aux = Mapa;
	if (aux == NULL)
		printf("Essa Celula nao existe!");
	else
	{
		if (aux->N != NULL)
			printf("    %d,%d\n", aux->N->x, aux->N->y);
		else
			printf("    NULL\n");
		if (aux->NO != NULL)
			printf("%d,%d	", aux->NO->x, aux->NO->y);
		else
			printf("NULL	");
		if (aux->NE != NULL)
			printf("%d,%d\n", aux->NE->x, aux->NE->y);
		else
			printf("NULL\n");
		printf("    %d,%d    	\n", aux->x, aux->y);
		if (aux->SO != NULL)
			printf("%d,%d     ",aux->SO->x, aux->SO->y);
		else
			printf("NULL	");
		if (aux->SE != NULL)
			printf("%d,%d\n", aux->SE->x, aux->SE->y);
		else
			printf("NULL\n");
		if (aux->S != NULL)
			printf("    %d,%d\n", aux->S->x, aux->S->y);
		else
			printf("    NULL\n\n");
	}
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


void main()
{
	HEX Mapa = NULL;
	Mapa = Inicializar(Mapa);
	Mostrar_Celula(Mapa);
	Add_Hex(Mapa, 1);
	Add_Hex(Mapa, 2);
	Mostrar_Celula(Mapa);
	Mostrar_Celula(Mapa->N);
	system("pause");
	free(Mapa);
}