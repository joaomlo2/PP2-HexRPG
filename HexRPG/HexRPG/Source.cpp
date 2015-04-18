#include <stdio.h>
#include <stdlib.h>

typedef struct hexagono
{                                       /*																			EX:Se x=5 e y=5:		 ___		*/
	struct hexagono*NO;					/*	     2(N)		    	-Se for por 1, as coordenadas serão (x-1,y-1)	(5-1,5-1)=(4,4)		 ___/5,3\___	*/
	struct hexagono*N;					/* 		 ___		    	-"	"	"	2, "  "			  "		(x,y-2)		(5,5-2)=(5,3)		/4,4\___/6,4\	*/
	struct hexagono*NE;					/* 1(NO)/x,y\3(NE)	    	-"	"	"	3, "  "           "		(x+1,y-1)	(5+1,5-1)=(6,4)		\___/5,5\___/	*/
	struct hexagono*SO;					/* 6(SO)\___/4(SE)			-"	"	"	4, "  "           "		(x+1,y+1)	(5+1,5+1)=(6,6)		/4,6\___/6,6\	*/
	struct hexagono*S;					/*		 5(S)				-"	"	"	5, "  "           "		(x,y+2)		(5,5+2)=(5,7)		\___/5,7\___/	*/
	struct hexagon*SE;					/*							-"	"	"	6, "  "           "     (x-1,y+1)	(5-1,5+1)=(4,6)			\___/		*/
	int x;
	int y;
}*HEX;

void Mostrar_Celula(HEX Mapa,int px,int py)
{
	HEX aux = Mapa;
	if (aux == NULL)
		printf("Essa Celula nao existe!");
	else
	{
		printf("    %d,%d\n",px,px-2);
		printf("%d,%d	%d,%d\n", px - 1, py - 1, px + 1, py - 1);
		printf("    %d,%d    	\n", px, py);
		printf("%d,%d     %d,%d\n", px - 1, py + 1, px + 1, py + 1);
		printf("    %d,%d\n", px, py + 2);
	}
}

HEX Add_Hex(HEX Mapa,int X,int Y)
{
	HEX aux = Mapa;
	if (aux == NULL)
	{
		aux = (HEX)malloc(sizeof(struct hexagono));
		aux->N = NULL;
		aux->NE = NULL;
		aux->NO = NULL;
		aux->S = NULL;
		aux->SE = NULL;
		aux->SO = NULL;
		aux->x = 0;
		aux->y = 0;
	}
	else
	{
		while (aux->x == X&&aux->y == Y&&aux != NULL)
		{
			if (X == (aux->x))
			{
				if (Y == (aux->y))
				{
					return aux;
				}
				else
				{
					if (Y < (aux->y))
					{

					}
				}
			}
		}
	}
}

HEX Add_Coluna(HEX Mapa, int dim, int dir)
{

}

void main()
{
	HEX Mapa = NULL;
	Mapa = Add_Hex(Mapa, 0,0);
	printf("x:%d\ny:%d\n",Mapa->x,Mapa->y);
	Mostrar_Celula(Mapa,Mapa->x,Mapa->y);
	system("pause");
	free(Mapa);
}